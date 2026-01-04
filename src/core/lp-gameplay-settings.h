/* lp-gameplay-settings.h - Game-specific settings group
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpGameplaySettings manages gameplay-specific settings that are unique
 * to Lich's Portfolio. These include autosave configuration and event
 * notification preferences.
 */

#ifndef LP_GAMEPLAY_SETTINGS_H
#define LP_GAMEPLAY_SETTINGS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_GAMEPLAY_SETTINGS (lp_gameplay_settings_get_type ())

G_DECLARE_FINAL_TYPE (LpGameplaySettings, lp_gameplay_settings,
                      LP, GAMEPLAY_SETTINGS, LrgSettingsGroup)

/**
 * lp_gameplay_settings_new:
 *
 * Creates a new #LpGameplaySettings with default values.
 *
 * Returns: (transfer full): A new #LpGameplaySettings
 */
LpGameplaySettings *
lp_gameplay_settings_new (void);

/* ==========================================================================
 * Autosave Settings
 * ========================================================================== */

/**
 * lp_gameplay_settings_get_autosave_enabled:
 * @self: an #LpGameplaySettings
 *
 * Gets whether autosave is enabled.
 *
 * Returns: %TRUE if autosave is enabled
 */
gboolean
lp_gameplay_settings_get_autosave_enabled (LpGameplaySettings *self);

/**
 * lp_gameplay_settings_set_autosave_enabled:
 * @self: an #LpGameplaySettings
 * @enabled: whether to enable autosave
 *
 * Sets whether autosave is enabled.
 */
void
lp_gameplay_settings_set_autosave_enabled (LpGameplaySettings *self,
                                           gboolean            enabled);

/**
 * lp_gameplay_settings_get_autosave_interval:
 * @self: an #LpGameplaySettings
 *
 * Gets the autosave interval in minutes.
 *
 * Returns: Autosave interval in minutes
 */
guint
lp_gameplay_settings_get_autosave_interval (LpGameplaySettings *self);

/**
 * lp_gameplay_settings_set_autosave_interval:
 * @self: an #LpGameplaySettings
 * @minutes: autosave interval in minutes (1-60)
 *
 * Sets the autosave interval in minutes.
 */
void
lp_gameplay_settings_set_autosave_interval (LpGameplaySettings *self,
                                            guint               minutes);

/* ==========================================================================
 * Event Settings
 * ========================================================================== */

/**
 * lp_gameplay_settings_get_pause_on_events:
 * @self: an #LpGameplaySettings
 *
 * Gets whether the game pauses on major events.
 *
 * Returns: %TRUE if game pauses on events
 */
gboolean
lp_gameplay_settings_get_pause_on_events (LpGameplaySettings *self);

/**
 * lp_gameplay_settings_set_pause_on_events:
 * @self: an #LpGameplaySettings
 * @pause: whether to pause on events
 *
 * Sets whether the game pauses on major events.
 */
void
lp_gameplay_settings_set_pause_on_events (LpGameplaySettings *self,
                                          gboolean            pause);

/**
 * lp_gameplay_settings_get_show_notifications:
 * @self: an #LpGameplaySettings
 *
 * Gets whether to show event notifications.
 *
 * Returns: %TRUE if notifications are shown
 */
gboolean
lp_gameplay_settings_get_show_notifications (LpGameplaySettings *self);

/**
 * lp_gameplay_settings_set_show_notifications:
 * @self: an #LpGameplaySettings
 * @show: whether to show notifications
 *
 * Sets whether to show event notifications.
 */
void
lp_gameplay_settings_set_show_notifications (LpGameplaySettings *self,
                                             gboolean            show);

G_END_DECLS

#endif /* LP_GAMEPLAY_SETTINGS_H */
