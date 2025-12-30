/* lp-settings-manager.h - Game Settings Management
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Manages game settings persistence and access.
 *
 * Settings are organized into groups:
 * - Graphics: resolution, fullscreen, VSync
 * - Audio: volume levels, mute state
 * - Gameplay: autosave interval, notifications
 * - Accessibility: UI scale, colorblind modes
 */

#ifndef LP_SETTINGS_MANAGER_H
#define LP_SETTINGS_MANAGER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SETTINGS_MANAGER (lp_settings_manager_get_type ())

G_DECLARE_FINAL_TYPE (LpSettingsManager, lp_settings_manager, LP, SETTINGS_MANAGER, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_settings_manager_get_default:
 *
 * Gets the singleton settings manager instance.
 *
 * Returns: (transfer none): The default #LpSettingsManager
 */
LpSettingsManager * lp_settings_manager_get_default (void);

/* ==========================================================================
 * Persistence
 * ========================================================================== */

/**
 * lp_settings_manager_load:
 * @self: an #LpSettingsManager
 * @error: (nullable): return location for error
 *
 * Loads settings from disk.
 *
 * Returns: %TRUE on success (or if no settings file exists)
 */
gboolean lp_settings_manager_load (LpSettingsManager  *self,
                                   GError            **error);

/**
 * lp_settings_manager_save:
 * @self: an #LpSettingsManager
 * @error: (nullable): return location for error
 *
 * Saves settings to disk.
 *
 * Returns: %TRUE on success
 */
gboolean lp_settings_manager_save (LpSettingsManager  *self,
                                   GError            **error);

/**
 * lp_settings_manager_reset_to_defaults:
 * @self: an #LpSettingsManager
 *
 * Resets all settings to their default values.
 */
void lp_settings_manager_reset_to_defaults (LpSettingsManager *self);

/* ==========================================================================
 * Graphics Settings
 * ========================================================================== */

/**
 * lp_settings_manager_get_fullscreen:
 * @self: an #LpSettingsManager
 *
 * Returns: %TRUE if fullscreen is enabled
 */
gboolean lp_settings_manager_get_fullscreen (LpSettingsManager *self);

/**
 * lp_settings_manager_set_fullscreen:
 * @self: an #LpSettingsManager
 * @fullscreen: whether to enable fullscreen
 *
 * Sets the fullscreen setting.
 */
void lp_settings_manager_set_fullscreen (LpSettingsManager *self,
                                         gboolean           fullscreen);

/**
 * lp_settings_manager_get_vsync:
 * @self: an #LpSettingsManager
 *
 * Returns: %TRUE if VSync is enabled
 */
gboolean lp_settings_manager_get_vsync (LpSettingsManager *self);

/**
 * lp_settings_manager_set_vsync:
 * @self: an #LpSettingsManager
 * @vsync: whether to enable VSync
 *
 * Sets the VSync setting.
 */
void lp_settings_manager_set_vsync (LpSettingsManager *self,
                                    gboolean           vsync);

/**
 * lp_settings_manager_get_window_width:
 * @self: an #LpSettingsManager
 *
 * Returns: The window width in pixels
 */
gint lp_settings_manager_get_window_width (LpSettingsManager *self);

/**
 * lp_settings_manager_get_window_height:
 * @self: an #LpSettingsManager
 *
 * Returns: The window height in pixels
 */
gint lp_settings_manager_get_window_height (LpSettingsManager *self);

/**
 * lp_settings_manager_set_window_size:
 * @self: an #LpSettingsManager
 * @width: window width in pixels
 * @height: window height in pixels
 *
 * Sets the window size.
 */
void lp_settings_manager_set_window_size (LpSettingsManager *self,
                                          gint               width,
                                          gint               height);

/* ==========================================================================
 * Audio Settings
 * ========================================================================== */

/**
 * lp_settings_manager_get_master_volume:
 * @self: an #LpSettingsManager
 *
 * Returns: Master volume level (0.0 to 1.0)
 */
gfloat lp_settings_manager_get_master_volume (LpSettingsManager *self);

/**
 * lp_settings_manager_set_master_volume:
 * @self: an #LpSettingsManager
 * @volume: volume level (0.0 to 1.0)
 *
 * Sets the master volume.
 */
void lp_settings_manager_set_master_volume (LpSettingsManager *self,
                                            gfloat             volume);

/**
 * lp_settings_manager_get_music_volume:
 * @self: an #LpSettingsManager
 *
 * Returns: Music volume level (0.0 to 1.0)
 */
gfloat lp_settings_manager_get_music_volume (LpSettingsManager *self);

/**
 * lp_settings_manager_set_music_volume:
 * @self: an #LpSettingsManager
 * @volume: volume level (0.0 to 1.0)
 *
 * Sets the music volume.
 */
void lp_settings_manager_set_music_volume (LpSettingsManager *self,
                                           gfloat             volume);

/**
 * lp_settings_manager_get_sfx_volume:
 * @self: an #LpSettingsManager
 *
 * Returns: SFX volume level (0.0 to 1.0)
 */
gfloat lp_settings_manager_get_sfx_volume (LpSettingsManager *self);

/**
 * lp_settings_manager_set_sfx_volume:
 * @self: an #LpSettingsManager
 * @volume: volume level (0.0 to 1.0)
 *
 * Sets the SFX volume.
 */
void lp_settings_manager_set_sfx_volume (LpSettingsManager *self,
                                         gfloat             volume);

/**
 * lp_settings_manager_get_muted:
 * @self: an #LpSettingsManager
 *
 * Returns: %TRUE if audio is muted
 */
gboolean lp_settings_manager_get_muted (LpSettingsManager *self);

/**
 * lp_settings_manager_set_muted:
 * @self: an #LpSettingsManager
 * @muted: whether to mute audio
 *
 * Sets the mute state.
 */
void lp_settings_manager_set_muted (LpSettingsManager *self,
                                    gboolean           muted);

/* ==========================================================================
 * Gameplay Settings
 * ========================================================================== */

/**
 * lp_settings_manager_get_autosave_enabled:
 * @self: an #LpSettingsManager
 *
 * Returns: %TRUE if autosave is enabled
 */
gboolean lp_settings_manager_get_autosave_enabled (LpSettingsManager *self);

/**
 * lp_settings_manager_set_autosave_enabled:
 * @self: an #LpSettingsManager
 * @enabled: whether to enable autosave
 *
 * Sets whether autosave is enabled.
 */
void lp_settings_manager_set_autosave_enabled (LpSettingsManager *self,
                                               gboolean           enabled);

/**
 * lp_settings_manager_get_autosave_interval:
 * @self: an #LpSettingsManager
 *
 * Gets the autosave interval in minutes.
 *
 * Returns: Autosave interval in minutes
 */
guint lp_settings_manager_get_autosave_interval (LpSettingsManager *self);

/**
 * lp_settings_manager_set_autosave_interval:
 * @self: an #LpSettingsManager
 * @minutes: autosave interval in minutes
 *
 * Sets the autosave interval.
 */
void lp_settings_manager_set_autosave_interval (LpSettingsManager *self,
                                                guint              minutes);

/**
 * lp_settings_manager_get_pause_on_events:
 * @self: an #LpSettingsManager
 *
 * Returns: %TRUE if game should pause on important events
 */
gboolean lp_settings_manager_get_pause_on_events (LpSettingsManager *self);

/**
 * lp_settings_manager_set_pause_on_events:
 * @self: an #LpSettingsManager
 * @pause: whether to pause on events
 *
 * Sets whether to auto-pause on important events.
 */
void lp_settings_manager_set_pause_on_events (LpSettingsManager *self,
                                              gboolean           pause);

/**
 * lp_settings_manager_get_show_notifications:
 * @self: an #LpSettingsManager
 *
 * Returns: %TRUE if in-game notifications are shown
 */
gboolean lp_settings_manager_get_show_notifications (LpSettingsManager *self);

/**
 * lp_settings_manager_set_show_notifications:
 * @self: an #LpSettingsManager
 * @show: whether to show notifications
 *
 * Sets whether to show in-game notifications.
 */
void lp_settings_manager_set_show_notifications (LpSettingsManager *self,
                                                 gboolean           show);

/* ==========================================================================
 * Accessibility Settings
 * ========================================================================== */

/**
 * lp_settings_manager_get_ui_scale:
 * @self: an #LpSettingsManager
 *
 * Gets the UI scale factor.
 *
 * Returns: UI scale (1.0 = normal, 1.5 = 150%, etc.)
 */
gfloat lp_settings_manager_get_ui_scale (LpSettingsManager *self);

/**
 * lp_settings_manager_set_ui_scale:
 * @self: an #LpSettingsManager
 * @scale: UI scale factor (0.75 to 2.0)
 *
 * Sets the UI scale factor.
 */
void lp_settings_manager_set_ui_scale (LpSettingsManager *self,
                                       gfloat             scale);

G_END_DECLS

#endif /* LP_SETTINGS_MANAGER_H */
