/* lp-trait.c - Bloodline Trait System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_AGENT
#include "../lp-log.h"

#include "lp-trait.h"
#include "lp-agent.h"

/* Private data for the base class */
typedef struct
{
    gchar     *id;
    gchar     *name;
    gchar     *description;

    gfloat     inheritance_chance;   /* 0.0-1.0 */
    gfloat     income_modifier;      /* 1.0 = no change */
    gint       loyalty_modifier;     /* Bonus/penalty */
    gfloat     discovery_modifier;   /* 1.0 = no change */

    GPtrArray *conflicts_with;       /* Array of trait ID strings */
} LpTraitPrivate;

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_DESCRIPTION,
    PROP_INHERITANCE_CHANCE,
    PROP_INCOME_MODIFIER,
    PROP_LOYALTY_MODIFIER,
    PROP_DISCOVERY_MODIFIER,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* Forward declarations for LrgSaveable interface */
static void lp_trait_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpTrait, lp_trait, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (LpTrait)
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_trait_saveable_init))

/* ==========================================================================
 * Default Virtual Method Implementations
 * ========================================================================== */

static void
lp_trait_real_apply_effects (LpTrait *self,
                             LpAgent *agent)
{
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);
    gint current_loyalty;

    /* Apply loyalty modifier */
    if (priv->loyalty_modifier != 0)
    {
        current_loyalty = lp_agent_get_loyalty (agent);
        lp_agent_set_loyalty (agent, current_loyalty + priv->loyalty_modifier);

        lp_log_debug ("Applied trait %s loyalty modifier (%+d) to agent",
                      priv->name, priv->loyalty_modifier);
    }

    /*
     * Note: Income and discovery modifiers are applied during calculations,
     * not stored as agent properties. See lp_agent_get_income_modifier().
     */
}

static gboolean
lp_trait_real_roll_inheritance (LpTrait *self,
                                guint    generation)
{
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);
    gfloat effective_chance;
    gfloat roll;

    /*
     * Inheritance chance increases slightly with each generation
     * as the trait becomes more "established" in the bloodline.
     * +2% per generation, capped at 95%.
     */
    effective_chance = priv->inheritance_chance + (generation * 0.02f);
    effective_chance = MIN (effective_chance, 0.95f);

    roll = g_random_double ();

    lp_log_debug ("Trait %s inheritance roll: %.2f < %.2f (gen %u) = %s",
                  priv->name, roll, effective_chance, generation,
                  roll < effective_chance ? "inherited" : "not inherited");

    return roll < effective_chance;
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_trait_get_save_id (LrgSaveable *saveable)
{
    LpTrait *self = LP_TRAIT (saveable);
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);

    return priv->id;
}

static gboolean
lp_trait_save (LrgSaveable    *saveable,
               LrgSaveContext *context,
               GError        **error)
{
    LpTrait *self = LP_TRAIT (saveable);
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);
    guint i;

    lrg_save_context_write_string (context, "id", priv->id);
    lrg_save_context_write_string (context, "name", priv->name);

    if (priv->description != NULL)
        lrg_save_context_write_string (context, "description", priv->description);

    lrg_save_context_write_double (context, "inheritance-chance", priv->inheritance_chance);
    lrg_save_context_write_double (context, "income-modifier", priv->income_modifier);
    lrg_save_context_write_int (context, "loyalty-modifier", priv->loyalty_modifier);
    lrg_save_context_write_double (context, "discovery-modifier", priv->discovery_modifier);

    /* Save conflicts */
    lrg_save_context_write_uint (context, "conflict-count", priv->conflicts_with->len);

    for (i = 0; i < priv->conflicts_with->len; i++)
    {
        const gchar *conflict_id = g_ptr_array_index (priv->conflicts_with, i);
        g_autofree gchar *key = g_strdup_printf ("conflict-%u", i);

        lrg_save_context_write_string (context, key, conflict_id);
    }

    return TRUE;
}

static gboolean
lp_trait_load (LrgSaveable    *saveable,
               LrgSaveContext *context,
               GError        **error)
{
    LpTrait *self = LP_TRAIT (saveable);
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);
    guint conflict_count;
    guint i;

    g_clear_pointer (&priv->id, g_free);
    priv->id = g_strdup (lrg_save_context_read_string (context, "id", "unknown"));

    g_clear_pointer (&priv->name, g_free);
    priv->name = g_strdup (lrg_save_context_read_string (context, "name", "Unknown Trait"));

    g_clear_pointer (&priv->description, g_free);
    priv->description = g_strdup (lrg_save_context_read_string (context, "description", NULL));

    priv->inheritance_chance = lrg_save_context_read_double (context, "inheritance-chance", 0.5);
    priv->income_modifier = lrg_save_context_read_double (context, "income-modifier", 1.0);
    priv->loyalty_modifier = lrg_save_context_read_int (context, "loyalty-modifier", 0);
    priv->discovery_modifier = lrg_save_context_read_double (context, "discovery-modifier", 1.0);

    /* Load conflicts */
    g_ptr_array_set_size (priv->conflicts_with, 0);
    conflict_count = lrg_save_context_read_uint (context, "conflict-count", 0);

    for (i = 0; i < conflict_count; i++)
    {
        g_autofree gchar *key = g_strdup_printf ("conflict-%u", i);
        const gchar *conflict_id;

        conflict_id = lrg_save_context_read_string (context, key, NULL);
        if (conflict_id != NULL)
            g_ptr_array_add (priv->conflicts_with, g_strdup (conflict_id));
    }

    lp_log_debug ("Loaded trait: %s (%s)", priv->name, priv->id);

    return TRUE;
}

static void
lp_trait_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_trait_get_save_id;
    iface->save = lp_trait_save;
    iface->load = lp_trait_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_trait_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    LpTrait *self = LP_TRAIT (object);
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);

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

    case PROP_INHERITANCE_CHANCE:
        g_value_set_float (value, priv->inheritance_chance);
        break;

    case PROP_INCOME_MODIFIER:
        g_value_set_float (value, priv->income_modifier);
        break;

    case PROP_LOYALTY_MODIFIER:
        g_value_set_int (value, priv->loyalty_modifier);
        break;

    case PROP_DISCOVERY_MODIFIER:
        g_value_set_float (value, priv->discovery_modifier);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_trait_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    LpTrait *self = LP_TRAIT (object);
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ID:
        g_clear_pointer (&priv->id, g_free);
        priv->id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        lp_trait_set_name (self, g_value_get_string (value));
        break;

    case PROP_DESCRIPTION:
        lp_trait_set_description (self, g_value_get_string (value));
        break;

    case PROP_INHERITANCE_CHANCE:
        lp_trait_set_inheritance_chance (self, g_value_get_float (value));
        break;

    case PROP_INCOME_MODIFIER:
        lp_trait_set_income_modifier (self, g_value_get_float (value));
        break;

    case PROP_LOYALTY_MODIFIER:
        lp_trait_set_loyalty_modifier (self, g_value_get_int (value));
        break;

    case PROP_DISCOVERY_MODIFIER:
        lp_trait_set_discovery_modifier (self, g_value_get_float (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_trait_finalize (GObject *object)
{
    LpTrait *self = LP_TRAIT (object);
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);

    lp_log_debug ("Finalizing trait: %s", priv->id ? priv->id : "(unknown)");

    g_clear_pointer (&priv->id, g_free);
    g_clear_pointer (&priv->name, g_free);
    g_clear_pointer (&priv->description, g_free);
    g_clear_pointer (&priv->conflicts_with, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_trait_parent_class)->finalize (object);
}

static void
lp_trait_class_init (LpTraitClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_trait_get_property;
    object_class->set_property = lp_trait_set_property;
    object_class->finalize = lp_trait_finalize;

    /* Default virtual method implementations */
    klass->apply_effects = lp_trait_real_apply_effects;
    klass->roll_inheritance = lp_trait_real_roll_inheritance;

    /**
     * LpTrait:id:
     *
     * Unique identifier.
     */
    properties[PROP_ID] =
        g_param_spec_string ("id",
                             "ID",
                             "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpTrait:name:
     *
     * Display name.
     */
    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Display name",
                             "Unknown Trait",
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpTrait:description:
     *
     * Detailed description.
     */
    properties[PROP_DESCRIPTION] =
        g_param_spec_string ("description",
                             "Description",
                             "Detailed description",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpTrait:inheritance-chance:
     *
     * Base chance to inherit (0.0-1.0).
     */
    properties[PROP_INHERITANCE_CHANCE] =
        g_param_spec_float ("inheritance-chance",
                            "Inheritance Chance",
                            "Base inheritance probability",
                            0.0f, 1.0f, 0.5f,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * LpTrait:income-modifier:
     *
     * Income multiplier (1.0 = no change).
     */
    properties[PROP_INCOME_MODIFIER] =
        g_param_spec_float ("income-modifier",
                            "Income Modifier",
                            "Income multiplier",
                            0.0f, 10.0f, 1.0f,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * LpTrait:loyalty-modifier:
     *
     * Loyalty bonus/penalty.
     */
    properties[PROP_LOYALTY_MODIFIER] =
        g_param_spec_int ("loyalty-modifier",
                          "Loyalty Modifier",
                          "Loyalty bonus/penalty",
                          -100, 100, 0,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpTrait:discovery-modifier:
     *
     * Discovery chance multiplier (1.0 = no change).
     */
    properties[PROP_DISCOVERY_MODIFIER] =
        g_param_spec_float ("discovery-modifier",
                            "Discovery Modifier",
                            "Discovery chance multiplier",
                            0.0f, 10.0f, 1.0f,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_trait_init (LpTrait *self)
{
    LpTraitPrivate *priv = lp_trait_get_instance_private (self);

    priv->id = NULL;
    priv->name = g_strdup ("Unknown Trait");
    priv->description = NULL;
    priv->inheritance_chance = 0.5f;
    priv->income_modifier = 1.0f;
    priv->loyalty_modifier = 0;
    priv->discovery_modifier = 1.0f;
    priv->conflicts_with = g_ptr_array_new_with_free_func (g_free);
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpTrait *
lp_trait_new (const gchar *id,
              const gchar *name)
{
    return g_object_new (LP_TYPE_TRAIT,
                         "id", id,
                         "name", name,
                         NULL);
}

LpTrait *
lp_trait_new_full (const gchar *id,
                   const gchar *name,
                   const gchar *description,
                   gfloat       inheritance_chance,
                   gfloat       income_modifier,
                   gint         loyalty_modifier,
                   gfloat       discovery_modifier)
{
    return g_object_new (LP_TYPE_TRAIT,
                         "id", id,
                         "name", name,
                         "description", description,
                         "inheritance-chance", inheritance_chance,
                         "income-modifier", income_modifier,
                         "loyalty-modifier", loyalty_modifier,
                         "discovery-modifier", discovery_modifier,
                         NULL);
}

/* ==========================================================================
 * Property Getters/Setters
 * ========================================================================== */

const gchar *
lp_trait_get_id (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), NULL);

    priv = lp_trait_get_instance_private (self);
    return priv->id;
}

const gchar *
lp_trait_get_name (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), NULL);

    priv = lp_trait_get_instance_private (self);
    return priv->name;
}

void
lp_trait_set_name (LpTrait     *self,
                   const gchar *name)
{
    LpTraitPrivate *priv;

    g_return_if_fail (LP_IS_TRAIT (self));

    priv = lp_trait_get_instance_private (self);

    if (g_strcmp0 (priv->name, name) == 0)
        return;

    g_clear_pointer (&priv->name, g_free);
    priv->name = g_strdup (name);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}

const gchar *
lp_trait_get_description (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), NULL);

    priv = lp_trait_get_instance_private (self);
    return priv->description;
}

void
lp_trait_set_description (LpTrait     *self,
                          const gchar *description)
{
    LpTraitPrivate *priv;

    g_return_if_fail (LP_IS_TRAIT (self));

    priv = lp_trait_get_instance_private (self);

    if (g_strcmp0 (priv->description, description) == 0)
        return;

    g_clear_pointer (&priv->description, g_free);
    priv->description = g_strdup (description);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESCRIPTION]);
}

gfloat
lp_trait_get_inheritance_chance (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), 0.5f);

    priv = lp_trait_get_instance_private (self);
    return priv->inheritance_chance;
}

void
lp_trait_set_inheritance_chance (LpTrait *self,
                                 gfloat   chance)
{
    LpTraitPrivate *priv;

    g_return_if_fail (LP_IS_TRAIT (self));

    priv = lp_trait_get_instance_private (self);

    chance = CLAMP (chance, 0.0f, 1.0f);

    if (priv->inheritance_chance == chance)
        return;

    priv->inheritance_chance = chance;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INHERITANCE_CHANCE]);
}

gfloat
lp_trait_get_income_modifier (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), 1.0f);

    priv = lp_trait_get_instance_private (self);
    return priv->income_modifier;
}

void
lp_trait_set_income_modifier (LpTrait *self,
                              gfloat   modifier)
{
    LpTraitPrivate *priv;

    g_return_if_fail (LP_IS_TRAIT (self));

    priv = lp_trait_get_instance_private (self);

    if (priv->income_modifier == modifier)
        return;

    priv->income_modifier = modifier;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INCOME_MODIFIER]);
}

gint
lp_trait_get_loyalty_modifier (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), 0);

    priv = lp_trait_get_instance_private (self);
    return priv->loyalty_modifier;
}

void
lp_trait_set_loyalty_modifier (LpTrait *self,
                               gint     modifier)
{
    LpTraitPrivate *priv;

    g_return_if_fail (LP_IS_TRAIT (self));

    priv = lp_trait_get_instance_private (self);

    if (priv->loyalty_modifier == modifier)
        return;

    priv->loyalty_modifier = modifier;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOYALTY_MODIFIER]);
}

gfloat
lp_trait_get_discovery_modifier (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), 1.0f);

    priv = lp_trait_get_instance_private (self);
    return priv->discovery_modifier;
}

void
lp_trait_set_discovery_modifier (LpTrait *self,
                                 gfloat   modifier)
{
    LpTraitPrivate *priv;

    g_return_if_fail (LP_IS_TRAIT (self));

    priv = lp_trait_get_instance_private (self);

    if (priv->discovery_modifier == modifier)
        return;

    priv->discovery_modifier = modifier;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DISCOVERY_MODIFIER]);
}

GPtrArray *
lp_trait_get_conflicts_with (LpTrait *self)
{
    LpTraitPrivate *priv;

    g_return_val_if_fail (LP_IS_TRAIT (self), NULL);

    priv = lp_trait_get_instance_private (self);
    return priv->conflicts_with;
}

void
lp_trait_add_conflict (LpTrait     *self,
                       const gchar *trait_id)
{
    LpTraitPrivate *priv;

    g_return_if_fail (LP_IS_TRAIT (self));
    g_return_if_fail (trait_id != NULL);

    priv = lp_trait_get_instance_private (self);

    /* Check if already conflicting */
    if (lp_trait_conflicts_with_id (self, trait_id))
        return;

    g_ptr_array_add (priv->conflicts_with, g_strdup (trait_id));
}

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

void
lp_trait_apply_effects (LpTrait *self,
                        LpAgent *agent)
{
    LpTraitClass *klass;

    g_return_if_fail (LP_IS_TRAIT (self));
    g_return_if_fail (LP_IS_AGENT (agent));

    klass = LP_TRAIT_GET_CLASS (self);
    if (klass->apply_effects != NULL)
        klass->apply_effects (self, agent);
}

gboolean
lp_trait_roll_inheritance (LpTrait *self,
                           guint    generation)
{
    LpTraitClass *klass;

    g_return_val_if_fail (LP_IS_TRAIT (self), FALSE);

    klass = LP_TRAIT_GET_CLASS (self);
    g_return_val_if_fail (klass->roll_inheritance != NULL, FALSE);

    return klass->roll_inheritance (self, generation);
}

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

gboolean
lp_trait_conflicts_with (LpTrait *self,
                         LpTrait *other)
{
    g_return_val_if_fail (LP_IS_TRAIT (self), FALSE);
    g_return_val_if_fail (LP_IS_TRAIT (other), FALSE);

    return lp_trait_conflicts_with_id (self, lp_trait_get_id (other));
}

gboolean
lp_trait_conflicts_with_id (LpTrait     *self,
                            const gchar *trait_id)
{
    LpTraitPrivate *priv;
    guint i;

    g_return_val_if_fail (LP_IS_TRAIT (self), FALSE);
    g_return_val_if_fail (trait_id != NULL, FALSE);

    priv = lp_trait_get_instance_private (self);

    for (i = 0; i < priv->conflicts_with->len; i++)
    {
        const gchar *conflict_id = g_ptr_array_index (priv->conflicts_with, i);

        if (g_strcmp0 (conflict_id, trait_id) == 0)
            return TRUE;
    }

    return FALSE;
}

LpTrait *
lp_trait_copy (LpTrait *self)
{
    LpTraitPrivate *priv;
    LpTrait *copy;
    LpTraitPrivate *copy_priv;
    guint i;

    g_return_val_if_fail (LP_IS_TRAIT (self), NULL);

    priv = lp_trait_get_instance_private (self);

    copy = lp_trait_new_full (priv->id,
                              priv->name,
                              priv->description,
                              priv->inheritance_chance,
                              priv->income_modifier,
                              priv->loyalty_modifier,
                              priv->discovery_modifier);

    copy_priv = lp_trait_get_instance_private (copy);

    /* Copy conflicts */
    for (i = 0; i < priv->conflicts_with->len; i++)
    {
        const gchar *conflict_id = g_ptr_array_index (priv->conflicts_with, i);
        g_ptr_array_add (copy_priv->conflicts_with, g_strdup (conflict_id));
    }

    return copy;
}
