/* lp-world-simulation.c - World State Simulation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_SIMULATION
#include "../lp-log.h"

#include "lp-world-simulation.h"

/* Default starting year (Year of the Lich's awakening) */
#define DEFAULT_STARTING_YEAR (847)

/* Economic cycle length in years */
#define ECONOMIC_CYCLE_LENGTH (50)

struct _LpWorldSimulation
{
    GObject parent_instance;

    guint64    current_year;      /* Current simulation year */
    GPtrArray *kingdoms;          /* Array of LpKingdom (empty in Phase 1) */
    guint      economic_phase;    /* Current economic cycle phase (0-3) */
    gdouble    base_growth_rate;  /* Base economic growth rate */
};

enum
{
    PROP_0,
    PROP_CURRENT_YEAR,
    N_PROPS
};

enum
{
    SIGNAL_YEAR_ADVANCED,
    SIGNAL_EVENT_OCCURRED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_world_simulation_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpWorldSimulation, lp_world_simulation, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_world_simulation_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_world_simulation_get_save_id (LrgSaveable *saveable)
{
    return "world-simulation";
}

static gboolean
lp_world_simulation_save (LrgSaveable    *saveable,
                          LrgSaveContext *context,
                          GError        **error)
{
    LpWorldSimulation *self = LP_WORLD_SIMULATION (saveable);

    lrg_save_context_write_uint (context, "current-year", self->current_year);
    lrg_save_context_write_uint (context, "economic-phase", self->economic_phase);
    lrg_save_context_write_double (context, "base-growth-rate",
                                   self->base_growth_rate);

    /* Phase 4+: Save kingdoms */
    lrg_save_context_write_uint (context, "kingdom-count", 0);

    return TRUE;
}

static gboolean
lp_world_simulation_load (LrgSaveable    *saveable,
                          LrgSaveContext *context,
                          GError        **error)
{
    LpWorldSimulation *self = LP_WORLD_SIMULATION (saveable);

    self->current_year = lrg_save_context_read_uint (context, "current-year",
                                                     DEFAULT_STARTING_YEAR);
    self->economic_phase = (guint)lrg_save_context_read_uint (context,
                                                               "economic-phase", 0);
    self->base_growth_rate = lrg_save_context_read_double (context,
                                                           "base-growth-rate", 1.0);

    /* Phase 4+: Load kingdoms */
    g_ptr_array_set_size (self->kingdoms, 0);

    lp_log_debug ("Loaded world simulation: year %lu, phase %u",
                  self->current_year, self->economic_phase);

    return TRUE;
}

static void
lp_world_simulation_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_world_simulation_get_save_id;
    iface->save = lp_world_simulation_save;
    iface->load = lp_world_simulation_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_world_simulation_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    LpWorldSimulation *self = LP_WORLD_SIMULATION (object);

    switch (prop_id)
    {
    case PROP_CURRENT_YEAR:
        g_value_set_uint64 (value, self->current_year);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_world_simulation_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    LpWorldSimulation *self = LP_WORLD_SIMULATION (object);

    switch (prop_id)
    {
    case PROP_CURRENT_YEAR:
        lp_world_simulation_set_current_year (self, g_value_get_uint64 (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_world_simulation_finalize (GObject *object)
{
    LpWorldSimulation *self = LP_WORLD_SIMULATION (object);

    lp_log_debug ("Finalizing world simulation");

    g_clear_pointer (&self->kingdoms, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_world_simulation_parent_class)->finalize (object);
}

static void
lp_world_simulation_class_init (LpWorldSimulationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_world_simulation_get_property;
    object_class->set_property = lp_world_simulation_set_property;
    object_class->finalize = lp_world_simulation_finalize;

    /**
     * LpWorldSimulation:current-year:
     *
     * The current simulation year.
     */
    properties[PROP_CURRENT_YEAR] =
        g_param_spec_uint64 ("current-year",
                             "Current Year",
                             "The current simulation year",
                             0, G_MAXUINT64, DEFAULT_STARTING_YEAR,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpWorldSimulation::year-advanced:
     * @self: the #LpWorldSimulation
     * @year: the new year
     *
     * Emitted when the simulation advances a year.
     */
    signals[SIGNAL_YEAR_ADVANCED] =
        g_signal_new ("year-advanced",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_UINT64);

    /**
     * LpWorldSimulation::event-occurred:
     * @self: the #LpWorldSimulation
     * @event: (transfer none): the event that occurred
     *
     * Emitted when a world event occurs.
     */
    signals[SIGNAL_EVENT_OCCURRED] =
        g_signal_new ("event-occurred",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_OBJECT);
}

static void
lp_world_simulation_init (LpWorldSimulation *self)
{
    self->current_year = DEFAULT_STARTING_YEAR;
    self->kingdoms = g_ptr_array_new_with_free_func (g_object_unref);
    self->economic_phase = 0;
    self->base_growth_rate = 1.0;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_world_simulation_new:
 *
 * Creates a new world simulation.
 *
 * Returns: (transfer full): A new #LpWorldSimulation
 */
LpWorldSimulation *
lp_world_simulation_new (void)
{
    return g_object_new (LP_TYPE_WORLD_SIMULATION, NULL);
}

/* ==========================================================================
 * Simulation Control
 * ========================================================================== */

/**
 * lp_world_simulation_advance_year:
 * @self: an #LpWorldSimulation
 *
 * Advances the simulation by one year.
 *
 * Returns: (transfer full) (element-type LpEvent): List of events
 */
GList *
lp_world_simulation_advance_year (LpWorldSimulation *self)
{
    GList *events = NULL;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    self->current_year++;

    /*
     * Phase 1 skeleton: Only advance year, no events.
     * Phase 4+: Process kingdoms, generate events, update economics.
     */

    /* Update economic cycle phase */
    self->economic_phase = (guint)((self->current_year / (ECONOMIC_CYCLE_LENGTH / 4)) % 4);

    lp_log_debug ("Advanced to year %lu (economic phase: %u)",
                  self->current_year, self->economic_phase);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_YEAR]);
    g_signal_emit (self, signals[SIGNAL_YEAR_ADVANCED], 0, self->current_year);

    return events;
}

/**
 * lp_world_simulation_advance_years:
 * @self: an #LpWorldSimulation
 * @years: number of years to advance
 *
 * Advances the simulation by multiple years.
 *
 * Returns: (transfer full) (element-type LpEvent): List of all events
 */
GList *
lp_world_simulation_advance_years (LpWorldSimulation *self,
                                   guint              years)
{
    GList *all_events = NULL;
    guint i;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    lp_log_info ("Advancing world simulation by %u years", years);

    for (i = 0; i < years; i++)
    {
        GList *year_events = lp_world_simulation_advance_year (self);
        all_events = g_list_concat (all_events, year_events);
    }

    return all_events;
}

/**
 * lp_world_simulation_get_current_year:
 * @self: an #LpWorldSimulation
 *
 * Gets the current simulation year.
 *
 * Returns: The current year
 */
guint64
lp_world_simulation_get_current_year (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->current_year;
}

/**
 * lp_world_simulation_set_current_year:
 * @self: an #LpWorldSimulation
 * @year: the year to set
 *
 * Sets the current simulation year directly.
 */
void
lp_world_simulation_set_current_year (LpWorldSimulation *self,
                                      guint64            year)
{
    g_return_if_fail (LP_IS_WORLD_SIMULATION (self));

    if (self->current_year == year)
        return;

    self->current_year = year;
    self->economic_phase = (guint)((year / (ECONOMIC_CYCLE_LENGTH / 4)) % 4);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_YEAR]);
}

/* ==========================================================================
 * Kingdom/Region Access (Skeleton)
 * ========================================================================== */

/**
 * lp_world_simulation_get_kingdoms:
 * @self: an #LpWorldSimulation
 *
 * Gets all kingdoms in the world.
 *
 * Returns: (transfer none) (element-type LpKingdom): Array of kingdoms
 */
GPtrArray *
lp_world_simulation_get_kingdoms (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    return self->kingdoms;
}

/**
 * lp_world_simulation_get_kingdom_count:
 * @self: an #LpWorldSimulation
 *
 * Gets the number of kingdoms.
 *
 * Returns: Number of kingdoms
 */
guint
lp_world_simulation_get_kingdom_count (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->kingdoms->len;
}

/* ==========================================================================
 * Economic State (Skeleton)
 * ========================================================================== */

/**
 * lp_world_simulation_get_economic_cycle_phase:
 * @self: an #LpWorldSimulation
 *
 * Gets the current economic cycle phase.
 *
 * Returns: Economic cycle phase (0-3)
 */
guint
lp_world_simulation_get_economic_cycle_phase (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->economic_phase;
}

/**
 * lp_world_simulation_get_base_growth_rate:
 * @self: an #LpWorldSimulation
 *
 * Gets the current base economic growth rate.
 *
 * Returns: Growth rate as multiplier
 */
gdouble
lp_world_simulation_get_base_growth_rate (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 1.0);

    /*
     * Phase 1 skeleton: Base rate modified by economic phase.
     * 0 = Expansion (+3%), 1 = Peak (+1%), 2 = Contraction (-2%), 3 = Trough (-1%)
     */
    switch (self->economic_phase)
    {
    case 0:
        return 1.03;
    case 1:
        return 1.01;
    case 2:
        return 0.98;
    case 3:
        return 0.99;
    default:
        return 1.0;
    }
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_world_simulation_reset:
 * @self: an #LpWorldSimulation
 * @starting_year: the year to start at
 *
 * Resets the world simulation to initial state.
 */
void
lp_world_simulation_reset (LpWorldSimulation *self,
                           guint64            starting_year)
{
    g_return_if_fail (LP_IS_WORLD_SIMULATION (self));

    lp_log_debug ("Resetting world simulation to year %lu", starting_year);

    self->current_year = starting_year;
    self->economic_phase = 0;
    self->base_growth_rate = 1.0;
    g_ptr_array_set_size (self->kingdoms, 0);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_YEAR]);
}
