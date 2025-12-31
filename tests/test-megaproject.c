/* test-megaproject.c - Multi-Century Project Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include <libregnum.h>
#include "core/lp-megaproject.h"
#include "lp-enums.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpMegaproject *project;
} MegaprojectFixture;

static void
fixture_set_up (MegaprojectFixture *fixture,
                gconstpointer       user_data)
{
    LpMegaprojectPhase *phase1;
    LpMegaprojectPhase *phase2;
    LpMegaprojectPhase *phase3;

    (void)user_data;

    fixture->project = lp_megaproject_new ("shadow-network", "Shadow Network");
    g_assert_nonnull (fixture->project);

    /* Add test phases */
    phase1 = lp_megaproject_phase_new ("Foundation", 50);
    phase2 = lp_megaproject_phase_new ("Expansion", 100);
    phase3 = lp_megaproject_phase_new ("Completion", 50);

    lp_megaproject_add_phase (fixture->project, phase1);
    lp_megaproject_add_phase (fixture->project, phase2);
    lp_megaproject_add_phase (fixture->project, phase3);

    lp_megaproject_set_unlock_level (fixture->project, 5);
    lp_megaproject_set_discovery_risk (fixture->project, 5);
}

static void
fixture_tear_down (MegaprojectFixture *fixture,
                   gconstpointer       user_data)
{
    (void)user_data;

    g_clear_object (&fixture->project);
}

/* ==========================================================================
 * Construction Tests
 * ========================================================================== */

static void
test_megaproject_new (MegaprojectFixture *fixture,
                      gconstpointer       user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_MEGAPROJECT (fixture->project));
    g_assert_cmpstr (lp_megaproject_get_id (fixture->project), ==, "shadow-network");
    g_assert_cmpstr (lp_megaproject_get_name (fixture->project), ==, "Shadow Network");
}

static void
test_megaproject_phases (MegaprojectFixture *fixture,
                         gconstpointer       user_data)
{
    GPtrArray *phases;

    (void)user_data;

    phases = lp_megaproject_get_phases (fixture->project);
    g_assert_nonnull (phases);
    g_assert_cmpuint (phases->len, ==, 3);
}

static void
test_megaproject_total_duration (MegaprojectFixture *fixture,
                                 gconstpointer       user_data)
{
    guint duration;

    (void)user_data;

    /* 50 + 100 + 50 = 200 years */
    duration = lp_megaproject_get_total_duration (fixture->project);
    g_assert_cmpuint (duration, ==, 200);
}

/* ==========================================================================
 * State Tests
 * ========================================================================== */

static void
test_megaproject_initial_state (MegaprojectFixture *fixture,
                                gconstpointer       user_data)
{
    LpMegaprojectState state;

    (void)user_data;

    state = lp_megaproject_get_state (fixture->project);

    /* New projects should be LOCKED until unlock level met */
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_LOCKED);
}

static void
test_megaproject_can_start (MegaprojectFixture *fixture,
                            gconstpointer       user_data)
{
    (void)user_data;

    /*
     * can_start requires AVAILABLE state. Transition from LOCKED
     * by temporarily setting unlock_level to 0 and calling reset.
     */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);
    lp_megaproject_set_unlock_level (fixture->project, 5);

    /* Unlock level is 5, so level 4 should fail */
    g_assert_false (lp_megaproject_can_start (fixture->project, 4));

    /* Level 5 should succeed */
    g_assert_true (lp_megaproject_can_start (fixture->project, 5));

    /* Level 10 should also succeed */
    g_assert_true (lp_megaproject_can_start (fixture->project, 10));
}

static void
test_megaproject_start (MegaprojectFixture *fixture,
                        gconstpointer       user_data)
{
    gboolean result;
    LpMegaprojectState state;

    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    result = lp_megaproject_start (fixture->project);
    g_assert_true (result);

    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_ACTIVE);
}

static void
test_megaproject_pause_resume (MegaprojectFixture *fixture,
                               gconstpointer       user_data)
{
    LpMegaprojectState state;

    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    lp_megaproject_start (fixture->project);
    lp_megaproject_pause (fixture->project);

    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_PAUSED);

    lp_megaproject_resume (fixture->project);

    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_ACTIVE);
}

/* ==========================================================================
 * Progress Tests
 * ========================================================================== */

static void
test_megaproject_initial_progress (MegaprojectFixture *fixture,
                                   gconstpointer       user_data)
{
    (void)user_data;

    g_assert_cmpuint (lp_megaproject_get_years_invested (fixture->project), ==, 0);
    g_assert_cmpuint (lp_megaproject_get_years_remaining (fixture->project), ==, 200);
    g_assert_cmpfloat (lp_megaproject_get_progress (fixture->project), ==, 0.0f);
}

static void
test_megaproject_advance_years (MegaprojectFixture *fixture,
                                gconstpointer       user_data)
{
    guint invested;

    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    lp_megaproject_start (fixture->project);
    lp_megaproject_advance_years (fixture->project, 25);

    invested = lp_megaproject_get_years_invested (fixture->project);
    g_assert_cmpuint (invested, ==, 25);
}

static void
test_megaproject_phase_transitions (MegaprojectFixture *fixture,
                                    gconstpointer       user_data)
{
    const LpMegaprojectPhase *phase;
    guint phase_idx;

    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    lp_megaproject_start (fixture->project);

    /* Initially on phase 0 */
    phase_idx = lp_megaproject_get_current_phase_index (fixture->project);
    g_assert_cmpuint (phase_idx, ==, 0);

    phase = lp_megaproject_get_current_phase (fixture->project);
    g_assert_cmpstr (phase->name, ==, "Foundation");

    /* Advance past first phase (50 years) */
    lp_megaproject_advance_years (fixture->project, 55);

    phase_idx = lp_megaproject_get_current_phase_index (fixture->project);
    g_assert_cmpuint (phase_idx, ==, 1);

    phase = lp_megaproject_get_current_phase (fixture->project);
    g_assert_cmpstr (phase->name, ==, "Expansion");
}

static void
test_megaproject_completion (MegaprojectFixture *fixture,
                             gconstpointer       user_data)
{
    LpMegaprojectState state;

    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    lp_megaproject_start (fixture->project);
    lp_megaproject_advance_years (fixture->project, 250); /* More than total */

    g_assert_true (lp_megaproject_is_complete (fixture->project));

    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_COMPLETE);
}

static void
test_megaproject_progress_percentage (MegaprojectFixture *fixture,
                                      gconstpointer       user_data)
{
    gfloat progress;

    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    lp_megaproject_start (fixture->project);

    /* 50 years out of 200 = 25% */
    lp_megaproject_advance_years (fixture->project, 50);
    progress = lp_megaproject_get_progress (fixture->project);
    g_assert_cmpfloat (progress, ==, 0.25f);

    /* 100 more years = 75% total */
    lp_megaproject_advance_years (fixture->project, 100);
    progress = lp_megaproject_get_progress (fixture->project);
    g_assert_cmpfloat (progress, ==, 0.75f);
}

/* ==========================================================================
 * Discovery Risk Tests
 * ========================================================================== */

static void
test_megaproject_discovery_risk (MegaprojectFixture *fixture,
                                 gconstpointer       user_data)
{
    guint risk;

    (void)user_data;

    risk = lp_megaproject_get_discovery_risk (fixture->project);
    g_assert_cmpuint (risk, ==, 5);
}

static void
test_megaproject_destroy (MegaprojectFixture *fixture,
                          gconstpointer       user_data)
{
    LpMegaprojectState state;

    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    lp_megaproject_start (fixture->project);
    lp_megaproject_destroy (fixture->project);

    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_DESTROYED);
}

/* ==========================================================================
 * Phase Boxed Type Tests
 * ========================================================================== */

static void
test_megaproject_phase_new (void)
{
    LpMegaprojectPhase *phase;

    phase = lp_megaproject_phase_new ("Test Phase", 75);

    g_assert_nonnull (phase);
    g_assert_cmpstr (phase->name, ==, "Test Phase");
    g_assert_cmpuint (phase->years, ==, 75);

    lp_megaproject_phase_free (phase);
}

static void
test_megaproject_phase_copy (void)
{
    LpMegaprojectPhase *phase;
    LpMegaprojectPhase *copy;

    phase = lp_megaproject_phase_new ("Original", 100);
    phase->effect_type = g_strdup ("income_bonus");
    phase->effect_value = 0.15;

    copy = lp_megaproject_phase_copy (phase);

    g_assert_nonnull (copy);
    g_assert_true (copy != phase);
    g_assert_cmpstr (copy->name, ==, phase->name);
    g_assert_cmpuint (copy->years, ==, phase->years);
    g_assert_cmpstr (copy->effect_type, ==, phase->effect_type);
    g_assert_cmpfloat (copy->effect_value, ==, phase->effect_value);

    lp_megaproject_phase_free (phase);
    lp_megaproject_phase_free (copy);
}

/* ==========================================================================
 * Reset Tests
 * ========================================================================== */

static void
test_megaproject_reset (MegaprojectFixture *fixture,
                        gconstpointer       user_data)
{
    (void)user_data;

    /* Transition to AVAILABLE state first */
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);

    /* Make progress */
    lp_megaproject_start (fixture->project);
    lp_megaproject_advance_years (fixture->project, 75);

    /* Reset should clear progress */
    lp_megaproject_reset (fixture->project);

    g_assert_cmpuint (lp_megaproject_get_years_invested (fixture->project), ==, 0);
    g_assert_cmpfloat (lp_megaproject_get_progress (fixture->project), ==, 0.0f);
}

/* ==========================================================================
 * Configuration Tests
 * ========================================================================== */

static void
test_megaproject_description (MegaprojectFixture *fixture,
                              gconstpointer       user_data)
{
    (void)user_data;

    lp_megaproject_set_description (fixture->project,
                                    "A vast underground network.");

    g_assert_cmpstr (lp_megaproject_get_description (fixture->project), ==,
                     "A vast underground network.");
}

static void
test_megaproject_unlock_level (MegaprojectFixture *fixture,
                               gconstpointer       user_data)
{
    (void)user_data;

    lp_megaproject_set_unlock_level (fixture->project, 10);
    g_assert_cmpuint (lp_megaproject_get_unlock_level (fixture->project), ==, 10);
}

static void
test_megaproject_cost_per_year (MegaprojectFixture *fixture,
                                gconstpointer       user_data)
{
    g_autoptr(LrgBigNumber) cost = NULL;
    const LrgBigNumber *retrieved;

    (void)user_data;

    cost = lrg_big_number_new (500.0);
    lp_megaproject_set_cost_per_year (fixture->project, cost);

    retrieved = lp_megaproject_get_cost_per_year (fixture->project);
    g_assert_nonnull (retrieved);
    g_assert_cmpfloat (lrg_big_number_to_double (retrieved), ==, 500.0);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Construction tests */
    g_test_add ("/megaproject/new",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_new, fixture_tear_down);

    g_test_add ("/megaproject/phases",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_phases, fixture_tear_down);

    g_test_add ("/megaproject/total-duration",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_total_duration, fixture_tear_down);

    /* State tests */
    g_test_add ("/megaproject/initial-state",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_initial_state, fixture_tear_down);

    g_test_add ("/megaproject/can-start",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_can_start, fixture_tear_down);

    g_test_add ("/megaproject/start",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_start, fixture_tear_down);

    g_test_add ("/megaproject/pause-resume",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_pause_resume, fixture_tear_down);

    /* Progress tests */
    g_test_add ("/megaproject/initial-progress",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_initial_progress, fixture_tear_down);

    g_test_add ("/megaproject/advance-years",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_advance_years, fixture_tear_down);

    g_test_add ("/megaproject/phase-transitions",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_phase_transitions, fixture_tear_down);

    g_test_add ("/megaproject/completion",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_completion, fixture_tear_down);

    g_test_add ("/megaproject/progress-percentage",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_progress_percentage, fixture_tear_down);

    /* Discovery risk tests */
    g_test_add ("/megaproject/discovery-risk",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_discovery_risk, fixture_tear_down);

    g_test_add ("/megaproject/destroy",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_destroy, fixture_tear_down);

    /* Phase boxed type tests */
    g_test_add_func ("/megaproject/phase/new", test_megaproject_phase_new);
    g_test_add_func ("/megaproject/phase/copy", test_megaproject_phase_copy);

    /* Reset tests */
    g_test_add ("/megaproject/reset",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_reset, fixture_tear_down);

    /* Configuration tests */
    g_test_add ("/megaproject/description",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_description, fixture_tear_down);

    g_test_add ("/megaproject/unlock-level",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_unlock_level, fixture_tear_down);

    g_test_add ("/megaproject/cost-per-year",
                MegaprojectFixture, NULL,
                fixture_set_up, test_megaproject_cost_per_year, fixture_tear_down);

    return g_test_run ();
}
