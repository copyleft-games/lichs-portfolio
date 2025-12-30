/* test-exposure.c - Exposure Manager Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include "core/lp-exposure-manager.h"
#include "lp-enums.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpExposureManager *manager;
} ExposureFixture;

static void
fixture_set_up (ExposureFixture *fixture,
                gconstpointer    user_data)
{
    (void)user_data;

    /* Get the singleton */
    fixture->manager = lp_exposure_manager_get_default ();
    g_assert_nonnull (fixture->manager);

    /* Reset to known state */
    lp_exposure_manager_set_exposure (fixture->manager, 0);
}

static void
fixture_tear_down (ExposureFixture *fixture,
                   gconstpointer    user_data)
{
    (void)user_data;
    (void)fixture;

    /* Don't unref the singleton */
}

/* ==========================================================================
 * Tests
 * ========================================================================== */

static void
test_exposure_singleton (ExposureFixture *fixture,
                         gconstpointer    user_data)
{
    LpExposureManager *manager2;

    (void)user_data;

    /* Singleton should return the same instance */
    manager2 = lp_exposure_manager_get_default ();
    g_assert_true (fixture->manager == manager2);
}

static void
test_exposure_initial_value (ExposureFixture *fixture,
                             gconstpointer    user_data)
{
    guint exposure;

    (void)user_data;

    /* Exposure should be 0 after reset */
    exposure = lp_exposure_manager_get_exposure (fixture->manager);
    g_assert_cmpuint (exposure, ==, 0);
}

static void
test_exposure_set_get (ExposureFixture *fixture,
                       gconstpointer    user_data)
{
    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 50);
    g_assert_cmpuint (lp_exposure_manager_get_exposure (fixture->manager), ==, 50);

    lp_exposure_manager_set_exposure (fixture->manager, 100);
    g_assert_cmpuint (lp_exposure_manager_get_exposure (fixture->manager), ==, 100);
}

static void
test_exposure_clamp_max (ExposureFixture *fixture,
                         gconstpointer    user_data)
{
    (void)user_data;

    /* Exposure should be clamped to 100 */
    lp_exposure_manager_set_exposure (fixture->manager, 150);
    g_assert_cmpuint (lp_exposure_manager_get_exposure (fixture->manager), ==, 100);
}

static void
test_exposure_add (ExposureFixture *fixture,
                   gconstpointer    user_data)
{
    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 10);
    lp_exposure_manager_add_exposure (fixture->manager, 25);
    g_assert_cmpuint (lp_exposure_manager_get_exposure (fixture->manager), ==, 35);
}

static void
test_exposure_add_clamp (ExposureFixture *fixture,
                         gconstpointer    user_data)
{
    (void)user_data;

    /* Adding exposure should clamp at 100 */
    lp_exposure_manager_set_exposure (fixture->manager, 90);
    lp_exposure_manager_add_exposure (fixture->manager, 50);
    g_assert_cmpuint (lp_exposure_manager_get_exposure (fixture->manager), ==, 100);
}

static void
test_exposure_level_hidden (ExposureFixture *fixture,
                            gconstpointer    user_data)
{
    LpExposureLevel level;

    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 0);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_HIDDEN);

    lp_exposure_manager_set_exposure (fixture->manager, 24);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_HIDDEN);
}

static void
test_exposure_level_scrutiny (ExposureFixture *fixture,
                              gconstpointer    user_data)
{
    LpExposureLevel level;

    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 25);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_SCRUTINY);

    lp_exposure_manager_set_exposure (fixture->manager, 49);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_SCRUTINY);
}

static void
test_exposure_level_suspicion (ExposureFixture *fixture,
                               gconstpointer    user_data)
{
    LpExposureLevel level;

    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 50);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_SUSPICION);

    lp_exposure_manager_set_exposure (fixture->manager, 74);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_SUSPICION);
}

static void
test_exposure_level_hunt (ExposureFixture *fixture,
                          gconstpointer    user_data)
{
    LpExposureLevel level;

    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 75);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_HUNT);

    lp_exposure_manager_set_exposure (fixture->manager, 99);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_HUNT);
}

static void
test_exposure_level_crusade (ExposureFixture *fixture,
                             gconstpointer    user_data)
{
    LpExposureLevel level;

    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 100);
    level = lp_exposure_manager_get_level (fixture->manager);
    g_assert_cmpint (level, ==, LP_EXPOSURE_LEVEL_CRUSADE);
}

static void
test_exposure_decay (ExposureFixture *fixture,
                     gconstpointer    user_data)
{
    guint before;
    guint after;

    (void)user_data;

    lp_exposure_manager_set_exposure (fixture->manager, 50);
    before = lp_exposure_manager_get_exposure (fixture->manager);

    lp_exposure_manager_apply_decay (fixture->manager, 10);
    after = lp_exposure_manager_get_exposure (fixture->manager);

    g_assert_cmpuint (after, <, before);
}

static void
test_exposure_decay_minimum (ExposureFixture *fixture,
                             gconstpointer    user_data)
{
    (void)user_data;

    /* Decay should not go below 0 */
    lp_exposure_manager_set_exposure (fixture->manager, 5);
    lp_exposure_manager_apply_decay (fixture->manager, 100);

    g_assert_cmpuint (lp_exposure_manager_get_exposure (fixture->manager), ==, 0);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/exposure/singleton",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_singleton, fixture_tear_down);

    g_test_add ("/exposure/initial-value",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_initial_value, fixture_tear_down);

    g_test_add ("/exposure/set-get",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_set_get, fixture_tear_down);

    g_test_add ("/exposure/clamp-max",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_clamp_max, fixture_tear_down);

    g_test_add ("/exposure/add",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_add, fixture_tear_down);

    g_test_add ("/exposure/add-clamp",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_add_clamp, fixture_tear_down);

    g_test_add ("/exposure/level/hidden",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_level_hidden, fixture_tear_down);

    g_test_add ("/exposure/level/scrutiny",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_level_scrutiny, fixture_tear_down);

    g_test_add ("/exposure/level/suspicion",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_level_suspicion, fixture_tear_down);

    g_test_add ("/exposure/level/hunt",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_level_hunt, fixture_tear_down);

    g_test_add ("/exposure/level/crusade",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_level_crusade, fixture_tear_down);

    g_test_add ("/exposure/decay",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_decay, fixture_tear_down);

    g_test_add ("/exposure/decay-minimum",
                ExposureFixture, NULL,
                fixture_set_up, test_exposure_decay_minimum, fixture_tear_down);

    return g_test_run ();
}
