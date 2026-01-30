/* lp-ambient-audio.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Procedural ambient audio for Lich's Portfolio.
 *
 * LpAmbientAudio generates dark, atmospheric drone music using
 * additive synthesis. Multiple slow oscillators create evolving
 * textures appropriate for the game's undead financial theme.
 */

#pragma once

#include <libregnum.h>

G_BEGIN_DECLS

#define LP_TYPE_AMBIENT_AUDIO (lp_ambient_audio_get_type ())

G_DECLARE_FINAL_TYPE (LpAmbientAudio, lp_ambient_audio, LP, AMBIENT_AUDIO, LrgProceduralAudio)

/**
 * LpAmbientMood:
 * @LP_AMBIENT_MOOD_NEUTRAL: Default atmospheric drone
 * @LP_AMBIENT_MOOD_TENSION: Higher tension, more dissonance
 * @LP_AMBIENT_MOOD_TRIUMPH: Brighter, achievement-oriented
 * @LP_AMBIENT_MOOD_SLUMBER: Deep, slow, peaceful
 *
 * Mood settings that affect the ambient audio character.
 */
typedef enum
{
    LP_AMBIENT_MOOD_NEUTRAL,
    LP_AMBIENT_MOOD_TENSION,
    LP_AMBIENT_MOOD_TRIUMPH,
    LP_AMBIENT_MOOD_SLUMBER
} LpAmbientMood;

/**
 * lp_ambient_audio_new:
 *
 * Creates a new ambient audio generator.
 *
 * Returns: (transfer full): A new #LpAmbientAudio
 */
LpAmbientAudio * lp_ambient_audio_new (void);

/**
 * lp_ambient_audio_set_mood:
 * @self: an #LpAmbientAudio
 * @mood: the mood to transition to
 *
 * Sets the ambient mood. Transitions smoothly over a few seconds.
 */
void lp_ambient_audio_set_mood (LpAmbientAudio *self,
                                 LpAmbientMood   mood);

/**
 * lp_ambient_audio_get_mood:
 * @self: an #LpAmbientAudio
 *
 * Gets the current ambient mood.
 *
 * Returns: the current mood
 */
LpAmbientMood lp_ambient_audio_get_mood (LpAmbientAudio *self);

G_END_DECLS
