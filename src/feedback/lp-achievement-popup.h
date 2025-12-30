/* lp-achievement-popup.h - Achievement Notification Popup
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Celebration popup when achievements unlock - slides in from top-right.
 */

#ifndef LP_ACHIEVEMENT_POPUP_H
#define LP_ACHIEVEMENT_POPUP_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_ACHIEVEMENT_POPUP (lp_achievement_popup_get_type ())
G_DECLARE_FINAL_TYPE (LpAchievementPopup, lp_achievement_popup, LP, ACHIEVEMENT_POPUP, LrgContainer)

/**
 * lp_achievement_popup_new:
 *
 * Creates a new achievement popup widget.
 *
 * Returns: (transfer full): a new #LpAchievementPopup
 */
LpAchievementPopup *lp_achievement_popup_new (void);

/**
 * lp_achievement_popup_show:
 * @self: a #LpAchievementPopup
 * @name: the achievement name
 * @description: the achievement description
 *
 * Shows the achievement popup with the given name and description.
 * The popup slides in from the right and auto-dismisses after timeout.
 */
void lp_achievement_popup_show (LpAchievementPopup *self,
                                const gchar        *name,
                                const gchar        *description);

/**
 * lp_achievement_popup_dismiss:
 * @self: a #LpAchievementPopup
 *
 * Immediately dismisses the achievement popup.
 */
void lp_achievement_popup_dismiss (LpAchievementPopup *self);

/**
 * lp_achievement_popup_is_visible:
 * @self: a #LpAchievementPopup
 *
 * Checks if the popup is currently visible.
 *
 * Returns: %TRUE if visible
 */
gboolean lp_achievement_popup_is_visible (LpAchievementPopup *self);

/**
 * lp_achievement_popup_get_name:
 * @self: a #LpAchievementPopup
 *
 * Gets the achievement name being displayed.
 *
 * Returns: (transfer none) (nullable): the achievement name
 */
const gchar *lp_achievement_popup_get_name (LpAchievementPopup *self);

/**
 * lp_achievement_popup_get_description:
 * @self: a #LpAchievementPopup
 *
 * Gets the achievement description being displayed.
 *
 * Returns: (transfer none) (nullable): the achievement description
 */
const gchar *lp_achievement_popup_get_description (LpAchievementPopup *self);

/**
 * lp_achievement_popup_get_auto_dismiss_time:
 * @self: a #LpAchievementPopup
 *
 * Gets the auto-dismiss timeout in seconds.
 *
 * Returns: timeout in seconds
 */
gfloat lp_achievement_popup_get_auto_dismiss_time (LpAchievementPopup *self);

/**
 * lp_achievement_popup_set_auto_dismiss_time:
 * @self: a #LpAchievementPopup
 * @seconds: timeout in seconds (0 to disable auto-dismiss)
 *
 * Sets the auto-dismiss timeout.
 */
void lp_achievement_popup_set_auto_dismiss_time (LpAchievementPopup *self,
                                                 gfloat              seconds);

/**
 * lp_achievement_popup_update:
 * @self: a #LpAchievementPopup
 * @delta: time elapsed since last update in seconds
 *
 * Updates the popup animation and auto-dismiss timer.
 */
void lp_achievement_popup_update (LpAchievementPopup *self,
                                  gfloat              delta);

G_END_DECLS

#endif /* LP_ACHIEVEMENT_POPUP_H */
