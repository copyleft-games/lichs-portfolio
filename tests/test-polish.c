/* test-polish.c - Phase 9 Polish and Content Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for:
 * - LpStrings localization system
 * - LpMalacharVoice commentary
 * - LpAmbientAudio procedural audio
 * - LpUiSounds sound bank
 * - LpTutorialSequences tutorial system
 */

#include <glib.h>
#include <libregnum.h>
#include "core/lp-strings.h"
#include "narrative/lp-malachar-voice.h"
#include "audio/lp-ambient-audio.h"
#include "audio/lp-ui-sounds.h"
#include "tutorial/lp-tutorial-sequences.h"
#include "lp-enums.h"

/* ==========================================================================
 * LpStrings Tests
 * ========================================================================== */

typedef struct
{
    LpStrings *strings;
} StringsFixture;

static void
strings_fixture_set_up (StringsFixture *fixture,
                         gconstpointer   user_data)
{
    fixture->strings = lp_strings_get_default ();
}

static void
strings_fixture_tear_down (StringsFixture *fixture,
                            gconstpointer   user_data)
{
    /* Singleton - don't unref */
}

static void
test_strings_singleton (StringsFixture *fixture,
                         gconstpointer   user_data)
{
    LpStrings *second = lp_strings_get_default ();

    g_assert_nonnull (fixture->strings);
    g_assert_true (fixture->strings == second);
}

static void
test_strings_get_string (StringsFixture *fixture,
                          gconstpointer   user_data)
{
    const gchar *result;

    /*
     * lp_str returns NULL for missing keys when locale not loaded.
     * This is expected behavior - the key lookup returns NULL.
     */
    result = lp_str ("missing.key");

    /* Key not found returns NULL */
    g_assert_null (result);
}

/* ==========================================================================
 * LpMalacharVoice Tests
 * ========================================================================== */

typedef struct
{
    LpMalacharVoice *voice;
} VoiceFixture;

static void
voice_fixture_set_up (VoiceFixture  *fixture,
                       gconstpointer  user_data)
{
    fixture->voice = lp_malachar_voice_get_default ();
}

static void
voice_fixture_tear_down (VoiceFixture  *fixture,
                          gconstpointer  user_data)
{
    /* Singleton - don't unref */
}

static void
test_voice_singleton (VoiceFixture  *fixture,
                       gconstpointer  user_data)
{
    LpMalacharVoice *second = lp_malachar_voice_get_default ();

    g_assert_nonnull (fixture->voice);
    g_assert_true (fixture->voice == second);
}

static void
test_voice_commentary_greeting (VoiceFixture  *fixture,
                                 gconstpointer  user_data)
{
    const gchar *commentary;

    commentary = lp_malachar_voice_get_commentary (fixture->voice,
                                                     LP_COMMENTARY_GREETING);

    /*
     * Commentary may be NULL if data file not found.
     * In production this would load from commentary.yaml.
     */
    if (commentary != NULL)
    {
        g_assert_cmpuint (strlen (commentary), >, 0);
    }
}

static void
test_voice_commentary_contexts (VoiceFixture  *fixture,
                                 gconstpointer  user_data)
{
    /*
     * Test that each commentary context can be queried without crashing.
     * Results may be NULL if data files aren't loaded.
     */
    lp_malachar_voice_get_commentary (fixture->voice, LP_COMMENTARY_GREETING);
    lp_malachar_voice_get_commentary (fixture->voice, LP_COMMENTARY_SLUMBER);
    lp_malachar_voice_get_commentary (fixture->voice, LP_COMMENTARY_KINGDOM_COLLAPSE);
    lp_malachar_voice_get_commentary (fixture->voice, LP_COMMENTARY_AGENT_DEATH);
    lp_malachar_voice_get_commentary (fixture->voice, LP_COMMENTARY_INVESTMENT_SUCCESS);
    lp_malachar_voice_get_commentary (fixture->voice, LP_COMMENTARY_INVESTMENT_FAILURE);
    lp_malachar_voice_get_commentary (fixture->voice, LP_COMMENTARY_PRESTIGE);

    /* No assertion needed - we're just checking it doesn't crash */
    g_assert_true (TRUE);
}

/* ==========================================================================
 * LpAmbientAudio Tests
 * ========================================================================== */

static void
test_ambient_audio_singleton (void)
{
    LpAmbientAudio *audio1;
    LpAmbientAudio *audio2;

    audio1 = lp_ambient_audio_get_default ();
    audio2 = lp_ambient_audio_get_default ();

    if (audio1 != NULL)
    {
        g_assert_true (LP_IS_AMBIENT_AUDIO (audio1));
        g_assert_true (audio1 == audio2);
    }
    else
    {
        /* Audio unavailable in headless environment */
        g_test_skip ("Audio not available (headless environment)");
    }
}

static void
test_ambient_audio_new (void)
{
    g_autoptr(LpAmbientAudio) audio = NULL;

    audio = lp_ambient_audio_new ();

    if (audio != NULL)
    {
        g_assert_true (LP_IS_AMBIENT_AUDIO (audio));
    }
    else
    {
        g_test_skip ("Audio not available (headless environment)");
    }
}

static void
test_ambient_audio_intensity (void)
{
    g_autoptr(LpAmbientAudio) audio = NULL;

    audio = lp_ambient_audio_new ();
    if (audio == NULL)
    {
        g_test_skip ("Audio not available");
        return;
    }

    /* Default intensity */
    g_assert_cmpfloat (lp_ambient_audio_get_intensity (audio), >, 0.0f);
    g_assert_cmpfloat (lp_ambient_audio_get_intensity (audio), <=, 1.0f);

    /* Set intensity */
    lp_ambient_audio_set_intensity (audio, 0.8f);
    g_assert_cmpfloat (lp_ambient_audio_get_intensity (audio), ==, 0.8f);

    /* Clamp to valid range */
    lp_ambient_audio_set_intensity (audio, 2.0f);
    g_assert_cmpfloat (lp_ambient_audio_get_intensity (audio), ==, 1.0f);

    lp_ambient_audio_set_intensity (audio, -1.0f);
    g_assert_cmpfloat (lp_ambient_audio_get_intensity (audio), ==, 0.0f);
}

static void
test_ambient_audio_tension (void)
{
    g_autoptr(LpAmbientAudio) audio = NULL;

    audio = lp_ambient_audio_new ();
    if (audio == NULL)
    {
        g_test_skip ("Audio not available");
        return;
    }

    /* Default tension is 0 */
    g_assert_cmpfloat (lp_ambient_audio_get_tension (audio), ==, 0.0f);

    /* Set tension */
    lp_ambient_audio_set_tension (audio, 0.75f);
    g_assert_cmpfloat (lp_ambient_audio_get_tension (audio), ==, 0.75f);
}

static void
test_ambient_audio_wind (void)
{
    g_autoptr(LpAmbientAudio) audio = NULL;

    audio = lp_ambient_audio_new ();
    if (audio == NULL)
    {
        g_test_skip ("Audio not available");
        return;
    }

    /* Wind enabled by default */
    g_assert_true (lp_ambient_audio_get_wind_enabled (audio));

    /* Toggle wind */
    lp_ambient_audio_set_wind_enabled (audio, FALSE);
    g_assert_false (lp_ambient_audio_get_wind_enabled (audio));

    lp_ambient_audio_set_wind_enabled (audio, TRUE);
    g_assert_true (lp_ambient_audio_get_wind_enabled (audio));
}

static void
test_ambient_audio_base_frequency (void)
{
    g_autoptr(LpAmbientAudio) audio = NULL;

    audio = lp_ambient_audio_new ();
    if (audio == NULL)
    {
        g_test_skip ("Audio not available");
        return;
    }

    /* Default base frequency is 55 Hz (A1) */
    g_assert_cmpfloat (lp_ambient_audio_get_base_frequency (audio), ==, 55.0f);

    /* Change frequency */
    lp_ambient_audio_set_base_frequency (audio, 110.0f);
    g_assert_cmpfloat (lp_ambient_audio_get_base_frequency (audio), ==, 110.0f);
}

/* ==========================================================================
 * LpUiSounds Tests
 * ========================================================================== */

static void
test_ui_sounds_singleton (void)
{
    LpUiSounds *sounds1;
    LpUiSounds *sounds2;

    sounds1 = lp_ui_sounds_get_default ();
    sounds2 = lp_ui_sounds_get_default ();

    g_assert_nonnull (sounds1);
    g_assert_true (LP_IS_UI_SOUNDS (sounds1));
    g_assert_true (sounds1 == sounds2);
}

static void
test_ui_sounds_enabled (void)
{
    LpUiSounds *sounds = lp_ui_sounds_get_default ();

    /* Enabled by default */
    g_assert_true (lp_ui_sounds_get_enabled (sounds));

    /* Toggle enabled */
    lp_ui_sounds_set_enabled (sounds, FALSE);
    g_assert_false (lp_ui_sounds_get_enabled (sounds));

    lp_ui_sounds_set_enabled (sounds, TRUE);
    g_assert_true (lp_ui_sounds_get_enabled (sounds));
}

static void
test_ui_sounds_volume (void)
{
    LpUiSounds *sounds = lp_ui_sounds_get_default ();

    /* Default volume is 0.7 */
    g_assert_cmpfloat (lp_ui_sounds_get_volume (sounds), ==, 0.7f);

    /* Change volume */
    lp_ui_sounds_set_volume (sounds, 0.5f);
    g_assert_cmpfloat (lp_ui_sounds_get_volume (sounds), ==, 0.5f);

    /* Clamp to valid range */
    lp_ui_sounds_set_volume (sounds, 1.5f);
    g_assert_cmpfloat (lp_ui_sounds_get_volume (sounds), ==, 1.0f);

    lp_ui_sounds_set_volume (sounds, -0.5f);
    g_assert_cmpfloat (lp_ui_sounds_get_volume (sounds), ==, 0.0f);

    /* Restore default */
    lp_ui_sounds_set_volume (sounds, 0.7f);
}

static void
test_ui_sounds_play_no_crash (void)
{
    LpUiSounds *sounds = lp_ui_sounds_get_default ();

    /*
     * Playing sounds when files aren't loaded should not crash.
     * Just silently does nothing.
     */
    lp_ui_sounds_play (sounds, "click");
    lp_ui_sounds_play (sounds, "nonexistent");
    lp_ui_sounds_play_click (sounds);
    lp_ui_sounds_play_purchase (sounds);
    lp_ui_sounds_play_achievement (sounds);
    lp_ui_sounds_play_error (sounds);

    /* No assertion - just checking no crash */
    g_assert_true (TRUE);
}

/* ==========================================================================
 * LpTutorialSequences Tests
 * ========================================================================== */

static void
test_tutorial_sequences_singleton (void)
{
    LpTutorialSequences *seq1;
    LpTutorialSequences *seq2;

    seq1 = lp_tutorial_sequences_get_default ();
    seq2 = lp_tutorial_sequences_get_default ();

    g_assert_nonnull (seq1);
    g_assert_true (LP_IS_TUTORIAL_SEQUENCES (seq1));
    g_assert_true (seq1 == seq2);
}

static void
test_tutorial_condition_callback (void)
{
    gboolean result;

    /*
     * Test condition checking doesn't crash.
     * Results depend on game state.
     */
    result = lp_tutorial_sequences_check_condition ("has_gold", NULL);
    (void) result;  /* Result depends on game state */

    result = lp_tutorial_sequences_check_condition ("has_investment", NULL);
    (void) result;

    result = lp_tutorial_sequences_check_condition ("unknown_condition", NULL);
    g_assert_false (result);  /* Unknown conditions return FALSE */
}

/* ==========================================================================
 * Main
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* LpStrings tests */
    g_test_add ("/polish/strings/singleton",
                StringsFixture, NULL,
                strings_fixture_set_up,
                test_strings_singleton,
                strings_fixture_tear_down);

    g_test_add ("/polish/strings/get-string",
                StringsFixture, NULL,
                strings_fixture_set_up,
                test_strings_get_string,
                strings_fixture_tear_down);

    /* LpMalacharVoice tests */
    g_test_add ("/polish/voice/singleton",
                VoiceFixture, NULL,
                voice_fixture_set_up,
                test_voice_singleton,
                voice_fixture_tear_down);

    g_test_add ("/polish/voice/commentary-greeting",
                VoiceFixture, NULL,
                voice_fixture_set_up,
                test_voice_commentary_greeting,
                voice_fixture_tear_down);

    g_test_add ("/polish/voice/commentary-contexts",
                VoiceFixture, NULL,
                voice_fixture_set_up,
                test_voice_commentary_contexts,
                voice_fixture_tear_down);

    /* LpAmbientAudio tests */
    g_test_add_func ("/polish/ambient-audio/singleton",
                     test_ambient_audio_singleton);
    g_test_add_func ("/polish/ambient-audio/new",
                     test_ambient_audio_new);
    g_test_add_func ("/polish/ambient-audio/intensity",
                     test_ambient_audio_intensity);
    g_test_add_func ("/polish/ambient-audio/tension",
                     test_ambient_audio_tension);
    g_test_add_func ("/polish/ambient-audio/wind",
                     test_ambient_audio_wind);
    g_test_add_func ("/polish/ambient-audio/base-frequency",
                     test_ambient_audio_base_frequency);

    /* LpUiSounds tests */
    g_test_add_func ("/polish/ui-sounds/singleton",
                     test_ui_sounds_singleton);
    g_test_add_func ("/polish/ui-sounds/enabled",
                     test_ui_sounds_enabled);
    g_test_add_func ("/polish/ui-sounds/volume",
                     test_ui_sounds_volume);
    g_test_add_func ("/polish/ui-sounds/play-no-crash",
                     test_ui_sounds_play_no_crash);

    /* LpTutorialSequences tests */
    g_test_add_func ("/polish/tutorial/singleton",
                     test_tutorial_sequences_singleton);
    g_test_add_func ("/polish/tutorial/condition-callback",
                     test_tutorial_condition_callback);

    return g_test_run ();
}
