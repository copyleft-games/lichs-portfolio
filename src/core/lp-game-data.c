/* lp-game-data.c - Central Game State Container
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include <math.h>

#include "lp-game-data.h"
#include "lp-phylactery.h"
#include "lp-ledger.h"
#include "lp-exposure-manager.h"
#include "lp-portfolio-history.h"
#include "../investment/lp-portfolio.h"
#include "../agent/lp-agent-manager.h"
#include "../simulation/lp-world-simulation.h"

/* Default starting year (Year of the Lich's awakening) */
#define DEFAULT_STARTING_YEAR (847)

/* Starting gold amount */
#define DEFAULT_STARTING_GOLD (1000.0)

struct _LpGameData
{
    GObject parent_instance;

    /* Core state */
    guint64 total_years_played;

    /* Owned subsystems */
    LpPortfolio        *portfolio;
    LpAgentManager     *agent_manager;
    LpPhylactery       *phylactery;
    LpLedger           *ledger;
    LpWorldSimulation  *world_simulation;
    LpPortfolioHistory *portfolio_history;
};

enum
{
    PROP_0,
    PROP_TOTAL_YEARS_PLAYED,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* Forward declarations for LrgSaveable interface */
static void lp_game_data_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpGameData, lp_game_data, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_game_data_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_game_data_get_save_id (LrgSaveable *saveable)
{
    return "game-data";
}

static gboolean
lp_game_data_save (LrgSaveable    *saveable,
                   LrgSaveContext *context,
                   GError        **error)
{
    LpGameData *self = LP_GAME_DATA (saveable);

    /* Save core state */
    lrg_save_context_write_uint (context, "total-years-played",
                                 self->total_years_played);

    /* Save subsystems in sections */
    lrg_save_context_begin_section (context, "portfolio");
    if (!lrg_saveable_save (LRG_SAVEABLE (self->portfolio), context, error))
        return FALSE;
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "agent-manager");
    if (!lrg_saveable_save (LRG_SAVEABLE (self->agent_manager), context, error))
        return FALSE;
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "phylactery");
    if (!lrg_saveable_save (LRG_SAVEABLE (self->phylactery), context, error))
        return FALSE;
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "ledger");
    if (!lrg_saveable_save (LRG_SAVEABLE (self->ledger), context, error))
        return FALSE;
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "world-simulation");
    if (!lrg_saveable_save (LRG_SAVEABLE (self->world_simulation), context, error))
        return FALSE;
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "portfolio-history");
    if (!lrg_saveable_save (LRG_SAVEABLE (self->portfolio_history), context, error))
        return FALSE;
    lrg_save_context_end_section (context);

    /* Save exposure manager state */
    lrg_save_context_write_uint (context, "exposure",
                                 lp_exposure_manager_get_exposure (
                                     lp_exposure_manager_get_default ()));

    return TRUE;
}

static gboolean
lp_game_data_load (LrgSaveable    *saveable,
                   LrgSaveContext *context,
                   GError        **error)
{
    LpGameData *self = LP_GAME_DATA (saveable);
    guint exposure;

    /* Load core state */
    self->total_years_played = lrg_save_context_read_uint (context,
                                                            "total-years-played", 0);

    /* Load subsystems from sections */
    if (lrg_save_context_enter_section (context, "portfolio"))
    {
        lrg_saveable_load (LRG_SAVEABLE (self->portfolio), context, error);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "agent-manager"))
    {
        lrg_saveable_load (LRG_SAVEABLE (self->agent_manager), context, error);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "phylactery"))
    {
        lrg_saveable_load (LRG_SAVEABLE (self->phylactery), context, error);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "ledger"))
    {
        lrg_saveable_load (LRG_SAVEABLE (self->ledger), context, error);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "world-simulation"))
    {
        lrg_saveable_load (LRG_SAVEABLE (self->world_simulation), context, error);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "portfolio-history"))
    {
        lrg_saveable_load (LRG_SAVEABLE (self->portfolio_history), context, error);
        lrg_save_context_leave_section (context);
    }

    /* Load exposure manager state */
    exposure = (guint)lrg_save_context_read_uint (context, "exposure", 0);
    lp_exposure_manager_set_exposure (lp_exposure_manager_get_default (), exposure);

    lp_log_info ("Loaded game data: year %lu, total played %lu",
                 lp_world_simulation_get_current_year (self->world_simulation),
                 self->total_years_played);

    return TRUE;
}

static void
lp_game_data_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_game_data_get_save_id;
    iface->save = lp_game_data_save;
    iface->load = lp_game_data_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_game_data_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    LpGameData *self = LP_GAME_DATA (object);

    switch (prop_id)
    {
    case PROP_TOTAL_YEARS_PLAYED:
        g_value_set_uint64 (value, self->total_years_played);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_game_data_dispose (GObject *object)
{
    LpGameData *self = LP_GAME_DATA (object);

    g_clear_object (&self->portfolio);
    g_clear_object (&self->agent_manager);
    g_clear_object (&self->phylactery);
    g_clear_object (&self->ledger);
    g_clear_object (&self->world_simulation);
    g_clear_object (&self->portfolio_history);

    G_OBJECT_CLASS (lp_game_data_parent_class)->dispose (object);
}

static void
lp_game_data_finalize (GObject *object)
{
    lp_log_debug ("Finalizing game data");

    G_OBJECT_CLASS (lp_game_data_parent_class)->finalize (object);
}

static void
lp_game_data_class_init (LpGameDataClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_game_data_get_property;
    object_class->dispose = lp_game_data_dispose;
    object_class->finalize = lp_game_data_finalize;

    /**
     * LpGameData:total-years-played:
     *
     * Total years played across all runs.
     */
    properties[PROP_TOTAL_YEARS_PLAYED] =
        g_param_spec_uint64 ("total-years-played",
                             "Total Years Played",
                             "Total years played across all runs",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_game_data_init (LpGameData *self)
{
    self->total_years_played = 0;

    /* Create all subsystems */
    self->portfolio = lp_portfolio_new ();
    self->agent_manager = lp_agent_manager_new ();
    self->phylactery = lp_phylactery_new ();
    self->ledger = lp_ledger_new ();
    self->world_simulation = lp_world_simulation_new ();
    self->portfolio_history = lp_portfolio_history_new ();
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_game_data_new:
 *
 * Creates a new game data container.
 *
 * Returns: (transfer full): A new #LpGameData
 */
LpGameData *
lp_game_data_new (void)
{
    return g_object_new (LP_TYPE_GAME_DATA, NULL);
}

/* ==========================================================================
 * Time/Year Management
 * ========================================================================== */

/**
 * lp_game_data_get_current_year:
 * @self: an #LpGameData
 *
 * Gets the current in-game year.
 *
 * Returns: The current year
 */
guint64
lp_game_data_get_current_year (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), 0);

    return lp_world_simulation_get_current_year (self->world_simulation);
}

/**
 * lp_game_data_get_total_years_played:
 * @self: an #LpGameData
 *
 * Gets the total years played.
 *
 * Returns: Total years played
 */
guint64
lp_game_data_get_total_years_played (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), 0);

    return self->total_years_played;
}

/* ==========================================================================
 * Subsystem Access
 * ========================================================================== */

/**
 * lp_game_data_get_portfolio:
 * @self: an #LpGameData
 *
 * Returns: (transfer none): The #LpPortfolio
 */
LpPortfolio *
lp_game_data_get_portfolio (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), NULL);

    return self->portfolio;
}

/**
 * lp_game_data_get_agent_manager:
 * @self: an #LpGameData
 *
 * Returns: (transfer none): The #LpAgentManager
 */
LpAgentManager *
lp_game_data_get_agent_manager (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), NULL);

    return self->agent_manager;
}

/**
 * lp_game_data_get_phylactery:
 * @self: an #LpGameData
 *
 * Returns: (transfer none): The #LpPhylactery
 */
LpPhylactery *
lp_game_data_get_phylactery (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), NULL);

    return self->phylactery;
}

/**
 * lp_game_data_get_ledger:
 * @self: an #LpGameData
 *
 * Returns: (transfer none): The #LpLedger
 */
LpLedger *
lp_game_data_get_ledger (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), NULL);

    return self->ledger;
}

/**
 * lp_game_data_get_world_simulation:
 * @self: an #LpGameData
 *
 * Returns: (transfer none): The #LpWorldSimulation
 */
LpWorldSimulation *
lp_game_data_get_world_simulation (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), NULL);

    return self->world_simulation;
}

/**
 * lp_game_data_get_portfolio_history:
 * @self: an #LpGameData
 *
 * Gets the portfolio history tracker.
 *
 * Returns: (transfer none): The #LpPortfolioHistory
 */
LpPortfolioHistory *
lp_game_data_get_portfolio_history (LpGameData *self)
{
    g_return_val_if_fail (LP_IS_GAME_DATA (self), NULL);

    return self->portfolio_history;
}

/* ==========================================================================
 * Game Actions
 * ========================================================================== */

/**
 * lp_game_data_start_new_game:
 * @self: an #LpGameData
 *
 * Starts a new game from scratch.
 */
void
lp_game_data_start_new_game (LpGameData *self)
{
    g_autoptr(LrgBigNumber) starting_gold = NULL;

    g_return_if_fail (LP_IS_GAME_DATA (self));

    lp_log_info ("Starting new game");

    /* Reset everything */
    self->total_years_played = 0;

    /* Reset subsystems */
    starting_gold = lrg_big_number_new (DEFAULT_STARTING_GOLD);
    lp_portfolio_reset (self->portfolio, lrg_big_number_copy (starting_gold));
    lp_agent_manager_reset (self->agent_manager);
    lp_phylactery_reset_upgrades (self->phylactery);
    lp_ledger_clear_all (self->ledger);
    lp_world_simulation_reset (self->world_simulation, DEFAULT_STARTING_YEAR);
    lp_portfolio_history_clear (self->portfolio_history);

    /* Reset exposure */
    lp_exposure_manager_reset (lp_exposure_manager_get_default ());
}

/**
 * lp_game_data_prestige:
 * @self: an #LpGameData
 *
 * Performs a prestige reset.
 *
 * Returns: Number of phylactery points earned
 */
guint64
lp_game_data_prestige (LpGameData *self)
{
    g_autoptr(LrgBigNumber) total_value = NULL;
    g_autoptr(LrgBigNumber) starting_gold = NULL;
    guint64 points_earned;
    gdouble value_double;

    g_return_val_if_fail (LP_IS_GAME_DATA (self), 0);

    lp_log_info ("Performing prestige reset");

    /* Calculate phylactery points based on portfolio value */
    total_value = lp_portfolio_get_total_value (self->portfolio);
    value_double = lrg_big_number_to_double (total_value);

    /*
     * Phase 1 skeleton: Simple point calculation.
     * Points = log10(portfolio value) - 3
     * So 1000 gold = 0 points, 10000 gold = 1 point, etc.
     */
    if (value_double > 1000.0)
        points_earned = (guint64)(log10 (value_double) - 3.0);
    else
        points_earned = 0;

    /* Add points to phylactery */
    if (points_earned > 0)
        lp_phylactery_add_points (self->phylactery, points_earned);

    /* Reset per-run state (keep ledger and phylactery) */
    starting_gold = lrg_big_number_new (
        DEFAULT_STARTING_GOLD * lp_phylactery_get_starting_gold_bonus (self->phylactery));
    lp_portfolio_reset (self->portfolio, lrg_big_number_copy (starting_gold));
    lp_agent_manager_reset (self->agent_manager);
    lp_world_simulation_reset (self->world_simulation, DEFAULT_STARTING_YEAR);
    lp_portfolio_history_clear (self->portfolio_history);
    lp_exposure_manager_reset (lp_exposure_manager_get_default ());

    lp_log_info ("Prestige complete: earned %lu phylactery points", points_earned);

    return points_earned;
}

/**
 * lp_game_data_slumber:
 * @self: an #LpGameData
 * @years: number of years to slumber
 *
 * Enters slumber for the specified number of years.
 *
 * Returns: (transfer full) (element-type LpEvent): List of events
 */
GList *
lp_game_data_slumber (LpGameData *self,
                      guint       years)
{
    GList *events;

    g_return_val_if_fail (LP_IS_GAME_DATA (self), NULL);

    lp_log_info ("Entering slumber for %u years", years);

    /* Track total years */
    self->total_years_played += years;

    /* Advance world simulation */
    events = lp_world_simulation_advance_years (self->world_simulation, years);

    /* Advance agents */
    lp_agent_manager_advance_years (self->agent_manager, years);

    /* Apply exposure decay */
    lp_exposure_manager_apply_decay (lp_exposure_manager_get_default (), years);

    /* Apply slumber to portfolio - calculate returns and update values */
    lp_portfolio_apply_slumber (self->portfolio, years);

    /* Record portfolio snapshot for history tracking */
    {
        g_autoptr(LrgBigNumber) total_value = NULL;
        LrgBigNumber *gold;
        g_autoptr(LrgBigNumber) investment_value = NULL;
        guint64 current_year;

        total_value = lp_portfolio_get_total_value (self->portfolio);
        gold = lp_portfolio_get_gold (self->portfolio);
        investment_value = lp_portfolio_get_investment_value (self->portfolio);
        current_year = lp_world_simulation_get_current_year (self->world_simulation);

        lp_portfolio_history_record_snapshot (self->portfolio_history,
                                               current_year,
                                               total_value,
                                               gold,
                                               investment_value);
    }

    return events;
}
