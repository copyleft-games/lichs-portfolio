/* lp-achievement-manager.h - Achievement Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tracks and unlocks achievements based on player actions.
 * Integrates with Steam achievements when available.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_ACHIEVEMENT_MANAGER_H
#define LP_ACHIEVEMENT_MANAGER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_ACHIEVEMENT_MANAGER (lp_achievement_manager_get_type ())

G_DECLARE_FINAL_TYPE (LpAchievementManager, lp_achievement_manager,
                      LP, ACHIEVEMENT_MANAGER, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_achievement_manager_get_default:
 *
 * Gets the default achievement manager instance.
 * Creates it if it doesn't exist.
 *
 * Returns: (transfer none): The default #LpAchievementManager instance
 */
LpAchievementManager *
lp_achievement_manager_get_default (void);

/* ==========================================================================
 * Achievement Tracking (Skeleton - Phase 5+)
 * ========================================================================== */

/**
 * lp_achievement_manager_unlock:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID to unlock
 *
 * Unlocks an achievement if not already unlocked.
 *
 * Returns: %TRUE if newly unlocked
 */
gboolean
lp_achievement_manager_unlock (LpAchievementManager *self,
                               const gchar          *achievement_id);

/**
 * lp_achievement_manager_is_unlocked:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID to check
 *
 * Checks if an achievement is unlocked.
 *
 * Returns: %TRUE if the achievement is unlocked
 */
gboolean
lp_achievement_manager_is_unlocked (LpAchievementManager *self,
                                    const gchar          *achievement_id);

/**
 * lp_achievement_manager_get_unlocked_count:
 * @self: an #LpAchievementManager
 *
 * Gets the number of unlocked achievements.
 *
 * Returns: Number of unlocked achievements
 */
guint
lp_achievement_manager_get_unlocked_count (LpAchievementManager *self);

/**
 * lp_achievement_manager_get_total_count:
 * @self: an #LpAchievementManager
 *
 * Gets the total number of achievements.
 *
 * Returns: Total number of achievements
 */
guint
lp_achievement_manager_get_total_count (LpAchievementManager *self);

/**
 * lp_achievement_manager_get_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets the progress towards an achievement (0-100).
 *
 * Returns: Progress percentage
 */
guint
lp_achievement_manager_get_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id);

/**
 * lp_achievement_manager_set_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 * @progress: the progress value (0-100)
 *
 * Sets the progress towards an achievement.
 * Automatically unlocks at 100%.
 */
void
lp_achievement_manager_set_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id,
                                     guint                 progress);

/* ==========================================================================
 * Game Event Hooks (Skeleton - Phase 5+)
 * ========================================================================== */

/**
 * lp_achievement_manager_on_gold_earned:
 * @self: an #LpAchievementManager
 * @amount: amount of gold earned
 *
 * Called when gold is earned. Updates relevant achievements.
 */
void
lp_achievement_manager_on_gold_earned (LpAchievementManager *self,
                                       gdouble               amount);

/**
 * lp_achievement_manager_on_year_passed:
 * @self: an #LpAchievementManager
 * @year: the new year
 *
 * Called when a year passes. Updates time-based achievements.
 */
void
lp_achievement_manager_on_year_passed (LpAchievementManager *self,
                                       guint64               year);

/**
 * lp_achievement_manager_on_prestige:
 * @self: an #LpAchievementManager
 * @points_earned: phylactery points earned
 *
 * Called when player prestiges. Updates prestige achievements.
 */
void
lp_achievement_manager_on_prestige (LpAchievementManager *self,
                                    guint64               points_earned);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_achievement_manager_reset:
 * @self: an #LpAchievementManager
 *
 * Resets all achievement progress.
 * WARNING: This clears all achievements. Use with caution.
 */
void
lp_achievement_manager_reset (LpAchievementManager *self);

G_END_DECLS

#endif /* LP_ACHIEVEMENT_MANAGER_H */
