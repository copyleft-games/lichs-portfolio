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

/* ==========================================================================
 * Difficulty Settings
 * ========================================================================== */

/**
 * LpDifficulty:
 * @LP_DIFFICULTY_EASY: Forgiving economy, reduced exposure penalties
 * @LP_DIFFICULTY_NORMAL: Balanced gameplay experience
 * @LP_DIFFICULTY_HARD: Harsher penalties, more aggressive competitors
 *
 * Game difficulty levels affecting economic modifiers and AI behavior.
 */
typedef enum
{
    LP_DIFFICULTY_EASY = 0,
    LP_DIFFICULTY_NORMAL = 1,
    LP_DIFFICULTY_HARD = 2
} LpDifficulty;

/**
 * lp_gameplay_settings_get_difficulty:
 * @self: an #LpGameplaySettings
 *
 * Gets the current game difficulty.
 *
 * Returns: The #LpDifficulty setting
 */
LpDifficulty
lp_gameplay_settings_get_difficulty (LpGameplaySettings *self);

/**
 * lp_gameplay_settings_set_difficulty:
 * @self: an #LpGameplaySettings
 * @difficulty: the #LpDifficulty to set
 *
 * Sets the game difficulty level.
 */
void
lp_gameplay_settings_set_difficulty (LpGameplaySettings *self,
                                     LpDifficulty        difficulty);

/* ==========================================================================
 * Simulation Speed Settings
 * ========================================================================== */

/**
 * LpGameSpeed:
 * @LP_GAME_SPEED_NORMAL: Standard simulation speed (1x)
 * @LP_GAME_SPEED_FAST: Accelerated simulation (2x)
 * @LP_GAME_SPEED_FASTER: Rapid simulation (4x)
 * @LP_GAME_SPEED_FASTEST: Maximum speed (10x)
 *
 * Simulation speed multipliers for time passage during slumber.
 */
typedef enum
{
    LP_GAME_SPEED_NORMAL = 0,   /* 1x */
    LP_GAME_SPEED_FAST = 1,     /* 2x */
    LP_GAME_SPEED_FASTER = 2,   /* 4x */
    LP_GAME_SPEED_FASTEST = 3   /* 10x */
} LpGameSpeed;

/**
 * lp_gameplay_settings_get_game_speed:
 * @self: an #LpGameplaySettings
 *
 * Gets the current simulation speed setting.
 *
 * Returns: The #LpGameSpeed setting
 */
LpGameSpeed
lp_gameplay_settings_get_game_speed (LpGameplaySettings *self);

/**
 * lp_gameplay_settings_set_game_speed:
 * @self: an #LpGameplaySettings
 * @speed: the #LpGameSpeed to set
 *
 * Sets the simulation speed multiplier.
 */
void
lp_gameplay_settings_set_game_speed (LpGameplaySettings *self,
                                     LpGameSpeed         speed);

/**
 * lp_gameplay_settings_get_speed_multiplier:
 * @self: an #LpGameplaySettings
 *
 * Gets the numeric speed multiplier based on current game speed setting.
 *
 * Returns: Speed multiplier (1.0, 2.0, 4.0, or 10.0)
 */
gdouble
lp_gameplay_settings_get_speed_multiplier (LpGameplaySettings *self);

G_END_DECLS

#endif /* LP_GAMEPLAY_SETTINGS_H */
