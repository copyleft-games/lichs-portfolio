/* lp-event.c - World Event Base Class
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_SIMULATION
#include "../lp-log.h"

#include "lp-event.h"
#include "lp-world-simulation.h"
#include "../investment/lp-investment.h"

/* ==========================================================================
 * LpEventChoice - Boxed Type Implementation
 * ========================================================================== */

G_DEFINE_BOXED_TYPE (LpEventChoice, lp_event_choice,
                     lp_event_choice_copy, lp_event_choice_free)

LpEventChoice *
lp_event_choice_new (const gchar *id,
                     const gchar *text)
{
    LpEventChoice *choice;

    choice = g_slice_new0 (LpEventChoice);
    choice->id = g_strdup (id);
    choice->text = g_strdup (text);
    choice->consequence = NULL;
    choice->requires_gold = FALSE;
    choice->gold_cost = 0;
    choice->requires_agent = FALSE;

    return choice;
}

LpEventChoice *
lp_event_choice_copy (const LpEventChoice *choice)
{
    LpEventChoice *copy;

    if (choice == NULL)
        return NULL;

    copy = g_slice_new0 (LpEventChoice);
    copy->id = g_strdup (choice->id);
    copy->text = g_strdup (choice->text);
    copy->consequence = g_strdup (choice->consequence);
    copy->requires_gold = choice->requires_gold;
    copy->gold_cost = choice->gold_cost;
    copy->requires_agent = choice->requires_agent;

    return copy;
}

void
lp_event_choice_free (LpEventChoice *choice)
{
    if (choice == NULL)
        return;

    g_free (choice->id);
    g_free (choice->text);
    g_free (choice->consequence);
    g_slice_free (LpEventChoice, choice);
}

/* ==========================================================================
 * LpEvent - Private Data
 * ========================================================================== */

typedef struct
{
    gchar          *id;
    gchar          *name;
    gchar          *description;
    LpEventType     event_type;
    LpEventSeverity severity;
    guint64         year_occurred;
    gchar          *affects_region_id;
    gchar          *affects_kingdom_id;
    guint           duration_years;
    guint           years_remaining;
    gboolean        is_active;
} LpEventPrivate;

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_DESCRIPTION,
    PROP_EVENT_TYPE,
    PROP_SEVERITY,
    PROP_YEAR_OCCURRED,
    PROP_AFFECTS_REGION_ID,
    PROP_AFFECTS_KINGDOM_ID,
    PROP_DURATION_YEARS,
    PROP_IS_ACTIVE,
    N_PROPS
};

enum
{
    SIGNAL_APPLIED,
    SIGNAL_RESOLVED,
    SIGNAL_CHOICE_MADE,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations */
static void lp_event_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpEvent, lp_event, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (LpEvent)
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_event_saveable_init))

/* ==========================================================================
 * Default Virtual Method Implementations
 * ========================================================================== */

static void
lp_event_real_apply_effects (LpEvent           *self,
                             LpWorldSimulation *simulation)
{
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    (void)simulation;

    lp_log_debug ("Event %s: default apply_effects (no-op)", priv->name);

    g_signal_emit (self, signals[SIGNAL_APPLIED], 0);
}

static GPtrArray *
lp_event_real_get_choices (LpEvent *self)
{
    /* Default: no choices */
    (void)self;
    return NULL;
}

static gdouble
lp_event_real_get_investment_modifier (LpEvent      *self,
                                       LpInvestment *investment)
{
    /* Default: no modification */
    (void)self;
    (void)investment;
    return 1.0;
}

static gchar *
lp_event_real_get_narrative_text (LpEvent *self)
{
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    if (priv->description != NULL)
        return g_strdup (priv->description);

    return g_strdup_printf ("The event '%s' has occurred.", priv->name);
}

static gboolean
lp_event_real_can_occur (LpEvent           *self,
                         LpWorldSimulation *simulation)
{
    /* Default: can always occur */
    (void)self;
    (void)simulation;
    return TRUE;
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_event_get_save_id (LrgSaveable *saveable)
{
    LpEvent *self = LP_EVENT (saveable);
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    return priv->id;
}

static gboolean
lp_event_save (LrgSaveable    *saveable,
               LrgSaveContext *context,
               GError        **error)
{
    LpEvent *self = LP_EVENT (saveable);
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    (void)error;

    /* Save type name for polymorphic loading */
    lrg_save_context_write_string (context, "type-name",
                                   G_OBJECT_TYPE_NAME (self));

    lrg_save_context_write_string (context, "id", priv->id);
    lrg_save_context_write_string (context, "name", priv->name);

    if (priv->description != NULL)
        lrg_save_context_write_string (context, "description", priv->description);

    lrg_save_context_write_int (context, "event-type", priv->event_type);
    lrg_save_context_write_int (context, "severity", priv->severity);
    lrg_save_context_write_uint (context, "year-occurred", priv->year_occurred);

    if (priv->affects_region_id != NULL)
        lrg_save_context_write_string (context, "affects-region-id",
                                       priv->affects_region_id);

    if (priv->affects_kingdom_id != NULL)
        lrg_save_context_write_string (context, "affects-kingdom-id",
                                       priv->affects_kingdom_id);

    lrg_save_context_write_uint (context, "duration-years", priv->duration_years);
    lrg_save_context_write_uint (context, "years-remaining", priv->years_remaining);
    lrg_save_context_write_boolean (context, "is-active", priv->is_active);

    return TRUE;
}

static gboolean
lp_event_load (LrgSaveable    *saveable,
               LrgSaveContext *context,
               GError        **error)
{
    LpEvent *self = LP_EVENT (saveable);
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    (void)error;

    /* Clear existing */
    g_clear_pointer (&priv->id, g_free);
    g_clear_pointer (&priv->name, g_free);
    g_clear_pointer (&priv->description, g_free);
    g_clear_pointer (&priv->affects_region_id, g_free);
    g_clear_pointer (&priv->affects_kingdom_id, g_free);

    /* Load properties */
    priv->id = lrg_save_context_read_string (context, "id", "unknown");
    priv->name = lrg_save_context_read_string (context, "name", "Unknown Event");
    priv->description = lrg_save_context_read_string (context, "description", NULL);
    priv->event_type = lrg_save_context_read_int (context, "event-type",
                                                  LP_EVENT_TYPE_ECONOMIC);
    priv->severity = lrg_save_context_read_int (context, "severity",
                                                LP_EVENT_SEVERITY_MINOR);
    priv->year_occurred = lrg_save_context_read_uint (context, "year-occurred", 0);
    priv->affects_region_id = lrg_save_context_read_string (context,
                                                            "affects-region-id", NULL);
    priv->affects_kingdom_id = lrg_save_context_read_string (context,
                                                             "affects-kingdom-id", NULL);
    priv->duration_years = lrg_save_context_read_uint (context, "duration-years", 0);
    priv->years_remaining = lrg_save_context_read_uint (context, "years-remaining", 0);
    priv->is_active = lrg_save_context_read_boolean (context, "is-active", FALSE);

    return TRUE;
}

static void
lp_event_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_event_get_save_id;
    iface->save = lp_event_save;
    iface->load = lp_event_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_event_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    LpEvent *self = LP_EVENT (object);
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ID:
        g_value_set_string (value, priv->id);
        break;

    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_DESCRIPTION:
        g_value_set_string (value, priv->description);
        break;

    case PROP_EVENT_TYPE:
        g_value_set_enum (value, priv->event_type);
        break;

    case PROP_SEVERITY:
        g_value_set_enum (value, priv->severity);
        break;

    case PROP_YEAR_OCCURRED:
        g_value_set_uint64 (value, priv->year_occurred);
        break;

    case PROP_AFFECTS_REGION_ID:
        g_value_set_string (value, priv->affects_region_id);
        break;

    case PROP_AFFECTS_KINGDOM_ID:
        g_value_set_string (value, priv->affects_kingdom_id);
        break;

    case PROP_DURATION_YEARS:
        g_value_set_uint (value, priv->duration_years);
        break;

    case PROP_IS_ACTIVE:
        g_value_set_boolean (value, priv->is_active);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_event_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    LpEvent *self = LP_EVENT (object);
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ID:
        g_free (priv->id);
        priv->id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        lp_event_set_name (self, g_value_get_string (value));
        break;

    case PROP_DESCRIPTION:
        lp_event_set_description (self, g_value_get_string (value));
        break;

    case PROP_EVENT_TYPE:
        priv->event_type = g_value_get_enum (value);
        break;

    case PROP_SEVERITY:
        lp_event_set_severity (self, g_value_get_enum (value));
        break;

    case PROP_YEAR_OCCURRED:
        lp_event_set_year_occurred (self, g_value_get_uint64 (value));
        break;

    case PROP_AFFECTS_REGION_ID:
        lp_event_set_affects_region_id (self, g_value_get_string (value));
        break;

    case PROP_AFFECTS_KINGDOM_ID:
        lp_event_set_affects_kingdom_id (self, g_value_get_string (value));
        break;

    case PROP_DURATION_YEARS:
        lp_event_set_duration_years (self, g_value_get_uint (value));
        break;

    case PROP_IS_ACTIVE:
        lp_event_set_is_active (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_event_finalize (GObject *object)
{
    LpEvent *self = LP_EVENT (object);
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    g_clear_pointer (&priv->id, g_free);
    g_clear_pointer (&priv->name, g_free);
    g_clear_pointer (&priv->description, g_free);
    g_clear_pointer (&priv->affects_region_id, g_free);
    g_clear_pointer (&priv->affects_kingdom_id, g_free);

    G_OBJECT_CLASS (lp_event_parent_class)->finalize (object);
}

static void
lp_event_class_init (LpEventClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_event_get_property;
    object_class->set_property = lp_event_set_property;
    object_class->finalize = lp_event_finalize;

    /* Set default virtual method implementations */
    klass->apply_effects = lp_event_real_apply_effects;
    klass->get_choices = lp_event_real_get_choices;
    klass->get_investment_modifier = lp_event_real_get_investment_modifier;
    klass->get_narrative_text = lp_event_real_get_narrative_text;
    klass->can_occur = lp_event_real_can_occur;

    /* Properties */
    properties[PROP_ID] =
        g_param_spec_string ("id", "ID", "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_NAME] =
        g_param_spec_string ("name", "Name", "Display name",
                             "Unknown Event",
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_DESCRIPTION] =
        g_param_spec_string ("description", "Description", "Event description",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_EVENT_TYPE] =
        g_param_spec_enum ("event-type", "Event Type", "Type of event",
                           LP_TYPE_EVENT_TYPE,
                           LP_EVENT_TYPE_ECONOMIC,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS);

    properties[PROP_SEVERITY] =
        g_param_spec_enum ("severity", "Severity", "Event severity",
                           LP_TYPE_EVENT_SEVERITY,
                           LP_EVENT_SEVERITY_MINOR,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    properties[PROP_YEAR_OCCURRED] =
        g_param_spec_uint64 ("year-occurred", "Year Occurred",
                             "Year the event occurred",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_AFFECTS_REGION_ID] =
        g_param_spec_string ("affects-region-id", "Affects Region",
                             "ID of affected region",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_AFFECTS_KINGDOM_ID] =
        g_param_spec_string ("affects-kingdom-id", "Affects Kingdom",
                             "ID of affected kingdom",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_DURATION_YEARS] =
        g_param_spec_uint ("duration-years", "Duration",
                           "Duration in years (0 = instant)",
                           0, G_MAXUINT, 0,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    properties[PROP_IS_ACTIVE] =
        g_param_spec_boolean ("is-active", "Is Active",
                              "Whether the event is currently active",
                              FALSE,
                              G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /* Signals */

    /**
     * LpEvent::applied:
     * @self: the #LpEvent
     *
     * Emitted after the event's effects have been applied.
     */
    signals[SIGNAL_APPLIED] =
        g_signal_new ("applied",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpEvent::resolved:
     * @self: the #LpEvent
     *
     * Emitted when the event is resolved (duration ends or choice made).
     */
    signals[SIGNAL_RESOLVED] =
        g_signal_new ("resolved",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpEvent::choice-made:
     * @self: the #LpEvent
     * @choice_id: the ID of the chosen option
     *
     * Emitted when a player choice is made.
     */
    signals[SIGNAL_CHOICE_MADE] =
        g_signal_new ("choice-made",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
lp_event_init (LpEvent *self)
{
    LpEventPrivate *priv = lp_event_get_instance_private (self);

    priv->id = NULL;
    priv->name = g_strdup ("Unknown Event");
    priv->description = NULL;
    priv->event_type = LP_EVENT_TYPE_ECONOMIC;
    priv->severity = LP_EVENT_SEVERITY_MINOR;
    priv->year_occurred = 0;
    priv->affects_region_id = NULL;
    priv->affects_kingdom_id = NULL;
    priv->duration_years = 0;
    priv->years_remaining = 0;
    priv->is_active = FALSE;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpEvent *
lp_event_new (const gchar *id,
              const gchar *name,
              LpEventType  event_type)
{
    return g_object_new (LP_TYPE_EVENT,
                         "id", id,
                         "name", name,
                         "event-type", event_type,
                         NULL);
}

/* ==========================================================================
 * Property Accessors
 * ========================================================================== */

const gchar *
lp_event_get_id (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), NULL);

    priv = lp_event_get_instance_private (self);
    return priv->id;
}

const gchar *
lp_event_get_name (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), NULL);

    priv = lp_event_get_instance_private (self);
    return priv->name;
}

void
lp_event_set_name (LpEvent     *self,
                   const gchar *name)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (g_strcmp0 (priv->name, name) == 0)
        return;

    g_free (priv->name);
    priv->name = g_strdup (name);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}

const gchar *
lp_event_get_description (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), NULL);

    priv = lp_event_get_instance_private (self);
    return priv->description;
}

void
lp_event_set_description (LpEvent     *self,
                          const gchar *description)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (g_strcmp0 (priv->description, description) == 0)
        return;

    g_free (priv->description);
    priv->description = g_strdup (description);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESCRIPTION]);
}

LpEventType
lp_event_get_event_type (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), LP_EVENT_TYPE_ECONOMIC);

    priv = lp_event_get_instance_private (self);
    return priv->event_type;
}

LpEventSeverity
lp_event_get_severity (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), LP_EVENT_SEVERITY_MINOR);

    priv = lp_event_get_instance_private (self);
    return priv->severity;
}

void
lp_event_set_severity (LpEvent         *self,
                       LpEventSeverity  severity)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (priv->severity == severity)
        return;

    priv->severity = severity;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SEVERITY]);
}

guint64
lp_event_get_year_occurred (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), 0);

    priv = lp_event_get_instance_private (self);
    return priv->year_occurred;
}

void
lp_event_set_year_occurred (LpEvent *self,
                            guint64  year)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (priv->year_occurred == year)
        return;

    priv->year_occurred = year;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_YEAR_OCCURRED]);
}

const gchar *
lp_event_get_affects_region_id (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), NULL);

    priv = lp_event_get_instance_private (self);
    return priv->affects_region_id;
}

void
lp_event_set_affects_region_id (LpEvent     *self,
                                const gchar *region_id)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (g_strcmp0 (priv->affects_region_id, region_id) == 0)
        return;

    g_free (priv->affects_region_id);
    priv->affects_region_id = g_strdup (region_id);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AFFECTS_REGION_ID]);
}

const gchar *
lp_event_get_affects_kingdom_id (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), NULL);

    priv = lp_event_get_instance_private (self);
    return priv->affects_kingdom_id;
}

void
lp_event_set_affects_kingdom_id (LpEvent     *self,
                                 const gchar *kingdom_id)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (g_strcmp0 (priv->affects_kingdom_id, kingdom_id) == 0)
        return;

    g_free (priv->affects_kingdom_id);
    priv->affects_kingdom_id = g_strdup (kingdom_id);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AFFECTS_KINGDOM_ID]);
}

guint
lp_event_get_duration_years (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), 0);

    priv = lp_event_get_instance_private (self);
    return priv->duration_years;
}

void
lp_event_set_duration_years (LpEvent *self,
                             guint    years)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (priv->duration_years == years)
        return;

    priv->duration_years = years;
    priv->years_remaining = years;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DURATION_YEARS]);
}

gboolean
lp_event_get_is_active (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), FALSE);

    priv = lp_event_get_instance_private (self);
    return priv->is_active;
}

void
lp_event_set_is_active (LpEvent  *self,
                        gboolean  active)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    active = !!active;
    if (priv->is_active == active)
        return;

    priv->is_active = active;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_ACTIVE]);
}

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

void
lp_event_apply_effects (LpEvent           *self,
                        LpWorldSimulation *simulation)
{
    LpEventClass *klass;

    g_return_if_fail (LP_IS_EVENT (self));
    g_return_if_fail (LP_IS_WORLD_SIMULATION (simulation));

    klass = LP_EVENT_GET_CLASS (self);
    g_return_if_fail (klass->apply_effects != NULL);

    klass->apply_effects (self, simulation);
}

GPtrArray *
lp_event_get_choices (LpEvent *self)
{
    LpEventClass *klass;

    g_return_val_if_fail (LP_IS_EVENT (self), NULL);

    klass = LP_EVENT_GET_CLASS (self);
    g_return_val_if_fail (klass->get_choices != NULL, NULL);

    return klass->get_choices (self);
}

gdouble
lp_event_get_investment_modifier (LpEvent      *self,
                                  LpInvestment *investment)
{
    LpEventClass *klass;

    g_return_val_if_fail (LP_IS_EVENT (self), 1.0);
    g_return_val_if_fail (LP_IS_INVESTMENT (investment), 1.0);

    klass = LP_EVENT_GET_CLASS (self);
    g_return_val_if_fail (klass->get_investment_modifier != NULL, 1.0);

    return klass->get_investment_modifier (self, investment);
}

gchar *
lp_event_get_narrative_text (LpEvent *self)
{
    LpEventClass *klass;

    g_return_val_if_fail (LP_IS_EVENT (self), NULL);

    klass = LP_EVENT_GET_CLASS (self);
    g_return_val_if_fail (klass->get_narrative_text != NULL, NULL);

    return klass->get_narrative_text (self);
}

gboolean
lp_event_can_occur (LpEvent           *self,
                    LpWorldSimulation *simulation)
{
    LpEventClass *klass;

    g_return_val_if_fail (LP_IS_EVENT (self), FALSE);
    g_return_val_if_fail (LP_IS_WORLD_SIMULATION (simulation), FALSE);

    klass = LP_EVENT_GET_CLASS (self);
    g_return_val_if_fail (klass->can_occur != NULL, FALSE);

    return klass->can_occur (self, simulation);
}

/* ==========================================================================
 * Event Lifecycle
 * ========================================================================== */

gboolean
lp_event_tick_year (LpEvent *self)
{
    LpEventPrivate *priv;

    g_return_val_if_fail (LP_IS_EVENT (self), FALSE);

    priv = lp_event_get_instance_private (self);

    if (!priv->is_active)
        return FALSE;

    /* Instant events are never "ticked" */
    if (priv->duration_years == 0)
        return FALSE;

    if (priv->years_remaining > 0)
        priv->years_remaining--;

    if (priv->years_remaining == 0)
    {
        lp_event_set_is_active (self, FALSE);
        g_signal_emit (self, signals[SIGNAL_RESOLVED], 0);

        lp_log_debug ("Event %s resolved (duration ended)", priv->name);
        return FALSE;
    }

    return TRUE;
}

void
lp_event_resolve (LpEvent     *self,
                  const gchar *choice_id)
{
    LpEventPrivate *priv;

    g_return_if_fail (LP_IS_EVENT (self));

    priv = lp_event_get_instance_private (self);

    if (!priv->is_active)
        return;

    if (choice_id != NULL)
    {
        g_signal_emit (self, signals[SIGNAL_CHOICE_MADE], 0, choice_id);
        lp_log_debug ("Event %s: choice made '%s'", priv->name, choice_id);
    }

    lp_event_set_is_active (self, FALSE);
    g_signal_emit (self, signals[SIGNAL_RESOLVED], 0);

    lp_log_debug ("Event %s resolved", priv->name);
}
