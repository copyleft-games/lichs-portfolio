/* lp-ambient-audio.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-ambient-audio.h"
#include <math.h>

/* Number of oscillators for additive synthesis */
#define NUM_OSCILLATORS 6

/* LFO (Low Frequency Oscillator) for modulation */
#define NUM_LFOS 3

struct _LpAmbientAudio
{
    LrgProceduralAudio parent_instance;

    LpAmbientMood mood;
    LpAmbientMood target_mood;
    gfloat        mood_blend;        /* 0.0 = current, 1.0 = target */

    /* Oscillator state */
    gfloat osc_phase[NUM_OSCILLATORS];
    gfloat osc_freq[NUM_OSCILLATORS];

    /* LFO state for modulation */
    gfloat lfo_phase[NUM_LFOS];
    gfloat lfo_freq[NUM_LFOS];

    /* Filter state (simple low-pass) */
    gfloat filter_state_l;
    gfloat filter_state_r;
};

G_DEFINE_TYPE (LpAmbientAudio, lp_ambient_audio, LRG_TYPE_PROCEDURAL_AUDIO)

/* Frequency ratios for different moods (relative to base frequency) */
static const gfloat MOOD_FREQS[4][NUM_OSCILLATORS] = {
    /* NEUTRAL - minor chord drone */
    { 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f },
    /* TENSION - dissonant intervals */
    { 1.0f, 1.06f, 1.5f, 2.12f, 3.0f, 4.24f },
    /* TRIUMPH - major chord, brighter */
    { 1.0f, 1.25f, 1.5f, 2.0f, 2.5f, 3.0f },
    /* SLUMBER - very low, sparse */
    { 0.5f, 1.0f, 1.5f, 2.0f, 0.25f, 0.75f }
};

/* Amplitude for each oscillator */
static const gfloat OSC_AMPS[NUM_OSCILLATORS] = {
    0.25f, 0.15f, 0.12f, 0.08f, 0.05f, 0.03f
};

/* Base frequency in Hz (very low for drone) */
#define BASE_FREQ 55.0f  /* A1 */

static void
lp_ambient_audio_generate (LrgProceduralAudio *procedural,
                           gfloat             *buffer,
                           gint                frame_count)
{
    LpAmbientAudio *self = LP_AMBIENT_AUDIO (procedural);
    guint channels;
    guint sample_rate;
    gint i, j;
    gfloat sample_l, sample_r;
    gfloat lfo_val[NUM_LFOS];
    gfloat freq_mult;
    const gfloat *current_freqs;
    const gfloat *target_freqs;
    gfloat blended_freq;

    channels = lrg_procedural_audio_get_channels (procedural);
    sample_rate = lrg_procedural_audio_get_sample_rate (procedural);

    current_freqs = MOOD_FREQS[self->mood];
    target_freqs = MOOD_FREQS[self->target_mood];

    for (i = 0; i < frame_count; i++)
    {
        /* Update LFOs (very slow modulation) */
        for (j = 0; j < NUM_LFOS; j++)
        {
            lfo_val[j] = sinf (self->lfo_phase[j] * 2.0f * G_PI);
            self->lfo_phase[j] += self->lfo_freq[j] / (gfloat)sample_rate;
            if (self->lfo_phase[j] >= 1.0f)
                self->lfo_phase[j] -= 1.0f;
        }

        /* Frequency modulation from LFO 0 */
        freq_mult = 1.0f + lfo_val[0] * 0.02f;

        /* Generate oscillators */
        sample_l = 0.0f;
        sample_r = 0.0f;

        for (j = 0; j < NUM_OSCILLATORS; j++)
        {
            gfloat osc_sample;
            gfloat pan;

            /* Blend frequency ratios between moods */
            blended_freq = current_freqs[j] * (1.0f - self->mood_blend)
                         + target_freqs[j] * self->mood_blend;

            /* Calculate oscillator sample (sine wave) */
            osc_sample = sinf (self->osc_phase[j] * 2.0f * G_PI);
            osc_sample *= OSC_AMPS[j];

            /* Slight amplitude modulation from LFO 1 */
            osc_sample *= 1.0f + lfo_val[1] * 0.1f * (j % 2 == 0 ? 1.0f : -1.0f);

            /* Stereo panning - spread oscillators across stereo field */
            pan = (gfloat)j / (NUM_OSCILLATORS - 1) - 0.5f;
            pan += lfo_val[2] * 0.1f;  /* Slow stereo movement */
            pan = CLAMP (pan, -1.0f, 1.0f);

            sample_l += osc_sample * (1.0f - pan) * 0.5f;
            sample_r += osc_sample * (1.0f + pan) * 0.5f;

            /* Update oscillator phase */
            self->osc_phase[j] += (BASE_FREQ * blended_freq * freq_mult)
                                 / (gfloat)sample_rate;
            if (self->osc_phase[j] >= 1.0f)
                self->osc_phase[j] -= 1.0f;
        }

        /* Simple low-pass filter for smoother sound */
        self->filter_state_l = self->filter_state_l * 0.95f + sample_l * 0.05f;
        self->filter_state_r = self->filter_state_r * 0.95f + sample_r * 0.05f;

        sample_l = self->filter_state_l;
        sample_r = self->filter_state_r;

        /* Output to buffer */
        if (channels == 2)
        {
            buffer[i * 2] = CLAMP (sample_l, -1.0f, 1.0f);
            buffer[i * 2 + 1] = CLAMP (sample_r, -1.0f, 1.0f);
        }
        else
        {
            buffer[i] = CLAMP ((sample_l + sample_r) * 0.5f, -1.0f, 1.0f);
        }

        /* Slowly blend toward target mood */
        if (self->mood_blend < 1.0f && self->mood != self->target_mood)
        {
            self->mood_blend += 0.00001f;  /* Very slow transition */
            if (self->mood_blend >= 1.0f)
            {
                self->mood_blend = 0.0f;
                self->mood = self->target_mood;
            }
        }
    }
}

static void
lp_ambient_audio_init (LpAmbientAudio *self)
{
    gint i;

    self->mood = LP_AMBIENT_MOOD_NEUTRAL;
    self->target_mood = LP_AMBIENT_MOOD_NEUTRAL;
    self->mood_blend = 0.0f;

    /* Initialize oscillator phases with slight offsets */
    for (i = 0; i < NUM_OSCILLATORS; i++)
    {
        self->osc_phase[i] = (gfloat)i / NUM_OSCILLATORS;
        self->osc_freq[i] = BASE_FREQ * MOOD_FREQS[0][i];
    }

    /* Initialize LFOs with very low frequencies */
    self->lfo_phase[0] = 0.0f;
    self->lfo_freq[0] = 0.05f;   /* 20 second cycle for pitch modulation */

    self->lfo_phase[1] = 0.33f;
    self->lfo_freq[1] = 0.08f;   /* 12.5 second cycle for amplitude */

    self->lfo_phase[2] = 0.66f;
    self->lfo_freq[2] = 0.03f;   /* 33 second cycle for stereo panning */

    /* Initialize filter state */
    self->filter_state_l = 0.0f;
    self->filter_state_r = 0.0f;
}

static void
lp_ambient_audio_class_init (LpAmbientAudioClass *klass)
{
    LrgProceduralAudioClass *procedural_class;

    procedural_class = LRG_PROCEDURAL_AUDIO_CLASS (klass);
    procedural_class->generate = lp_ambient_audio_generate;
}

LpAmbientAudio *
lp_ambient_audio_new (void)
{
    LpAmbientAudio *self;

    self = g_object_new (LP_TYPE_AMBIENT_AUDIO,
                         "sample-rate", 44100,
                         "channels", 2,
                         NULL);

    lrg_procedural_audio_set_name (LRG_PROCEDURAL_AUDIO (self),
                                   "ambient-drone");

    return self;
}

void
lp_ambient_audio_set_mood (LpAmbientAudio *self,
                            LpAmbientMood   mood)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    if (mood != self->mood && mood != self->target_mood)
    {
        self->target_mood = mood;
        self->mood_blend = 0.0f;
    }
}

LpAmbientMood
lp_ambient_audio_get_mood (LpAmbientAudio *self)
{
    g_return_val_if_fail (LP_IS_AMBIENT_AUDIO (self), LP_AMBIENT_MOOD_NEUTRAL);
    return self->mood;
}
