/* test-save-load.c - Phase 7 Save/Load System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for save/load management and gameplay settings.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <libregnum.h>

#include "save/lp-save-manager.h"
#include "core/lp-gameplay-settings.h"
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
 * Gameplay Settings Tests
 * ========================================================================== */

static void
test_gameplay_settings_creation (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;

    settings = lp_gameplay_settings_new ();
    g_assert_nonnull (settings);
    g_assert_true (LP_IS_GAMEPLAY_SETTINGS (settings));
    g_assert_true (LRG_IS_SETTINGS_GROUP (settings));
}

static void
test_gameplay_settings_group_name (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;
    const gchar *name;

    settings = lp_gameplay_settings_new ();
    name = lrg_settings_group_get_group_name (LRG_SETTINGS_GROUP (settings));

    g_assert_cmpstr (name, ==, "gameplay");
}

static void
test_gameplay_settings_defaults (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;

    settings = lp_gameplay_settings_new ();

    /* Test default values */
    g_assert_true (lp_gameplay_settings_get_autosave_enabled (settings));
    g_assert_cmpuint (lp_gameplay_settings_get_autosave_interval (settings), ==, 5);
    g_assert_true (lp_gameplay_settings_get_pause_on_events (settings));
    g_assert_true (lp_gameplay_settings_get_show_notifications (settings));
}

static void
test_gameplay_settings_autosave (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;

    settings = lp_gameplay_settings_new ();

    /* Test autosave enabled */
    lp_gameplay_settings_set_autosave_enabled (settings, FALSE);
    g_assert_false (lp_gameplay_settings_get_autosave_enabled (settings));
    lp_gameplay_settings_set_autosave_enabled (settings, TRUE);
    g_assert_true (lp_gameplay_settings_get_autosave_enabled (settings));

    /* Test autosave interval */
    lp_gameplay_settings_set_autosave_interval (settings, 10);
    g_assert_cmpuint (lp_gameplay_settings_get_autosave_interval (settings), ==, 10);

    /* Test interval clamping (min 1, max 60) */
    lp_gameplay_settings_set_autosave_interval (settings, 0);
    g_assert_cmpuint (lp_gameplay_settings_get_autosave_interval (settings), ==, 1);

    lp_gameplay_settings_set_autosave_interval (settings, 100);
    g_assert_cmpuint (lp_gameplay_settings_get_autosave_interval (settings), ==, 60);
}

static void
test_gameplay_settings_events (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;

    settings = lp_gameplay_settings_new ();

    /* Test pause on events */
    lp_gameplay_settings_set_pause_on_events (settings, FALSE);
    g_assert_false (lp_gameplay_settings_get_pause_on_events (settings));
    lp_gameplay_settings_set_pause_on_events (settings, TRUE);
    g_assert_true (lp_gameplay_settings_get_pause_on_events (settings));

    /* Test show notifications */
    lp_gameplay_settings_set_show_notifications (settings, FALSE);
    g_assert_false (lp_gameplay_settings_get_show_notifications (settings));
    lp_gameplay_settings_set_show_notifications (settings, TRUE);
    g_assert_true (lp_gameplay_settings_get_show_notifications (settings));
}

static void
test_gameplay_settings_reset (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;

    settings = lp_gameplay_settings_new ();

    /* Change all settings */
    lp_gameplay_settings_set_autosave_enabled (settings, FALSE);
    lp_gameplay_settings_set_autosave_interval (settings, 30);
    lp_gameplay_settings_set_pause_on_events (settings, FALSE);
    lp_gameplay_settings_set_show_notifications (settings, FALSE);

    /* Reset to defaults */
    lrg_settings_group_reset (LRG_SETTINGS_GROUP (settings));

    /* Verify all are back to defaults */
    g_assert_true (lp_gameplay_settings_get_autosave_enabled (settings));
    g_assert_cmpuint (lp_gameplay_settings_get_autosave_interval (settings), ==, 5);
    g_assert_true (lp_gameplay_settings_get_pause_on_events (settings));
    g_assert_true (lp_gameplay_settings_get_show_notifications (settings));
}

static void
test_gameplay_settings_serialization (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;
    g_autoptr(LpGameplaySettings) loaded = NULL;
    g_autoptr(GVariant) data = NULL;
    g_autoptr(GError) error = NULL;
    gboolean result;

    settings = lp_gameplay_settings_new ();

    /* Change settings from defaults */
    lp_gameplay_settings_set_autosave_enabled (settings, FALSE);
    lp_gameplay_settings_set_autosave_interval (settings, 15);
    lp_gameplay_settings_set_pause_on_events (settings, FALSE);
    lp_gameplay_settings_set_show_notifications (settings, TRUE);

    /* Serialize */
    data = lrg_settings_group_serialize (LRG_SETTINGS_GROUP (settings), &error);
    g_assert_no_error (error);
    g_assert_nonnull (data);

    /* Create new settings and deserialize */
    loaded = lp_gameplay_settings_new ();
    result = lrg_settings_group_deserialize (LRG_SETTINGS_GROUP (loaded), data, &error);
    g_assert_true (result);
    g_assert_no_error (error);

    /* Verify values match */
    g_assert_false (lp_gameplay_settings_get_autosave_enabled (loaded));
    g_assert_cmpuint (lp_gameplay_settings_get_autosave_interval (loaded), ==, 15);
    g_assert_false (lp_gameplay_settings_get_pause_on_events (loaded));
    g_assert_true (lp_gameplay_settings_get_show_notifications (loaded));
}

static void
test_gameplay_settings_dirty_tracking (void)
{
    g_autoptr(LpGameplaySettings) settings = NULL;

    settings = lp_gameplay_settings_new ();

    /* Initially not dirty */
    g_assert_false (lrg_settings_group_is_dirty (LRG_SETTINGS_GROUP (settings)));

    /* Change a setting - should become dirty */
    lp_gameplay_settings_set_autosave_interval (settings, 10);
    g_assert_true (lrg_settings_group_is_dirty (LRG_SETTINGS_GROUP (settings)));

    /* mark_clean clears dirty flag (called by LrgSettings after save) */
    lrg_settings_group_mark_clean (LRG_SETTINGS_GROUP (settings));
    g_assert_false (lrg_settings_group_is_dirty (LRG_SETTINGS_GROUP (settings)));
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

    /* Gameplay settings tests */
    g_test_add_func ("/settings/gameplay/creation", test_gameplay_settings_creation);
    g_test_add_func ("/settings/gameplay/group-name", test_gameplay_settings_group_name);
    g_test_add_func ("/settings/gameplay/defaults", test_gameplay_settings_defaults);
    g_test_add_func ("/settings/gameplay/autosave", test_gameplay_settings_autosave);
    g_test_add_func ("/settings/gameplay/events", test_gameplay_settings_events);
    g_test_add_func ("/settings/gameplay/reset", test_gameplay_settings_reset);
    g_test_add_func ("/settings/gameplay/serialization", test_gameplay_settings_serialization);
    g_test_add_func ("/settings/gameplay/dirty-tracking", test_gameplay_settings_dirty_tracking);

    return g_test_run ();
}
