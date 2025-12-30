/* lp-ambient-audio.h - Dark Fantasy Ambient Audio Generator
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpAmbientAudio extends LrgProceduralAudio to generate dark fantasy
 * ambient drone sounds appropriate for an undead lich's portfolio.
 *
 * Audio characteristics:
 * - Base drone at 55 Hz (A1) with slight detuning
 * - Harmonics at 2nd, 3rd, and 5th intervals
 * - LFO modulation on amplitude for "breathing" effect
 * - Optional "crypt wind" noise layer
 * - Tension parameter increases dissonance during events
 */

#ifndef LP_AMBIENT_AUDIO_H
#define LP_AMBIENT_AUDIO_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_AMBIENT_AUDIO (lp_ambient_audio_get_type ())

G_DECLARE_FINAL_TYPE (LpAmbientAudio, lp_ambient_audio, LP, AMBIENT_AUDIO, LrgProceduralAudio)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_ambient_audio_new:
 *
 * Creates a new ambient audio generator.
 * Uses default sample rate (44100) and stereo output.
 *
 * Returns: (transfer full) (nullable): A new #LpAmbientAudio
 */
LpAmbientAudio *
lp_ambient_audio_new (void);

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_ambient_audio_get_default:
 *
 * Gets the default ambient audio instance.
 * Creates one if it doesn't exist.
 *
 * Returns: (transfer none) (nullable): The default #LpAmbientAudio
 */
LpAmbientAudio *
lp_ambient_audio_get_default (void);

/* ==========================================================================
 * Audio Control
 * ========================================================================== */

/**
 * lp_ambient_audio_start:
 * @self: an #LpAmbientAudio
 *
 * Starts ambient audio playback.
 * The audio will fade in over 2 seconds.
 */
void
lp_ambient_audio_start (LpAmbientAudio *self);

/**
 * lp_ambient_audio_stop:
 * @self: an #LpAmbientAudio
 *
 * Stops ambient audio playback.
 * The audio will fade out over 2 seconds.
 */
void
lp_ambient_audio_stop (LpAmbientAudio *self);

/**
 * lp_ambient_audio_update:
 * @self: an #LpAmbientAudio
 * @delta: time since last update in seconds
 *
 * Updates the audio generator. Must be called each frame
 * while audio is playing.
 */
void
lp_ambient_audio_update (LpAmbientAudio *self,
                          gfloat          delta);

/* ==========================================================================
 * Parameters
 * ========================================================================== */

/**
 * lp_ambient_audio_set_intensity:
 * @self: an #LpAmbientAudio
 * @intensity: intensity level (0.0 to 1.0)
 *
 * Sets the drone intensity (overall amplitude).
 */
void
lp_ambient_audio_set_intensity (LpAmbientAudio *self,
                                 gfloat          intensity);

/**
 * lp_ambient_audio_get_intensity:
 * @self: an #LpAmbientAudio
 *
 * Gets the current drone intensity.
 *
 * Returns: intensity level (0.0 to 1.0)
 */
gfloat
lp_ambient_audio_get_intensity (LpAmbientAudio *self);

/**
 * lp_ambient_audio_set_wind_enabled:
 * @self: an #LpAmbientAudio
 * @enabled: whether to enable wind noise layer
 *
 * Enables or disables the "crypt wind" noise layer.
 */
void
lp_ambient_audio_set_wind_enabled (LpAmbientAudio *self,
                                    gboolean        enabled);

/**
 * lp_ambient_audio_get_wind_enabled:
 * @self: an #LpAmbientAudio
 *
 * Gets whether wind noise is enabled.
 *
 * Returns: %TRUE if wind noise is enabled
 */
gboolean
lp_ambient_audio_get_wind_enabled (LpAmbientAudio *self);

/**
 * lp_ambient_audio_set_tension:
 * @self: an #LpAmbientAudio
 * @tension: tension level (0.0 to 1.0)
 *
 * Sets the tension level. Higher tension adds dissonance
 * and darker tones, appropriate for dangerous events.
 */
void
lp_ambient_audio_set_tension (LpAmbientAudio *self,
                               gfloat          tension);

/**
 * lp_ambient_audio_get_tension:
 * @self: an #LpAmbientAudio
 *
 * Gets the current tension level.
 *
 * Returns: tension level (0.0 to 1.0)
 */
gfloat
lp_ambient_audio_get_tension (LpAmbientAudio *self);

/**
 * lp_ambient_audio_set_base_frequency:
 * @self: an #LpAmbientAudio
 * @frequency: base frequency in Hz (default: 55.0)
 *
 * Sets the base drone frequency.
 */
void
lp_ambient_audio_set_base_frequency (LpAmbientAudio *self,
                                      gfloat          frequency);

/**
 * lp_ambient_audio_get_base_frequency:
 * @self: an #LpAmbientAudio
 *
 * Gets the base drone frequency.
 *
 * Returns: frequency in Hz
 */
gfloat
lp_ambient_audio_get_base_frequency (LpAmbientAudio *self);

G_END_DECLS

#endif /* LP_AMBIENT_AUDIO_H */
