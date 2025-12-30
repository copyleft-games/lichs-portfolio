/* lp-agent-individual.c - Individual Mortal Agent
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_AGENT
#include "../lp-log.h"

#include "lp-agent-individual.h"
#include "lp-trait.h"
#include "../investment/lp-investment.h"

struct _LpAgentIndividual
{
    LpAgent parent_instance;

    LpAgentIndividual *successor;       /* Weak ref to avoid circular */
    gfloat             training_progress;
};

enum
{
    PROP_0,
    PROP_SUCCESSOR,
    PROP_TRAINING_PROGRESS,
    N_PROPS
};

enum
{
    SIGNAL_SUCCESSOR_TRAINED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for interface and virtual methods */
static void lp_agent_individual_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpAgentIndividual, lp_agent_individual, LP_TYPE_AGENT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_agent_individual_saveable_init))

/* ==========================================================================
 * Virtual Method Overrides
 * ========================================================================== */

static void
lp_agent_individual_on_year_passed (LpAgent *agent)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (agent);

    /* Chain up to parent implementation for aging, loyalty decay, etc. */
    LP_AGENT_CLASS (lp_agent_individual_parent_class)->on_year_passed (agent);

    /* Auto-train successor if we have one */
    if (self->successor != NULL && self->training_progress < 1.0f)
    {
        lp_agent_individual_train_successor (self, 1);
    }
}

static void
lp_agent_individual_on_death (LpAgent *agent)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (agent);

    lp_log_info ("Individual agent %s is dying, processing succession",
                 lp_agent_get_name (agent));

    /* Process succession before emitting death signal */
    lp_agent_individual_process_succession (self);

    /* Chain up to emit death signal */
    LP_AGENT_CLASS (lp_agent_individual_parent_class)->on_death (agent);
}

static gboolean
lp_agent_individual_can_recruit (LpAgent *agent)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (agent);

    /* Can't recruit if we already have a successor */
    if (self->successor != NULL)
        return FALSE;

    /* Chain up to check other requirements */
    return LP_AGENT_CLASS (lp_agent_individual_parent_class)->can_recruit (agent);
}

static LpAgentType
lp_agent_individual_get_agent_type (LpAgent *agent)
{
    (void)agent;
    return LP_AGENT_TYPE_INDIVIDUAL;
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_agent_individual_get_save_id (LrgSaveable *saveable)
{
    /* Delegate to parent */
    LrgSaveableInterface *parent_iface;

    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (saveable), LRG_TYPE_SAVEABLE));

    return parent_iface->get_save_id (saveable);
}

static gboolean
lp_agent_individual_save (LrgSaveable    *saveable,
                          LrgSaveContext *context,
                          GError        **error)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Call parent save first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (saveable), LRG_TYPE_SAVEABLE));

    if (!parent_iface->save (saveable, context, error))
        return FALSE;

    /* Save individual-specific properties */
    lrg_save_context_write_double (context, "training-progress", self->training_progress);

    /* Save successor reference by ID */
    if (self->successor != NULL)
    {
        lrg_save_context_write_string (context, "successor-id",
                                       lp_agent_get_id (LP_AGENT (self->successor)));
    }

    return TRUE;
}

static gboolean
lp_agent_individual_load (LrgSaveable    *saveable,
                          LrgSaveContext *context,
                          GError        **error)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Call parent load first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (saveable), LRG_TYPE_SAVEABLE));

    if (!parent_iface->load (saveable, context, error))
        return FALSE;

    /* Load individual-specific properties */
    self->training_progress = lrg_save_context_read_double (context, "training-progress", 0.0);

    /*
     * Note: Successor reference is resolved after all agents are loaded
     * by the agent manager using the successor-id field.
     */

    return TRUE;
}

static void
lp_agent_individual_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_agent_individual_get_save_id;
    iface->save = lp_agent_individual_save;
    iface->load = lp_agent_individual_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_agent_individual_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (object);

    switch (prop_id)
    {
    case PROP_SUCCESSOR:
        g_value_set_object (value, self->successor);
        break;

    case PROP_TRAINING_PROGRESS:
        g_value_set_float (value, self->training_progress);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_agent_individual_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (object);

    switch (prop_id)
    {
    case PROP_SUCCESSOR:
        lp_agent_individual_set_successor (self, g_value_get_object (value));
        break;

    case PROP_TRAINING_PROGRESS:
        lp_agent_individual_set_training_progress (self, g_value_get_float (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_agent_individual_dispose (GObject *object)
{
    LpAgentIndividual *self = LP_AGENT_INDIVIDUAL (object);

    /* Clear weak reference to successor */
    self->successor = NULL;

    G_OBJECT_CLASS (lp_agent_individual_parent_class)->dispose (object);
}

static void
lp_agent_individual_class_init (LpAgentIndividualClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpAgentClass *agent_class = LP_AGENT_CLASS (klass);

    object_class->get_property = lp_agent_individual_get_property;
    object_class->set_property = lp_agent_individual_set_property;
    object_class->dispose = lp_agent_individual_dispose;

    /* Override virtual methods */
    agent_class->on_year_passed = lp_agent_individual_on_year_passed;
    agent_class->on_death = lp_agent_individual_on_death;
    agent_class->can_recruit = lp_agent_individual_can_recruit;
    agent_class->get_agent_type = lp_agent_individual_get_agent_type;

    /**
     * LpAgentIndividual:successor:
     *
     * The designated successor agent.
     */
    properties[PROP_SUCCESSOR] =
        g_param_spec_object ("successor",
                             "Successor",
                             "Designated successor agent",
                             LP_TYPE_AGENT_INDIVIDUAL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpAgentIndividual:training-progress:
     *
     * Successor training progress (0.0-1.0).
     */
    properties[PROP_TRAINING_PROGRESS] =
        g_param_spec_float ("training-progress",
                            "Training Progress",
                            "Successor training progress",
                            0.0f, 1.0f, 0.0f,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpAgentIndividual::successor-trained:
     * @self: the #LpAgentIndividual
     * @successor: the trained #LpAgentIndividual
     *
     * Emitted when the successor is fully trained.
     */
    signals[SIGNAL_SUCCESSOR_TRAINED] =
        g_signal_new ("successor-trained",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      LP_TYPE_AGENT_INDIVIDUAL);
}

static void
lp_agent_individual_init (LpAgentIndividual *self)
{
    self->successor = NULL;
    self->training_progress = 0.0f;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpAgentIndividual *
lp_agent_individual_new (const gchar *id,
                         const gchar *name)
{
    return g_object_new (LP_TYPE_AGENT_INDIVIDUAL,
                         "id", id,
                         "name", name,
                         NULL);
}

LpAgentIndividual *
lp_agent_individual_new_full (const gchar *id,
                              const gchar *name,
                              guint        age,
                              guint        max_age,
                              gint         loyalty,
                              gint         competence)
{
    return g_object_new (LP_TYPE_AGENT_INDIVIDUAL,
                         "id", id,
                         "name", name,
                         "age", age,
                         "max-age", max_age,
                         "loyalty", loyalty,
                         "competence", competence,
                         NULL);
}

/* ==========================================================================
 * Successor Management
 * ========================================================================== */

LpAgentIndividual *
lp_agent_individual_get_successor (LpAgentIndividual *self)
{
    g_return_val_if_fail (LP_IS_AGENT_INDIVIDUAL (self), NULL);

    return self->successor;
}

void
lp_agent_individual_set_successor (LpAgentIndividual *self,
                                   LpAgentIndividual *successor)
{
    g_return_if_fail (LP_IS_AGENT_INDIVIDUAL (self));
    g_return_if_fail (successor == NULL || LP_IS_AGENT_INDIVIDUAL (successor));

    if (self->successor == successor)
        return;

    self->successor = successor;
    self->training_progress = 0.0f;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SUCCESSOR]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRAINING_PROGRESS]);

    if (successor != NULL)
    {
        lp_log_debug ("Agent %s now has successor %s",
                      lp_agent_get_name (LP_AGENT (self)),
                      lp_agent_get_name (LP_AGENT (successor)));
    }
}

gfloat
lp_agent_individual_get_training_progress (LpAgentIndividual *self)
{
    g_return_val_if_fail (LP_IS_AGENT_INDIVIDUAL (self), 0.0f);

    return self->training_progress;
}

void
lp_agent_individual_set_training_progress (LpAgentIndividual *self,
                                           gfloat             progress)
{
    gboolean was_trained;
    gboolean now_trained;

    g_return_if_fail (LP_IS_AGENT_INDIVIDUAL (self));

    progress = CLAMP (progress, 0.0f, 1.0f);

    if (self->training_progress == progress)
        return;

    was_trained = self->training_progress >= 1.0f;
    self->training_progress = progress;
    now_trained = self->training_progress >= 1.0f;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRAINING_PROGRESS]);

    /* Emit signal when training completes */
    if (!was_trained && now_trained && self->successor != NULL)
    {
        g_signal_emit (self, signals[SIGNAL_SUCCESSOR_TRAINED], 0, self->successor);
    }
}

gboolean
lp_agent_individual_has_trained_successor (LpAgentIndividual *self)
{
    g_return_val_if_fail (LP_IS_AGENT_INDIVIDUAL (self), FALSE);

    return (self->successor != NULL && self->training_progress >= 1.0f);
}

/* ==========================================================================
 * Training Methods
 * ========================================================================== */

void
lp_agent_individual_train_successor (LpAgentIndividual *self,
                                     guint              years)
{
    gint mentor_competence;
    gfloat progress_per_year;
    gfloat new_progress;

    g_return_if_fail (LP_IS_AGENT_INDIVIDUAL (self));

    if (self->successor == NULL)
    {
        lp_log_warning ("Cannot train: agent %s has no successor",
                        lp_agent_get_name (LP_AGENT (self)));
        return;
    }

    if (self->training_progress >= 1.0f)
        return;  /* Already trained */

    /*
     * Training progress per year based on mentor's competence:
     * - 0 competence: 5% per year (20 years to train)
     * - 50 competence: 10% per year (10 years to train)
     * - 100 competence: 20% per year (5 years to train)
     */
    mentor_competence = lp_agent_get_competence (LP_AGENT (self));
    progress_per_year = 0.05f + (mentor_competence / 100.0f) * 0.15f;

    new_progress = self->training_progress + (progress_per_year * years);
    lp_agent_individual_set_training_progress (self, new_progress);

    lp_log_debug ("Agent %s trained successor for %u years, progress: %.0f%%",
                  lp_agent_get_name (LP_AGENT (self)),
                  years,
                  self->training_progress * 100.0f);
}

LpAgentIndividual *
lp_agent_individual_recruit_successor (LpAgentIndividual *self)
{
    LpAgentIndividual *successor;
    g_autofree gchar *id = NULL;
    g_autofree gchar *name = NULL;
    guint age;
    guint max_age;
    gint base_loyalty;
    gint base_competence;

    g_return_val_if_fail (LP_IS_AGENT_INDIVIDUAL (self), NULL);

    if (!lp_agent_can_recruit (LP_AGENT (self)))
    {
        lp_log_warning ("Agent %s cannot recruit",
                        lp_agent_get_name (LP_AGENT (self)));
        return NULL;
    }

    /* Generate random successor */
    id = g_strdup_printf ("agent-%u", g_random_int ());
    name = g_strdup_printf ("Recruit of %s", lp_agent_get_name (LP_AGENT (self)));

    /* Random starting stats */
    age = g_random_int_range (18, 30);
    max_age = g_random_int_range (60, 85);
    base_loyalty = g_random_int_range (40, 70);
    base_competence = g_random_int_range (20, 50);

    successor = lp_agent_individual_new_full (id, name, age, max_age,
                                              base_loyalty, base_competence);

    lp_agent_individual_set_successor (self, successor);

    lp_log_info ("Agent %s recruited successor: %s (age %u, loyalty %d, competence %d)",
                 lp_agent_get_name (LP_AGENT (self)),
                 name, age, base_loyalty, base_competence);

    return successor;
}

/* ==========================================================================
 * Succession Methods
 * ========================================================================== */

LpAgentIndividual *
lp_agent_individual_process_succession (LpAgentIndividual *self)
{
    LpAgentIndividual *successor;
    gfloat skill_retention;
    gint parent_competence;
    gint transferred_competence;
    GPtrArray *investments;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_INDIVIDUAL (self), NULL);

    successor = self->successor;

    if (successor == NULL)
    {
        lp_log_warning ("Agent %s died with no successor",
                        lp_agent_get_name (LP_AGENT (self)));
        return NULL;
    }

    skill_retention = lp_agent_individual_get_skill_retention (self);
    parent_competence = lp_agent_get_competence (LP_AGENT (self));

    /*
     * Transfer competence based on training:
     * - Untrained: 25% of parent's competence
     * - Trained: 75% of parent's competence
     *
     * Successor gains the higher of their own competence or transferred.
     */
    transferred_competence = (gint)(parent_competence * skill_retention);
    transferred_competence = MAX (transferred_competence,
                                  lp_agent_get_competence (LP_AGENT (successor)));

    lp_agent_set_competence (LP_AGENT (successor), transferred_competence);

    /* Transfer investment assignments */
    investments = lp_agent_get_assigned_investments (LP_AGENT (self));

    for (i = 0; i < investments->len; i++)
    {
        LpInvestment *investment = g_ptr_array_index (investments, i);
        lp_agent_assign_investment (LP_AGENT (successor), investment);
    }

    /* Clear from parent */
    g_ptr_array_set_size (investments, 0);

    lp_log_info ("Succession: %s -> %s (%.0f%% skill retention, %d competence)",
                 lp_agent_get_name (LP_AGENT (self)),
                 lp_agent_get_name (LP_AGENT (successor)),
                 skill_retention * 100.0f,
                 transferred_competence);

    /* Clear successor reference */
    self->successor = NULL;

    return successor;
}

gfloat
lp_agent_individual_get_skill_retention (LpAgentIndividual *self)
{
    g_return_val_if_fail (LP_IS_AGENT_INDIVIDUAL (self), 0.25f);

    if (self->successor == NULL)
        return 0.25f;

    /*
     * Skill retention scales with training:
     * 0% training: 25%
     * 100% training: 75%
     */
    return 0.25f + (self->training_progress * 0.50f);
}
