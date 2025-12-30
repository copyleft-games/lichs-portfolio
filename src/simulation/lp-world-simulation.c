/* lp-world-simulation.c - World State Simulation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_SIMULATION
#include "../lp-log.h"

#include "lp-world-simulation.h"
#include "lp-kingdom.h"
#include "lp-region.h"
#include "lp-event.h"
#include "lp-event-generator.h"
#include "lp-competitor.h"

/* Default starting year (Year of the Lich's awakening) */
#define DEFAULT_STARTING_YEAR (847)

/* Economic cycle length in years */
#define ECONOMIC_CYCLE_LENGTH (50)

struct _LpWorldSimulation
{
    GObject parent_instance;

    guint64    current_year;       /* Current simulation year */
    guint      economic_phase;     /* Current economic cycle phase (0-3) */
    gdouble    base_growth_rate;   /* Base economic growth rate */

    /* World entities */
    GPtrArray *kingdoms;           /* Array of LpKingdom */
    GPtrArray *regions;            /* Array of LpRegion */
    GPtrArray *competitors;        /* Array of LpCompetitor */
    GPtrArray *active_events;      /* Array of LpEvent (ongoing) */

    /* Event generation */
    LpEventGenerator *event_generator;
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
    SIGNAL_KINGDOM_COLLAPSED,
    SIGNAL_WAR_STARTED,
    SIGNAL_WAR_ENDED,
    SIGNAL_COMPETITOR_DISCOVERED,
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
 * Helper Functions
 * ========================================================================== */

static void
process_active_events (LpWorldSimulation *self)
{
    guint i;

    /*
     * Tick all active events and remove resolved ones.
     */
    for (i = 0; i < self->active_events->len; )
    {
        LpEvent *event = g_ptr_array_index (self->active_events, i);

        lp_event_tick_year (event);

        if (!lp_event_get_is_active (event))
        {
            g_ptr_array_remove_index (self->active_events, i);
        }
        else
        {
            i++;
        }
    }
}

static void
tick_kingdoms (LpWorldSimulation *self)
{
    guint i;

    /*
     * Tick all kingdoms for annual attribute changes, war checks, etc.
     */
    for (i = 0; i < self->kingdoms->len; i++)
    {
        LpKingdom *kingdom = g_ptr_array_index (self->kingdoms, i);
        lp_kingdom_tick_year (kingdom);
    }
}

static void
tick_competitors (LpWorldSimulation *self)
{
    guint i;

    /*
     * Tick all competitors for AI decisions.
     */
    for (i = 0; i < self->competitors->len; i++)
    {
        LpCompetitor *competitor = g_ptr_array_index (self->competitors, i);
        lp_competitor_tick_year (competitor, self);
    }
}

static void
notify_competitors_of_event (LpWorldSimulation *self,
                             LpEvent           *event)
{
    guint i;

    /*
     * Notify all competitors of the event so they can react.
     */
    for (i = 0; i < self->competitors->len; i++)
    {
        LpCompetitor *competitor = g_ptr_array_index (self->competitors, i);
        lp_competitor_react_to_event (competitor, event);
    }
}

static void
apply_event_to_world (LpWorldSimulation *self,
                      LpEvent           *event)
{
    guint64 current_year;

    current_year = self->current_year;

    /* Set the event's occurrence year */
    lp_event_set_year_occurred (event, current_year);
    lp_event_set_is_active (event, TRUE);

    /* Apply immediate effects */
    lp_event_apply_effects (event, self);

    /* Notify competitors */
    notify_competitors_of_event (self, event);

    /* Add to active events if duration > 0 */
    if (lp_event_get_duration_years (event) > 0)
    {
        g_ptr_array_add (self->active_events, g_object_ref (event));
    }

    /* Emit signal */
    g_signal_emit (self, signals[SIGNAL_EVENT_OCCURRED], 0, event);
}

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
    guint i;
    g_autofree gchar *key = NULL;

    lrg_save_context_write_uint (context, "current-year", self->current_year);
    lrg_save_context_write_uint (context, "economic-phase", self->economic_phase);
    lrg_save_context_write_double (context, "base-growth-rate",
                                   self->base_growth_rate);

    /* Save kingdoms using numbered sections */
    lrg_save_context_begin_section (context, "kingdoms");
    lrg_save_context_write_uint (context, "count", self->kingdoms->len);
    for (i = 0; i < self->kingdoms->len; i++)
    {
        LpKingdom *kingdom = g_ptr_array_index (self->kingdoms, i);
        key = g_strdup_printf ("%u", i);
        lrg_save_context_begin_section (context, key);
        lrg_saveable_save (LRG_SAVEABLE (kingdom), context, error);
        lrg_save_context_end_section (context);
        g_clear_pointer (&key, g_free);
    }
    lrg_save_context_end_section (context);

    /* Save regions using numbered sections */
    lrg_save_context_begin_section (context, "regions");
    lrg_save_context_write_uint (context, "count", self->regions->len);
    for (i = 0; i < self->regions->len; i++)
    {
        LpRegion *region = g_ptr_array_index (self->regions, i);
        key = g_strdup_printf ("%u", i);
        lrg_save_context_begin_section (context, key);
        lrg_saveable_save (LRG_SAVEABLE (region), context, error);
        lrg_save_context_end_section (context);
        g_clear_pointer (&key, g_free);
    }
    lrg_save_context_end_section (context);

    /* Save competitors using numbered sections */
    lrg_save_context_begin_section (context, "competitors");
    lrg_save_context_write_uint (context, "count", self->competitors->len);
    for (i = 0; i < self->competitors->len; i++)
    {
        LpCompetitor *competitor = g_ptr_array_index (self->competitors, i);
        key = g_strdup_printf ("%u", i);
        lrg_save_context_begin_section (context, key);
        lrg_saveable_save (LRG_SAVEABLE (competitor), context, error);
        lrg_save_context_end_section (context);
        g_clear_pointer (&key, g_free);
    }
    lrg_save_context_end_section (context);

    /* Note: Active events are transient and not saved */

    return TRUE;
}

static gboolean
lp_world_simulation_load (LrgSaveable    *saveable,
                          LrgSaveContext *context,
                          GError        **error)
{
    LpWorldSimulation *self = LP_WORLD_SIMULATION (saveable);
    guint count;
    guint i;
    g_autofree gchar *key = NULL;

    self->current_year = lrg_save_context_read_uint (context, "current-year",
                                                     DEFAULT_STARTING_YEAR);
    self->economic_phase = (guint)lrg_save_context_read_uint (context,
                                                               "economic-phase", 0);
    self->base_growth_rate = lrg_save_context_read_double (context,
                                                           "base-growth-rate", 1.0);

    /* Load kingdoms from numbered sections */
    g_ptr_array_set_size (self->kingdoms, 0);
    if (lrg_save_context_enter_section (context, "kingdoms"))
    {
        count = (guint)lrg_save_context_read_uint (context, "count", 0);
        for (i = 0; i < count; i++)
        {
            key = g_strdup_printf ("%u", i);
            if (lrg_save_context_enter_section (context, key))
            {
                LpKingdom *kingdom = lp_kingdom_new (NULL, NULL);
                lrg_saveable_load (LRG_SAVEABLE (kingdom), context, error);
                g_ptr_array_add (self->kingdoms, kingdom);
                lrg_save_context_leave_section (context);
            }
            g_clear_pointer (&key, g_free);
        }
        lrg_save_context_leave_section (context);
    }

    /* Load regions from numbered sections */
    g_ptr_array_set_size (self->regions, 0);
    if (lrg_save_context_enter_section (context, "regions"))
    {
        count = (guint)lrg_save_context_read_uint (context, "count", 0);
        for (i = 0; i < count; i++)
        {
            key = g_strdup_printf ("%u", i);
            if (lrg_save_context_enter_section (context, key))
            {
                LpRegion *region = lp_region_new (NULL, NULL, LP_GEOGRAPHY_TYPE_INLAND);
                lrg_saveable_load (LRG_SAVEABLE (region), context, error);
                g_ptr_array_add (self->regions, region);
                lrg_save_context_leave_section (context);
            }
            g_clear_pointer (&key, g_free);
        }
        lrg_save_context_leave_section (context);
    }

    /* Load competitors from numbered sections */
    g_ptr_array_set_size (self->competitors, 0);
    if (lrg_save_context_enter_section (context, "competitors"))
    {
        count = (guint)lrg_save_context_read_uint (context, "count", 0);
        for (i = 0; i < count; i++)
        {
            key = g_strdup_printf ("%u", i);
            if (lrg_save_context_enter_section (context, key))
            {
                LpCompetitor *competitor = lp_competitor_new (NULL, NULL,
                                                              LP_COMPETITOR_TYPE_DRAGON);
                lrg_saveable_load (LRG_SAVEABLE (competitor), context, error);
                g_ptr_array_add (self->competitors, competitor);
                lrg_save_context_leave_section (context);
            }
            g_clear_pointer (&key, g_free);
        }
        lrg_save_context_leave_section (context);
    }

    /* Active events are transient and not loaded */
    g_ptr_array_set_size (self->active_events, 0);

    lp_log_debug ("Loaded world simulation: year %lu, phase %u, %u kingdoms, %u regions, %u competitors",
                  self->current_year, self->economic_phase,
                  self->kingdoms->len, self->regions->len, self->competitors->len);

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
    g_clear_pointer (&self->regions, g_ptr_array_unref);
    g_clear_pointer (&self->competitors, g_ptr_array_unref);
    g_clear_pointer (&self->active_events, g_ptr_array_unref);

    /* Event generator is a singleton, don't unref it */

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

    /**
     * LpWorldSimulation::kingdom-collapsed:
     * @self: the #LpWorldSimulation
     * @kingdom: (transfer none): the kingdom that collapsed
     *
     * Emitted when a kingdom collapses.
     */
    signals[SIGNAL_KINGDOM_COLLAPSED] =
        g_signal_new ("kingdom-collapsed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_OBJECT);

    /**
     * LpWorldSimulation::war-started:
     * @self: the #LpWorldSimulation
     * @aggressor: (transfer none): the kingdom that declared war
     * @defender: (transfer none): the kingdom that was attacked
     *
     * Emitted when a war starts.
     */
    signals[SIGNAL_WAR_STARTED] =
        g_signal_new ("war-started",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_OBJECT, G_TYPE_OBJECT);

    /**
     * LpWorldSimulation::war-ended:
     * @self: the #LpWorldSimulation
     * @kingdom1: (transfer none): first kingdom
     * @kingdom2: (transfer none): second kingdom
     *
     * Emitted when a war ends.
     */
    signals[SIGNAL_WAR_ENDED] =
        g_signal_new ("war-ended",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_OBJECT, G_TYPE_OBJECT);

    /**
     * LpWorldSimulation::competitor-discovered:
     * @self: the #LpWorldSimulation
     * @competitor: (transfer none): the competitor that was discovered
     *
     * Emitted when a competitor is discovered by the player.
     */
    signals[SIGNAL_COMPETITOR_DISCOVERED] =
        g_signal_new ("competitor-discovered",
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
    self->economic_phase = 0;
    self->base_growth_rate = 1.0;

    self->kingdoms = g_ptr_array_new_with_free_func (g_object_unref);
    self->regions = g_ptr_array_new_with_free_func (g_object_unref);
    self->competitors = g_ptr_array_new_with_free_func (g_object_unref);
    self->active_events = g_ptr_array_new_with_free_func (g_object_unref);

    self->event_generator = lp_event_generator_get_default ();
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpWorldSimulation *
lp_world_simulation_new (void)
{
    return g_object_new (LP_TYPE_WORLD_SIMULATION, NULL);
}

/* ==========================================================================
 * Simulation Control
 * ========================================================================== */

GList *
lp_world_simulation_advance_year (LpWorldSimulation *self)
{
    GList *events = NULL;
    GList *generated_events;
    GList *iter;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    self->current_year++;

    /* Update economic cycle phase */
    self->economic_phase = (guint)((self->current_year / (ECONOMIC_CYCLE_LENGTH / 4)) % 4);

    /* Process active events */
    process_active_events (self);

    /* Tick kingdoms */
    tick_kingdoms (self);

    /* Tick competitors */
    tick_competitors (self);

    /* Generate yearly events */
    generated_events = lp_event_generator_generate_yearly_events (
        self->event_generator, self);

    for (iter = generated_events; iter != NULL; iter = iter->next)
    {
        LpEvent *event = LP_EVENT (iter->data);
        apply_event_to_world (self, event);
        events = g_list_prepend (events, g_object_ref (event));
    }

    g_list_free_full (generated_events, g_object_unref);

    /* Generate decade events (every 10 years) */
    if (self->current_year % 10 == 0)
    {
        generated_events = lp_event_generator_generate_decade_events (
            self->event_generator, self);

        for (iter = generated_events; iter != NULL; iter = iter->next)
        {
            LpEvent *event = LP_EVENT (iter->data);
            apply_event_to_world (self, event);
            events = g_list_prepend (events, g_object_ref (event));
        }

        g_list_free_full (generated_events, g_object_unref);
    }

    /* Generate era events (every 100 years) */
    if (self->current_year % 100 == 0)
    {
        generated_events = lp_event_generator_generate_era_events (
            self->event_generator, self);

        for (iter = generated_events; iter != NULL; iter = iter->next)
        {
            LpEvent *event = LP_EVENT (iter->data);
            apply_event_to_world (self, event);
            events = g_list_prepend (events, g_object_ref (event));
        }

        g_list_free_full (generated_events, g_object_unref);
    }

    lp_log_debug ("Advanced to year %lu (economic phase: %u, events: %u)",
                  self->current_year, self->economic_phase,
                  g_list_length (events));

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_YEAR]);
    g_signal_emit (self, signals[SIGNAL_YEAR_ADVANCED], 0, self->current_year);

    return g_list_reverse (events);
}

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

guint64
lp_world_simulation_get_current_year (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->current_year;
}

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
 * Kingdom Management
 * ========================================================================== */

GPtrArray *
lp_world_simulation_get_kingdoms (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    return self->kingdoms;
}

guint
lp_world_simulation_get_kingdom_count (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->kingdoms->len;
}

void
lp_world_simulation_add_kingdom (LpWorldSimulation *self,
                                 LpKingdom         *kingdom)
{
    g_return_if_fail (LP_IS_WORLD_SIMULATION (self));
    g_return_if_fail (LP_IS_KINGDOM (kingdom));

    g_ptr_array_add (self->kingdoms, kingdom);

    lp_log_debug ("Added kingdom '%s' to world simulation",
                  lp_kingdom_get_name (kingdom));
}

LpKingdom *
lp_world_simulation_get_kingdom_by_id (LpWorldSimulation *self,
                                       const gchar       *id)
{
    guint i;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    for (i = 0; i < self->kingdoms->len; i++)
    {
        LpKingdom *kingdom = g_ptr_array_index (self->kingdoms, i);
        if (g_strcmp0 (lp_kingdom_get_id (kingdom), id) == 0)
            return kingdom;
    }

    return NULL;
}

gboolean
lp_world_simulation_remove_kingdom (LpWorldSimulation *self,
                                    const gchar       *id)
{
    guint i;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), FALSE);
    g_return_val_if_fail (id != NULL, FALSE);

    for (i = 0; i < self->kingdoms->len; i++)
    {
        LpKingdom *kingdom = g_ptr_array_index (self->kingdoms, i);
        if (g_strcmp0 (lp_kingdom_get_id (kingdom), id) == 0)
        {
            g_ptr_array_remove_index (self->kingdoms, i);
            return TRUE;
        }
    }

    return FALSE;
}

/* ==========================================================================
 * Region Management
 * ========================================================================== */

GPtrArray *
lp_world_simulation_get_regions (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    return self->regions;
}

guint
lp_world_simulation_get_region_count (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->regions->len;
}

void
lp_world_simulation_add_region (LpWorldSimulation *self,
                                LpRegion          *region)
{
    g_return_if_fail (LP_IS_WORLD_SIMULATION (self));
    g_return_if_fail (LP_IS_REGION (region));

    g_ptr_array_add (self->regions, region);

    lp_log_debug ("Added region '%s' to world simulation",
                  lp_region_get_name (region));
}

LpRegion *
lp_world_simulation_get_region_by_id (LpWorldSimulation *self,
                                      const gchar       *id)
{
    guint i;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    for (i = 0; i < self->regions->len; i++)
    {
        LpRegion *region = g_ptr_array_index (self->regions, i);
        if (g_strcmp0 (lp_region_get_id (region), id) == 0)
            return region;
    }

    return NULL;
}

/* ==========================================================================
 * Competitor Management
 * ========================================================================== */

GPtrArray *
lp_world_simulation_get_competitors (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    return self->competitors;
}

guint
lp_world_simulation_get_competitor_count (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->competitors->len;
}

void
lp_world_simulation_add_competitor (LpWorldSimulation *self,
                                    LpCompetitor      *competitor)
{
    g_return_if_fail (LP_IS_WORLD_SIMULATION (self));
    g_return_if_fail (LP_IS_COMPETITOR (competitor));

    g_ptr_array_add (self->competitors, competitor);

    lp_log_debug ("Added competitor '%s' to world simulation",
                  lp_competitor_get_name (competitor));
}

LpCompetitor *
lp_world_simulation_get_competitor_by_id (LpWorldSimulation *self,
                                          const gchar       *id)
{
    guint i;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    for (i = 0; i < self->competitors->len; i++)
    {
        LpCompetitor *competitor = g_ptr_array_index (self->competitors, i);
        if (g_strcmp0 (lp_competitor_get_id (competitor), id) == 0)
            return competitor;
    }

    return NULL;
}

GList *
lp_world_simulation_get_known_competitors (LpWorldSimulation *self)
{
    GList *known = NULL;
    guint i;

    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    for (i = 0; i < self->competitors->len; i++)
    {
        LpCompetitor *competitor = g_ptr_array_index (self->competitors, i);
        if (lp_competitor_get_is_known (competitor))
            known = g_list_prepend (known, competitor);
    }

    return g_list_reverse (known);
}

/* ==========================================================================
 * Event Management
 * ========================================================================== */

GPtrArray *
lp_world_simulation_get_active_events (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    return self->active_events;
}

LpEventGenerator *
lp_world_simulation_get_event_generator (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), NULL);

    return self->event_generator;
}

/* ==========================================================================
 * Economic State
 * ========================================================================== */

guint
lp_world_simulation_get_economic_cycle_phase (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 0);

    return self->economic_phase;
}

gdouble
lp_world_simulation_get_base_growth_rate (LpWorldSimulation *self)
{
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (self), 1.0);

    /*
     * Base rate modified by economic phase.
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
    g_ptr_array_set_size (self->regions, 0);
    g_ptr_array_set_size (self->competitors, 0);
    g_ptr_array_set_size (self->active_events, 0);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_YEAR]);
}
