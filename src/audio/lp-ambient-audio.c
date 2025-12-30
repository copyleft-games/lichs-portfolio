/* lp-ambient-audio.c - Dark Fantasy Ambient Audio Generator
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-ambient-audio.h"
#include <math.h>

/* Audio constants */
#define DEFAULT_SAMPLE_RATE     44100
#define DEFAULT_CHANNELS        2
#define DEFAULT_BASE_FREQ       55.0f  /* A1 - dark, low drone */
#define LFO_RATE                0.1f   /* Slow "breathing" rate */
#define FADE_DURATION           2.0f   /* Fade in/out duration in seconds */

struct _LpAmbientAudio
{
    LrgProceduralAudio parent_instance;

    /* Synthesis parameters */
    gfloat    base_frequency;
    gfloat    intensity;
    gfloat    tension;
    gboolean  wind_enabled;

    /* Oscillator phases (0.0 to 1.0) */
    gfloat    phase_base;
    gfloat    phase_harm2;
    gfloat    phase_harm3;
    gfloat    phase_harm5;
    gfloat    phase_lfo;

    /* LFO and envelope state */
    gfloat    lfo_value;
    gfloat    envelope;
    gfloat    target_envelope;

    /* Wind noise state */
    gfloat    wind_phase;
    gfloat    wind_filter_state;

    /* Playback state */
    gboolean  is_fading;
    gfloat    fade_timer;
};

G_DEFINE_FINAL_TYPE (LpAmbientAudio, lp_ambient_audio, LRG_TYPE_PROCEDURAL_AUDIO)

static LpAmbientAudio *default_ambient = NULL;

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

static gfloat
rand_float (void)
{
    /*
     * Generate a random float between -1.0 and 1.0
     * for noise generation.
     */
    return ((gfloat) g_random_int () / (gfloat) G_MAXUINT32) * 2.0f - 1.0f;
}

static gfloat
low_pass_filter (gfloat input,
                 gfloat *state,
                 gfloat  alpha)
{
    /*
     * Simple one-pole low-pass filter.
     * alpha = dt / (RC + dt), where lower alpha = more filtering.
     */
    *state = *state + alpha * (input - *state);
    return *state;
}

/* ==========================================================================
 * Audio Generation (Virtual Method Override)
 * ========================================================================== */

static void
lp_ambient_audio_generate (LrgProceduralAudio *audio,
                           gfloat             *buffer,
                           gint                frame_count)
{
    LpAmbientAudio *self = LP_AMBIENT_AUDIO (audio);
    guint sample_rate = lrg_procedural_audio_get_sample_rate (audio);
    guint channels = lrg_procedural_audio_get_channels (audio);
    gint i;

    gfloat freq_base = self->base_frequency;
    gfloat freq_harm2 = freq_base * 2.0f;    /* Octave */
    gfloat freq_harm3 = freq_base * 3.0f;    /* Perfect fifth + octave */
    gfloat freq_harm5 = freq_base * 5.0f;    /* Major third + 2 octaves */

    /* Tension adds dissonance via detuning */
    gfloat detune = 1.0f + self->tension * 0.02f;

    /*
     * Generate audio samples.
     * The drone is created through additive synthesis:
     * - Base sine at 55 Hz (A1)
     * - Harmonics at 110 Hz, 165 Hz, 275 Hz
     * - LFO modulates amplitude for "breathing"
     * - Optional filtered noise for wind
     */
    for (i = 0; i < frame_count; i++)
    {
        gfloat sample = 0.0f;
        gfloat lfo;
        guint c;

        /* Update LFO */
        self->phase_lfo += LFO_RATE / sample_rate;
        if (self->phase_lfo >= 1.0f)
            self->phase_lfo -= 1.0f;
        lfo = sinf (self->phase_lfo * 2.0f * G_PI);
        self->lfo_value = 0.7f + 0.3f * lfo;  /* 0.7 to 1.0 range */

        /* Base oscillator */
        sample += sinf (self->phase_base * 2.0f * G_PI) * 0.5f;
        self->phase_base += freq_base * detune / sample_rate;
        if (self->phase_base >= 1.0f)
            self->phase_base -= 1.0f;

        /* 2nd harmonic (octave) - quieter */
        sample += sinf (self->phase_harm2 * 2.0f * G_PI) * 0.2f;
        self->phase_harm2 += freq_harm2 / sample_rate;
        if (self->phase_harm2 >= 1.0f)
            self->phase_harm2 -= 1.0f;

        /* 3rd harmonic - even quieter */
        sample += sinf (self->phase_harm3 * 2.0f * G_PI) * 0.1f;
        self->phase_harm3 += freq_harm3 / sample_rate;
        if (self->phase_harm3 >= 1.0f)
            self->phase_harm3 -= 1.0f;

        /* 5th harmonic - barely audible */
        sample += sinf (self->phase_harm5 * 2.0f * G_PI) * 0.05f;
        self->phase_harm5 += freq_harm5 / sample_rate;
        if (self->phase_harm5 >= 1.0f)
            self->phase_harm5 -= 1.0f;

        /* Add tension-based dissonance (tritone at high tension) */
        if (self->tension > 0.5f)
        {
            gfloat tritone_freq = freq_base * 1.414f;  /* Tritone */
            gfloat tritone_amp = (self->tension - 0.5f) * 0.2f;
            sample += sinf (self->wind_phase * 2.0f * G_PI) * tritone_amp;
            self->wind_phase += tritone_freq / sample_rate;
            if (self->wind_phase >= 1.0f)
                self->wind_phase -= 1.0f;
        }

        /* Add wind noise if enabled */
        if (self->wind_enabled)
        {
            gfloat noise = rand_float ();
            gfloat filtered_noise = low_pass_filter (noise,
                                                      &self->wind_filter_state,
                                                      0.01f);
            sample += filtered_noise * 0.1f;
        }

        /* Apply LFO modulation */
        sample *= self->lfo_value;

        /* Apply intensity and envelope */
        sample *= self->intensity * self->envelope;

        /* Soft clipping to avoid harsh distortion */
        if (sample > 0.8f)
            sample = 0.8f + (sample - 0.8f) * 0.2f;
        else if (sample < -0.8f)
            sample = -0.8f + (sample + 0.8f) * 0.2f;

        /* Output to all channels */
        for (c = 0; c < channels; c++)
        {
            buffer[i * channels + c] = sample;
        }
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_ambient_audio_class_init (LpAmbientAudioClass *klass)
{
    LrgProceduralAudioClass *audio_class = LRG_PROCEDURAL_AUDIO_CLASS (klass);

    /* Override the generate virtual method */
    audio_class->generate = lp_ambient_audio_generate;
}

static void
lp_ambient_audio_init (LpAmbientAudio *self)
{
    /* Initialize synthesis parameters */
    self->base_frequency = DEFAULT_BASE_FREQ;
    self->intensity = 0.5f;
    self->tension = 0.0f;
    self->wind_enabled = TRUE;

    /* Initialize oscillator phases */
    self->phase_base = 0.0f;
    self->phase_harm2 = 0.0f;
    self->phase_harm3 = 0.0f;
    self->phase_harm5 = 0.0f;
    self->phase_lfo = 0.0f;

    /* Initialize state */
    self->lfo_value = 1.0f;
    self->envelope = 0.0f;
    self->target_envelope = 0.0f;
    self->wind_filter_state = 0.0f;
    self->is_fading = FALSE;
    self->fade_timer = 0.0f;
}

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
lp_ambient_audio_new (void)
{
    return g_object_new (LP_TYPE_AMBIENT_AUDIO,
                         "sample-rate", DEFAULT_SAMPLE_RATE,
                         "channels", DEFAULT_CHANNELS,
                         NULL);
}

/**
 * lp_ambient_audio_get_default:
 *
 * Gets the default ambient audio instance.
 * Creates one if it doesn't exist.
 *
 * Returns: (transfer none) (nullable): The default #LpAmbientAudio
 */
LpAmbientAudio *
lp_ambient_audio_get_default (void)
{
    if (default_ambient == NULL)
    {
        default_ambient = lp_ambient_audio_new ();
        if (default_ambient != NULL)
        {
            lrg_procedural_audio_set_name (
                LRG_PROCEDURAL_AUDIO (default_ambient),
                "lp-ambient-drone");
        }
    }

    return default_ambient;
}

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
lp_ambient_audio_start (LpAmbientAudio *self)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    self->target_envelope = 1.0f;
    self->is_fading = TRUE;
    self->fade_timer = FADE_DURATION;

    lrg_procedural_audio_play (LRG_PROCEDURAL_AUDIO (self));
    lp_log_debug ("Ambient audio started");
}

/**
 * lp_ambient_audio_stop:
 * @self: an #LpAmbientAudio
 *
 * Stops ambient audio playback.
 * The audio will fade out over 2 seconds.
 */
void
lp_ambient_audio_stop (LpAmbientAudio *self)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    self->target_envelope = 0.0f;
    self->is_fading = TRUE;
    self->fade_timer = FADE_DURATION;

    lp_log_debug ("Ambient audio stopping");
}

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
                          gfloat          delta)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    /* Handle fade in/out */
    if (self->is_fading && self->fade_timer > 0.0f)
    {
        gfloat t = 1.0f - (self->fade_timer / FADE_DURATION);

        /* Smooth interpolation */
        self->envelope = self->envelope +
            (self->target_envelope - self->envelope) * t;

        self->fade_timer -= delta;

        if (self->fade_timer <= 0.0f)
        {
            self->envelope = self->target_envelope;
            self->is_fading = FALSE;

            /* Stop playback if faded out completely */
            if (self->envelope <= 0.0f)
            {
                lrg_procedural_audio_stop (LRG_PROCEDURAL_AUDIO (self));
                lp_log_debug ("Ambient audio stopped");
            }
        }
    }

    /* Call base class update to generate samples */
    lrg_procedural_audio_update (LRG_PROCEDURAL_AUDIO (self));
}

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
                                 gfloat          intensity)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    self->intensity = CLAMP (intensity, 0.0f, 1.0f);
}

/**
 * lp_ambient_audio_get_intensity:
 * @self: an #LpAmbientAudio
 *
 * Gets the current drone intensity.
 *
 * Returns: intensity level (0.0 to 1.0)
 */
gfloat
lp_ambient_audio_get_intensity (LpAmbientAudio *self)
{
    g_return_val_if_fail (LP_IS_AMBIENT_AUDIO (self), 0.0f);

    return self->intensity;
}

/**
 * lp_ambient_audio_set_wind_enabled:
 * @self: an #LpAmbientAudio
 * @enabled: whether to enable wind noise layer
 *
 * Enables or disables the "crypt wind" noise layer.
 */
void
lp_ambient_audio_set_wind_enabled (LpAmbientAudio *self,
                                    gboolean        enabled)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    self->wind_enabled = enabled;
}

/**
 * lp_ambient_audio_get_wind_enabled:
 * @self: an #LpAmbientAudio
 *
 * Gets whether wind noise is enabled.
 *
 * Returns: %TRUE if wind noise is enabled
 */
gboolean
lp_ambient_audio_get_wind_enabled (LpAmbientAudio *self)
{
    g_return_val_if_fail (LP_IS_AMBIENT_AUDIO (self), FALSE);

    return self->wind_enabled;
}

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
                               gfloat          tension)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    self->tension = CLAMP (tension, 0.0f, 1.0f);
}

/**
 * lp_ambient_audio_get_tension:
 * @self: an #LpAmbientAudio
 *
 * Gets the current tension level.
 *
 * Returns: tension level (0.0 to 1.0)
 */
gfloat
lp_ambient_audio_get_tension (LpAmbientAudio *self)
{
    g_return_val_if_fail (LP_IS_AMBIENT_AUDIO (self), 0.0f);

    return self->tension;
}

/**
 * lp_ambient_audio_set_base_frequency:
 * @self: an #LpAmbientAudio
 * @frequency: base frequency in Hz (default: 55.0)
 *
 * Sets the base drone frequency.
 */
void
lp_ambient_audio_set_base_frequency (LpAmbientAudio *self,
                                      gfloat          frequency)
{
    g_return_if_fail (LP_IS_AMBIENT_AUDIO (self));

    self->base_frequency = CLAMP (frequency, 20.0f, 200.0f);
}

/**
 * lp_ambient_audio_get_base_frequency:
 * @self: an #LpAmbientAudio
 *
 * Gets the base drone frequency.
 *
 * Returns: frequency in Hz
 */
gfloat
lp_ambient_audio_get_base_frequency (LpAmbientAudio *self)
{
    g_return_val_if_fail (LP_IS_AMBIENT_AUDIO (self), DEFAULT_BASE_FREQ);

    return self->base_frequency;
}
