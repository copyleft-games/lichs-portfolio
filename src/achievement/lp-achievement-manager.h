/* lp-achievement-manager.h - Achievement Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tracks and unlocks achievements based on player actions.
 * Wraps LrgAchievementManager from libregnum for local tracking.
 * Integrates with LpSteamBridge for optional Steam sync.
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
 * Initialization
 * ========================================================================== */

/**
 * lp_achievement_manager_load_definitions:
 * @self: an #LpAchievementManager
 * @data_dir: path to the data/achievements/ directory
 * @error: (nullable): return location for error
 *
 * Loads achievement definitions from YAML files in the given directory.
 * Should be called once at startup after the manager is created.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_achievement_manager_load_definitions (LpAchievementManager  *self,
                                         const gchar           *data_dir,
                                         GError               **error);

/**
 * lp_achievement_manager_set_popup:
 * @self: an #LpAchievementManager
 * @popup: (nullable): the #LpAchievementPopup to use for notifications
 *
 * Sets the popup widget to use for achievement unlock notifications.
 * Pass %NULL to disable popup notifications.
 */
void
lp_achievement_manager_set_popup (LpAchievementManager *self,
                                  LpAchievementPopup   *popup);

/**
 * lp_achievement_manager_get_popup:
 * @self: an #LpAchievementManager
 *
 * Gets the current popup widget.
 *
 * Returns: (transfer none) (nullable): the #LpAchievementPopup, or %NULL
 */
LpAchievementPopup *
lp_achievement_manager_get_popup (LpAchievementManager *self);

/* ==========================================================================
 * Achievement Access
 * ========================================================================== */

/**
 * lp_achievement_manager_get_achievement:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets an achievement by ID.
 *
 * Returns: (transfer none) (nullable): the #LrgAchievement, or %NULL
 */
LrgAchievement *
lp_achievement_manager_get_achievement (LpAchievementManager *self,
                                        const gchar          *achievement_id);

/**
 * lp_achievement_manager_get_all:
 * @self: an #LpAchievementManager
 *
 * Gets all registered achievements.
 *
 * Returns: (transfer container) (element-type LrgAchievement): list of achievements
 */
GList *
lp_achievement_manager_get_all (LpAchievementManager *self);

/* ==========================================================================
 * Achievement Tracking
 * ========================================================================== */

/**
 * lp_achievement_manager_unlock:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID to unlock
 *
 * Unlocks an achievement if not already unlocked.
 * Triggers popup notification and Steam sync if configured.
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
 * lp_achievement_manager_get_completion_percentage:
 * @self: an #LpAchievementManager
 *
 * Gets the completion percentage (0.0 to 1.0).
 *
 * Returns: Completion percentage
 */
gdouble
lp_achievement_manager_get_completion_percentage (LpAchievementManager *self);

/* ==========================================================================
 * Progress Tracking
 * ========================================================================== */

/**
 * lp_achievement_manager_get_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets the raw progress value towards an achievement.
 *
 * Returns: Current progress value
 */
gint64
lp_achievement_manager_get_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id);

/**
 * lp_achievement_manager_get_progress_percentage:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets the progress percentage (0-100) towards an achievement.
 *
 * Returns: Progress percentage
 */
guint
lp_achievement_manager_get_progress_percentage (LpAchievementManager *self,
                                                const gchar          *achievement_id);

/**
 * lp_achievement_manager_set_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 * @value: the progress value
 *
 * Sets the progress towards an achievement.
 * Automatically unlocks when target is reached.
 */
void
lp_achievement_manager_set_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id,
                                     gint64                value);

/**
 * lp_achievement_manager_increment_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 * @amount: the amount to add
 *
 * Increments progress towards an achievement.
 */
void
lp_achievement_manager_increment_progress (LpAchievementManager *self,
                                           const gchar          *achievement_id,
                                           gint64                amount);

/* ==========================================================================
 * Statistics (for complex achievement tracking)
 * ========================================================================== */

/**
 * lp_achievement_manager_set_stat:
 * @self: an #LpAchievementManager
 * @name: statistic name
 * @value: the value
 *
 * Sets a tracked statistic value.
 */
void
lp_achievement_manager_set_stat (LpAchievementManager *self,
                                 const gchar          *name,
                                 gint64                value);

/**
 * lp_achievement_manager_get_stat:
 * @self: an #LpAchievementManager
 * @name: statistic name
 *
 * Gets a tracked statistic value.
 *
 * Returns: The value, or 0 if not found
 */
gint64
lp_achievement_manager_get_stat (LpAchievementManager *self,
                                 const gchar          *name);

/**
 * lp_achievement_manager_increment_stat:
 * @self: an #LpAchievementManager
 * @name: statistic name
 * @amount: the amount to add
 *
 * Increments a tracked statistic.
 */
void
lp_achievement_manager_increment_stat (LpAchievementManager *self,
                                       const gchar          *name,
                                       gint64                amount);

/* ==========================================================================
 * Game Event Hooks
 * ========================================================================== */

/**
 * lp_achievement_manager_on_gold_changed:
 * @self: an #LpAchievementManager
 * @total_gold: current total gold amount
 *
 * Called when gold balance changes. Updates wealth-based achievements.
 */
void
lp_achievement_manager_on_gold_changed (LpAchievementManager *self,
                                        gdouble               total_gold);

/**
 * lp_achievement_manager_on_slumber_complete:
 * @self: an #LpAchievementManager
 * @years_slumbered: number of years slumbered
 *
 * Called when a slumber cycle completes. Updates time-based achievements.
 */
void
lp_achievement_manager_on_slumber_complete (LpAchievementManager *self,
                                            guint                 years_slumbered);

/**
 * lp_achievement_manager_on_family_succession:
 * @self: an #LpAchievementManager
 * @generation: the new generation number
 *
 * Called when an agent family has a succession. Updates dynasty achievements.
 */
void
lp_achievement_manager_on_family_succession (LpAchievementManager *self,
                                             guint                 generation);

/**
 * lp_achievement_manager_on_investment_held:
 * @self: an #LpAchievementManager
 * @investment_id: the investment ID
 * @years_held: number of years the investment has been held
 *
 * Called during slumber to track long-term investment holdings.
 */
void
lp_achievement_manager_on_investment_held (LpAchievementManager *self,
                                           const gchar          *investment_id,
                                           guint                 years_held);

/**
 * lp_achievement_manager_on_dark_unlock:
 * @self: an #LpAchievementManager
 *
 * Called when dark investments are unlocked.
 */
void
lp_achievement_manager_on_dark_unlock (LpAchievementManager *self);

/**
 * lp_achievement_manager_on_soul_trade:
 * @self: an #LpAchievementManager
 *
 * Called when a soul trade is completed.
 */
void
lp_achievement_manager_on_soul_trade (LpAchievementManager *self);

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

/**
 * lp_achievement_manager_on_kingdom_debt_owned:
 * @self: an #LpAchievementManager
 * @kingdom_id: the kingdom ID
 * @debt_percentage: percentage of kingdom's debt owned (0.0-1.0)
 *
 * Called when player's ownership of a kingdom's debt changes.
 */
void
lp_achievement_manager_on_kingdom_debt_owned (LpAchievementManager *self,
                                              const gchar          *kingdom_id,
                                              gdouble               debt_percentage);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_achievement_manager_reset:
 * @self: an #LpAchievementManager
 *
 * Resets all achievement progress and unlocks.
 * WARNING: This clears all achievements. Use with caution.
 */
void
lp_achievement_manager_reset (LpAchievementManager *self);

G_END_DECLS

#endif /* LP_ACHIEVEMENT_MANAGER_H */
