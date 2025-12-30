/* lp-event-personal.c - Personal Event Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-event-personal.h"
#include "../investment/lp-investment.h"
#include "lp-world-simulation.h"
#include <libregnum.h>

struct _LpEventPersonal
{
    LpEvent  parent_instance;

    gchar   *target_agent_id;
    gboolean is_betrayal;
    gboolean is_death;
};

static void lp_event_personal_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (LpEventPersonal, lp_event_personal, LP_TYPE_EVENT,
                               G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                      lp_event_personal_saveable_init))

enum
{
    PROP_0,
    PROP_TARGET_AGENT_ID,
    PROP_IS_BETRAYAL,
    PROP_IS_DEATH,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Virtual method implementations
 */

static void
lp_event_personal_apply_effects (LpEvent           *event,
                                 LpWorldSimulation *sim)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (event);

    g_return_if_fail (LP_IS_WORLD_SIMULATION (sim));

    /*
     * Personal events affect individual agents.
     * Deaths remove agents from the pool.
     * Betrayals can expose the lich or damage investments.
     */
    if (self->target_agent_id != NULL)
    {
        g_debug ("Personal event '%s' targeting agent '%s'",
                 lp_event_get_name (event), self->target_agent_id);

        if (self->is_death)
        {
            g_debug ("Agent '%s' has died", self->target_agent_id);
            /* Actual agent removal would happen via AgentManager */
        }

        if (self->is_betrayal)
        {
            g_debug ("Agent '%s' has betrayed the master", self->target_agent_id);
            /* Betrayal consequences applied via ExposureManager */
        }
    }
}

static GPtrArray *
lp_event_personal_get_choices (LpEvent *event)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (event);
    GPtrArray *choices;
    LpEventChoice *choice;

    /*
     * Personal events often present choices.
     * Betrayals can be handled through punishment or forgiveness.
     * Deaths may allow resurrection attempts.
     */
    if (!self->is_betrayal && !self->is_death)
        return NULL;

    choices = g_ptr_array_new_with_free_func ((GDestroyNotify)lp_event_choice_free);

    if (self->is_betrayal)
    {
        choice = lp_event_choice_new ("punish", "Make an example of the traitor");
        choice->consequence = g_strdup ("The traitor is destroyed. Other agents take note.");
        g_ptr_array_add (choices, choice);

        choice = lp_event_choice_new ("forgive", "Show unexpected mercy");
        choice->consequence = g_strdup ("The agent's loyalty wavers. Some see wisdom, others weakness.");
        g_ptr_array_add (choices, choice);

        choice = lp_event_choice_new ("turn", "Bind them more tightly to your will");
        choice->consequence = g_strdup ("Dark magic ensures future loyalty, but at great cost.");
        choice->requires_gold = TRUE;
        choice->gold_cost = 10000;
        g_ptr_array_add (choices, choice);
    }

    if (self->is_death && !self->is_betrayal)
    {
        choice = lp_event_choice_new ("accept", "Accept the natural order");
        choice->consequence = g_strdup ("The agent passes. Their knowledge is lost.");
        g_ptr_array_add (choices, choice);

        choice = lp_event_choice_new ("raise", "Raise them from death");
        choice->consequence = g_strdup ("The agent returns, changed. Exposure increases significantly.");
        choice->requires_gold = TRUE;
        choice->gold_cost = 50000;
        g_ptr_array_add (choices, choice);
    }

    return choices;
}

static gchar *
lp_event_personal_get_narrative_text (LpEvent *event)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (event);
    const gchar *name;
    const gchar *description;
    const gchar *personal_note;

    name = lp_event_get_name (event);
    description = lp_event_get_description (event);

    if (self->is_death && self->is_betrayal)
        personal_note = "Treachery and death intertwine - a fitting end for the disloyal";
    else if (self->is_death)
        personal_note = "The mortal coil releases another servant";
    else if (self->is_betrayal)
        personal_note = "Trust, once broken, demands response";
    else
        personal_note = "The affairs of mortals demand attention";

    if (self->target_agent_id != NULL)
    {
        return g_strdup_printf ("%s\n\n%s\n\n%s\n\n[Involves: %s]",
                                name ? name : "Personal Event",
                                description ? description : "",
                                personal_note,
                                self->target_agent_id);
    }

    return g_strdup_printf ("%s\n\n%s\n\n%s",
                            name ? name : "Personal Event",
                            description ? description : "",
                            personal_note);
}

/*
 * LrgSaveable interface
 */

static const gchar *
lp_event_personal_get_save_id (LrgSaveable *saveable)
{
    return lp_event_get_id (LP_EVENT (saveable));
}

static gboolean
lp_event_personal_save (LrgSaveable    *saveable,
                        LrgSaveContext *ctx,
                        GError        **error)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Save parent class data first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->save != NULL)
    {
        if (!parent_iface->save (saveable, ctx, error))
            return FALSE;
    }

    /* Save personal-specific data */
    if (self->target_agent_id != NULL)
        lrg_save_context_write_string (ctx, "target-agent-id", self->target_agent_id);

    lrg_save_context_write_boolean (ctx, "is-betrayal", self->is_betrayal);
    lrg_save_context_write_boolean (ctx, "is-death", self->is_death);

    return TRUE;
}

static gboolean
lp_event_personal_load (LrgSaveable    *saveable,
                        LrgSaveContext *ctx,
                        GError        **error)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Load parent class data first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->load != NULL)
    {
        if (!parent_iface->load (saveable, ctx, error))
            return FALSE;
    }

    /* Load personal-specific data */
    g_clear_pointer (&self->target_agent_id, g_free);
    self->target_agent_id = lrg_save_context_read_string (ctx, "target-agent-id", NULL);
    self->is_betrayal = lrg_save_context_read_boolean (ctx, "is-betrayal", FALSE);
    self->is_death = lrg_save_context_read_boolean (ctx, "is-death", FALSE);

    return TRUE;
}

static void
lp_event_personal_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_event_personal_get_save_id;
    iface->save = lp_event_personal_save;
    iface->load = lp_event_personal_load;
}

/*
 * GObject implementation
 */

static void
lp_event_personal_finalize (GObject *object)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (object);

    g_clear_pointer (&self->target_agent_id, g_free);

    G_OBJECT_CLASS (lp_event_personal_parent_class)->finalize (object);
}

static void
lp_event_personal_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (object);

    switch (prop_id)
    {
    case PROP_TARGET_AGENT_ID:
        g_value_set_string (value, self->target_agent_id);
        break;

    case PROP_IS_BETRAYAL:
        g_value_set_boolean (value, self->is_betrayal);
        break;

    case PROP_IS_DEATH:
        g_value_set_boolean (value, self->is_death);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_personal_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    LpEventPersonal *self = LP_EVENT_PERSONAL (object);

    switch (prop_id)
    {
    case PROP_TARGET_AGENT_ID:
        lp_event_personal_set_target_agent_id (self, g_value_get_string (value));
        break;

    case PROP_IS_BETRAYAL:
        lp_event_personal_set_is_betrayal (self, g_value_get_boolean (value));
        break;

    case PROP_IS_DEATH:
        lp_event_personal_set_is_death (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_personal_class_init (LpEventPersonalClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpEventClass *event_class = LP_EVENT_CLASS (klass);

    object_class->finalize = lp_event_personal_finalize;
    object_class->get_property = lp_event_personal_get_property;
    object_class->set_property = lp_event_personal_set_property;

    /* Override virtual methods */
    event_class->apply_effects = lp_event_personal_apply_effects;
    event_class->get_choices = lp_event_personal_get_choices;
    event_class->get_narrative_text = lp_event_personal_get_narrative_text;

    /**
     * LpEventPersonal:target-agent-id:
     *
     * The ID of the agent targeted by this event.
     */
    properties[PROP_TARGET_AGENT_ID] =
        g_param_spec_string ("target-agent-id",
                             "Target Agent ID",
                             "ID of the targeted agent",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpEventPersonal:is-betrayal:
     *
     * Whether this event is a betrayal.
     */
    properties[PROP_IS_BETRAYAL] =
        g_param_spec_boolean ("is-betrayal",
                              "Is Betrayal",
                              "Whether this is a betrayal event",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    /**
     * LpEventPersonal:is-death:
     *
     * Whether this event is an agent death.
     */
    properties[PROP_IS_DEATH] =
        g_param_spec_boolean ("is-death",
                              "Is Death",
                              "Whether this is a death event",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_event_personal_init (LpEventPersonal *self)
{
    self->target_agent_id = NULL;
    self->is_betrayal = FALSE;
    self->is_death = FALSE;
}

/*
 * Public API
 */

/**
 * lp_event_personal_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new personal event.
 *
 * Returns: (transfer full): A new #LpEventPersonal
 */
LpEventPersonal *
lp_event_personal_new (const gchar *id,
                       const gchar *name)
{
    return g_object_new (LP_TYPE_EVENT_PERSONAL,
                         "id", id,
                         "name", name,
                         "event-type", LP_EVENT_TYPE_PERSONAL,
                         NULL);
}

/**
 * lp_event_personal_get_target_agent_id:
 * @self: an #LpEventPersonal
 *
 * Gets the ID of the targeted agent.
 *
 * Returns: (transfer none) (nullable): The agent ID
 */
const gchar *
lp_event_personal_get_target_agent_id (LpEventPersonal *self)
{
    g_return_val_if_fail (LP_IS_EVENT_PERSONAL (self), NULL);

    return self->target_agent_id;
}

/**
 * lp_event_personal_set_target_agent_id:
 * @self: an #LpEventPersonal
 * @agent_id: (nullable): the agent ID
 *
 * Sets the targeted agent.
 */
void
lp_event_personal_set_target_agent_id (LpEventPersonal *self,
                                       const gchar     *agent_id)
{
    g_return_if_fail (LP_IS_EVENT_PERSONAL (self));

    if (g_strcmp0 (self->target_agent_id, agent_id) != 0)
    {
        g_free (self->target_agent_id);
        self->target_agent_id = g_strdup (agent_id);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_AGENT_ID]);
    }
}

/**
 * lp_event_personal_get_is_betrayal:
 * @self: an #LpEventPersonal
 *
 * Gets whether this event is a betrayal.
 *
 * Returns: %TRUE if betrayal event
 */
gboolean
lp_event_personal_get_is_betrayal (LpEventPersonal *self)
{
    g_return_val_if_fail (LP_IS_EVENT_PERSONAL (self), FALSE);

    return self->is_betrayal;
}

/**
 * lp_event_personal_set_is_betrayal:
 * @self: an #LpEventPersonal
 * @is_betrayal: whether betrayal
 *
 * Sets whether this is a betrayal event.
 */
void
lp_event_personal_set_is_betrayal (LpEventPersonal *self,
                                   gboolean         is_betrayal)
{
    g_return_if_fail (LP_IS_EVENT_PERSONAL (self));

    is_betrayal = !!is_betrayal;

    if (self->is_betrayal != is_betrayal)
    {
        self->is_betrayal = is_betrayal;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_BETRAYAL]);
    }
}

/**
 * lp_event_personal_get_is_death:
 * @self: an #LpEventPersonal
 *
 * Gets whether this event is an agent death.
 *
 * Returns: %TRUE if death event
 */
gboolean
lp_event_personal_get_is_death (LpEventPersonal *self)
{
    g_return_val_if_fail (LP_IS_EVENT_PERSONAL (self), FALSE);

    return self->is_death;
}

/**
 * lp_event_personal_set_is_death:
 * @self: an #LpEventPersonal
 * @is_death: whether death
 *
 * Sets whether this is a death event.
 */
void
lp_event_personal_set_is_death (LpEventPersonal *self,
                                gboolean         is_death)
{
    g_return_if_fail (LP_IS_EVENT_PERSONAL (self));

    is_death = !!is_death;

    if (self->is_death != is_death)
    {
        self->is_death = is_death;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_DEATH]);
    }
}
