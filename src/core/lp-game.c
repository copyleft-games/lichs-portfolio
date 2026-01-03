/* lp-game.c - Main Game (LrgIdleTemplate Subclass)
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_APP
#include "../lp-log.h"

#include "lp-game.h"
#include "lp-game-data.h"
#include "lp-phylactery.h"
#include "lp-prestige.h"
#include "../achievement/lp-achievement-manager.h"
#include "../investment/lp-portfolio.h"
#include "../investment/lp-investment.h"
#include "../states/lp-state-main-menu.h"
#include "../states/lp-state-pause.h"
#include "../states/lp-state-settings.h"
#include "../states/lp-state-welcome-back.h"
#include "../tutorial/lp-tutorial-sequences.h"

#include <raylib.h>
#include <math.h>

/* Window configuration */
#define WINDOW_WIDTH   1280
#define WINDOW_HEIGHT  720
#define WINDOW_TITLE   "Lich's Portfolio"

/* Offline progress configuration */
#define OFFLINE_EFFICIENCY  0.10   /* 10% of normal production */
#define MAX_OFFLINE_HOURS   168.0  /* 1 week cap */

/* Seconds per game year for conversion */
#define SECONDS_PER_YEAR (365.25 * 24.0 * 3600.0)

struct _LpGame
{
    LrgIdleTemplate parent_instance;

    LpGameData           *game_data;
    LpAchievementManager *achievement_manager;

    /* Offline progress tracking */
    LrgBigNumber *offline_gold_earned;
    gdouble       offline_seconds;
    gboolean      show_offline_progress;
};

/*
 * Static instance tracking.
 * Since only one game runs at a time, this provides an easy way for
 * states to access the game without needing singleton machinery.
 */
static LpGame *current_game_instance = NULL;

G_DEFINE_TYPE (LpGame, lp_game, LRG_TYPE_IDLE_TEMPLATE)

/* ==========================================================================
 * Forward Declarations
 * ========================================================================== */

static void lp_game_sync_generators_internal (LpGame *self);

/* ==========================================================================
 * LrgGameTemplate Virtual Method Overrides
 * ========================================================================== */

/*
 * configure:
 *
 * Called before window creation. Configure window properties.
 */
static void
lp_game_real_configure (LrgGameTemplate *template)
{
    lp_log_info ("Configuring Lich's Portfolio...");

    /* Set window properties via parent class */
    lrg_game_template_set_title (template, WINDOW_TITLE);

    /* Configure idle template settings */
    lrg_idle_template_set_offline_efficiency (LRG_IDLE_TEMPLATE (template),
                                               OFFLINE_EFFICIENCY);
    lrg_idle_template_set_max_offline_hours (LRG_IDLE_TEMPLATE (template),
                                              MAX_OFFLINE_HOURS);
    lrg_idle_template_set_show_offline_popup (LRG_IDLE_TEMPLATE (template), FALSE);
}

/*
 * pre_startup:
 *
 * Called before initial state is pushed. Initialize subsystems.
 */
static void
lp_game_real_pre_startup (LrgGameTemplate *template)
{
    LpGame *self = LP_GAME (template);

    lp_log_info ("Pre-startup: Initializing subsystems...");

    /* Disable raylib's ESC-to-close behavior */
    SetExitKey (KEY_NULL);

    /* Create achievement manager singleton */
    self->achievement_manager = lp_achievement_manager_get_default ();

    /* Game data is created when starting/loading a game, not at startup */
    self->game_data = NULL;

    /* Set as current instance for state access */
    current_game_instance = self;

    /* Set game reference on tutorial sequences */
    lp_tutorial_sequences_set_game (lp_tutorial_sequences_get_default (), self);

    lp_log_info ("Pre-startup complete");
}

/*
 * post_startup:
 *
 * Called after initial state is pushed. Process any offline progress.
 */
static void
lp_game_real_post_startup (LrgGameTemplate *template)
{
    LpGame *self = LP_GAME (template);

    lp_log_info ("Post-startup complete");

    /*
     * Offline progress is processed automatically by the template
     * via process_offline_progress() which calls our on_offline_progress_calculated.
     * We just need to check if we should show a welcome back notification.
     */
    if (self->show_offline_progress && self->offline_gold_earned != NULL)
    {
        LrgGameStateManager *manager;
        LpStateWelcomeBack *welcome_back;

        lp_log_info ("Offline progress: %.2f gold earned over %.0f seconds",
                     lrg_big_number_to_double (self->offline_gold_earned),
                     self->offline_seconds);

        /* Create and configure welcome back state */
        welcome_back = lp_state_welcome_back_new ();
        lp_state_welcome_back_set_offline_data (welcome_back,
                                                 self->offline_seconds,
                                                 self->offline_gold_earned);

        /* Push as overlay on top of initial state */
        manager = lrg_game_template_get_state_manager (template);
        lrg_game_state_manager_push (manager, LRG_GAME_STATE (welcome_back));
    }
}

/*
 * shutdown:
 *
 * Called during shutdown. Save and cleanup.
 */
static void
lp_game_real_shutdown (LrgGameTemplate *template)
{
    LpGame *self = LP_GAME (template);

    lp_log_info ("Shutting down...");

    /* Clear current instance */
    if (current_game_instance == self)
    {
        current_game_instance = NULL;
    }

    /* Clear game data */
    g_clear_object (&self->game_data);

    /*
     * Achievement manager is a singleton; don't clear it here.
     */
    self->achievement_manager = NULL;

    /* Clear offline progress tracking */
    g_clear_pointer (&self->offline_gold_earned, lrg_big_number_free);

    lp_log_info ("Shutdown complete");
}

/*
 * create_initial_state:
 *
 * Creates the first game state (main menu).
 */
static LrgGameState *
lp_game_real_create_initial_state (LrgGameTemplate *template)
{
    (void)template;

    return LRG_GAME_STATE (lp_state_main_menu_new ());
}

/*
 * create_pause_state:
 *
 * Creates the pause menu state.
 */
static LrgGameState *
lp_game_real_create_pause_state (LrgGameTemplate *template)
{
    (void)template;

    return LRG_GAME_STATE (lp_state_pause_new ());
}

/*
 * create_settings_state:
 *
 * Creates the settings menu state.
 */
static LrgGameState *
lp_game_real_create_settings_state (LrgGameTemplate *template)
{
    (void)template;

    return LRG_GAME_STATE (lp_state_settings_new ());
}

/* ==========================================================================
 * LrgIdleTemplate Virtual Method Overrides
 * ========================================================================== */

/*
 * create_idle_calculator:
 *
 * Creates the idle calculator with investment generators.
 */
static LrgIdleCalculator *
lp_game_real_create_idle_calculator (LrgIdleTemplate *template)
{
    LrgIdleCalculator *calc;

    calc = lrg_idle_calculator_new ();

    /*
     * Generators will be added when game data is created/loaded
     * via lp_game_sync_generators().
     */

    return calc;
}

/*
 * create_prestige:
 *
 * Creates the custom prestige layer.
 */
static LrgPrestige *
lp_game_real_create_prestige (LrgIdleTemplate *template)
{
    LpPrestige *prestige;

    prestige = lp_prestige_new ();

    /*
     * Phylactery and portfolio will be set when game data is created/loaded.
     */

    return LRG_PRESTIGE (prestige);
}

/*
 * on_offline_progress_calculated:
 *
 * Called when offline progress is calculated. Store for welcome back display.
 */
static void
lp_game_real_on_offline_progress_calculated (LrgIdleTemplate    *template,
                                             const LrgBigNumber *progress,
                                             gdouble             seconds_offline)
{
    LpGame *self = LP_GAME (template);
    LpPortfolio *portfolio;

    /* Only process if we have game data */
    if (self->game_data == NULL)
    {
        return;
    }

    /* Only process if there's actual progress */
    if (progress == NULL || lrg_big_number_to_double (progress) <= 0.0)
    {
        return;
    }

    lp_log_info ("Offline progress calculated: %s gold over %.0f seconds",
                 lrg_big_number_format_short (progress),
                 seconds_offline);

    /* Add gold to portfolio */
    portfolio = lp_game_data_get_portfolio (self->game_data);
    lp_portfolio_add_gold (portfolio, progress);

    /* Store for welcome back display */
    g_clear_pointer (&self->offline_gold_earned, lrg_big_number_free);
    self->offline_gold_earned = lrg_big_number_copy (progress);
    self->offline_seconds = seconds_offline;
    self->show_offline_progress = TRUE;
}

/*
 * format_big_number:
 *
 * Formats numbers with gold notation.
 */
static gchar *
lp_game_real_format_big_number (LrgIdleTemplate    *template,
                                const LrgBigNumber *number)
{
    (void)template;

    /* Use short format with 2 decimal places */
    return lrg_big_number_format_short (number);
}

/*
 * get_offline_efficiency:
 *
 * Returns offline efficiency, potentially modified by phylactery.
 */
static gdouble
lp_game_real_get_offline_efficiency (LrgIdleTemplate *template)
{
    LpGame *self = LP_GAME (template);
    gdouble base_efficiency = OFFLINE_EFFICIENCY;

    if (self->game_data != NULL)
    {
        LpPhylactery *phylactery = lp_game_data_get_phylactery (self->game_data);

        /* Time efficiency bonus from phylactery applies to offline too */
        gdouble bonus = lp_phylactery_get_time_efficiency_bonus (phylactery);
        base_efficiency *= bonus;
    }

    return base_efficiency;
}

/*
 * get_max_offline_hours:
 *
 * Returns max offline hours, potentially modified by phylactery.
 */
static gdouble
lp_game_real_get_max_offline_hours (LrgIdleTemplate *template)
{
    LpGame *self = LP_GAME (template);
    gdouble base_hours = MAX_OFFLINE_HOURS;

    if (self->game_data != NULL)
    {
        LpPhylactery *phylactery = lp_game_data_get_phylactery (self->game_data);

        /* Temporal mastery might extend max offline time */
        guint max_slumber = lp_phylactery_get_max_slumber_years (phylactery);

        /* Scale: 100 years base = 168 hours, 200 years = 336 hours */
        if (max_slumber > 100)
        {
            base_hours = MAX_OFFLINE_HOURS * ((gdouble)max_slumber / 100.0);
        }
    }

    return base_hours;
}

/* ==========================================================================
 * Generator Sync
 * ========================================================================== */

/*
 * lp_game_sync_generators_internal:
 *
 * Synchronizes investments to idle calculator generators.
 */
static void
lp_game_sync_generators_internal (LpGame *self)
{
    LrgIdleCalculator *calc;
    LpPortfolio       *portfolio;
    GPtrArray         *investments;
    guint              i;

    if (self->game_data == NULL)
    {
        return;
    }

    calc = lrg_idle_template_get_idle_calculator (LRG_IDLE_TEMPLATE (self));
    if (calc == NULL)
    {
        return;
    }

    portfolio = lp_game_data_get_portfolio (self->game_data);
    investments = lp_portfolio_get_investments (portfolio);

    for (i = 0; i < investments->len; i++)
    {
        LpInvestment  *inv;
        const gchar   *id;
        gdouble        base_rate;
        gdouble        current_value;
        gdouble        annual_income;
        gdouble        per_second;

        inv = g_ptr_array_index (investments, i);
        id = lp_investment_get_id (inv);

        /*
         * Calculate annual income for this investment.
         * annual_income = current_value * base_return_rate
         */
        base_rate = lp_investment_get_base_return_rate (inv);
        current_value = lrg_big_number_to_double (lp_investment_get_current_value (inv));
        annual_income = current_value * base_rate;

        /* Convert to per-second rate */
        per_second = annual_income / SECONDS_PER_YEAR;

        /* Add or update generator */
        lrg_idle_template_add_generator (LRG_IDLE_TEMPLATE (self), id, per_second);
        lrg_idle_template_set_generator_count (LRG_IDLE_TEMPLATE (self), id, 1);
    }

    lp_log_debug ("Synced %u investments to idle generators", investments->len);
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_game_dispose (GObject *object)
{
    LpGame *self = LP_GAME (object);

    g_clear_object (&self->game_data);
    g_clear_pointer (&self->offline_gold_earned, lrg_big_number_free);

    if (current_game_instance == self)
    {
        current_game_instance = NULL;
    }

    G_OBJECT_CLASS (lp_game_parent_class)->dispose (object);
}

static void
lp_game_class_init (LpGameClass *klass)
{
    GObjectClass         *object_class = G_OBJECT_CLASS (klass);
    LrgGameTemplateClass *template_class = LRG_GAME_TEMPLATE_CLASS (klass);
    LrgIdleTemplateClass *idle_class = LRG_IDLE_TEMPLATE_CLASS (klass);

    object_class->dispose = lp_game_dispose;

    /* Override LrgGameTemplate virtual methods */
    template_class->configure = lp_game_real_configure;
    template_class->pre_startup = lp_game_real_pre_startup;
    template_class->post_startup = lp_game_real_post_startup;
    template_class->shutdown = lp_game_real_shutdown;
    template_class->create_initial_state = lp_game_real_create_initial_state;
    template_class->create_pause_state = lp_game_real_create_pause_state;
    template_class->create_settings_state = lp_game_real_create_settings_state;

    /* Override LrgIdleTemplate virtual methods */
    idle_class->create_idle_calculator = lp_game_real_create_idle_calculator;
    idle_class->create_prestige = lp_game_real_create_prestige;
    idle_class->on_offline_progress_calculated = lp_game_real_on_offline_progress_calculated;
    idle_class->format_big_number = lp_game_real_format_big_number;
    idle_class->get_offline_efficiency = lp_game_real_get_offline_efficiency;
    idle_class->get_max_offline_hours = lp_game_real_get_max_offline_hours;
}

static void
lp_game_init (LpGame *self)
{
    self->game_data = NULL;
    self->achievement_manager = NULL;
    self->offline_gold_earned = NULL;
    self->offline_seconds = 0.0;
    self->show_offline_progress = FALSE;
}

/* ==========================================================================
 * Public API - Construction
 * ========================================================================== */

/**
 * lp_game_new:
 *
 * Creates a new game instance.
 *
 * Returns: (transfer full): A new #LpGame
 */
LpGame *
lp_game_new (void)
{
    return g_object_new (LP_TYPE_GAME,
                         "window-width", WINDOW_WIDTH,
                         "window-height", WINDOW_HEIGHT,
                         NULL);
}

/* ==========================================================================
 * Public API - State Access Helper
 * ========================================================================== */

/**
 * lp_game_get_from_state:
 * @state: an #LrgGameState
 *
 * Gets the game instance from a game state.
 *
 * Returns: (transfer none) (nullable): The #LpGame, or %NULL if not found
 */
LpGame *
lp_game_get_from_state (LrgGameState *state)
{
    (void)state;

    /*
     * Since only one game runs at a time, we use the static instance.
     * This avoids complex plumbing while maintaining a clean API.
     */
    return current_game_instance;
}

/* ==========================================================================
 * Public API - Subsystem Access
 * ========================================================================== */

/**
 * lp_game_get_game_data:
 * @self: an #LpGame
 *
 * Gets the current game data.
 *
 * Returns: (transfer none) (nullable): The #LpGameData, or %NULL
 */
LpGameData *
lp_game_get_game_data (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    return self->game_data;
}

/**
 * lp_game_get_phylactery:
 * @self: an #LpGame
 *
 * Gets the phylactery from the current game data.
 *
 * Returns: (transfer none) (nullable): The #LpPhylactery, or %NULL
 */
LpPhylactery *
lp_game_get_phylactery (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    if (self->game_data == NULL)
    {
        return NULL;
    }

    return lp_game_data_get_phylactery (self->game_data);
}

/**
 * lp_game_get_prestige_layer:
 * @self: an #LpGame
 *
 * Gets the prestige layer from the template.
 *
 * Returns: (transfer none) (nullable): The #LpPrestige, or %NULL
 */
LpPrestige *
lp_game_get_prestige_layer (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    return LP_PRESTIGE (lrg_idle_template_get_prestige (LRG_IDLE_TEMPLATE (self)));
}

/**
 * lp_game_get_achievement_manager:
 * @self: an #LpGame
 *
 * Gets the achievement manager.
 *
 * Returns: (transfer none): The #LpAchievementManager
 */
LpAchievementManager *
lp_game_get_achievement_manager (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    return self->achievement_manager;
}

/* ==========================================================================
 * Public API - Game Management
 * ========================================================================== */

/**
 * lp_game_new_game:
 * @self: an #LpGame
 *
 * Starts a new game, creating fresh game data.
 */
void
lp_game_new_game (LpGame *self)
{
    LpPrestige   *prestige;
    LpPortfolio  *portfolio;
    LpPhylactery *phylactery;

    g_return_if_fail (LP_IS_GAME (self));

    lp_log_info ("Starting new game");

    /* Clear any existing game data */
    g_clear_object (&self->game_data);

    /* Create fresh game data */
    self->game_data = lp_game_data_new ();

    /* Configure prestige layer with new game's phylactery and portfolio */
    prestige = lp_game_get_prestige_layer (self);
    if (prestige != NULL)
    {
        portfolio = lp_game_data_get_portfolio (self->game_data);
        phylactery = lp_game_data_get_phylactery (self->game_data);

        lp_prestige_set_portfolio (prestige, portfolio);
        lp_prestige_set_phylactery (prestige, phylactery);
    }

    /* Sync investments to idle calculator */
    lp_game_sync_generators_internal (self);

    lp_log_info ("New game created, starting year: %lu",
                 (gulong)lp_game_data_get_current_year (self->game_data));
}

/**
 * lp_game_load_game:
 * @self: an #LpGame
 * @slot: the save slot to load from
 * @error: (nullable): return location for error
 *
 * Loads a saved game from the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_game_load_game (LpGame   *self,
                   guint     slot,
                   GError  **error)
{
    g_return_val_if_fail (LP_IS_GAME (self), FALSE);

    lp_log_info ("Loading game from slot %u", slot);

    /*
     * TODO: Implement save/load using LrgSaveManager.
     * For now, this is a skeleton.
     */
    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_SUPPORTED,
                 "Save/load not yet implemented");

    return FALSE;
}

/**
 * lp_game_save_game:
 * @self: an #LpGame
 * @slot: the save slot to save to
 * @error: (nullable): return location for error
 *
 * Saves the current game to the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_game_save_game (LpGame   *self,
                   guint     slot,
                   GError  **error)
{
    g_return_val_if_fail (LP_IS_GAME (self), FALSE);

    if (self->game_data == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "No game data to save");
        return FALSE;
    }

    lp_log_info ("Saving game to slot %u", slot);

    /*
     * TODO: Implement save/load using LrgSaveManager.
     * For now, this is a skeleton.
     */
    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_SUPPORTED,
                 "Save/load not yet implemented");

    return FALSE;
}

/* ==========================================================================
 * Public API - Idle System Integration
 * ========================================================================== */

/**
 * lp_game_sync_generators:
 * @self: an #LpGame
 *
 * Synchronizes investments to idle calculator generators.
 */
void
lp_game_sync_generators (LpGame *self)
{
    g_return_if_fail (LP_IS_GAME (self));

    lp_game_sync_generators_internal (self);
}

/**
 * lp_game_has_offline_progress:
 * @self: an #LpGame
 *
 * Checks if there's pending offline progress to show.
 *
 * Returns: %TRUE if offline progress should be displayed
 */
gboolean
lp_game_has_offline_progress (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), FALSE);

    return self->show_offline_progress;
}

/**
 * lp_game_get_offline_gold_earned:
 * @self: an #LpGame
 *
 * Gets the gold earned while offline.
 *
 * Returns: (transfer none) (nullable): Offline gold amount
 */
const LrgBigNumber *
lp_game_get_offline_gold_earned (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    return self->offline_gold_earned;
}

/**
 * lp_game_get_offline_seconds:
 * @self: an #LpGame
 *
 * Gets the number of seconds the player was offline.
 *
 * Returns: Seconds offline
 */
gdouble
lp_game_get_offline_seconds (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), 0.0);

    return self->offline_seconds;
}

/**
 * lp_game_clear_offline_progress:
 * @self: an #LpGame
 *
 * Clears the offline progress flag.
 */
void
lp_game_clear_offline_progress (LpGame *self)
{
    g_return_if_fail (LP_IS_GAME (self));

    self->show_offline_progress = FALSE;
    g_clear_pointer (&self->offline_gold_earned, lrg_big_number_free);
    self->offline_seconds = 0.0;
}
