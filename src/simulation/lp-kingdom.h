/* lp-kingdom.h - Political Kingdom
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Kingdoms are political entities that control regions.
 * They have five core attributes that affect gameplay:
 * - Stability: Government stability (low = collapse risk)
 * - Prosperity: Economic health
 * - Military: War capability
 * - Culture: Resistance to change
 * - Tolerance: Magic/undead acceptance
 */

#ifndef LP_KINGDOM_H
#define LP_KINGDOM_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_KINGDOM (lp_kingdom_get_type ())

G_DECLARE_FINAL_TYPE (LpKingdom, lp_kingdom, LP, KINGDOM, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_kingdom_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new kingdom with default attributes.
 *
 * Returns: (transfer full): A new #LpKingdom
 */
LpKingdom *
lp_kingdom_new (const gchar *id,
                const gchar *name);

/**
 * lp_kingdom_new_full:
 * @id: unique identifier
 * @name: display name
 * @stability: initial stability (0-100)
 * @prosperity: initial prosperity (0-100)
 * @military: initial military strength (0-100)
 * @culture: initial culture level (0-100)
 * @tolerance: initial tolerance (0-100)
 *
 * Creates a new kingdom with specified attributes.
 *
 * Returns: (transfer full): A new #LpKingdom
 */
LpKingdom *
lp_kingdom_new_full (const gchar *id,
                     const gchar *name,
                     gint         stability,
                     gint         prosperity,
                     gint         military,
                     gint         culture,
                     gint         tolerance);

/* ==========================================================================
 * Property Accessors
 * ========================================================================== */

/**
 * lp_kingdom_get_id:
 * @self: an #LpKingdom
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The kingdom ID
 */
const gchar *
lp_kingdom_get_id (LpKingdom *self);

/**
 * lp_kingdom_get_name:
 * @self: an #LpKingdom
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The kingdom name
 */
const gchar *
lp_kingdom_get_name (LpKingdom *self);

/**
 * lp_kingdom_set_name:
 * @self: an #LpKingdom
 * @name: the new name
 *
 * Sets the display name.
 */
void
lp_kingdom_set_name (LpKingdom   *self,
                     const gchar *name);

/* Core attributes (0-100) */

/**
 * lp_kingdom_get_stability:
 * @self: an #LpKingdom
 *
 * Gets the stability attribute. Low stability increases collapse risk.
 *
 * Returns: Stability value (0-100)
 */
gint
lp_kingdom_get_stability (LpKingdom *self);

/**
 * lp_kingdom_set_stability:
 * @self: an #LpKingdom
 * @value: stability value (0-100)
 *
 * Sets the stability attribute.
 */
void
lp_kingdom_set_stability (LpKingdom *self,
                          gint       value);

/**
 * lp_kingdom_get_prosperity:
 * @self: an #LpKingdom
 *
 * Gets the prosperity attribute. Affects economic output.
 *
 * Returns: Prosperity value (0-100)
 */
gint
lp_kingdom_get_prosperity (LpKingdom *self);

/**
 * lp_kingdom_set_prosperity:
 * @self: an #LpKingdom
 * @value: prosperity value (0-100)
 *
 * Sets the prosperity attribute.
 */
void
lp_kingdom_set_prosperity (LpKingdom *self,
                           gint       value);

/**
 * lp_kingdom_get_military:
 * @self: an #LpKingdom
 *
 * Gets the military attribute. Affects war outcomes.
 *
 * Returns: Military value (0-100)
 */
gint
lp_kingdom_get_military (LpKingdom *self);

/**
 * lp_kingdom_set_military:
 * @self: an #LpKingdom
 * @value: military value (0-100)
 *
 * Sets the military attribute.
 */
void
lp_kingdom_set_military (LpKingdom *self,
                         gint       value);

/**
 * lp_kingdom_get_culture:
 * @self: an #LpKingdom
 *
 * Gets the culture attribute. High culture resists change.
 *
 * Returns: Culture value (0-100)
 */
gint
lp_kingdom_get_culture (LpKingdom *self);

/**
 * lp_kingdom_set_culture:
 * @self: an #LpKingdom
 * @value: culture value (0-100)
 *
 * Sets the culture attribute.
 */
void
lp_kingdom_set_culture (LpKingdom *self,
                        gint       value);

/**
 * lp_kingdom_get_tolerance:
 * @self: an #LpKingdom
 *
 * Gets the tolerance attribute. Low tolerance triggers crusades.
 *
 * Returns: Tolerance value (0-100)
 */
gint
lp_kingdom_get_tolerance (LpKingdom *self);

/**
 * lp_kingdom_set_tolerance:
 * @self: an #LpKingdom
 * @value: tolerance value (0-100)
 *
 * Sets the tolerance attribute.
 */
void
lp_kingdom_set_tolerance (LpKingdom *self,
                          gint       value);

/* State properties */

/**
 * lp_kingdom_get_ruler_name:
 * @self: an #LpKingdom
 *
 * Gets the current ruler's name.
 *
 * Returns: (transfer none): The ruler name
 */
const gchar *
lp_kingdom_get_ruler_name (LpKingdom *self);

/**
 * lp_kingdom_set_ruler_name:
 * @self: an #LpKingdom
 * @name: the ruler's name
 *
 * Sets the current ruler's name.
 */
void
lp_kingdom_set_ruler_name (LpKingdom   *self,
                           const gchar *name);

/**
 * lp_kingdom_get_dynasty_years:
 * @self: an #LpKingdom
 *
 * Gets the years the current dynasty has ruled.
 *
 * Returns: Dynasty years
 */
guint
lp_kingdom_get_dynasty_years (LpKingdom *self);

/**
 * lp_kingdom_set_dynasty_years:
 * @self: an #LpKingdom
 * @years: dynasty years
 *
 * Sets the dynasty years.
 */
void
lp_kingdom_set_dynasty_years (LpKingdom *self,
                              guint      years);

/**
 * lp_kingdom_get_is_collapsed:
 * @self: an #LpKingdom
 *
 * Gets whether the kingdom has collapsed.
 *
 * Returns: %TRUE if collapsed
 */
gboolean
lp_kingdom_get_is_collapsed (LpKingdom *self);

/**
 * lp_kingdom_get_at_war_with_id:
 * @self: an #LpKingdom
 *
 * Gets the ID of the kingdom this one is at war with.
 *
 * Returns: (transfer none) (nullable): Enemy kingdom ID, or %NULL
 */
const gchar *
lp_kingdom_get_at_war_with_id (LpKingdom *self);

/* ==========================================================================
 * Region Management
 * ========================================================================== */

/**
 * lp_kingdom_get_region_ids:
 * @self: an #LpKingdom
 *
 * Gets the IDs of regions this kingdom owns.
 *
 * Returns: (transfer none) (element-type utf8): Array of region IDs
 */
GPtrArray *
lp_kingdom_get_region_ids (LpKingdom *self);

/**
 * lp_kingdom_add_region:
 * @self: an #LpKingdom
 * @region_id: ID of the region to add
 *
 * Adds a region to this kingdom's control.
 */
void
lp_kingdom_add_region (LpKingdom   *self,
                       const gchar *region_id);

/**
 * lp_kingdom_remove_region:
 * @self: an #LpKingdom
 * @region_id: ID of the region to remove
 *
 * Removes a region from this kingdom's control.
 *
 * Returns: %TRUE if the region was removed
 */
gboolean
lp_kingdom_remove_region (LpKingdom   *self,
                          const gchar *region_id);

/**
 * lp_kingdom_owns_region:
 * @self: an #LpKingdom
 * @region_id: ID of the region to check
 *
 * Checks if this kingdom owns the specified region.
 *
 * Returns: %TRUE if the kingdom owns this region
 */
gboolean
lp_kingdom_owns_region (LpKingdom   *self,
                        const gchar *region_id);

/* ==========================================================================
 * Diplomatic Relations
 * ========================================================================== */

/**
 * lp_kingdom_get_relation:
 * @self: an #LpKingdom
 * @other_kingdom_id: ID of the other kingdom
 *
 * Gets the diplomatic relation with another kingdom.
 *
 * Returns: The relation type (defaults to NEUTRAL)
 */
LpKingdomRelation
lp_kingdom_get_relation (LpKingdom   *self,
                         const gchar *other_kingdom_id);

/**
 * lp_kingdom_set_relation:
 * @self: an #LpKingdom
 * @other_kingdom_id: ID of the other kingdom
 * @relation: the new relation
 *
 * Sets the diplomatic relation with another kingdom.
 */
void
lp_kingdom_set_relation (LpKingdom        *self,
                         const gchar      *other_kingdom_id,
                         LpKingdomRelation relation);

/* ==========================================================================
 * Yearly Tick and Rolls
 * ========================================================================== */

/**
 * lp_kingdom_tick_year:
 * @self: an #LpKingdom
 *
 * Processes one year of kingdom simulation.
 * Applies attribute drift, increments dynasty years, etc.
 */
void
lp_kingdom_tick_year (LpKingdom *self);

/**
 * lp_kingdom_roll_collapse:
 * @self: an #LpKingdom
 *
 * Rolls to see if the kingdom collapses (based on stability).
 * If collapse occurs, emits ::collapsed signal.
 *
 * Returns: %TRUE if the kingdom collapsed
 */
gboolean
lp_kingdom_roll_collapse (LpKingdom *self);

/**
 * lp_kingdom_roll_war:
 * @self: an #LpKingdom
 * @target_kingdom_id: ID of potential target
 *
 * Rolls to see if the kingdom declares war.
 * Based on military and relationship.
 *
 * Returns: %TRUE if war was declared
 */
gboolean
lp_kingdom_roll_war (LpKingdom   *self,
                     const gchar *target_kingdom_id);

/**
 * lp_kingdom_roll_crusade:
 * @self: an #LpKingdom
 * @exposure_detected: whether undead activity was detected
 *
 * Rolls to see if the kingdom launches a crusade.
 * Based on tolerance and detected exposure.
 *
 * Returns: %TRUE if a crusade was launched
 */
gboolean
lp_kingdom_roll_crusade (LpKingdom *self,
                         gboolean   exposure_detected);

/**
 * lp_kingdom_end_war:
 * @self: an #LpKingdom
 * @victory: whether this kingdom won
 *
 * Ends the current war and applies consequences.
 */
void
lp_kingdom_end_war (LpKingdom *self,
                    gboolean   victory);

/**
 * lp_kingdom_collapse:
 * @self: an #LpKingdom
 *
 * Forces the kingdom to collapse.
 * Emits ::collapsed signal.
 */
void
lp_kingdom_collapse (LpKingdom *self);

G_END_DECLS

#endif /* LP_KINGDOM_H */
