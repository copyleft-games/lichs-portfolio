/* test-synergy.c - Synergy Manager Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include "core/lp-synergy-manager.h"
#include "lp-enums.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpSynergyManager *manager;
} SynergyFixture;

static void
fixture_set_up (SynergyFixture *fixture,
                gconstpointer   user_data)
{
    (void)user_data;

    /* Get the singleton */
    fixture->manager = lp_synergy_manager_get_default ();
    g_assert_nonnull (fixture->manager);

    /* Reset to known state */
    lp_synergy_manager_reset (fixture->manager);
}

static void
fixture_tear_down (SynergyFixture *fixture,
                   gconstpointer   user_data)
{
    (void)user_data;
    (void)fixture;

    /* Don't unref the singleton */
}

/* ==========================================================================
 * Tests
 * ========================================================================== */

static void
test_synergy_singleton (SynergyFixture *fixture,
                        gconstpointer   user_data)
{
    LpSynergyManager *manager2;

    (void)user_data;

    /* Singleton should return the same instance */
    manager2 = lp_synergy_manager_get_default ();
    g_assert_true (fixture->manager == manager2);
}

static void
test_synergy_initial_state (SynergyFixture *fixture,
                            gconstpointer   user_data)
{
    guint count;
    gdouble bonus;

    (void)user_data;

    /* After reset, should have no active synergies */
    count = lp_synergy_manager_get_synergy_count (fixture->manager);
    g_assert_cmpuint (count, ==, 0);

    /* Bonus should be 1.0 (no effect) */
    bonus = lp_synergy_manager_get_total_bonus (fixture->manager);
    g_assert_cmpfloat (bonus, ==, 1.0);
}

static void
test_synergy_active_synergies_empty (SynergyFixture *fixture,
                                     gconstpointer   user_data)
{
    GPtrArray *synergies;

    (void)user_data;

    /* Get active synergies array */
    synergies = lp_synergy_manager_get_active_synergies (fixture->manager);
    g_assert_nonnull (synergies);

    /* Should be empty after reset */
    g_assert_cmpuint (synergies->len, ==, 0);
}

static void
test_synergy_recalculate_null_portfolio (SynergyFixture *fixture,
                                         gconstpointer   user_data)
{
    guint count;

    (void)user_data;

    /*
     * Recalculating with NULL portfolio should not crash
     * and should result in no active synergies.
     */
    lp_synergy_manager_recalculate (fixture->manager, NULL);
    count = lp_synergy_manager_get_synergy_count (fixture->manager);
    g_assert_cmpuint (count, ==, 0);
}

static void
test_synergy_reset_clears_state (SynergyFixture *fixture,
                                 gconstpointer   user_data)
{
    (void)user_data;

    /* Reset should return manager to clean state */
    lp_synergy_manager_reset (fixture->manager);

    g_assert_cmpuint (
        lp_synergy_manager_get_synergy_count (fixture->manager), ==, 0);
    g_assert_cmpfloat (
        lp_synergy_manager_get_total_bonus (fixture->manager), ==, 1.0);
}

static void
test_synergy_bonus_never_below_one (SynergyFixture *fixture,
                                    gconstpointer   user_data)
{
    gdouble bonus;

    (void)user_data;

    /*
     * Bonus should never be below 1.0 (would reduce income).
     * Even with no synergies, bonus should be 1.0 (no effect).
     */
    bonus = lp_synergy_manager_get_total_bonus (fixture->manager);
    g_assert_cmpfloat (bonus, >=, 1.0);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/synergy/singleton",
                SynergyFixture, NULL,
                fixture_set_up, test_synergy_singleton, fixture_tear_down);

    g_test_add ("/synergy/initial-state",
                SynergyFixture, NULL,
                fixture_set_up, test_synergy_initial_state, fixture_tear_down);

    g_test_add ("/synergy/active-synergies-empty",
                SynergyFixture, NULL,
                fixture_set_up, test_synergy_active_synergies_empty, fixture_tear_down);

    g_test_add ("/synergy/recalculate-null-portfolio",
                SynergyFixture, NULL,
                fixture_set_up, test_synergy_recalculate_null_portfolio, fixture_tear_down);

    g_test_add ("/synergy/reset-clears-state",
                SynergyFixture, NULL,
                fixture_set_up, test_synergy_reset_clears_state, fixture_tear_down);

    g_test_add ("/synergy/bonus-never-below-one",
                SynergyFixture, NULL,
                fixture_set_up, test_synergy_bonus_never_below_one, fixture_tear_down);

    return g_test_run ();
}
