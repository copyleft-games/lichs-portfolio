/* lp-event-generator.c - Event Generator Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-event-generator.h"
#include "lp-event-economic.h"
#include "lp-event-political.h"
#include "lp-event-magical.h"
#include "lp-event-personal.h"
#include "lp-world-simulation.h"

struct _LpEventGenerator
{
    GObject parent_instance;

    gdouble base_yearly_event_chance;
    gdouble base_decade_event_chance;
    gdouble base_era_event_chance;

    guint   event_counter;  /* For unique event IDs */
    GRand  *rng;
};

G_DEFINE_FINAL_TYPE (LpEventGenerator, lp_event_generator, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_BASE_YEARLY_EVENT_CHANCE,
    PROP_BASE_DECADE_EVENT_CHANCE,
    PROP_BASE_ERA_EVENT_CHANCE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* Singleton instance */
static LpEventGenerator *default_generator = NULL;

/*
 * Event template tables for generation
 */

typedef struct
{
    const gchar *name;
    const gchar *description;
    gdouble      market_modifier;
    gint         affected_class;  /* -1 for all */
} EconomicEventTemplate;

typedef struct
{
    const gchar *name;
    const gchar *description;
    gint         stability_impact;
    gboolean     causes_war;
} PoliticalEventTemplate;

typedef struct
{
    const gchar *name;
    const gchar *description;
    gint         exposure_impact;
    gboolean     affects_dark;
} MagicalEventTemplate;

typedef struct
{
    const gchar *name;
    const gchar *description;
    gboolean     is_betrayal;
    gboolean     is_death;
} PersonalEventTemplate;

/* Minor economic events */
static const EconomicEventTemplate economic_minor[] = {
    { "Trade Fair", "A regional trade fair boosts commerce", 1.05, LP_ASSET_CLASS_TRADE },
    { "Poor Harvest", "A below-average harvest affects food prices", 0.95, LP_ASSET_CLASS_PROPERTY },
    { "New Mine Discovery", "A new vein of ore is discovered", 1.08, -1 },
    { "Tax Increase", "Local taxes are raised slightly", 0.97, LP_ASSET_CLASS_PROPERTY },
};

/* Moderate economic events */
static const EconomicEventTemplate economic_moderate[] = {
    { "Trade Route Opens", "A new trade route brings prosperity", 1.15, LP_ASSET_CLASS_TRADE },
    { "Banking Crisis", "Several money lenders fail", 0.85, LP_ASSET_CLASS_FINANCIAL },
    { "Resource Boom", "Valuable resources flood the market", 1.20, -1 },
    { "Trade Embargo", "Political tensions disrupt trade", 0.80, LP_ASSET_CLASS_TRADE },
};

/* Major economic events */
static const EconomicEventTemplate economic_major[] = {
    { "Market Crash", "Financial markets collapse", 0.60, -1 },
    { "Golden Age", "Unprecedented prosperity sweeps the land", 1.40, -1 },
    { "Currency Devaluation", "The currency loses significant value", 0.70, LP_ASSET_CLASS_FINANCIAL },
    { "Discovery of New Lands", "New territories bring vast opportunity", 1.50, LP_ASSET_CLASS_TRADE },
};

/* Minor political events */
static const PoliticalEventTemplate political_minor[] = {
    { "Noble Scandal", "A minor noble is caught in scandal", -5, FALSE },
    { "Royal Proclamation", "The crown issues new edicts", 5, FALSE },
    { "Border Skirmish", "Minor conflict on the frontier", -10, FALSE },
    { "Diplomatic Visit", "Foreign dignitaries improve relations", 10, FALSE },
};

/* Moderate political events */
static const PoliticalEventTemplate political_moderate[] = {
    { "Succession Dispute", "Questions arise about the line of succession", -25, FALSE },
    { "Reform Movement", "Calls for change sweep the populace", -15, FALSE },
    { "Alliance Formed", "A powerful alliance is announced", 20, FALSE },
    { "Peasant Unrest", "The common folk grow restless", -20, FALSE },
};

/* Major political events */
static const PoliticalEventTemplate political_major[] = {
    { "Civil War", "The realm tears itself apart", -50, TRUE },
    { "Revolution", "The old order is overthrown", -60, TRUE },
    { "Conquest", "Foreign armies march on the capital", -40, TRUE },
    { "Golden Peace", "A century-long peace treaty is signed", 50, FALSE },
};

/* Minor magical events */
static const MagicalEventTemplate magical_minor[] = {
    { "Strange Lights", "Unusual lights seen in the sky", 5, FALSE },
    { "Witch Accusations", "Rumors of witchcraft spread", 10, FALSE },
    { "Blessed Harvest", "The harvest is miraculously bountiful", -5, FALSE },
    { "Cursed Well", "A village well turns bitter", 8, TRUE },
};

/* Moderate magical events */
static const MagicalEventTemplate magical_moderate[] = {
    { "Artifact Discovered", "An ancient artifact is unearthed", 20, TRUE },
    { "Magical Plague", "A mysterious illness spreads", 25, TRUE },
    { "Divine Vision", "A saint receives a holy vision", -15, FALSE },
    { "Demonic Sighting", "Reports of demon activity", 30, TRUE },
};

/* Major magical events */
static const MagicalEventTemplate magical_major[] = {
    { "The Veil Thins", "The barrier between worlds weakens", 50, TRUE },
    { "Divine Intervention", "The gods manifest their power", -40, FALSE },
    { "Magical Catastrophe", "A spell goes terribly wrong", 60, TRUE },
    { "Age of Miracles", "Magic becomes commonplace", 40, TRUE },
};

/* Minor personal events */
static const PersonalEventTemplate personal_minor[] = {
    { "Agent Illness", "One of your agents falls ill", FALSE, FALSE },
    { "Agent Promotion", "An agent gains influence", FALSE, FALSE },
    { "Family Dispute", "Quarrel among your servants", FALSE, FALSE },
    { "New Contact", "An agent makes a valuable connection", FALSE, FALSE },
};

/* Moderate personal events */
static const PersonalEventTemplate personal_moderate[] = {
    { "Agent Investigated", "Authorities take interest in an agent", FALSE, FALSE },
    { "Wavering Loyalty", "An agent questions their service", TRUE, FALSE },
    { "Agent Marriage", "An agent's family grows", FALSE, FALSE },
    { "Agent Accident", "Serious injury befalls an agent", FALSE, FALSE },
};

/* Major personal events */
static const PersonalEventTemplate personal_major[] = {
    { "Betrayal", "An agent reveals secrets to your enemies", TRUE, FALSE },
    { "Agent Death", "A valued servant meets their end", FALSE, TRUE },
    { "Inquisitor Interest", "Church investigators target your network", TRUE, FALSE },
    { "Martyr's End", "An agent dies protecting your secrets", TRUE, TRUE },
};

/*
 * Helper functions
 */

static gchar *
generate_event_id (LpEventGenerator *self,
                   const gchar      *prefix)
{
    self->event_counter++;
    return g_strdup_printf ("%s-%u-%u", prefix,
                            (guint)g_get_real_time (),
                            self->event_counter);
}

static gboolean
roll_chance (LpEventGenerator *self,
             gdouble           chance)
{
    return g_rand_double (self->rng) < chance;
}

static guint
pick_random_index (LpEventGenerator *self,
                   guint             count)
{
    return g_rand_int_range (self->rng, 0, (gint32)count);
}

/*
 * Event creation functions
 */

static LpEvent *
create_economic_from_template (LpEventGenerator           *self,
                               const EconomicEventTemplate *tmpl,
                               LpEventSeverity             severity)
{
    g_autofree gchar *id = generate_event_id (self, "econ");
    LpEventEconomic *event;

    event = lp_event_economic_new (id, tmpl->name);
    lp_event_set_description (LP_EVENT (event), tmpl->description);
    lp_event_set_severity (LP_EVENT (event), severity);
    lp_event_economic_set_market_modifier (event, tmpl->market_modifier);
    lp_event_economic_set_affected_asset_class (event, tmpl->affected_class);

    return LP_EVENT (event);
}

static LpEvent *
create_political_from_template (LpEventGenerator            *self,
                                const PoliticalEventTemplate *tmpl,
                                LpEventSeverity              severity)
{
    g_autofree gchar *id = generate_event_id (self, "poli");
    LpEventPolitical *event;

    event = lp_event_political_new (id, tmpl->name);
    lp_event_set_description (LP_EVENT (event), tmpl->description);
    lp_event_set_severity (LP_EVENT (event), severity);
    lp_event_political_set_stability_impact (event, tmpl->stability_impact);
    lp_event_political_set_causes_war (event, tmpl->causes_war);

    return LP_EVENT (event);
}

static LpEvent *
create_magical_from_template (LpEventGenerator          *self,
                              const MagicalEventTemplate *tmpl,
                              LpEventSeverity            severity)
{
    g_autofree gchar *id = generate_event_id (self, "magi");
    LpEventMagical *event;

    event = lp_event_magical_new (id, tmpl->name);
    lp_event_set_description (LP_EVENT (event), tmpl->description);
    lp_event_set_severity (LP_EVENT (event), severity);
    lp_event_magical_set_exposure_impact (event, tmpl->exposure_impact);
    lp_event_magical_set_affects_dark_investments (event, tmpl->affects_dark);

    return LP_EVENT (event);
}

static LpEvent *
create_personal_from_template (LpEventGenerator           *self,
                               const PersonalEventTemplate *tmpl,
                               LpEventSeverity             severity)
{
    g_autofree gchar *id = generate_event_id (self, "pers");
    LpEventPersonal *event;

    event = lp_event_personal_new (id, tmpl->name);
    lp_event_set_description (LP_EVENT (event), tmpl->description);
    lp_event_set_severity (LP_EVENT (event), severity);
    lp_event_personal_set_is_betrayal (event, tmpl->is_betrayal);
    lp_event_personal_set_is_death (event, tmpl->is_death);

    return LP_EVENT (event);
}

/*
 * GObject implementation
 */

static void
lp_event_generator_finalize (GObject *object)
{
    LpEventGenerator *self = LP_EVENT_GENERATOR (object);

    g_rand_free (self->rng);

    G_OBJECT_CLASS (lp_event_generator_parent_class)->finalize (object);
}

static void
lp_event_generator_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    LpEventGenerator *self = LP_EVENT_GENERATOR (object);

    switch (prop_id)
    {
    case PROP_BASE_YEARLY_EVENT_CHANCE:
        g_value_set_double (value, self->base_yearly_event_chance);
        break;

    case PROP_BASE_DECADE_EVENT_CHANCE:
        g_value_set_double (value, self->base_decade_event_chance);
        break;

    case PROP_BASE_ERA_EVENT_CHANCE:
        g_value_set_double (value, self->base_era_event_chance);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_generator_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    LpEventGenerator *self = LP_EVENT_GENERATOR (object);

    switch (prop_id)
    {
    case PROP_BASE_YEARLY_EVENT_CHANCE:
        lp_event_generator_set_base_yearly_event_chance (self, g_value_get_double (value));
        break;

    case PROP_BASE_DECADE_EVENT_CHANCE:
        lp_event_generator_set_base_decade_event_chance (self, g_value_get_double (value));
        break;

    case PROP_BASE_ERA_EVENT_CHANCE:
        lp_event_generator_set_base_era_event_chance (self, g_value_get_double (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_generator_class_init (LpEventGeneratorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_event_generator_finalize;
    object_class->get_property = lp_event_generator_get_property;
    object_class->set_property = lp_event_generator_set_property;

    /**
     * LpEventGenerator:base-yearly-event-chance:
     *
     * Base probability of a yearly event occurring.
     */
    properties[PROP_BASE_YEARLY_EVENT_CHANCE] =
        g_param_spec_double ("base-yearly-event-chance",
                             "Base Yearly Event Chance",
                             "Base probability of yearly events",
                             0.0, 1.0, 0.3,
                             G_PARAM_READWRITE |
                             G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpEventGenerator:base-decade-event-chance:
     *
     * Base probability of a decade event occurring.
     */
    properties[PROP_BASE_DECADE_EVENT_CHANCE] =
        g_param_spec_double ("base-decade-event-chance",
                             "Base Decade Event Chance",
                             "Base probability of decade events",
                             0.0, 1.0, 0.7,
                             G_PARAM_READWRITE |
                             G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpEventGenerator:base-era-event-chance:
     *
     * Base probability of an era event occurring.
     */
    properties[PROP_BASE_ERA_EVENT_CHANCE] =
        g_param_spec_double ("base-era-event-chance",
                             "Base Era Event Chance",
                             "Base probability of era events",
                             0.0, 1.0, 0.9,
                             G_PARAM_READWRITE |
                             G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_event_generator_init (LpEventGenerator *self)
{
    self->base_yearly_event_chance = 0.3;
    self->base_decade_event_chance = 0.7;
    self->base_era_event_chance = 0.9;
    self->event_counter = 0;
    self->rng = g_rand_new ();
}

/*
 * Public API
 */

/**
 * lp_event_generator_get_default:
 *
 * Gets the singleton event generator instance.
 *
 * Returns: (transfer none): The default #LpEventGenerator
 */
LpEventGenerator *
lp_event_generator_get_default (void)
{
    if (default_generator == NULL)
    {
        default_generator = g_object_new (LP_TYPE_EVENT_GENERATOR, NULL);
    }

    return default_generator;
}

gdouble
lp_event_generator_get_base_yearly_event_chance (LpEventGenerator *self)
{
    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), 0.3);

    return self->base_yearly_event_chance;
}

void
lp_event_generator_set_base_yearly_event_chance (LpEventGenerator *self,
                                                 gdouble           chance)
{
    g_return_if_fail (LP_IS_EVENT_GENERATOR (self));

    chance = CLAMP (chance, 0.0, 1.0);

    if (self->base_yearly_event_chance != chance)
    {
        self->base_yearly_event_chance = chance;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BASE_YEARLY_EVENT_CHANCE]);
    }
}

gdouble
lp_event_generator_get_base_decade_event_chance (LpEventGenerator *self)
{
    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), 0.7);

    return self->base_decade_event_chance;
}

void
lp_event_generator_set_base_decade_event_chance (LpEventGenerator *self,
                                                 gdouble           chance)
{
    g_return_if_fail (LP_IS_EVENT_GENERATOR (self));

    chance = CLAMP (chance, 0.0, 1.0);

    if (self->base_decade_event_chance != chance)
    {
        self->base_decade_event_chance = chance;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BASE_DECADE_EVENT_CHANCE]);
    }
}

gdouble
lp_event_generator_get_base_era_event_chance (LpEventGenerator *self)
{
    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), 0.9);

    return self->base_era_event_chance;
}

void
lp_event_generator_set_base_era_event_chance (LpEventGenerator *self,
                                              gdouble           chance)
{
    g_return_if_fail (LP_IS_EVENT_GENERATOR (self));

    chance = CLAMP (chance, 0.0, 1.0);

    if (self->base_era_event_chance != chance)
    {
        self->base_era_event_chance = chance;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BASE_ERA_EVENT_CHANCE]);
    }
}

/**
 * lp_event_generator_generate_yearly_events:
 * @self: an #LpEventGenerator
 * @sim: the #LpWorldSimulation for context
 *
 * Generates events for the current year.
 * Yearly events are typically minor or moderate severity.
 *
 * Returns: (transfer full) (element-type LpEvent): List of generated events
 */
GList *
lp_event_generator_generate_yearly_events (LpEventGenerator  *self,
                                           LpWorldSimulation *sim)
{
    GList *events = NULL;
    LpEventType event_type;
    LpEventSeverity severity;

    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), NULL);

    /* Check if we generate an event this year */
    if (!roll_chance (self, self->base_yearly_event_chance))
        return NULL;

    /* Randomly select event type with weighting */
    switch (pick_random_index (self, 4))
    {
    case 0:
        event_type = LP_EVENT_TYPE_ECONOMIC;
        break;
    case 1:
        event_type = LP_EVENT_TYPE_POLITICAL;
        break;
    case 2:
        event_type = LP_EVENT_TYPE_MAGICAL;
        break;
    default:
        event_type = LP_EVENT_TYPE_PERSONAL;
        break;
    }

    /* Yearly events are usually minor, occasionally moderate */
    severity = roll_chance (self, 0.75) ? LP_EVENT_SEVERITY_MINOR : LP_EVENT_SEVERITY_MODERATE;

    switch (event_type)
    {
    case LP_EVENT_TYPE_ECONOMIC:
        events = g_list_prepend (events,
            lp_event_generator_create_economic_event (self, severity));
        break;

    case LP_EVENT_TYPE_POLITICAL:
        events = g_list_prepend (events,
            lp_event_generator_create_political_event (self, severity));
        break;

    case LP_EVENT_TYPE_MAGICAL:
        events = g_list_prepend (events,
            lp_event_generator_create_magical_event (self, severity));
        break;

    case LP_EVENT_TYPE_PERSONAL:
        events = g_list_prepend (events,
            lp_event_generator_create_personal_event (self, severity));
        break;

    default:
        break;
    }

    return events;
}

/**
 * lp_event_generator_generate_decade_events:
 * @self: an #LpEventGenerator
 * @sim: the #LpWorldSimulation for context
 *
 * Generates events for the current decade.
 * Decade events are typically moderate or major.
 *
 * Returns: (transfer full) (element-type LpEvent): List of generated events
 */
GList *
lp_event_generator_generate_decade_events (LpEventGenerator  *self,
                                           LpWorldSimulation *sim)
{
    GList *events = NULL;
    LpEventSeverity severity;
    guint event_count;
    guint i;

    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), NULL);

    /* Check if we generate decade events */
    if (!roll_chance (self, self->base_decade_event_chance))
        return NULL;

    /* Decade events can be 1-2 events */
    event_count = roll_chance (self, 0.3) ? 2 : 1;

    for (i = 0; i < event_count; i++)
    {
        LpEventType event_type;

        /* Decade events are usually moderate, sometimes major */
        severity = roll_chance (self, 0.6) ? LP_EVENT_SEVERITY_MODERATE : LP_EVENT_SEVERITY_MAJOR;

        event_type = pick_random_index (self, 4);

        switch (event_type)
        {
        case LP_EVENT_TYPE_ECONOMIC:
            events = g_list_prepend (events,
                lp_event_generator_create_economic_event (self, severity));
            break;

        case LP_EVENT_TYPE_POLITICAL:
            events = g_list_prepend (events,
                lp_event_generator_create_political_event (self, severity));
            break;

        case LP_EVENT_TYPE_MAGICAL:
            events = g_list_prepend (events,
                lp_event_generator_create_magical_event (self, severity));
            break;

        case LP_EVENT_TYPE_PERSONAL:
            events = g_list_prepend (events,
                lp_event_generator_create_personal_event (self, severity));
            break;

        default:
            break;
        }
    }

    return events;
}

/**
 * lp_event_generator_generate_era_events:
 * @self: an #LpEventGenerator
 * @sim: the #LpWorldSimulation for context
 *
 * Generates events for the current era (century).
 * Era events are typically major or catastrophic.
 *
 * Returns: (transfer full) (element-type LpEvent): List of generated events
 */
GList *
lp_event_generator_generate_era_events (LpEventGenerator  *self,
                                        LpWorldSimulation *sim)
{
    GList *events = NULL;
    LpEventSeverity severity;

    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), NULL);

    /* Check if we generate era event */
    if (!roll_chance (self, self->base_era_event_chance))
        return NULL;

    /* Era events are usually major, sometimes catastrophic */
    severity = roll_chance (self, 0.7) ? LP_EVENT_SEVERITY_MAJOR : LP_EVENT_SEVERITY_CATASTROPHIC;

    /*
     * Era events are world-shaking - generate one significant event
     * that affects multiple domains
     */
    switch (pick_random_index (self, 3))
    {
    case 0:
        /* Political upheaval with economic consequences */
        events = g_list_prepend (events,
            lp_event_generator_create_political_event (self, severity));
        events = g_list_prepend (events,
            lp_event_generator_create_economic_event (self, LP_EVENT_SEVERITY_MODERATE));
        break;

    case 1:
        /* Magical cataclysm */
        events = g_list_prepend (events,
            lp_event_generator_create_magical_event (self, severity));
        break;

    default:
        /* Economic transformation */
        events = g_list_prepend (events,
            lp_event_generator_create_economic_event (self, severity));
        break;
    }

    return events;
}

/**
 * lp_event_generator_create_economic_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random economic event of the given severity.
 *
 * Returns: (transfer full): A new economic event
 */
LpEvent *
lp_event_generator_create_economic_event (LpEventGenerator *self,
                                          LpEventSeverity   severity)
{
    const EconomicEventTemplate *templates;
    guint count;
    guint idx;

    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), NULL);

    switch (severity)
    {
    case LP_EVENT_SEVERITY_MINOR:
        templates = economic_minor;
        count = G_N_ELEMENTS (economic_minor);
        break;

    case LP_EVENT_SEVERITY_MODERATE:
        templates = economic_moderate;
        count = G_N_ELEMENTS (economic_moderate);
        break;

    case LP_EVENT_SEVERITY_MAJOR:
    case LP_EVENT_SEVERITY_CATASTROPHIC:
        templates = economic_major;
        count = G_N_ELEMENTS (economic_major);
        break;

    default:
        templates = economic_minor;
        count = G_N_ELEMENTS (economic_minor);
        break;
    }

    idx = pick_random_index (self, count);
    return create_economic_from_template (self, &templates[idx], severity);
}

/**
 * lp_event_generator_create_political_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random political event of the given severity.
 *
 * Returns: (transfer full): A new political event
 */
LpEvent *
lp_event_generator_create_political_event (LpEventGenerator *self,
                                           LpEventSeverity   severity)
{
    const PoliticalEventTemplate *templates;
    guint count;
    guint idx;

    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), NULL);

    switch (severity)
    {
    case LP_EVENT_SEVERITY_MINOR:
        templates = political_minor;
        count = G_N_ELEMENTS (political_minor);
        break;

    case LP_EVENT_SEVERITY_MODERATE:
        templates = political_moderate;
        count = G_N_ELEMENTS (political_moderate);
        break;

    case LP_EVENT_SEVERITY_MAJOR:
    case LP_EVENT_SEVERITY_CATASTROPHIC:
        templates = political_major;
        count = G_N_ELEMENTS (political_major);
        break;

    default:
        templates = political_minor;
        count = G_N_ELEMENTS (political_minor);
        break;
    }

    idx = pick_random_index (self, count);
    return create_political_from_template (self, &templates[idx], severity);
}

/**
 * lp_event_generator_create_magical_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random magical event of the given severity.
 *
 * Returns: (transfer full): A new magical event
 */
LpEvent *
lp_event_generator_create_magical_event (LpEventGenerator *self,
                                         LpEventSeverity   severity)
{
    const MagicalEventTemplate *templates;
    guint count;
    guint idx;

    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), NULL);

    switch (severity)
    {
    case LP_EVENT_SEVERITY_MINOR:
        templates = magical_minor;
        count = G_N_ELEMENTS (magical_minor);
        break;

    case LP_EVENT_SEVERITY_MODERATE:
        templates = magical_moderate;
        count = G_N_ELEMENTS (magical_moderate);
        break;

    case LP_EVENT_SEVERITY_MAJOR:
    case LP_EVENT_SEVERITY_CATASTROPHIC:
        templates = magical_major;
        count = G_N_ELEMENTS (magical_major);
        break;

    default:
        templates = magical_minor;
        count = G_N_ELEMENTS (magical_minor);
        break;
    }

    idx = pick_random_index (self, count);
    return create_magical_from_template (self, &templates[idx], severity);
}

/**
 * lp_event_generator_create_personal_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random personal event of the given severity.
 *
 * Returns: (transfer full): A new personal event
 */
LpEvent *
lp_event_generator_create_personal_event (LpEventGenerator *self,
                                          LpEventSeverity   severity)
{
    const PersonalEventTemplate *templates;
    guint count;
    guint idx;

    g_return_val_if_fail (LP_IS_EVENT_GENERATOR (self), NULL);

    switch (severity)
    {
    case LP_EVENT_SEVERITY_MINOR:
        templates = personal_minor;
        count = G_N_ELEMENTS (personal_minor);
        break;

    case LP_EVENT_SEVERITY_MODERATE:
        templates = personal_moderate;
        count = G_N_ELEMENTS (personal_moderate);
        break;

    case LP_EVENT_SEVERITY_MAJOR:
    case LP_EVENT_SEVERITY_CATASTROPHIC:
        templates = personal_major;
        count = G_N_ELEMENTS (personal_major);
        break;

    default:
        templates = personal_minor;
        count = G_N_ELEMENTS (personal_minor);
        break;
    }

    idx = pick_random_index (self, count);
    return create_personal_from_template (self, &templates[idx], severity);
}
