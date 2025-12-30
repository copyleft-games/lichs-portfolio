/* lp-trait.h - Bloodline Trait System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Traits represent inheritable characteristics in agent bloodlines.
 * They modify agent effectiveness, loyalty, and can be passed down
 * through generations in family agents.
 */

#ifndef LP_TRAIT_H
#define LP_TRAIT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_TRAIT (lp_trait_get_type ())

G_DECLARE_DERIVABLE_TYPE (LpTrait, lp_trait, LP, TRAIT, GObject)

/**
 * LpTraitClass:
 * @parent_class: parent class
 * @apply_effects: apply trait effects to an agent
 * @roll_inheritance: roll for trait inheritance
 *
 * Virtual methods for trait subclasses.
 */
struct _LpTraitClass
{
    GObjectClass parent_class;

    /*< public >*/

    /**
     * LpTraitClass::apply_effects:
     * @self: an #LpTrait
     * @agent: the #LpAgent to apply effects to
     *
     * Applies this trait's effects to an agent.
     * Called when the trait is gained or during calculations.
     */
    void (*apply_effects) (LpTrait *self,
                           LpAgent *agent);

    /**
     * LpTraitClass::roll_inheritance:
     * @self: an #LpTrait
     * @generation: the generation number of the child
     *
     * Rolls to see if this trait is inherited.
     *
     * Returns: %TRUE if the trait is inherited
     */
    gboolean (*roll_inheritance) (LpTrait *self,
                                  guint    generation);

    /*< private >*/
    gpointer _reserved[8];
};

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_trait_new:
 * @id: unique identifier for this trait
 * @name: display name
 *
 * Creates a new trait.
 *
 * Returns: (transfer full): A new #LpTrait
 */
LpTrait *
lp_trait_new (const gchar *id,
              const gchar *name);

/**
 * lp_trait_new_full:
 * @id: unique identifier
 * @name: display name
 * @description: detailed description
 * @inheritance_chance: base chance to inherit (0.0-1.0)
 * @income_modifier: income multiplier (1.0 = no change)
 * @loyalty_modifier: loyalty bonus/penalty
 * @discovery_modifier: discovery chance multiplier (1.0 = no change)
 *
 * Creates a new trait with all properties set.
 *
 * Returns: (transfer full): A new #LpTrait
 */
LpTrait *
lp_trait_new_full (const gchar *id,
                   const gchar *name,
                   const gchar *description,
                   gfloat       inheritance_chance,
                   gfloat       income_modifier,
                   gint         loyalty_modifier,
                   gfloat       discovery_modifier);

/* ==========================================================================
 * Property Getters/Setters
 * ========================================================================== */

/**
 * lp_trait_get_id:
 * @self: an #LpTrait
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The trait ID
 */
const gchar *
lp_trait_get_id (LpTrait *self);

/**
 * lp_trait_get_name:
 * @self: an #LpTrait
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The trait name
 */
const gchar *
lp_trait_get_name (LpTrait *self);

/**
 * lp_trait_set_name:
 * @self: an #LpTrait
 * @name: the new name
 *
 * Sets the display name.
 */
void
lp_trait_set_name (LpTrait     *self,
                   const gchar *name);

/**
 * lp_trait_get_description:
 * @self: an #LpTrait
 *
 * Gets the description.
 *
 * Returns: (transfer none) (nullable): The description
 */
const gchar *
lp_trait_get_description (LpTrait *self);

/**
 * lp_trait_set_description:
 * @self: an #LpTrait
 * @description: (nullable): the new description
 *
 * Sets the description.
 */
void
lp_trait_set_description (LpTrait     *self,
                          const gchar *description);

/**
 * lp_trait_get_inheritance_chance:
 * @self: an #LpTrait
 *
 * Gets the base inheritance chance (0.0-1.0).
 *
 * Returns: Inheritance probability
 */
gfloat
lp_trait_get_inheritance_chance (LpTrait *self);

/**
 * lp_trait_set_inheritance_chance:
 * @self: an #LpTrait
 * @chance: inheritance probability (0.0-1.0)
 *
 * Sets the inheritance chance.
 */
void
lp_trait_set_inheritance_chance (LpTrait *self,
                                 gfloat   chance);

/**
 * lp_trait_get_income_modifier:
 * @self: an #LpTrait
 *
 * Gets the income modifier (1.0 = no change).
 *
 * Returns: Income multiplier
 */
gfloat
lp_trait_get_income_modifier (LpTrait *self);

/**
 * lp_trait_set_income_modifier:
 * @self: an #LpTrait
 * @modifier: income multiplier
 *
 * Sets the income modifier.
 */
void
lp_trait_set_income_modifier (LpTrait *self,
                              gfloat   modifier);

/**
 * lp_trait_get_loyalty_modifier:
 * @self: an #LpTrait
 *
 * Gets the loyalty bonus/penalty.
 *
 * Returns: Loyalty modifier
 */
gint
lp_trait_get_loyalty_modifier (LpTrait *self);

/**
 * lp_trait_set_loyalty_modifier:
 * @self: an #LpTrait
 * @modifier: loyalty bonus/penalty
 *
 * Sets the loyalty modifier.
 */
void
lp_trait_set_loyalty_modifier (LpTrait *self,
                               gint     modifier);

/**
 * lp_trait_get_discovery_modifier:
 * @self: an #LpTrait
 *
 * Gets the discovery chance modifier (1.0 = no change).
 *
 * Returns: Discovery multiplier
 */
gfloat
lp_trait_get_discovery_modifier (LpTrait *self);

/**
 * lp_trait_set_discovery_modifier:
 * @self: an #LpTrait
 * @modifier: discovery multiplier
 *
 * Sets the discovery modifier.
 */
void
lp_trait_set_discovery_modifier (LpTrait *self,
                                 gfloat   modifier);

/**
 * lp_trait_get_conflicts_with:
 * @self: an #LpTrait
 *
 * Gets the list of trait IDs that conflict with this trait.
 *
 * Returns: (transfer none) (nullable) (element-type utf8): Array of conflicting trait IDs
 */
GPtrArray *
lp_trait_get_conflicts_with (LpTrait *self);

/**
 * lp_trait_add_conflict:
 * @self: an #LpTrait
 * @trait_id: ID of a conflicting trait
 *
 * Adds a trait conflict.
 */
void
lp_trait_add_conflict (LpTrait     *self,
                       const gchar *trait_id);

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

/**
 * lp_trait_apply_effects:
 * @self: an #LpTrait
 * @agent: the #LpAgent to apply effects to
 *
 * Applies trait effects to an agent.
 */
void
lp_trait_apply_effects (LpTrait *self,
                        LpAgent *agent);

/**
 * lp_trait_roll_inheritance:
 * @self: an #LpTrait
 * @generation: generation number of the potential inheritor
 *
 * Rolls for trait inheritance.
 *
 * Returns: %TRUE if trait is inherited
 */
gboolean
lp_trait_roll_inheritance (LpTrait *self,
                           guint    generation);

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

/**
 * lp_trait_conflicts_with:
 * @self: an #LpTrait
 * @other: another #LpTrait
 *
 * Checks if this trait conflicts with another trait.
 *
 * Returns: %TRUE if the traits conflict
 */
gboolean
lp_trait_conflicts_with (LpTrait *self,
                         LpTrait *other);

/**
 * lp_trait_conflicts_with_id:
 * @self: an #LpTrait
 * @trait_id: ID of another trait
 *
 * Checks if this trait conflicts with a trait ID.
 *
 * Returns: %TRUE if they conflict
 */
gboolean
lp_trait_conflicts_with_id (LpTrait     *self,
                            const gchar *trait_id);

/**
 * lp_trait_copy:
 * @self: an #LpTrait
 *
 * Creates a copy of this trait.
 *
 * Returns: (transfer full): A new #LpTrait with same properties
 */
LpTrait *
lp_trait_copy (LpTrait *self);

G_END_DECLS

#endif /* LP_TRAIT_H */
