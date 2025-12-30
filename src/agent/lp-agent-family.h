/* lp-agent-family.h - Bloodline Dynasty Agent
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Family agents represent bloodline dynasties that serve the lich
 * across generations. Traits can be inherited and accumulate over
 * generations, creating increasingly powerful lineages.
 */

#ifndef LP_AGENT_FAMILY_H
#define LP_AGENT_FAMILY_H

#include <glib-object.h>
#include "lp-agent.h"
#include "lp-trait.h"

G_BEGIN_DECLS

#define LP_TYPE_AGENT_FAMILY (lp_agent_family_get_type ())

G_DECLARE_FINAL_TYPE (LpAgentFamily, lp_agent_family, LP, AGENT_FAMILY, LpAgent)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_agent_family_new:
 * @id: unique identifier
 * @family_name: the family/dynasty name
 * @founding_year: year the family was established
 *
 * Creates a new family agent (first generation).
 *
 * Returns: (transfer full): A new #LpAgentFamily
 */
LpAgentFamily *
lp_agent_family_new (const gchar *id,
                     const gchar *family_name,
                     guint64      founding_year);

/**
 * lp_agent_family_new_with_head:
 * @id: unique identifier
 * @family_name: the family/dynasty name
 * @head_name: name of the current family head
 * @founding_year: year the family was established
 * @head_age: age of the current head
 * @head_max_age: maximum lifespan of the current head
 *
 * Creates a new family with a named head.
 *
 * Returns: (transfer full): A new #LpAgentFamily
 */
LpAgentFamily *
lp_agent_family_new_with_head (const gchar *id,
                               const gchar *family_name,
                               const gchar *head_name,
                               guint64      founding_year,
                               guint        head_age,
                               guint        head_max_age);

/* ==========================================================================
 * Property Getters/Setters
 * ========================================================================== */

/**
 * lp_agent_family_get_family_name:
 * @self: an #LpAgentFamily
 *
 * Gets the family/dynasty name.
 *
 * Returns: (transfer none): The family name
 */
const gchar *
lp_agent_family_get_family_name (LpAgentFamily *self);

/**
 * lp_agent_family_get_generation:
 * @self: an #LpAgentFamily
 *
 * Gets the current generation number (1 = founding generation).
 *
 * Returns: Generation number
 */
guint
lp_agent_family_get_generation (LpAgentFamily *self);

/**
 * lp_agent_family_get_founding_year:
 * @self: an #LpAgentFamily
 *
 * Gets the year the family was established.
 *
 * Returns: Founding year
 */
guint64
lp_agent_family_get_founding_year (LpAgentFamily *self);

/**
 * lp_agent_family_get_bloodline_traits:
 * @self: an #LpAgentFamily
 *
 * Gets the traits that have been accumulated in this bloodline.
 * These are the traits that can be inherited by new generations.
 *
 * Returns: (transfer none) (element-type LpTrait): Array of bloodline traits
 */
GPtrArray *
lp_agent_family_get_bloodline_traits (LpAgentFamily *self);

/* ==========================================================================
 * Trait Management
 * ========================================================================== */

/**
 * lp_agent_family_add_bloodline_trait:
 * @self: an #LpAgentFamily
 * @trait: (transfer none): the #LpTrait to add to the bloodline
 *
 * Adds a trait to the bloodline's inheritable traits.
 * This does not immediately give the trait to the current head.
 */
void
lp_agent_family_add_bloodline_trait (LpAgentFamily *self,
                                     LpTrait       *trait);

/**
 * lp_agent_family_remove_bloodline_trait:
 * @self: an #LpAgentFamily
 * @trait: the #LpTrait to remove
 *
 * Removes a trait from the bloodline.
 *
 * Returns: %TRUE if the trait was removed
 */
gboolean
lp_agent_family_remove_bloodline_trait (LpAgentFamily *self,
                                        LpTrait       *trait);

/**
 * lp_agent_family_has_bloodline_trait:
 * @self: an #LpAgentFamily
 * @trait_id: ID of the trait to check
 *
 * Checks if a trait is in the bloodline.
 *
 * Returns: %TRUE if the bloodline has this trait
 */
gboolean
lp_agent_family_has_bloodline_trait (LpAgentFamily *self,
                                     const gchar   *trait_id);

/* ==========================================================================
 * Succession Methods
 * ========================================================================== */

/**
 * lp_agent_family_advance_generation:
 * @self: an #LpAgentFamily
 *
 * Advances to the next generation when the current head dies.
 * This:
 * - Increments the generation counter
 * - Rolls for trait inheritance from bloodline
 * - Has a chance to generate new traits
 * - Resets age to a young adult
 */
void
lp_agent_family_advance_generation (LpAgentFamily *self);

/**
 * lp_agent_family_roll_inheritance:
 * @self: an #LpAgentFamily
 *
 * Rolls trait inheritance for a new generation.
 * Called during advance_generation, but exposed for testing.
 *
 * Returns: (transfer full) (element-type LpTrait): Array of inherited traits
 */
GPtrArray *
lp_agent_family_roll_inheritance (LpAgentFamily *self);

/**
 * lp_agent_family_roll_new_trait:
 * @self: an #LpAgentFamily
 *
 * Rolls for a random new trait to emerge in the bloodline.
 * 5% base chance, modified by generation number.
 *
 * Returns: (transfer full) (nullable): A new trait, or %NULL
 */
LpTrait *
lp_agent_family_roll_new_trait (LpAgentFamily *self);

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

/**
 * lp_agent_family_get_years_established:
 * @self: an #LpAgentFamily
 * @current_year: the current game year
 *
 * Gets how many years the family has been serving.
 *
 * Returns: Years since founding
 */
guint64
lp_agent_family_get_years_established (LpAgentFamily *self,
                                       guint64        current_year);

/**
 * lp_agent_family_get_max_traits:
 *
 * Gets the maximum number of traits an agent can have.
 *
 * Returns: Maximum trait count (4)
 */
guint
lp_agent_family_get_max_traits (void);

G_END_DECLS

#endif /* LP_AGENT_FAMILY_H */
