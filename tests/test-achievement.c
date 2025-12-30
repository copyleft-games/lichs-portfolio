/* test-achievement.c - Achievement System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the achievement system (Phase 8).
 * Tests LpAchievementManager and LpSteamBridge functionality.
 */

#include <glib.h>
#include <libregnum.h>

#include "achievement/lp-achievement-manager.h"
#include "steam/lp-steam-bridge.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpAchievementManager *manager;
} AchievementFixture;

static void
fixture_set_up (AchievementFixture *fixture,
                gconstpointer       user_data)
{
    (void)user_data;

    /* Get fresh manager - need to reset between tests */
    fixture->manager = lp_achievement_manager_get_default ();

    /* Load achievement definitions */
    lp_achievement_manager_load_definitions (fixture->manager, NULL, NULL);
}

static void
fixture_tear_down (AchievementFixture *fixture,
                   gconstpointer       user_data)
{
    (void)user_data;

    /* Reset achievements between tests */
    if (fixture->manager != NULL)
        lp_achievement_manager_reset (fixture->manager);

    /* Note: Don't unref singleton - it's owned by the system */
    fixture->manager = NULL;
}

/* ==========================================================================
 * Basic Achievement Manager Tests
 * ========================================================================== */

static void
test_achievement_manager_singleton (void)
{
    LpAchievementManager *manager1;
    LpAchievementManager *manager2;

    manager1 = lp_achievement_manager_get_default ();
    manager2 = lp_achievement_manager_get_default ();

    g_assert_nonnull (manager1);
    g_assert_true (manager1 == manager2);
}

static void
test_achievement_manager_type (void)
{
    LpAchievementManager *manager;

    manager = lp_achievement_manager_get_default ();

    g_assert_true (LP_IS_ACHIEVEMENT_MANAGER (manager));
    g_assert_true (G_IS_OBJECT (manager));
}

static void
test_achievement_manager_saveable (void)
{
    LpAchievementManager *manager;

    manager = lp_achievement_manager_get_default ();

    g_assert_true (LRG_IS_SAVEABLE (manager));
}

/* ==========================================================================
 * Achievement Definition Tests
 * ========================================================================== */

static void
test_achievement_definitions_loaded (AchievementFixture *fixture,
                                     gconstpointer       user_data)
{
    guint total;

    (void)user_data;

    total = lp_achievement_manager_get_total_count (fixture->manager);

    /* Should have 8 built-in achievements */
    g_assert_cmpuint (total, ==, 8);
}

static void
test_achievement_get_by_id (AchievementFixture *fixture,
                            gconstpointer       user_data)
{
    LrgAchievement *achievement;

    (void)user_data;

    /* Test valid ID */
    achievement = lp_achievement_manager_get_achievement (fixture->manager,
                                                          "first_million");
    g_assert_nonnull (achievement);
    g_assert_cmpstr (lrg_achievement_get_name (achievement), ==, "First Million");

    /* Test another valid ID */
    achievement = lp_achievement_manager_get_achievement (fixture->manager,
                                                          "centennial");
    g_assert_nonnull (achievement);
    g_assert_cmpstr (lrg_achievement_get_name (achievement), ==, "Centennial");

    /* Test invalid ID */
    achievement = lp_achievement_manager_get_achievement (fixture->manager,
                                                          "nonexistent");
    g_assert_null (achievement);
}

static void
test_achievement_hidden_flag (AchievementFixture *fixture,
                              gconstpointer       user_data)
{
    LrgAchievement *visible;
    LrgAchievement *hidden;

    (void)user_data;

    /* First Million should be visible */
    visible = lp_achievement_manager_get_achievement (fixture->manager,
                                                      "first_million");
    g_assert_nonnull (visible);
    g_assert_false (lrg_achievement_is_hidden (visible));

    /* Dark Awakening should be hidden */
    hidden = lp_achievement_manager_get_achievement (fixture->manager,
                                                     "dark_awakening");
    g_assert_nonnull (hidden);
    g_assert_true (lrg_achievement_is_hidden (hidden));
}

static void
test_achievement_points (AchievementFixture *fixture,
                         gconstpointer       user_data)
{
    LrgAchievement *achievement;

    (void)user_data;

    achievement = lp_achievement_manager_get_achievement (fixture->manager,
                                                          "transcendence");
    g_assert_nonnull (achievement);
    g_assert_cmpuint (lrg_achievement_get_points (achievement), ==, 100);

    achievement = lp_achievement_manager_get_achievement (fixture->manager,
                                                          "first_million");
    g_assert_nonnull (achievement);
    g_assert_cmpuint (lrg_achievement_get_points (achievement), ==, 10);
}

/* ==========================================================================
 * Unlock Tests
 * ========================================================================== */

static void
test_achievement_unlock (AchievementFixture *fixture,
                         gconstpointer       user_data)
{
    gboolean unlocked;

    (void)user_data;

    /* Initially not unlocked */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "transcendence"));

    /* Unlock it */
    unlocked = lp_achievement_manager_unlock (fixture->manager, "transcendence");
    g_assert_true (unlocked);

    /* Should be unlocked now */
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager,
                                                       "transcendence"));

    /* Second unlock attempt should return FALSE */
    unlocked = lp_achievement_manager_unlock (fixture->manager, "transcendence");
    g_assert_false (unlocked);
}

static void
test_achievement_unlocked_count (AchievementFixture *fixture,
                                 gconstpointer       user_data)
{
    guint count;

    (void)user_data;

    /* Initially 0 unlocked */
    count = lp_achievement_manager_get_unlocked_count (fixture->manager);
    g_assert_cmpuint (count, ==, 0);

    /* Unlock one */
    lp_achievement_manager_unlock (fixture->manager, "first_million");
    count = lp_achievement_manager_get_unlocked_count (fixture->manager);
    g_assert_cmpuint (count, ==, 1);

    /* Unlock another */
    lp_achievement_manager_unlock (fixture->manager, "centennial");
    count = lp_achievement_manager_get_unlocked_count (fixture->manager);
    g_assert_cmpuint (count, ==, 2);
}

static void
test_achievement_completion_percentage (AchievementFixture *fixture,
                                        gconstpointer       user_data)
{
    gdouble percentage;

    (void)user_data;

    /* Initially 0% */
    percentage = lp_achievement_manager_get_completion_percentage (fixture->manager);
    g_assert_cmpfloat (percentage, ==, 0.0);

    /* Unlock 4 of 8 = 50% */
    lp_achievement_manager_unlock (fixture->manager, "first_million");
    lp_achievement_manager_unlock (fixture->manager, "centennial");
    lp_achievement_manager_unlock (fixture->manager, "dynasty");
    lp_achievement_manager_unlock (fixture->manager, "transcendence");

    percentage = lp_achievement_manager_get_completion_percentage (fixture->manager);
    g_assert_cmpfloat (percentage, ==, 0.5);
}

/* ==========================================================================
 * Progress Tests
 * ========================================================================== */

static void
test_achievement_progress_set (AchievementFixture *fixture,
                               gconstpointer       user_data)
{
    gint64 progress;
    guint percentage;

    (void)user_data;

    /* Initial progress is 0 */
    progress = lp_achievement_manager_get_progress (fixture->manager, "first_million");
    g_assert_cmpint (progress, ==, 0);

    /* Set progress */
    lp_achievement_manager_set_progress (fixture->manager, "first_million", 500000);
    progress = lp_achievement_manager_get_progress (fixture->manager, "first_million");
    g_assert_cmpint (progress, ==, 500000);

    /* Check percentage (500000 of 1000000 = 50%) */
    percentage = lp_achievement_manager_get_progress_percentage (fixture->manager,
                                                                  "first_million");
    g_assert_cmpuint (percentage, ==, 50);
}

static void
test_achievement_progress_increment (AchievementFixture *fixture,
                                     gconstpointer       user_data)
{
    gint64 progress;

    (void)user_data;

    /* Increment progress */
    lp_achievement_manager_increment_progress (fixture->manager, "centennial", 25);
    progress = lp_achievement_manager_get_progress (fixture->manager, "centennial");
    g_assert_cmpint (progress, ==, 25);

    /* Increment again */
    lp_achievement_manager_increment_progress (fixture->manager, "centennial", 50);
    progress = lp_achievement_manager_get_progress (fixture->manager, "centennial");
    g_assert_cmpint (progress, ==, 75);
}

static void
test_achievement_progress_auto_unlock (AchievementFixture *fixture,
                                       gconstpointer       user_data)
{
    (void)user_data;

    /* Not unlocked initially */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager, "centennial"));

    /* Set progress to target (100 years) */
    lp_achievement_manager_set_progress (fixture->manager, "centennial", 100);

    /* Should auto-unlock */
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager, "centennial"));
}

/* ==========================================================================
 * Statistics Tests
 * ========================================================================== */

static void
test_achievement_stats (AchievementFixture *fixture,
                        gconstpointer       user_data)
{
    gint64 value;

    (void)user_data;

    /* Initial value is 0 */
    value = lp_achievement_manager_get_stat (fixture->manager, "test_stat");
    g_assert_cmpint (value, ==, 0);

    /* Set value */
    lp_achievement_manager_set_stat (fixture->manager, "test_stat", 42);
    value = lp_achievement_manager_get_stat (fixture->manager, "test_stat");
    g_assert_cmpint (value, ==, 42);

    /* Increment */
    lp_achievement_manager_increment_stat (fixture->manager, "test_stat", 8);
    value = lp_achievement_manager_get_stat (fixture->manager, "test_stat");
    g_assert_cmpint (value, ==, 50);
}

/* ==========================================================================
 * Game Event Hook Tests
 * ========================================================================== */

static void
test_achievement_on_gold_changed (AchievementFixture *fixture,
                                  gconstpointer       user_data)
{
    (void)user_data;

    /* Not unlocked initially */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "first_million"));

    /* Trigger gold changed event with less than target */
    lp_achievement_manager_on_gold_changed (fixture->manager, 500000.0);
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "first_million"));

    /* Trigger with more than target */
    lp_achievement_manager_on_gold_changed (fixture->manager, 1500000.0);
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager,
                                                       "first_million"));
}

static void
test_achievement_on_slumber_complete (AchievementFixture *fixture,
                                      gconstpointer       user_data)
{
    (void)user_data;

    /* Not unlocked initially */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "centennial"));

    /* Short slumber - not enough */
    lp_achievement_manager_on_slumber_complete (fixture->manager, 50);
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "centennial"));

    /* 100 year slumber - should unlock */
    lp_achievement_manager_on_slumber_complete (fixture->manager, 100);
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager,
                                                       "centennial"));
}

static void
test_achievement_on_family_succession (AchievementFixture *fixture,
                                       gconstpointer       user_data)
{
    (void)user_data;

    /* Not unlocked initially */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "dynasty"));

    /* Succession events */
    lp_achievement_manager_on_family_succession (fixture->manager, 2);
    lp_achievement_manager_on_family_succession (fixture->manager, 3);
    lp_achievement_manager_on_family_succession (fixture->manager, 4);
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "dynasty"));

    /* 5th generation - should unlock */
    lp_achievement_manager_on_family_succession (fixture->manager, 5);
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager,
                                                       "dynasty"));
}

static void
test_achievement_on_prestige (AchievementFixture *fixture,
                              gconstpointer       user_data)
{
    (void)user_data;

    /* Not unlocked initially */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "transcendence"));

    /* Prestige event */
    lp_achievement_manager_on_prestige (fixture->manager, 1000);

    /* Should unlock transcendence */
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager,
                                                       "transcendence"));
}

static void
test_achievement_on_dark_unlock (AchievementFixture *fixture,
                                 gconstpointer       user_data)
{
    (void)user_data;

    /* Hidden achievement - not unlocked initially */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "dark_awakening"));

    /* Dark unlock event */
    lp_achievement_manager_on_dark_unlock (fixture->manager);

    /* Should unlock */
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager,
                                                       "dark_awakening"));
}

static void
test_achievement_on_kingdom_debt (AchievementFixture *fixture,
                                  gconstpointer       user_data)
{
    (void)user_data;

    /* Not unlocked initially */
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "hostile_takeover"));

    /* Partial debt ownership */
    lp_achievement_manager_on_kingdom_debt_owned (fixture->manager, "valdris", 0.5);
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager,
                                                        "hostile_takeover"));

    /* Full debt ownership */
    lp_achievement_manager_on_kingdom_debt_owned (fixture->manager, "valdris", 1.0);
    g_assert_true (lp_achievement_manager_is_unlocked (fixture->manager,
                                                       "hostile_takeover"));
}

/* ==========================================================================
 * Steam Bridge Tests
 * ========================================================================== */

static void
test_steam_bridge_singleton (void)
{
    LpSteamBridge *bridge1;
    LpSteamBridge *bridge2;

    bridge1 = lp_steam_bridge_get_default ();
    bridge2 = lp_steam_bridge_get_default ();

    g_assert_nonnull (bridge1);
    g_assert_true (bridge1 == bridge2);
}

static void
test_steam_bridge_type (void)
{
    LpSteamBridge *bridge;

    bridge = lp_steam_bridge_get_default ();

    g_assert_true (LP_IS_STEAM_BRIDGE (bridge));
    g_assert_true (G_IS_OBJECT (bridge));
}

static void
test_steam_bridge_unavailable (void)
{
    LpSteamBridge *bridge;

    bridge = lp_steam_bridge_get_default ();

    /*
     * Steam should be unavailable in test environment
     * (not built with STEAM=1 or Steam client not running)
     */
    g_assert_false (lp_steam_bridge_is_available (bridge));
}

static void
test_steam_bridge_graceful_fallback (void)
{
    LpSteamBridge *bridge;
    gboolean result;

    bridge = lp_steam_bridge_get_default ();

    /* Initialize should succeed (graceful fallback) */
    result = lp_steam_bridge_initialize (bridge, 480, NULL);
    g_assert_true (result);

    /* Still unavailable */
    g_assert_false (lp_steam_bridge_is_available (bridge));

    /* Sync should succeed (no-op) */
    result = lp_steam_bridge_sync_achievement (bridge, "first_million");
    g_assert_true (result);

    /* Store should succeed (no-op) */
    result = lp_steam_bridge_store_stats (bridge);
    g_assert_true (result);

    lp_steam_bridge_shutdown (bridge);
}

static void
test_steam_bridge_user_info_unavailable (void)
{
    LpSteamBridge *bridge;

    bridge = lp_steam_bridge_get_default ();

    /* Should return NULL/0 when unavailable */
    g_assert_null (lp_steam_bridge_get_user_name (bridge));
    g_assert_cmpuint (lp_steam_bridge_get_user_id (bridge), ==, 0);
}

/* ==========================================================================
 * Reset Tests
 * ========================================================================== */

static void
test_achievement_reset (AchievementFixture *fixture,
                        gconstpointer       user_data)
{
    (void)user_data;

    /* Unlock some achievements */
    lp_achievement_manager_unlock (fixture->manager, "first_million");
    lp_achievement_manager_unlock (fixture->manager, "centennial");
    g_assert_cmpuint (lp_achievement_manager_get_unlocked_count (fixture->manager), ==, 2);

    /* Set some stats */
    lp_achievement_manager_set_stat (fixture->manager, "test_stat", 100);

    /* Reset */
    lp_achievement_manager_reset (fixture->manager);

    /* All achievements should be locked */
    g_assert_cmpuint (lp_achievement_manager_get_unlocked_count (fixture->manager), ==, 0);
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager, "first_million"));
    g_assert_false (lp_achievement_manager_is_unlocked (fixture->manager, "centennial"));
}

/* ==========================================================================
 * Main
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Basic tests */
    g_test_add_func ("/achievement/manager/singleton",
                     test_achievement_manager_singleton);
    g_test_add_func ("/achievement/manager/type",
                     test_achievement_manager_type);
    g_test_add_func ("/achievement/manager/saveable",
                     test_achievement_manager_saveable);

    /* Definition tests */
    g_test_add ("/achievement/definitions/loaded",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_definitions_loaded,
                fixture_tear_down);
    g_test_add ("/achievement/definitions/get-by-id",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_get_by_id,
                fixture_tear_down);
    g_test_add ("/achievement/definitions/hidden-flag",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_hidden_flag,
                fixture_tear_down);
    g_test_add ("/achievement/definitions/points",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_points,
                fixture_tear_down);

    /* Unlock tests */
    g_test_add ("/achievement/unlock/basic",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_unlock,
                fixture_tear_down);
    g_test_add ("/achievement/unlock/count",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_unlocked_count,
                fixture_tear_down);
    g_test_add ("/achievement/unlock/completion-percentage",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_completion_percentage,
                fixture_tear_down);

    /* Progress tests */
    g_test_add ("/achievement/progress/set",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_progress_set,
                fixture_tear_down);
    g_test_add ("/achievement/progress/increment",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_progress_increment,
                fixture_tear_down);
    g_test_add ("/achievement/progress/auto-unlock",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_progress_auto_unlock,
                fixture_tear_down);

    /* Statistics tests */
    g_test_add ("/achievement/stats/basic",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_stats,
                fixture_tear_down);

    /* Game event hook tests */
    g_test_add ("/achievement/hooks/gold-changed",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_on_gold_changed,
                fixture_tear_down);
    g_test_add ("/achievement/hooks/slumber-complete",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_on_slumber_complete,
                fixture_tear_down);
    g_test_add ("/achievement/hooks/family-succession",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_on_family_succession,
                fixture_tear_down);
    g_test_add ("/achievement/hooks/prestige",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_on_prestige,
                fixture_tear_down);
    g_test_add ("/achievement/hooks/dark-unlock",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_on_dark_unlock,
                fixture_tear_down);
    g_test_add ("/achievement/hooks/kingdom-debt",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_on_kingdom_debt,
                fixture_tear_down);

    /* Reset tests */
    g_test_add ("/achievement/reset/basic",
                AchievementFixture, NULL,
                fixture_set_up,
                test_achievement_reset,
                fixture_tear_down);

    /* Steam bridge tests */
    g_test_add_func ("/steam/bridge/singleton",
                     test_steam_bridge_singleton);
    g_test_add_func ("/steam/bridge/type",
                     test_steam_bridge_type);
    g_test_add_func ("/steam/bridge/unavailable",
                     test_steam_bridge_unavailable);
    g_test_add_func ("/steam/bridge/graceful-fallback",
                     test_steam_bridge_graceful_fallback);
    g_test_add_func ("/steam/bridge/user-info-unavailable",
                     test_steam_bridge_user_info_unavailable);

    return g_test_run ();
}
