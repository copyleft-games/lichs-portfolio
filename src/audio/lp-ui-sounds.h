/* lp-ui-sounds.h - UI Sound Effect Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpUiSounds provides a simple interface for playing UI sound effects.
 * Sounds are loaded from a YAML manifest and can be played by name.
 *
 * Sound IDs:
 * - "click"       - Button/selection click
 * - "purchase"    - Investment purchased (coin clink)
 * - "sell"        - Investment sold
 * - "achievement" - Achievement unlock fanfare
 * - "event"       - Event notification chime
 * - "error"       - Invalid action buzz
 */

#ifndef LP_UI_SOUNDS_H
#define LP_UI_SOUNDS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_UI_SOUNDS (lp_ui_sounds_get_type ())

G_DECLARE_FINAL_TYPE (LpUiSounds, lp_ui_sounds, LP, UI_SOUNDS, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_ui_sounds_get_default:
 *
 * Gets the default UI sounds manager.
 * Initializes on first call, loading sounds from the manifest.
 *
 * Returns: (transfer none): The default #LpUiSounds
 */
LpUiSounds *
lp_ui_sounds_get_default (void);

/* ==========================================================================
 * Sound Playback
 * ========================================================================== */

/**
 * lp_ui_sounds_play:
 * @self: an #LpUiSounds
 * @sound_id: the sound ID to play
 *
 * Plays a UI sound effect by ID.
 * Does nothing if the sound ID is not found.
 */
void
lp_ui_sounds_play (LpUiSounds  *self,
                   const gchar *sound_id);

/**
 * lp_ui_sounds_play_click:
 * @self: an #LpUiSounds
 *
 * Plays the button click sound.
 */
void
lp_ui_sounds_play_click (LpUiSounds *self);

/**
 * lp_ui_sounds_play_purchase:
 * @self: an #LpUiSounds
 *
 * Plays the purchase confirmation sound.
 */
void
lp_ui_sounds_play_purchase (LpUiSounds *self);

/**
 * lp_ui_sounds_play_sell:
 * @self: an #LpUiSounds
 *
 * Plays the sell confirmation sound.
 */
void
lp_ui_sounds_play_sell (LpUiSounds *self);

/**
 * lp_ui_sounds_play_achievement:
 * @self: an #LpUiSounds
 *
 * Plays the achievement unlock fanfare.
 */
void
lp_ui_sounds_play_achievement (LpUiSounds *self);

/**
 * lp_ui_sounds_play_event:
 * @self: an #LpUiSounds
 *
 * Plays the event notification chime.
 */
void
lp_ui_sounds_play_event (LpUiSounds *self);

/**
 * lp_ui_sounds_play_error:
 * @self: an #LpUiSounds
 *
 * Plays the error/invalid action sound.
 */
void
lp_ui_sounds_play_error (LpUiSounds *self);

/* ==========================================================================
 * Configuration
 * ========================================================================== */

/**
 * lp_ui_sounds_set_enabled:
 * @self: an #LpUiSounds
 * @enabled: whether UI sounds are enabled
 *
 * Enables or disables all UI sounds.
 */
void
lp_ui_sounds_set_enabled (LpUiSounds *self,
                           gboolean    enabled);

/**
 * lp_ui_sounds_get_enabled:
 * @self: an #LpUiSounds
 *
 * Gets whether UI sounds are enabled.
 *
 * Returns: %TRUE if sounds are enabled
 */
gboolean
lp_ui_sounds_get_enabled (LpUiSounds *self);

/**
 * lp_ui_sounds_set_volume:
 * @self: an #LpUiSounds
 * @volume: volume level (0.0 to 1.0)
 *
 * Sets the UI sound volume.
 */
void
lp_ui_sounds_set_volume (LpUiSounds *self,
                          gfloat      volume);

/**
 * lp_ui_sounds_get_volume:
 * @self: an #LpUiSounds
 *
 * Gets the UI sound volume.
 *
 * Returns: volume level (0.0 to 1.0)
 */
gfloat
lp_ui_sounds_get_volume (LpUiSounds *self);

/**
 * lp_ui_sounds_load_manifest:
 * @self: an #LpUiSounds
 * @path: path to the sound manifest YAML file
 * @error: return location for error, or %NULL
 *
 * Loads sound definitions from a manifest file.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_ui_sounds_load_manifest (LpUiSounds   *self,
                             const gchar  *path,
                             GError      **error);

/* ==========================================================================
 * Sound ID Constants
 * ========================================================================== */

#define LP_UI_SOUND_CLICK       "click"
#define LP_UI_SOUND_PURCHASE    "purchase"
#define LP_UI_SOUND_SELL        "sell"
#define LP_UI_SOUND_ACHIEVEMENT "achievement"
#define LP_UI_SOUND_EVENT       "event"
#define LP_UI_SOUND_ERROR       "error"

G_END_DECLS

#endif /* LP_UI_SOUNDS_H */
