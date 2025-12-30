/* lp-phylactery.h - Upgrade Tree System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The Phylactery is the lich's upgrade tree / tech tree.
 * Upgrades are purchased with phylactery points (earned via prestige)
 * and provide permanent bonuses.
 *
 * Five upgrade categories organized into LrgUnlockTree structures:
 * - Temporal Mastery: Longer slumber, time efficiency
 * - Network Expansion: More agents, family/cult mechanics
 * - Divination: Better predictions, early warnings
 * - Resilience: Survive disasters, faster recovery
 * - Dark Arts: Unlock dark investments (hidden)
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_PHYLACTERY_H
#define LP_PHYLACTERY_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

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

/**
 * lp_phylactery_get_level:
 * @self: an #LpPhylactery
 *
 * Gets the phylactery level (derived from total upgrades purchased).
 *
 * Returns: Current level
 */
guint
lp_phylactery_get_level (LpPhylactery *self);

/* ==========================================================================
 * Upgrade Tree Access
 * ========================================================================== */

/**
 * lp_phylactery_get_upgrade_tree:
 * @self: an #LpPhylactery
 * @category: Which upgrade category tree to get
 *
 * Gets the unlock tree for a specific upgrade category.
 *
 * Returns: (transfer none): The unlock tree
 */
LrgUnlockTree *
lp_phylactery_get_upgrade_tree (LpPhylactery      *self,
                                LpUpgradeCategory  category);

/**
 * lp_phylactery_get_upgrade_count:
 * @self: an #LpPhylactery
 *
 * Gets the total number of purchased upgrades across all categories.
 *
 * Returns: Number of purchased upgrades
 */
guint
lp_phylactery_get_upgrade_count (LpPhylactery *self);

/**
 * lp_phylactery_get_category_upgrade_count:
 * @self: an #LpPhylactery
 * @category: Which category to count
 *
 * Gets the number of purchased upgrades in a specific category.
 *
 * Returns: Number of upgrades in category
 */
guint
lp_phylactery_get_category_upgrade_count (LpPhylactery      *self,
                                          LpUpgradeCategory  category);

/**
 * lp_phylactery_has_upgrade:
 * @self: an #LpPhylactery
 * @upgrade_id: the upgrade ID to check
 *
 * Checks if an upgrade has been purchased (searches all categories).
 *
 * Returns: %TRUE if the upgrade is owned
 */
gboolean
lp_phylactery_has_upgrade (LpPhylactery *self,
                           const gchar  *upgrade_id);

/**
 * lp_phylactery_has_category_upgrade:
 * @self: an #LpPhylactery
 * @category: Which category to check
 * @upgrade_id: the upgrade ID to check
 *
 * Checks if an upgrade has been purchased in a specific category.
 *
 * Returns: %TRUE if the upgrade is owned
 */
gboolean
lp_phylactery_has_category_upgrade (LpPhylactery      *self,
                                    LpUpgradeCategory  category,
                                    const gchar       *upgrade_id);

/**
 * lp_phylactery_can_purchase_upgrade:
 * @self: an #LpPhylactery
 * @category: Which category the upgrade is in
 * @upgrade_id: the upgrade ID to check
 *
 * Checks if an upgrade can be purchased (has enough points and prerequisites).
 *
 * Returns: %TRUE if the upgrade can be purchased
 */
gboolean
lp_phylactery_can_purchase_upgrade (LpPhylactery      *self,
                                    LpUpgradeCategory  category,
                                    const gchar       *upgrade_id);

/**
 * lp_phylactery_purchase_upgrade:
 * @self: an #LpPhylactery
 * @category: Which category the upgrade is in
 * @upgrade_id: the upgrade ID to purchase
 *
 * Purchases an upgrade if requirements are met.
 *
 * Returns: %TRUE if successfully purchased
 */
gboolean
lp_phylactery_purchase_upgrade (LpPhylactery      *self,
                                LpUpgradeCategory  category,
                                const gchar       *upgrade_id);

/**
 * lp_phylactery_get_upgrade_cost:
 * @self: an #LpPhylactery
 * @category: Which category the upgrade is in
 * @upgrade_id: the upgrade ID to check
 *
 * Gets the cost of a specific upgrade.
 *
 * Returns: Upgrade cost in points, or 0 if not found
 */
guint64
lp_phylactery_get_upgrade_cost (LpPhylactery      *self,
                                LpUpgradeCategory  category,
                                const gchar       *upgrade_id);

/* ==========================================================================
 * Bonus Calculation - Temporal Mastery
 * ========================================================================== */

/**
 * lp_phylactery_get_max_slumber_years:
 * @self: an #LpPhylactery
 *
 * Gets the maximum slumber duration in years.
 * Base: 100 years. Temporal upgrades increase this.
 *
 * Returns: Maximum slumber years
 */
guint
lp_phylactery_get_max_slumber_years (LpPhylactery *self);

/**
 * lp_phylactery_get_time_efficiency_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to time-based income calculations.
 * Higher efficiency means more income per year of slumber.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_phylactery_get_time_efficiency_bonus (LpPhylactery *self);

/* ==========================================================================
 * Bonus Calculation - Network Expansion
 * ========================================================================== */

/**
 * lp_phylactery_get_max_agents:
 * @self: an #LpPhylactery
 *
 * Gets the maximum number of agents.
 * Base: 3 agents. Network upgrades increase this.
 *
 * Returns: Maximum agent count
 */
guint
lp_phylactery_get_max_agents (LpPhylactery *self);

/**
 * lp_phylactery_has_family_agents:
 * @self: an #LpPhylactery
 *
 * Checks if family agents are unlocked.
 *
 * Returns: %TRUE if family agents are available
 */
gboolean
lp_phylactery_has_family_agents (LpPhylactery *self);

/**
 * lp_phylactery_has_cult_agents:
 * @self: an #LpPhylactery
 *
 * Checks if cult agents are unlocked.
 *
 * Returns: %TRUE if cult agents are available
 */
gboolean
lp_phylactery_has_cult_agents (LpPhylactery *self);

/* ==========================================================================
 * Bonus Calculation - Divination
 * ========================================================================== */

/**
 * lp_phylactery_get_prediction_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to event prediction accuracy.
 *
 * Returns: Bonus percentage (0-100)
 */
guint
lp_phylactery_get_prediction_bonus (LpPhylactery *self);

/**
 * lp_phylactery_get_warning_years:
 * @self: an #LpPhylactery
 *
 * Gets how many years of warning before major events.
 *
 * Returns: Warning years (0 = no warning)
 */
guint
lp_phylactery_get_warning_years (LpPhylactery *self);

/* ==========================================================================
 * Bonus Calculation - Resilience
 * ========================================================================== */

/**
 * lp_phylactery_get_disaster_survival_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to surviving disasters without loss.
 *
 * Returns: Survival bonus percentage (0-100)
 */
guint
lp_phylactery_get_disaster_survival_bonus (LpPhylactery *self);

/**
 * lp_phylactery_get_recovery_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to recovery speed after disasters.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_phylactery_get_recovery_bonus (LpPhylactery *self);

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
 * Bonus Calculation - Dark Arts
 * ========================================================================== */

/**
 * lp_phylactery_has_dark_investments:
 * @self: an #LpPhylactery
 *
 * Checks if dark investment class is unlocked.
 *
 * Returns: %TRUE if dark investments are available
 */
gboolean
lp_phylactery_has_dark_investments (LpPhylactery *self);

/**
 * lp_phylactery_has_bound_agents:
 * @self: an #LpPhylactery
 *
 * Checks if bound (undead) agents are unlocked.
 *
 * Returns: %TRUE if bound agents are available
 */
gboolean
lp_phylactery_has_bound_agents (LpPhylactery *self);

/**
 * lp_phylactery_get_dark_income_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to dark investment income.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_phylactery_get_dark_income_bonus (LpPhylactery *self);

/* ==========================================================================
 * Legacy Bonus Calculation (for backwards compatibility)
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

/**
 * lp_phylactery_reset:
 * @self: an #LpPhylactery
 *
 * Full reset including points (for new game).
 */
void
lp_phylactery_reset (LpPhylactery *self);

G_END_DECLS

#endif /* LP_PHYLACTERY_H */
