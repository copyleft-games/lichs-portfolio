/* lp-agent-family.c - Bloodline Dynasty Agent
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_AGENT
#include "../lp-log.h"

#include "lp-agent-family.h"

/* Maximum traits per agent */
#define MAX_TRAITS 4

/* New trait emergence chance per generation (5% base) */
#define NEW_TRAIT_CHANCE 0.05f

struct _LpAgentFamily
{
    LpAgent parent_instance;

    gchar    *family_name;
    guint     generation;
    guint64   founding_year;

    GPtrArray *bloodline_traits;  /* Accumulated inheritable traits */
};

enum
{
    PROP_0,
    PROP_FAMILY_NAME,
    PROP_GENERATION,
    PROP_FOUNDING_YEAR,
    N_PROPS
};

enum
{
    SIGNAL_GENERATION_ADVANCED,
    SIGNAL_NEW_TRAIT_EMERGED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations */
static void lp_agent_family_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpAgentFamily, lp_agent_family, LP_TYPE_AGENT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_agent_family_saveable_init))

/* ==========================================================================
 * Predefined Traits
 * ========================================================================== */

/*
 * Sample traits that can emerge in bloodlines.
 * In a full implementation, these would be loaded from data files.
 */
static LpTrait *
create_random_trait (void)
{
    static const struct
    {
        const gchar *id;
        const gchar *name;
        const gchar *description;
        gfloat inheritance;
        gfloat income;
        gint loyalty;
        gfloat discovery;
    } trait_templates[] = {
        { "shrewd", "Shrewd", "Natural business acumen", 0.6f, 1.15f, 0, 1.0f },
        { "loyal", "Devoted", "Exceptional loyalty", 0.5f, 1.0f, 15, 0.8f },
        { "cunning", "Cunning", "Skilled at deception", 0.4f, 1.1f, -5, 0.7f },
        { "ambitious", "Ambitious", "Driven to succeed", 0.5f, 1.2f, -10, 1.1f },
        { "cautious", "Cautious", "Avoids unnecessary risks", 0.6f, 0.95f, 5, 0.6f },
        { "charismatic", "Charismatic", "Natural leader", 0.4f, 1.1f, 5, 1.0f },
        { "secretive", "Secretive", "Keeps secrets well", 0.5f, 1.0f, 0, 0.5f },
        { "greedy", "Greedy", "Motivated by wealth", 0.4f, 1.25f, -15, 1.2f },
    };

    guint index = g_random_int_range (0, G_N_ELEMENTS (trait_templates));

    return lp_trait_new_full (
        trait_templates[index].id,
        trait_templates[index].name,
        trait_templates[index].description,
        trait_templates[index].inheritance,
        trait_templates[index].income,
        trait_templates[index].loyalty,
        trait_templates[index].discovery
    );
}

/* ==========================================================================
 * Virtual Method Overrides
 * ========================================================================== */

static void
lp_agent_family_on_year_passed (LpAgent *agent)
{
    /* Chain up for aging, loyalty, etc. */
    LP_AGENT_CLASS (lp_agent_family_parent_class)->on_year_passed (agent);
}

static void
lp_agent_family_on_death (LpAgent *agent)
{
    LpAgentFamily *self = LP_AGENT_FAMILY (agent);

    lp_log_info ("Family %s generation %u head is dying, advancing generation",
                 self->family_name, self->generation);

    /* Advance to next generation before death signal */
    lp_agent_family_advance_generation (self);

    /*
     * Note: We DON'T chain up to parent on_death here because
     * the family continues with the new generation head.
     * The death signal is for the individual, but the family agent persists.
     */
}

static gboolean
lp_agent_family_can_recruit (LpAgent *agent)
{
    /*
     * Families don't recruit - they advance generations.
     * Their "successors" come from within the bloodline.
     */
    (void)agent;
    return FALSE;
}

static LpAgentType
lp_agent_family_get_agent_type (LpAgent *agent)
{
    (void)agent;
    return LP_AGENT_TYPE_FAMILY;
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_agent_family_get_save_id (LrgSaveable *saveable)
{
    LrgSaveableInterface *parent_iface;

    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (saveable), LRG_TYPE_SAVEABLE));

    return parent_iface->get_save_id (saveable);
}

static gboolean
lp_agent_family_save (LrgSaveable    *saveable,
                      LrgSaveContext *context,
                      GError        **error)
{
    LpAgentFamily *self = LP_AGENT_FAMILY (saveable);
    LrgSaveableInterface *parent_iface;
    guint i;

    /* Call parent save first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (saveable), LRG_TYPE_SAVEABLE));

    if (!parent_iface->save (saveable, context, error))
        return FALSE;

    /* Save family-specific properties */
    lrg_save_context_write_string (context, "family-name", self->family_name);
    lrg_save_context_write_uint (context, "generation", self->generation);
    lrg_save_context_write_uint (context, "founding-year", self->founding_year);

    /* Save bloodline traits by ID */
    lrg_save_context_write_uint (context, "bloodline-trait-count",
                                 self->bloodline_traits->len);

    for (i = 0; i < self->bloodline_traits->len; i++)
    {
        LpTrait *trait = g_ptr_array_index (self->bloodline_traits, i);
        g_autofree gchar *key = g_strdup_printf ("bloodline-trait-%u", i);

        /* Save trait data in a subsection */
        lrg_save_context_begin_section (context, key);
        lrg_saveable_save (LRG_SAVEABLE (trait), context, error);
        lrg_save_context_end_section (context);
    }

    return TRUE;
}

static gboolean
lp_agent_family_load (LrgSaveable    *saveable,
                      LrgSaveContext *context,
                      GError        **error)
{
    LpAgentFamily *self = LP_AGENT_FAMILY (saveable);
    LrgSaveableInterface *parent_iface;
    guint trait_count;
    guint i;

    /* Call parent load first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (saveable), LRG_TYPE_SAVEABLE));

    if (!parent_iface->load (saveable, context, error))
        return FALSE;

    /* Load family-specific properties */
    g_clear_pointer (&self->family_name, g_free);
    self->family_name = g_strdup (
        lrg_save_context_read_string (context, "family-name", "Unknown Family"));

    self->generation = lrg_save_context_read_uint (context, "generation", 1);
    self->founding_year = lrg_save_context_read_uint (context, "founding-year", 847);

    /* Load bloodline traits */
    g_ptr_array_set_size (self->bloodline_traits, 0);
    trait_count = lrg_save_context_read_uint (context, "bloodline-trait-count", 0);

    for (i = 0; i < trait_count; i++)
    {
        g_autofree gchar *key = g_strdup_printf ("bloodline-trait-%u", i);
        LpTrait *trait;

        if (lrg_save_context_enter_section (context, key))
        {
            trait = lp_trait_new ("temp", "Temp");
            lrg_saveable_load (LRG_SAVEABLE (trait), context, error);
            g_ptr_array_add (self->bloodline_traits, trait);
            lrg_save_context_leave_section (context);
        }
    }

    lp_log_debug ("Loaded family: %s (gen %u, %u bloodline traits)",
                  self->family_name, self->generation, self->bloodline_traits->len);

    return TRUE;
}

static void
lp_agent_family_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_agent_family_get_save_id;
    iface->save = lp_agent_family_save;
    iface->load = lp_agent_family_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_agent_family_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    LpAgentFamily *self = LP_AGENT_FAMILY (object);

    switch (prop_id)
    {
    case PROP_FAMILY_NAME:
        g_value_set_string (value, self->family_name);
        break;

    case PROP_GENERATION:
        g_value_set_uint (value, self->generation);
        break;

    case PROP_FOUNDING_YEAR:
        g_value_set_uint64 (value, self->founding_year);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_agent_family_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    LpAgentFamily *self = LP_AGENT_FAMILY (object);

    switch (prop_id)
    {
    case PROP_FAMILY_NAME:
        g_clear_pointer (&self->family_name, g_free);
        self->family_name = g_value_dup_string (value);
        break;

    case PROP_GENERATION:
        self->generation = g_value_get_uint (value);
        break;

    case PROP_FOUNDING_YEAR:
        self->founding_year = g_value_get_uint64 (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_agent_family_finalize (GObject *object)
{
    LpAgentFamily *self = LP_AGENT_FAMILY (object);

    lp_log_debug ("Finalizing family: %s", self->family_name ? self->family_name : "(unknown)");

    g_clear_pointer (&self->family_name, g_free);
    g_clear_pointer (&self->bloodline_traits, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_agent_family_parent_class)->finalize (object);
}

static void
lp_agent_family_class_init (LpAgentFamilyClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpAgentClass *agent_class = LP_AGENT_CLASS (klass);

    object_class->get_property = lp_agent_family_get_property;
    object_class->set_property = lp_agent_family_set_property;
    object_class->finalize = lp_agent_family_finalize;

    /* Override virtual methods */
    agent_class->on_year_passed = lp_agent_family_on_year_passed;
    agent_class->on_death = lp_agent_family_on_death;
    agent_class->can_recruit = lp_agent_family_can_recruit;
    agent_class->get_agent_type = lp_agent_family_get_agent_type;

    /**
     * LpAgentFamily:family-name:
     *
     * The family/dynasty name.
     */
    properties[PROP_FAMILY_NAME] =
        g_param_spec_string ("family-name",
                             "Family Name",
                             "The family/dynasty name",
                             "Unknown Family",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpAgentFamily:generation:
     *
     * Current generation number (1 = founding).
     */
    properties[PROP_GENERATION] =
        g_param_spec_uint ("generation",
                           "Generation",
                           "Current generation number",
                           1, G_MAXUINT, 1,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpAgentFamily:founding-year:
     *
     * Year the family was established.
     */
    properties[PROP_FOUNDING_YEAR] =
        g_param_spec_uint64 ("founding-year",
                             "Founding Year",
                             "Year the family was established",
                             0, G_MAXUINT64, 847,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpAgentFamily::generation-advanced:
     * @self: the #LpAgentFamily
     * @new_generation: the new generation number
     *
     * Emitted when the family advances to a new generation.
     */
    signals[SIGNAL_GENERATION_ADVANCED] =
        g_signal_new ("generation-advanced",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_UINT);

    /**
     * LpAgentFamily::new-trait-emerged:
     * @self: the #LpAgentFamily
     * @trait: the new #LpTrait
     *
     * Emitted when a new trait emerges in the bloodline.
     */
    signals[SIGNAL_NEW_TRAIT_EMERGED] =
        g_signal_new ("new-trait-emerged",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      LP_TYPE_TRAIT);
}

static void
lp_agent_family_init (LpAgentFamily *self)
{
    self->family_name = g_strdup ("Unknown Family");
    self->generation = 1;
    self->founding_year = 847;
    self->bloodline_traits = g_ptr_array_new_with_free_func (g_object_unref);
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpAgentFamily *
lp_agent_family_new (const gchar *id,
                     const gchar *family_name,
                     guint64      founding_year)
{
    g_autofree gchar *head_name = g_strdup_printf ("Head of %s", family_name);

    return g_object_new (LP_TYPE_AGENT_FAMILY,
                         "id", id,
                         "name", head_name,
                         "family-name", family_name,
                         "founding-year", founding_year,
                         NULL);
}

LpAgentFamily *
lp_agent_family_new_with_head (const gchar *id,
                               const gchar *family_name,
                               const gchar *head_name,
                               guint64      founding_year,
                               guint        head_age,
                               guint        head_max_age)
{
    return g_object_new (LP_TYPE_AGENT_FAMILY,
                         "id", id,
                         "name", head_name,
                         "family-name", family_name,
                         "founding-year", founding_year,
                         "age", head_age,
                         "max-age", head_max_age,
                         NULL);
}

/* ==========================================================================
 * Property Getters/Setters
 * ========================================================================== */

const gchar *
lp_agent_family_get_family_name (LpAgentFamily *self)
{
    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), NULL);

    return self->family_name;
}

guint
lp_agent_family_get_generation (LpAgentFamily *self)
{
    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), 0);

    return self->generation;
}

guint64
lp_agent_family_get_founding_year (LpAgentFamily *self)
{
    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), 0);

    return self->founding_year;
}

GPtrArray *
lp_agent_family_get_bloodline_traits (LpAgentFamily *self)
{
    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), NULL);

    return self->bloodline_traits;
}

/* ==========================================================================
 * Trait Management
 * ========================================================================== */

void
lp_agent_family_add_bloodline_trait (LpAgentFamily *self,
                                     LpTrait       *trait)
{
    g_return_if_fail (LP_IS_AGENT_FAMILY (self));
    g_return_if_fail (LP_IS_TRAIT (trait));

    /* Check if already in bloodline */
    if (lp_agent_family_has_bloodline_trait (self, lp_trait_get_id (trait)))
        return;

    g_ptr_array_add (self->bloodline_traits, g_object_ref (trait));

    lp_log_debug ("Family %s gained bloodline trait: %s",
                  self->family_name, lp_trait_get_name (trait));
}

gboolean
lp_agent_family_remove_bloodline_trait (LpAgentFamily *self,
                                        LpTrait       *trait)
{
    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), FALSE);
    g_return_val_if_fail (LP_IS_TRAIT (trait), FALSE);

    return g_ptr_array_remove (self->bloodline_traits, trait);
}

gboolean
lp_agent_family_has_bloodline_trait (LpAgentFamily *self,
                                     const gchar   *trait_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), FALSE);
    g_return_val_if_fail (trait_id != NULL, FALSE);

    for (i = 0; i < self->bloodline_traits->len; i++)
    {
        LpTrait *trait = g_ptr_array_index (self->bloodline_traits, i);

        if (g_strcmp0 (lp_trait_get_id (trait), trait_id) == 0)
            return TRUE;
    }

    return FALSE;
}

/* ==========================================================================
 * Succession Methods
 * ========================================================================== */

void
lp_agent_family_advance_generation (LpAgentFamily *self)
{
    g_autoptr(GPtrArray) inherited_traits = NULL;
    LpTrait *new_trait;
    g_autofree gchar *new_head_name = NULL;
    GPtrArray *current_traits;
    guint i;

    g_return_if_fail (LP_IS_AGENT_FAMILY (self));

    self->generation++;

    lp_log_info ("Family %s advancing to generation %u",
                 self->family_name, self->generation);

    /* Roll for trait inheritance */
    inherited_traits = lp_agent_family_roll_inheritance (self);

    /* Clear current head's traits */
    current_traits = lp_agent_get_traits (LP_AGENT (self));
    g_ptr_array_set_size (current_traits, 0);

    /* Apply inherited traits to new head */
    for (i = 0; i < inherited_traits->len && i < MAX_TRAITS; i++)
    {
        LpTrait *trait = g_ptr_array_index (inherited_traits, i);
        lp_agent_add_trait (LP_AGENT (self), trait);
    }

    /* Roll for new trait emergence */
    new_trait = lp_agent_family_roll_new_trait (self);

    if (new_trait != NULL)
    {
        /* Add to bloodline */
        lp_agent_family_add_bloodline_trait (self, new_trait);

        /* Give to current head if room */
        if (lp_agent_get_traits (LP_AGENT (self))->len < MAX_TRAITS)
        {
            lp_agent_add_trait (LP_AGENT (self), new_trait);
        }

        g_signal_emit (self, signals[SIGNAL_NEW_TRAIT_EMERGED], 0, new_trait);
        g_object_unref (new_trait);
    }

    /* Reset age for new head */
    lp_agent_set_age (LP_AGENT (self), g_random_int_range (18, 25));
    lp_agent_set_max_age (LP_AGENT (self), g_random_int_range (60, 85));

    /* Generate new head name */
    new_head_name = g_strdup_printf ("%s %s (Gen %u)",
                                     self->family_name,
                                     g_random_int_range (0, 2) ? "Senior" : "Junior",
                                     self->generation);
    lp_agent_set_name (LP_AGENT (self), new_head_name);

    /* Reset loyalty slightly (new head may be less devoted) */
    lp_agent_set_loyalty (LP_AGENT (self),
                          lp_agent_get_loyalty (LP_AGENT (self)) - g_random_int_range (0, 10));

    g_signal_emit (self, signals[SIGNAL_GENERATION_ADVANCED], 0, self->generation);
}

GPtrArray *
lp_agent_family_roll_inheritance (LpAgentFamily *self)
{
    GPtrArray *inherited;
    GPtrArray *agent_traits;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), NULL);

    inherited = g_ptr_array_new_with_free_func (g_object_unref);

    /* First, roll for each bloodline trait */
    for (i = 0; i < self->bloodline_traits->len; i++)
    {
        LpTrait *trait = g_ptr_array_index (self->bloodline_traits, i);

        if (lp_trait_roll_inheritance (trait, self->generation))
        {
            /* Check for conflicts with already inherited traits */
            gboolean conflicts = FALSE;
            guint j;

            for (j = 0; j < inherited->len; j++)
            {
                LpTrait *existing = g_ptr_array_index (inherited, j);

                if (lp_trait_conflicts_with (trait, existing))
                {
                    conflicts = TRUE;
                    lp_log_debug ("Trait %s conflicts with %s, skipping",
                                  lp_trait_get_name (trait),
                                  lp_trait_get_name (existing));
                    break;
                }
            }

            if (!conflicts && inherited->len < MAX_TRAITS)
            {
                g_ptr_array_add (inherited, g_object_ref (trait));
            }
        }
    }

    /* Also consider traits from the dying head (current traits) */
    agent_traits = lp_agent_get_traits (LP_AGENT (self));

    for (i = 0; i < agent_traits->len; i++)
    {
        LpTrait *trait = g_ptr_array_index (agent_traits, i);

        /* Skip if already in inherited or bloodline */
        if (lp_agent_family_has_bloodline_trait (self, lp_trait_get_id (trait)))
            continue;

        /* 50% chance to add trait to bloodline if it was only on the head */
        if (g_random_int_range (0, 100) < 50)
        {
            lp_agent_family_add_bloodline_trait (self, trait);

            if (inherited->len < MAX_TRAITS)
            {
                g_ptr_array_add (inherited, g_object_ref (trait));
            }
        }
    }

    lp_log_debug ("Family %s generation %u inherited %u traits",
                  self->family_name, self->generation, inherited->len);

    return inherited;
}

LpTrait *
lp_agent_family_roll_new_trait (LpAgentFamily *self)
{
    gfloat emergence_chance;
    LpTrait *new_trait;
    guint attempts;
    guint i;
    gboolean conflicts;

    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), NULL);

    /*
     * New trait emergence chance:
     * Base 5%, +1% per generation (max 15%)
     */
    emergence_chance = NEW_TRAIT_CHANCE + (self->generation * 0.01f);
    emergence_chance = MIN (emergence_chance, 0.15f);

    if (g_random_double () >= emergence_chance)
        return NULL;

    /* Try to generate a trait that doesn't conflict with existing */
    for (attempts = 0; attempts < 5; attempts++)
    {
        new_trait = create_random_trait ();

        /* Check if trait already exists in bloodline */
        if (lp_agent_family_has_bloodline_trait (self, lp_trait_get_id (new_trait)))
        {
            g_object_unref (new_trait);
            continue;
        }

        /* Check for conflicts with bloodline */
        conflicts = FALSE;

        for (i = 0; i < self->bloodline_traits->len; i++)
        {
            LpTrait *existing = g_ptr_array_index (self->bloodline_traits, i);

            if (lp_trait_conflicts_with (new_trait, existing))
            {
                conflicts = TRUE;
                break;
            }
        }

        if (!conflicts)
        {
            lp_log_info ("New trait emerged in family %s: %s",
                         self->family_name, lp_trait_get_name (new_trait));
            return new_trait;
        }

        g_object_unref (new_trait);
    }

    return NULL;
}

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

guint64
lp_agent_family_get_years_established (LpAgentFamily *self,
                                       guint64        current_year)
{
    g_return_val_if_fail (LP_IS_AGENT_FAMILY (self), 0);

    if (current_year <= self->founding_year)
        return 0;

    return current_year - self->founding_year;
}

guint
lp_agent_family_get_max_traits (void)
{
    return MAX_TRAITS;
}
