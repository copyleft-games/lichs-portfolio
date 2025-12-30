/* test-save-load.c - Phase 7 Save/Load System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for save/load management and settings persistence.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <libregnum.h>

#include "save/lp-save-manager.h"
#include "save/lp-settings-manager.h"
#include "core/lp-game-data.h"

/* ==========================================================================
 * Test Fixture for Save Manager
 * ========================================================================== */

typedef struct
{
    LpSaveManager *save_manager;
    LpGameData    *game_data;
    gchar         *temp_dir;
} SaveFixture;

static void
save_fixture_set_up (SaveFixture   *fixture,
                     gconstpointer  user_data)
{
    /* Create a temp directory for test saves */
    fixture->temp_dir = g_dir_make_tmp ("lp-save-test-XXXXXX", NULL);
    g_assert_nonnull (fixture->temp_dir);

    /* Get managers */
    fixture->save_manager = lp_save_manager_get_default ();
    g_assert_nonnull (fixture->save_manager);

    /* Create game data for testing */
    fixture->game_data = lp_game_data_new ();
    g_assert_nonnull (fixture->game_data);
}

static void
save_fixture_tear_down (SaveFixture   *fixture,
                        gconstpointer  user_data)
{
    g_clear_object (&fixture->game_data);

    /* Clean up temp directory */
    if (fixture->temp_dir)
    {
        g_autofree gchar *pattern = g_build_filename (fixture->temp_dir, "*", NULL);
        g_autoptr(GDir) dir = g_dir_open (fixture->temp_dir, 0, NULL);
        if (dir)
        {
            const gchar *filename;
            while ((filename = g_dir_read_name (dir)) != NULL)
            {
                g_autofree gchar *filepath = g_build_filename (fixture->temp_dir, filename, NULL);
                g_unlink (filepath);
            }
        }
        g_rmdir (fixture->temp_dir);
        g_free (fixture->temp_dir);
        fixture->temp_dir = NULL;
    }
}

/* ==========================================================================
 * Save Manager Tests
 * ========================================================================== */

static void
test_save_manager_singleton (void)
{
    LpSaveManager *manager1;
    LpSaveManager *manager2;

    manager1 = lp_save_manager_get_default ();
    g_assert_nonnull (manager1);
    g_assert_true (LP_IS_SAVE_MANAGER (manager1));

    manager2 = lp_save_manager_get_default ();
    g_assert_nonnull (manager2);

    /* Should be the same instance */
    g_assert_true (manager1 == manager2);
}

static void
test_save_manager_save_directory (void)
{
    LpSaveManager *manager;
    const gchar   *save_dir;

    manager = lp_save_manager_get_default ();
    save_dir = lp_save_manager_get_save_directory (manager);

    g_assert_nonnull (save_dir);
    g_assert_cmpstr (save_dir, !=, "");

    /* Should contain the expected path component */
    g_assert_true (g_str_has_suffix (save_dir, "lichs-portfolio") ||
                   strstr (save_dir, "lichs-portfolio") != NULL);
}

static void
test_save_manager_slot_path (void)
{
    LpSaveManager *manager;
    g_autofree gchar *path0 = NULL;
    g_autofree gchar *path5 = NULL;

    manager = lp_save_manager_get_default ();

    path0 = lp_save_manager_get_slot_path (manager, 0);
    g_assert_nonnull (path0);
    g_assert_true (g_str_has_suffix (path0, "save0.yaml"));

    path5 = lp_save_manager_get_slot_path (manager, 5);
    g_assert_nonnull (path5);
    g_assert_true (g_str_has_suffix (path5, "save5.yaml"));
}

static void
test_save_manager_ensure_directory (void)
{
    LpSaveManager *manager;
    const gchar   *save_dir;
    g_autoptr(GError) error = NULL;
    gboolean result;

    manager = lp_save_manager_get_default ();
    result = lp_save_manager_ensure_directory (manager, &error);

    g_assert_true (result);
    g_assert_no_error (error);

    save_dir = lp_save_manager_get_save_directory (manager);
    g_assert_true (g_file_test (save_dir, G_FILE_TEST_IS_DIR));
}

static void
test_save_manager_empty_slot (void)
{
    LpSaveManager *manager;

    manager = lp_save_manager_get_default ();

    /* Slot 9 should typically not exist in test environment */
    g_assert_false (lp_save_manager_slot_exists (manager, 9));
}

static void
test_save_manager_save_to_file (SaveFixture   *fixture,
                                gconstpointer  user_data)
{
    g_autofree gchar *path = NULL;
    g_autoptr(GError) error = NULL;
    gboolean result;

    path = g_build_filename (fixture->temp_dir, "test_save.yaml", NULL);

    result = lp_save_manager_save_to_file (fixture->save_manager,
                                            fixture->game_data,
                                            path,
                                            &error);

    g_assert_true (result);
    g_assert_no_error (error);
    g_assert_true (g_file_test (path, G_FILE_TEST_EXISTS));
}

static void
test_save_manager_load_from_file (SaveFixture   *fixture,
                                  gconstpointer  user_data)
{
    g_autofree gchar *path = NULL;
    g_autoptr(GError) error = NULL;
    g_autoptr(LpGameData) loaded_data = NULL;
    gboolean result;
    guint64 original_year;

    path = g_build_filename (fixture->temp_dir, "test_roundtrip.yaml", NULL);

    /* Get the current year before saving */
    original_year = lp_game_data_get_current_year (fixture->game_data);

    /* Save */
    result = lp_save_manager_save_to_file (fixture->save_manager,
                                            fixture->game_data,
                                            path,
                                            &error);
    g_assert_true (result);
    g_assert_no_error (error);

    /* Create new game data and load into it */
    loaded_data = lp_game_data_new ();
    result = lp_save_manager_load_from_file (fixture->save_manager,
                                              loaded_data,
                                              path,
                                              &error);
    g_assert_true (result);
    g_assert_no_error (error);

    /* Verify data was loaded correctly - year should match original */
    g_assert_cmpuint (lp_game_data_get_current_year (loaded_data), ==, original_year);
}

/* ==========================================================================
 * Settings Manager Tests
 * ========================================================================== */

static void
test_settings_manager_singleton (void)
{
    LpSettingsManager *manager1;
    LpSettingsManager *manager2;

    manager1 = lp_settings_manager_get_default ();
    g_assert_nonnull (manager1);
    g_assert_true (LP_IS_SETTINGS_MANAGER (manager1));

    manager2 = lp_settings_manager_get_default ();
    g_assert_nonnull (manager2);

    /* Should be the same instance */
    g_assert_true (manager1 == manager2);
}

static void
test_settings_manager_graphics_defaults (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();

    /* Test default values */
    g_assert_false (lp_settings_manager_get_fullscreen (manager));
    g_assert_true (lp_settings_manager_get_vsync (manager));
    g_assert_cmpint (lp_settings_manager_get_window_width (manager), ==, 1280);
    g_assert_cmpint (lp_settings_manager_get_window_height (manager), ==, 720);
}

static void
test_settings_manager_graphics_setters (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();

    /* Test fullscreen */
    lp_settings_manager_set_fullscreen (manager, TRUE);
    g_assert_true (lp_settings_manager_get_fullscreen (manager));
    lp_settings_manager_set_fullscreen (manager, FALSE);
    g_assert_false (lp_settings_manager_get_fullscreen (manager));

    /* Test vsync */
    lp_settings_manager_set_vsync (manager, FALSE);
    g_assert_false (lp_settings_manager_get_vsync (manager));
    lp_settings_manager_set_vsync (manager, TRUE);
    g_assert_true (lp_settings_manager_get_vsync (manager));

    /* Test window size */
    lp_settings_manager_set_window_size (manager, 1920, 1080);
    g_assert_cmpint (lp_settings_manager_get_window_width (manager), ==, 1920);
    g_assert_cmpint (lp_settings_manager_get_window_height (manager), ==, 1080);

    /* Restore defaults */
    lp_settings_manager_reset_to_defaults (manager);
}

static void
test_settings_manager_audio_defaults (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();
    lp_settings_manager_reset_to_defaults (manager);

    /* Test default values (0.8 master, 0.7 music, 1.0 sfx) */
    g_assert_cmpfloat (lp_settings_manager_get_master_volume (manager), ==, 0.8f);
    g_assert_cmpfloat (lp_settings_manager_get_music_volume (manager), ==, 0.7f);
    g_assert_cmpfloat (lp_settings_manager_get_sfx_volume (manager), ==, 1.0f);
    g_assert_false (lp_settings_manager_get_muted (manager));
}

static void
test_settings_manager_audio_setters (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();

    /* Test master volume */
    lp_settings_manager_set_master_volume (manager, 0.5f);
    g_assert_cmpfloat (lp_settings_manager_get_master_volume (manager), ==, 0.5f);

    /* Test music volume */
    lp_settings_manager_set_music_volume (manager, 0.3f);
    g_assert_cmpfloat (lp_settings_manager_get_music_volume (manager), ==, 0.3f);

    /* Test sfx volume */
    lp_settings_manager_set_sfx_volume (manager, 0.9f);
    g_assert_cmpfloat (lp_settings_manager_get_sfx_volume (manager), ==, 0.9f);

    /* Test mute */
    lp_settings_manager_set_muted (manager, TRUE);
    g_assert_true (lp_settings_manager_get_muted (manager));
    lp_settings_manager_set_muted (manager, FALSE);
    g_assert_false (lp_settings_manager_get_muted (manager));

    /* Restore defaults */
    lp_settings_manager_reset_to_defaults (manager);
}

static void
test_settings_manager_gameplay_defaults (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();
    lp_settings_manager_reset_to_defaults (manager);

    /* Test default values */
    g_assert_true (lp_settings_manager_get_autosave_enabled (manager));
    g_assert_cmpuint (lp_settings_manager_get_autosave_interval (manager), ==, 5);
    g_assert_true (lp_settings_manager_get_pause_on_events (manager));
    g_assert_true (lp_settings_manager_get_show_notifications (manager));
}

static void
test_settings_manager_gameplay_setters (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();

    /* Test autosave enabled */
    lp_settings_manager_set_autosave_enabled (manager, FALSE);
    g_assert_false (lp_settings_manager_get_autosave_enabled (manager));
    lp_settings_manager_set_autosave_enabled (manager, TRUE);
    g_assert_true (lp_settings_manager_get_autosave_enabled (manager));

    /* Test autosave interval */
    lp_settings_manager_set_autosave_interval (manager, 10);
    g_assert_cmpuint (lp_settings_manager_get_autosave_interval (manager), ==, 10);

    /* Test pause on events */
    lp_settings_manager_set_pause_on_events (manager, FALSE);
    g_assert_false (lp_settings_manager_get_pause_on_events (manager));

    /* Test show notifications */
    lp_settings_manager_set_show_notifications (manager, FALSE);
    g_assert_false (lp_settings_manager_get_show_notifications (manager));

    /* Restore defaults */
    lp_settings_manager_reset_to_defaults (manager);
}

static void
test_settings_manager_accessibility_defaults (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();
    lp_settings_manager_reset_to_defaults (manager);

    /* Test default values */
    g_assert_cmpfloat (lp_settings_manager_get_ui_scale (manager), ==, 1.0f);
}

static void
test_settings_manager_accessibility_setters (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();

    /* Test UI scale */
    lp_settings_manager_set_ui_scale (manager, 1.5f);
    g_assert_cmpfloat (lp_settings_manager_get_ui_scale (manager), ==, 1.5f);

    /* Test bounds clamping (min 0.75, max 2.0) */
    lp_settings_manager_set_ui_scale (manager, 0.5f);
    g_assert_cmpfloat (lp_settings_manager_get_ui_scale (manager), ==, 0.75f);

    lp_settings_manager_set_ui_scale (manager, 3.0f);
    g_assert_cmpfloat (lp_settings_manager_get_ui_scale (manager), ==, 2.0f);

    /* Restore defaults */
    lp_settings_manager_reset_to_defaults (manager);
}

static void
test_settings_manager_reset (void)
{
    LpSettingsManager *manager;

    manager = lp_settings_manager_get_default ();

    /* Change some settings */
    lp_settings_manager_set_fullscreen (manager, TRUE);
    lp_settings_manager_set_master_volume (manager, 0.5f);
    lp_settings_manager_set_autosave_interval (manager, 15);
    lp_settings_manager_set_ui_scale (manager, 1.5f);

    /* Reset to defaults */
    lp_settings_manager_reset_to_defaults (manager);

    /* Verify all are back to defaults */
    g_assert_false (lp_settings_manager_get_fullscreen (manager));
    g_assert_cmpfloat (lp_settings_manager_get_master_volume (manager), ==, 0.8f);
    g_assert_cmpuint (lp_settings_manager_get_autosave_interval (manager), ==, 5);
    g_assert_cmpfloat (lp_settings_manager_get_ui_scale (manager), ==, 1.0f);
}

/* ==========================================================================
 * Main Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Save manager tests */
    g_test_add_func ("/save/manager/singleton", test_save_manager_singleton);
    g_test_add_func ("/save/manager/save-directory", test_save_manager_save_directory);
    g_test_add_func ("/save/manager/slot-path", test_save_manager_slot_path);
    g_test_add_func ("/save/manager/ensure-directory", test_save_manager_ensure_directory);
    g_test_add_func ("/save/manager/empty-slot", test_save_manager_empty_slot);

    /* Save manager fixture tests */
    g_test_add ("/save/manager/save-to-file",
                SaveFixture, NULL,
                save_fixture_set_up,
                test_save_manager_save_to_file,
                save_fixture_tear_down);

    g_test_add ("/save/manager/load-from-file",
                SaveFixture, NULL,
                save_fixture_set_up,
                test_save_manager_load_from_file,
                save_fixture_tear_down);

    /* Settings manager tests */
    g_test_add_func ("/settings/manager/singleton", test_settings_manager_singleton);
    g_test_add_func ("/settings/graphics/defaults", test_settings_manager_graphics_defaults);
    g_test_add_func ("/settings/graphics/setters", test_settings_manager_graphics_setters);
    g_test_add_func ("/settings/audio/defaults", test_settings_manager_audio_defaults);
    g_test_add_func ("/settings/audio/setters", test_settings_manager_audio_setters);
    g_test_add_func ("/settings/gameplay/defaults", test_settings_manager_gameplay_defaults);
    g_test_add_func ("/settings/gameplay/setters", test_settings_manager_gameplay_setters);
    g_test_add_func ("/settings/accessibility/defaults", test_settings_manager_accessibility_defaults);
    g_test_add_func ("/settings/accessibility/setters", test_settings_manager_accessibility_setters);
    g_test_add_func ("/settings/manager/reset", test_settings_manager_reset);

    return g_test_run ();
}
