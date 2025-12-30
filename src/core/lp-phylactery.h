/* lp-phylactery.h - Upgrade Tree System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The Phylactery is the lich's upgrade tree / tech tree.
 * Upgrades are purchased with phylactery points (earned via prestige)
 * and provide permanent bonuses.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_PHYLACTERY_H
#define LP_PHYLACTERY_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_PHYLACTERY (lp_phylactery_get_type ())

G_DECLARE_FINAL_TYPE (LpPhylactery, lp_phylactery, LP, PHYLACTERY, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_phylactery_new:
 *
 * Creates a new phylactery (upgrade tree).
 *
 * Returns: (transfer full): A new #LpPhylactery
 */
LpPhylactery *
lp_phylactery_new (void);

/* ==========================================================================
 * Points Management
 * ========================================================================== */

/**
 * lp_phylactery_get_points:
 * @self: an #LpPhylactery
 *
 * Gets the number of available phylactery points.
 *
 * Returns: Available points
 */
guint64
lp_phylactery_get_points (LpPhylactery *self);

/**
 * lp_phylactery_get_total_points_earned:
 * @self: an #LpPhylactery
 *
 * Gets the total points ever earned (includes spent points).
 *
 * Returns: Total points earned
 */
guint64
lp_phylactery_get_total_points_earned (LpPhylactery *self);

/**
 * lp_phylactery_add_points:
 * @self: an #LpPhylactery
 * @points: number of points to add
 *
 * Adds phylactery points. Called after prestige.
 */
void
lp_phylactery_add_points (LpPhylactery *self,
                          guint64       points);

/* ==========================================================================
 * Upgrade Management (Skeleton - Phase 7+)
 * ========================================================================== */

/**
 * lp_phylactery_get_upgrade_count:
 * @self: an #LpPhylactery
 *
 * Gets the number of purchased upgrades.
 *
 * Returns: Number of purchased upgrades
 */
guint
lp_phylactery_get_upgrade_count (LpPhylactery *self);

/**
 * lp_phylactery_has_upgrade:
 * @self: an #LpPhylactery
 * @upgrade_id: the upgrade ID to check
 *
 * Checks if an upgrade has been purchased.
 *
 * Returns: %TRUE if the upgrade is owned
 */
gboolean
lp_phylactery_has_upgrade (LpPhylactery *self,
                           const gchar  *upgrade_id);

/**
 * lp_phylactery_purchase_upgrade:
 * @self: an #LpPhylactery
 * @upgrade_id: the upgrade ID to purchase
 *
 * Purchases an upgrade if requirements are met.
 *
 * Note: Skeleton implementation - always returns FALSE in Phase 1.
 *
 * Returns: %TRUE if successfully purchased
 */
gboolean
lp_phylactery_purchase_upgrade (LpPhylactery *self,
                                const gchar  *upgrade_id);

/* ==========================================================================
 * Bonus Calculation (Skeleton - Phase 7+)
 * ========================================================================== */

/**
 * lp_phylactery_get_starting_gold_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to starting gold from upgrades.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_phylactery_get_starting_gold_bonus (LpPhylactery *self);

/**
 * lp_phylactery_get_income_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to all income from upgrades.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_phylactery_get_income_bonus (LpPhylactery *self);

/**
 * lp_phylactery_get_exposure_decay_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to exposure decay from upgrades.
 *
 * Returns: Flat bonus to decay rate
 */
guint
lp_phylactery_get_exposure_decay_bonus (LpPhylactery *self);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_phylactery_reset_upgrades:
 * @self: an #LpPhylactery
 *
 * Resets all upgrades and refunds points.
 * Used for full game reset (NOT prestige - prestige keeps upgrades).
 */
void
lp_phylactery_reset_upgrades (LpPhylactery *self);

G_END_DECLS

#endif /* LP_PHYLACTERY_H */
