/* lp-agent.c - Base Agent Class
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_AGENT
#include "../lp-log.h"

#include "lp-agent.h"
#include "lp-trait.h"
#include "../investment/lp-investment.h"

/* Private data for the base class */
typedef struct
{
    gchar            *id;
    gchar            *name;

    guint             age;
    guint             max_age;

    gint              loyalty;       /* 0-100 */
    gint              competence;    /* 0-100 */

    LpCoverStatus     cover_status;
    LpKnowledgeLevel  knowledge_level;

    GPtrArray        *traits;        /* Array of LpTrait* */
    GPtrArray        *assigned_investments;  /* Array of LpInvestment* (weak refs) */
} LpAgentPrivate;

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_AGE,
    PROP_MAX_AGE,
    PROP_LOYALTY,
    PROP_COMPETENCE,
    PROP_COVER_STATUS,
    PROP_KNOWLEDGE_LEVEL,
    N_PROPS
};

enum
{
    SIGNAL_DIED,
    SIGNAL_BETRAYED,
    SIGNAL_LOYALTY_CHANGED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_agent_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpAgent, lp_agent, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (LpAgent)
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_agent_saveable_init))

/* ==========================================================================
 * Default Virtual Method Implementations
 * ========================================================================== */

static void
lp_agent_real_on_year_passed (LpAgent *self)
{
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);
    gint old_loyalty;

    /* Age the agent */
    priv->age++;

    /* Check for death */
    if (priv->age >= priv->max_age)
    {
        lp_log_debug ("Agent %s has reached max age %u", priv->name, priv->max_age);
        lp_agent_on_death (self);
        return;
    }

    /* Loyalty decay - small chance of losing loyalty each year */
    old_loyalty = priv->loyalty;

    /*
     * Loyalty decays based on knowledge level:
     * - NONE: No decay (doesn't know enough to question)
     * - SUSPICIOUS: Small decay (starting to wonder)
     * - AWARE: Moderate decay (knows something supernatural)
     * - FULL: Larger decay (knows they serve undead)
     */
    if (priv->knowledge_level >= LP_KNOWLEDGE_LEVEL_SUSPICIOUS)
    {
        gint decay = 0;

        switch (priv->knowledge_level)
        {
        case LP_KNOWLEDGE_LEVEL_SUSPICIOUS:
            decay = (g_random_int_range (0, 100) < 10) ? 1 : 0;
            break;
        case LP_KNOWLEDGE_LEVEL_AWARE:
            decay = (g_random_int_range (0, 100) < 20) ? 1 : 0;
            break;
        case LP_KNOWLEDGE_LEVEL_FULL:
            decay = (g_random_int_range (0, 100) < 30) ? 1 : 0;
            break;
        default:
            break;
        }

        if (decay > 0)
        {
            priv->loyalty = MAX (0, priv->loyalty - decay);

            if (old_loyalty != priv->loyalty)
            {
                g_signal_emit (self, signals[SIGNAL_LOYALTY_CHANGED], 0,
                               old_loyalty, priv->loyalty);
            }
        }
    }

    /* Check for betrayal */
    if (lp_agent_roll_betrayal (self))
    {
        lp_agent_on_betrayal (self);
    }
}

static void
lp_agent_real_on_death (LpAgent *self)
{
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    lp_log_info ("Agent %s has died at age %u", priv->name, priv->age);

    /* Emit death signal */
    g_signal_emit (self, signals[SIGNAL_DIED], 0);
}

static void
lp_agent_real_on_betrayal (LpAgent *self)
{
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    lp_log_warning ("Agent %s has betrayed! Knowledge level: %d",
                    priv->name, priv->knowledge_level);

    /* Emit betrayal signal */
    g_signal_emit (self, signals[SIGNAL_BETRAYED], 0);
}

static gboolean
lp_agent_real_can_recruit (LpAgent *self)
{
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    /*
     * Default: Can recruit if:
     * - Loyalty >= 50
     * - Competence >= 30
     * - Cover is not exposed
     */
    if (priv->loyalty < 50)
        return FALSE;

    if (priv->competence < 30)
        return FALSE;

    if (priv->cover_status == LP_COVER_STATUS_EXPOSED)
        return FALSE;

    return TRUE;
}

static LpAgentType
lp_agent_real_get_agent_type (LpAgent *self)
{
    /* Base class defaults to individual */
    return LP_AGENT_TYPE_INDIVIDUAL;
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_agent_get_save_id (LrgSaveable *saveable)
{
    LpAgent *self = LP_AGENT (saveable);
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    return priv->id;
}

static gboolean
lp_agent_save (LrgSaveable    *saveable,
               LrgSaveContext *context,
               GError        **error)
{
    LpAgent *self = LP_AGENT (saveable);
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);
    guint i;

    /* Save basic properties */
    lrg_save_context_write_string (context, "id", priv->id);
    lrg_save_context_write_string (context, "name", priv->name);
    lrg_save_context_write_uint (context, "age", priv->age);
    lrg_save_context_write_uint (context, "max-age", priv->max_age);
    lrg_save_context_write_int (context, "loyalty", priv->loyalty);
    lrg_save_context_write_int (context, "competence", priv->competence);
    lrg_save_context_write_int (context, "cover-status", priv->cover_status);
    lrg_save_context_write_int (context, "knowledge-level", priv->knowledge_level);
    lrg_save_context_write_int (context, "agent-type", lp_agent_get_agent_type (self));

    /* Save traits */
    lrg_save_context_write_uint (context, "trait-count", priv->traits->len);

    for (i = 0; i < priv->traits->len; i++)
    {
        LpTrait *trait = g_ptr_array_index (priv->traits, i);
        g_autofree gchar *key = g_strdup_printf ("trait-%u", i);

        lrg_save_context_write_string (context, key, lp_trait_get_id (trait));
    }

    return TRUE;
}

static gboolean
lp_agent_load (LrgSaveable    *saveable,
               LrgSaveContext *context,
               GError        **error)
{
    LpAgent *self = LP_AGENT (saveable);
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    /* Load basic properties */
    g_clear_pointer (&priv->id, g_free);
    priv->id = g_strdup (lrg_save_context_read_string (context, "id", "unknown"));

    g_clear_pointer (&priv->name, g_free);
    priv->name = g_strdup (lrg_save_context_read_string (context, "name", "Unknown Agent"));

    priv->age = lrg_save_context_read_uint (context, "age", 25);
    priv->max_age = lrg_save_context_read_uint (context, "max-age", 70);
    priv->loyalty = lrg_save_context_read_int (context, "loyalty", 50);
    priv->competence = lrg_save_context_read_int (context, "competence", 50);
    priv->cover_status = lrg_save_context_read_int (context, "cover-status", LP_COVER_STATUS_SECURE);
    priv->knowledge_level = lrg_save_context_read_int (context, "knowledge-level", LP_KNOWLEDGE_LEVEL_NONE);

    /*
     * Note: Traits are loaded separately by the subclass or manager,
     * as they need to be looked up from the trait registry.
     */

    lp_log_debug ("Loaded agent: %s (%s)", priv->name, priv->id);

    return TRUE;
}

static void
lp_agent_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_agent_get_save_id;
    iface->save = lp_agent_save;
    iface->load = lp_agent_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_agent_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    LpAgent *self = LP_AGENT (object);
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ID:
        g_value_set_string (value, priv->id);
        break;

    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_AGE:
        g_value_set_uint (value, priv->age);
        break;

    case PROP_MAX_AGE:
        g_value_set_uint (value, priv->max_age);
        break;

    case PROP_LOYALTY:
        g_value_set_int (value, priv->loyalty);
        break;

    case PROP_COMPETENCE:
        g_value_set_int (value, priv->competence);
        break;

    case PROP_COVER_STATUS:
        g_value_set_enum (value, priv->cover_status);
        break;

    case PROP_KNOWLEDGE_LEVEL:
        g_value_set_enum (value, priv->knowledge_level);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_agent_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    LpAgent *self = LP_AGENT (object);
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ID:
        g_clear_pointer (&priv->id, g_free);
        priv->id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        lp_agent_set_name (self, g_value_get_string (value));
        break;

    case PROP_AGE:
        lp_agent_set_age (self, g_value_get_uint (value));
        break;

    case PROP_MAX_AGE:
        lp_agent_set_max_age (self, g_value_get_uint (value));
        break;

    case PROP_LOYALTY:
        lp_agent_set_loyalty (self, g_value_get_int (value));
        break;

    case PROP_COMPETENCE:
        lp_agent_set_competence (self, g_value_get_int (value));
        break;

    case PROP_COVER_STATUS:
        lp_agent_set_cover_status (self, g_value_get_enum (value));
        break;

    case PROP_KNOWLEDGE_LEVEL:
        lp_agent_set_knowledge_level (self, g_value_get_enum (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_agent_finalize (GObject *object)
{
    LpAgent *self = LP_AGENT (object);
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    lp_log_debug ("Finalizing agent: %s", priv->id ? priv->id : "(unknown)");

    g_clear_pointer (&priv->id, g_free);
    g_clear_pointer (&priv->name, g_free);
    g_clear_pointer (&priv->traits, g_ptr_array_unref);
    g_clear_pointer (&priv->assigned_investments, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_agent_parent_class)->finalize (object);
}

static void
lp_agent_class_init (LpAgentClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_agent_get_property;
    object_class->set_property = lp_agent_set_property;
    object_class->finalize = lp_agent_finalize;

    /* Default virtual method implementations */
    klass->on_year_passed = lp_agent_real_on_year_passed;
    klass->on_death = lp_agent_real_on_death;
    klass->on_betrayal = lp_agent_real_on_betrayal;
    klass->can_recruit = lp_agent_real_can_recruit;
    klass->get_agent_type = lp_agent_real_get_agent_type;

    /**
     * LpAgent:id:
     *
     * Unique identifier for this agent.
     */
    properties[PROP_ID] =
        g_param_spec_string ("id",
                             "ID",
                             "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpAgent:name:
     *
     * Display name.
     */
    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Display name",
                             "Unknown Agent",
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpAgent:age:
     *
     * Current age in years.
     */
    properties[PROP_AGE] =
        g_param_spec_uint ("age",
                           "Age",
                           "Current age in years",
                           0, G_MAXUINT, 25,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpAgent:max-age:
     *
     * Maximum lifespan in years.
     */
    properties[PROP_MAX_AGE] =
        g_param_spec_uint ("max-age",
                           "Maximum Age",
                           "Maximum lifespan in years",
                           1, G_MAXUINT, 70,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpAgent:loyalty:
     *
     * Loyalty to the lich (0-100).
     */
    properties[PROP_LOYALTY] =
        g_param_spec_int ("loyalty",
                          "Loyalty",
                          "Loyalty score (0-100)",
                          0, 100, 50,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpAgent:competence:
     *
     * Skill level (0-100).
     */
    properties[PROP_COMPETENCE] =
        g_param_spec_int ("competence",
                          "Competence",
                          "Skill level (0-100)",
                          0, 100, 50,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpAgent:cover-status:
     *
     * Cover identity status.
     */
    properties[PROP_COVER_STATUS] =
        g_param_spec_enum ("cover-status",
                           "Cover Status",
                           "Cover identity status",
                           LP_TYPE_COVER_STATUS,
                           LP_COVER_STATUS_SECURE,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpAgent:knowledge-level:
     *
     * Knowledge of true master.
     */
    properties[PROP_KNOWLEDGE_LEVEL] =
        g_param_spec_enum ("knowledge-level",
                           "Knowledge Level",
                           "Knowledge of true master",
                           LP_TYPE_KNOWLEDGE_LEVEL,
                           LP_KNOWLEDGE_LEVEL_NONE,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpAgent::died:
     * @self: the #LpAgent
     *
     * Emitted when the agent dies.
     */
    signals[SIGNAL_DIED] =
        g_signal_new ("died",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpAgent::betrayed:
     * @self: the #LpAgent
     *
     * Emitted when the agent betrays.
     */
    signals[SIGNAL_BETRAYED] =
        g_signal_new ("betrayed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpAgent::loyalty-changed:
     * @self: the #LpAgent
     * @old_loyalty: previous loyalty value
     * @new_loyalty: new loyalty value
     *
     * Emitted when loyalty changes.
     */
    signals[SIGNAL_LOYALTY_CHANGED] =
        g_signal_new ("loyalty-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_INT,
                      G_TYPE_INT);
}

static void
lp_agent_init (LpAgent *self)
{
    LpAgentPrivate *priv = lp_agent_get_instance_private (self);

    priv->id = NULL;
    priv->name = g_strdup ("Unknown Agent");
    priv->age = 25;
    priv->max_age = 70;
    priv->loyalty = 50;
    priv->competence = 50;
    priv->cover_status = LP_COVER_STATUS_SECURE;
    priv->knowledge_level = LP_KNOWLEDGE_LEVEL_NONE;
    priv->traits = g_ptr_array_new_with_free_func (g_object_unref);
    priv->assigned_investments = g_ptr_array_new ();  /* Weak refs, no free func */
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_agent_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new agent.
 *
 * Returns: (transfer full): A new #LpAgent
 */
LpAgent *
lp_agent_new (const gchar *id,
              const gchar *name)
{
    return g_object_new (LP_TYPE_AGENT,
                         "id", id,
                         "name", name,
                         NULL);
}

/* ==========================================================================
 * Property Getters/Setters
 * ========================================================================== */

const gchar *
lp_agent_get_id (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), NULL);

    priv = lp_agent_get_instance_private (self);
    return priv->id;
}

const gchar *
lp_agent_get_name (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), NULL);

    priv = lp_agent_get_instance_private (self);
    return priv->name;
}

void
lp_agent_set_name (LpAgent     *self,
                   const gchar *name)
{
    LpAgentPrivate *priv;

    g_return_if_fail (LP_IS_AGENT (self));

    priv = lp_agent_get_instance_private (self);

    if (g_strcmp0 (priv->name, name) == 0)
        return;

    g_clear_pointer (&priv->name, g_free);
    priv->name = g_strdup (name);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}

guint
lp_agent_get_age (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), 0);

    priv = lp_agent_get_instance_private (self);
    return priv->age;
}

void
lp_agent_set_age (LpAgent *self,
                  guint    age)
{
    LpAgentPrivate *priv;

    g_return_if_fail (LP_IS_AGENT (self));

    priv = lp_agent_get_instance_private (self);

    if (priv->age == age)
        return;

    priv->age = age;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AGE]);
}

guint
lp_agent_get_max_age (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), 70);

    priv = lp_agent_get_instance_private (self);
    return priv->max_age;
}

void
lp_agent_set_max_age (LpAgent *self,
                      guint    max_age)
{
    LpAgentPrivate *priv;

    g_return_if_fail (LP_IS_AGENT (self));
    g_return_if_fail (max_age > 0);

    priv = lp_agent_get_instance_private (self);

    if (priv->max_age == max_age)
        return;

    priv->max_age = max_age;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MAX_AGE]);
}

gint
lp_agent_get_loyalty (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), 0);

    priv = lp_agent_get_instance_private (self);
    return priv->loyalty;
}

void
lp_agent_set_loyalty (LpAgent *self,
                      gint     loyalty)
{
    LpAgentPrivate *priv;
    gint old_loyalty;

    g_return_if_fail (LP_IS_AGENT (self));

    priv = lp_agent_get_instance_private (self);
    old_loyalty = priv->loyalty;

    /* Clamp to 0-100 */
    loyalty = CLAMP (loyalty, 0, 100);

    if (priv->loyalty == loyalty)
        return;

    priv->loyalty = loyalty;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOYALTY]);
    g_signal_emit (self, signals[SIGNAL_LOYALTY_CHANGED], 0, old_loyalty, priv->loyalty);
}

gint
lp_agent_get_competence (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), 0);

    priv = lp_agent_get_instance_private (self);
    return priv->competence;
}

void
lp_agent_set_competence (LpAgent *self,
                         gint     competence)
{
    LpAgentPrivate *priv;

    g_return_if_fail (LP_IS_AGENT (self));

    priv = lp_agent_get_instance_private (self);

    /* Clamp to 0-100 */
    competence = CLAMP (competence, 0, 100);

    if (priv->competence == competence)
        return;

    priv->competence = competence;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMPETENCE]);
}

LpCoverStatus
lp_agent_get_cover_status (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), LP_COVER_STATUS_SECURE);

    priv = lp_agent_get_instance_private (self);
    return priv->cover_status;
}

void
lp_agent_set_cover_status (LpAgent       *self,
                           LpCoverStatus  status)
{
    LpAgentPrivate *priv;

    g_return_if_fail (LP_IS_AGENT (self));

    priv = lp_agent_get_instance_private (self);

    if (priv->cover_status == status)
        return;

    priv->cover_status = status;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COVER_STATUS]);
}

LpKnowledgeLevel
lp_agent_get_knowledge_level (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), LP_KNOWLEDGE_LEVEL_NONE);

    priv = lp_agent_get_instance_private (self);
    return priv->knowledge_level;
}

void
lp_agent_set_knowledge_level (LpAgent          *self,
                              LpKnowledgeLevel  level)
{
    LpAgentPrivate *priv;

    g_return_if_fail (LP_IS_AGENT (self));

    priv = lp_agent_get_instance_private (self);

    if (priv->knowledge_level == level)
        return;

    priv->knowledge_level = level;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_KNOWLEDGE_LEVEL]);
}

GPtrArray *
lp_agent_get_traits (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), NULL);

    priv = lp_agent_get_instance_private (self);
    return priv->traits;
}

void
lp_agent_add_trait (LpAgent *self,
                    LpTrait *trait)
{
    LpAgentPrivate *priv;

    g_return_if_fail (LP_IS_AGENT (self));
    g_return_if_fail (LP_IS_TRAIT (trait));

    priv = lp_agent_get_instance_private (self);

    /* Check if already has this trait */
    if (lp_agent_has_trait (self, lp_trait_get_id (trait)))
        return;

    g_ptr_array_add (priv->traits, g_object_ref (trait));

    lp_log_debug ("Added trait %s to agent %s",
                  lp_trait_get_id (trait), priv->name);
}

gboolean
lp_agent_remove_trait (LpAgent *self,
                       LpTrait *trait)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), FALSE);
    g_return_val_if_fail (LP_IS_TRAIT (trait), FALSE);

    priv = lp_agent_get_instance_private (self);

    return g_ptr_array_remove (priv->traits, trait);
}

gboolean
lp_agent_has_trait (LpAgent     *self,
                    const gchar *trait_id)
{
    LpAgentPrivate *priv;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT (self), FALSE);
    g_return_val_if_fail (trait_id != NULL, FALSE);

    priv = lp_agent_get_instance_private (self);

    for (i = 0; i < priv->traits->len; i++)
    {
        LpTrait *trait = g_ptr_array_index (priv->traits, i);

        if (g_strcmp0 (lp_trait_get_id (trait), trait_id) == 0)
            return TRUE;
    }

    return FALSE;
}

GPtrArray *
lp_agent_get_assigned_investments (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), NULL);

    priv = lp_agent_get_instance_private (self);
    return priv->assigned_investments;
}

void
lp_agent_assign_investment (LpAgent      *self,
                            LpInvestment *investment)
{
    LpAgentPrivate *priv;
    guint i;

    g_return_if_fail (LP_IS_AGENT (self));
    g_return_if_fail (LP_IS_INVESTMENT (investment));

    priv = lp_agent_get_instance_private (self);

    /* Check if already assigned */
    for (i = 0; i < priv->assigned_investments->len; i++)
    {
        if (g_ptr_array_index (priv->assigned_investments, i) == investment)
            return;
    }

    g_ptr_array_add (priv->assigned_investments, investment);

    lp_log_debug ("Assigned investment %s to agent %s",
                  lp_investment_get_id (investment), priv->name);
}

gboolean
lp_agent_unassign_investment (LpAgent      *self,
                              LpInvestment *investment)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), FALSE);
    g_return_val_if_fail (LP_IS_INVESTMENT (investment), FALSE);

    priv = lp_agent_get_instance_private (self);

    return g_ptr_array_remove (priv->assigned_investments, investment);
}

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

void
lp_agent_on_year_passed (LpAgent *self)
{
    LpAgentClass *klass;

    g_return_if_fail (LP_IS_AGENT (self));

    klass = LP_AGENT_GET_CLASS (self);
    if (klass->on_year_passed != NULL)
        klass->on_year_passed (self);
}

void
lp_agent_on_death (LpAgent *self)
{
    LpAgentClass *klass;

    g_return_if_fail (LP_IS_AGENT (self));

    klass = LP_AGENT_GET_CLASS (self);
    if (klass->on_death != NULL)
        klass->on_death (self);
}

void
lp_agent_on_betrayal (LpAgent *self)
{
    LpAgentClass *klass;

    g_return_if_fail (LP_IS_AGENT (self));

    klass = LP_AGENT_GET_CLASS (self);
    if (klass->on_betrayal != NULL)
        klass->on_betrayal (self);
}

gboolean
lp_agent_can_recruit (LpAgent *self)
{
    LpAgentClass *klass;

    g_return_val_if_fail (LP_IS_AGENT (self), FALSE);

    klass = LP_AGENT_GET_CLASS (self);
    g_return_val_if_fail (klass->can_recruit != NULL, FALSE);

    return klass->can_recruit (self);
}

LpAgentType
lp_agent_get_agent_type (LpAgent *self)
{
    LpAgentClass *klass;

    g_return_val_if_fail (LP_IS_AGENT (self), LP_AGENT_TYPE_INDIVIDUAL);

    klass = LP_AGENT_GET_CLASS (self);
    g_return_val_if_fail (klass->get_agent_type != NULL, LP_AGENT_TYPE_INDIVIDUAL);

    return klass->get_agent_type (self);
}

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

gboolean
lp_agent_is_alive (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), FALSE);

    priv = lp_agent_get_instance_private (self);
    return priv->age < priv->max_age;
}

guint
lp_agent_get_years_remaining (LpAgent *self)
{
    LpAgentPrivate *priv;

    g_return_val_if_fail (LP_IS_AGENT (self), 0);

    priv = lp_agent_get_instance_private (self);

    if (priv->age >= priv->max_age)
        return 0;

    return priv->max_age - priv->age;
}

gdouble
lp_agent_get_income_modifier (LpAgent *self)
{
    LpAgentPrivate *priv;
    gdouble modifier;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT (self), 1.0);

    priv = lp_agent_get_instance_private (self);

    /*
     * Base modifier from competence:
     * 0 competence = 0.5x income
     * 50 competence = 1.0x income
     * 100 competence = 1.5x income
     */
    modifier = 0.5 + (priv->competence / 100.0);

    /* Apply trait modifiers */
    for (i = 0; i < priv->traits->len; i++)
    {
        LpTrait *trait = g_ptr_array_index (priv->traits, i);
        modifier *= lp_trait_get_income_modifier (trait);
    }

    return modifier;
}

guint
lp_agent_get_exposure_contribution (LpAgent *self)
{
    LpAgentPrivate *priv;
    guint exposure;

    g_return_val_if_fail (LP_IS_AGENT (self), 0);

    priv = lp_agent_get_instance_private (self);
    exposure = 0;

    /* Cover status contribution */
    switch (priv->cover_status)
    {
    case LP_COVER_STATUS_SECURE:
        exposure = 0;
        break;
    case LP_COVER_STATUS_SUSPICIOUS:
        exposure = 2;
        break;
    case LP_COVER_STATUS_COMPROMISED:
        exposure = 5;
        break;
    case LP_COVER_STATUS_EXPOSED:
        exposure = 10;
        break;
    }

    /* Knowledge level multiplier */
    switch (priv->knowledge_level)
    {
    case LP_KNOWLEDGE_LEVEL_NONE:
        /* No multiplier - doesn't know enough to expose anything */
        break;
    case LP_KNOWLEDGE_LEVEL_SUSPICIOUS:
        exposure = (guint)(exposure * 1.5);
        break;
    case LP_KNOWLEDGE_LEVEL_AWARE:
        exposure *= 2;
        break;
    case LP_KNOWLEDGE_LEVEL_FULL:
        exposure *= 3;
        break;
    }

    return exposure;
}

gboolean
lp_agent_roll_betrayal (LpAgent *self)
{
    LpAgentPrivate *priv;
    gint betrayal_chance;

    g_return_val_if_fail (LP_IS_AGENT (self), FALSE);

    priv = lp_agent_get_instance_private (self);

    /*
     * Betrayal chance formula:
     * Base: (100 - loyalty)%
     * Modified by knowledge level
     */
    betrayal_chance = 100 - priv->loyalty;

    /* Knowledge increases betrayal chance */
    switch (priv->knowledge_level)
    {
    case LP_KNOWLEDGE_LEVEL_NONE:
        /* Very low chance - doesn't know enough to betray */
        betrayal_chance /= 10;
        break;
    case LP_KNOWLEDGE_LEVEL_SUSPICIOUS:
        betrayal_chance /= 5;
        break;
    case LP_KNOWLEDGE_LEVEL_AWARE:
        betrayal_chance /= 2;
        break;
    case LP_KNOWLEDGE_LEVEL_FULL:
        /* Full chance - knows exactly what they serve */
        break;
    }

    /* Minimum 1% per year for agents with full knowledge and low loyalty */
    betrayal_chance = MAX (0, betrayal_chance);

    /* Cap at 25% per year to prevent constant betrayals */
    betrayal_chance = MIN (25, betrayal_chance);

    /* Roll the dice */
    return g_random_int_range (0, 100) < betrayal_chance;
}
