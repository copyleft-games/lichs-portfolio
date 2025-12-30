/* test-progression.c - Phase 5 Progression System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests the progression systems:
 * - LpPrestigeManager (prestige/echoes/echo trees)
 * - LpMegaproject (multi-century projects)
 * - LpPhylactery (upgrade trees)
 * - LpLedger (discovery progress tracking)
 */

#include <glib.h>
#include <libregnum.h>
#include "core/lp-prestige-manager.h"
#include "core/lp-megaproject.h"
#include "core/lp-phylactery.h"
#include "core/lp-ledger.h"

/* ==========================================================================
 * Megaproject Fixture
 * ========================================================================== */

typedef struct
{
    LpMegaproject *project;
} MegaprojectFixture;

static void
megaproject_fixture_set_up (MegaprojectFixture *fixture,
                            gconstpointer       user_data)
{
    (void)user_data;

    fixture->project = lp_megaproject_new ("proj-001", "Shadow Network");
    g_assert_nonnull (fixture->project);
}

static void
megaproject_fixture_tear_down (MegaprojectFixture *fixture,
                               gconstpointer       user_data)
{
    (void)user_data;

    g_clear_object (&fixture->project);
}

/* ==========================================================================
 * Prestige Manager Fixture
 * ========================================================================== */

typedef struct
{
    LpPrestigeManager *manager;
} PrestigeFixture;

static void
prestige_fixture_set_up (PrestigeFixture *fixture,
                         gconstpointer    user_data)
{
    (void)user_data;

    fixture->manager = lp_prestige_manager_new ();
    g_assert_nonnull (fixture->manager);
}

static void
prestige_fixture_tear_down (PrestigeFixture *fixture,
                            gconstpointer    user_data)
{
    (void)user_data;

    g_clear_object (&fixture->manager);
}

/* ==========================================================================
 * Phylactery Fixture
 * ========================================================================== */

typedef struct
{
    LpPhylactery *phylactery;
} PhylacteryFixture;

static void
phylactery_fixture_set_up (PhylacteryFixture *fixture,
                           gconstpointer      user_data)
{
    (void)user_data;

    fixture->phylactery = lp_phylactery_new ();
    g_assert_nonnull (fixture->phylactery);
}

static void
phylactery_fixture_tear_down (PhylacteryFixture *fixture,
                              gconstpointer      user_data)
{
    (void)user_data;

    g_clear_object (&fixture->phylactery);
}

/* ==========================================================================
 * Ledger Fixture
 * ========================================================================== */

typedef struct
{
    LpLedger *ledger;
} LedgerProgressFixture;

static void
ledger_progress_fixture_set_up (LedgerProgressFixture *fixture,
                                gconstpointer          user_data)
{
    (void)user_data;

    fixture->ledger = lp_ledger_new ();
    g_assert_nonnull (fixture->ledger);
}

static void
ledger_progress_fixture_tear_down (LedgerProgressFixture *fixture,
                                   gconstpointer          user_data)
{
    (void)user_data;

    g_clear_object (&fixture->ledger);
}

/* ==========================================================================
 * Megaproject Phase Tests
 * ========================================================================== */

static void
test_megaproject_phase_new (void)
{
    LpMegaprojectPhase *phase;

    phase = lp_megaproject_phase_new ("Survey", 50);
    g_assert_nonnull (phase);
    g_assert_cmpstr (phase->name, ==, "Survey");
    g_assert_cmpuint (phase->years, ==, 50);
    g_assert_null (phase->effect_type);
    g_assert_cmpfloat (phase->effect_value, ==, 0.0);

    lp_megaproject_phase_free (phase);
}

static void
test_megaproject_phase_copy (void)
{
    LpMegaprojectPhase *phase;
    LpMegaprojectPhase *copy;

    phase = lp_megaproject_phase_new ("Construction", 100);
    phase->effect_type = g_strdup ("property_income_bonus");
    phase->effect_value = 0.1;

    copy = lp_megaproject_phase_copy (phase);
    g_assert_nonnull (copy);
    g_assert_cmpstr (copy->name, ==, "Construction");
    g_assert_cmpuint (copy->years, ==, 100);
    g_assert_cmpstr (copy->effect_type, ==, "property_income_bonus");
    g_assert_cmpfloat (copy->effect_value, ==, 0.1);

    lp_megaproject_phase_free (phase);
    lp_megaproject_phase_free (copy);
}

/* ==========================================================================
 * Megaproject Tests
 * ========================================================================== */

static void
test_megaproject_new (MegaprojectFixture *fixture,
                      gconstpointer       user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_MEGAPROJECT (fixture->project));
    g_assert_true (LRG_IS_SAVEABLE (fixture->project));
}

static void
test_megaproject_id (MegaprojectFixture *fixture,
                     gconstpointer       user_data)
{
    const gchar *id;

    (void)user_data;

    id = lp_megaproject_get_id (fixture->project);
    g_assert_cmpstr (id, ==, "proj-001");
}

static void
test_megaproject_name (MegaprojectFixture *fixture,
                       gconstpointer       user_data)
{
    const gchar *name;

    (void)user_data;

    name = lp_megaproject_get_name (fixture->project);
    g_assert_cmpstr (name, ==, "Shadow Network");
}

static void
test_megaproject_description (MegaprojectFixture *fixture,
                              gconstpointer       user_data)
{
    const gchar *desc;

    (void)user_data;

    lp_megaproject_set_description (fixture->project, "A test project");
    desc = lp_megaproject_get_description (fixture->project);
    g_assert_cmpstr (desc, ==, "A test project");
}

static void
test_megaproject_initial_state (MegaprojectFixture *fixture,
                                gconstpointer       user_data)
{
    LpMegaprojectState state;

    (void)user_data;

    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_LOCKED);
}

static void
test_megaproject_add_phase (MegaprojectFixture *fixture,
                            gconstpointer       user_data)
{
    LpMegaprojectPhase *phase;
    GPtrArray *phases;
    guint duration;

    (void)user_data;

    phase = lp_megaproject_phase_new ("Survey", 50);
    lp_megaproject_add_phase (fixture->project, phase);

    phases = lp_megaproject_get_phases (fixture->project);
    g_assert_cmpuint (phases->len, ==, 1);

    duration = lp_megaproject_get_total_duration (fixture->project);
    g_assert_cmpuint (duration, ==, 50);

    /* Add second phase */
    phase = lp_megaproject_phase_new ("Construction", 100);
    lp_megaproject_add_phase (fixture->project, phase);

    duration = lp_megaproject_get_total_duration (fixture->project);
    g_assert_cmpuint (duration, ==, 150);
}

static void
test_megaproject_unlock_level (MegaprojectFixture *fixture,
                               gconstpointer       user_data)
{
    guint level;

    (void)user_data;

    lp_megaproject_set_unlock_level (fixture->project, 5);
    level = lp_megaproject_get_unlock_level (fixture->project);
    g_assert_cmpuint (level, ==, 5);
}

static void
test_megaproject_discovery_risk (MegaprojectFixture *fixture,
                                 gconstpointer       user_data)
{
    guint risk;

    (void)user_data;

    lp_megaproject_set_discovery_risk (fixture->project, 15);
    risk = lp_megaproject_get_discovery_risk (fixture->project);
    g_assert_cmpuint (risk, ==, 15);

    /* Test clamping to 100 */
    lp_megaproject_set_discovery_risk (fixture->project, 150);
    risk = lp_megaproject_get_discovery_risk (fixture->project);
    g_assert_cmpuint (risk, ==, 100);
}

static void
test_megaproject_cost_per_year (MegaprojectFixture *fixture,
                                gconstpointer       user_data)
{
    g_autoptr(LrgBigNumber) cost = NULL;
    const LrgBigNumber *retrieved;
    gdouble value;

    (void)user_data;

    cost = lrg_big_number_new (1000.0);
    lp_megaproject_set_cost_per_year (fixture->project, cost);

    retrieved = lp_megaproject_get_cost_per_year (fixture->project);
    value = lrg_big_number_to_double (retrieved);
    g_assert_cmpfloat (value, ==, 1000.0);
}

static void
test_megaproject_state_transitions (MegaprojectFixture *fixture,
                                    gconstpointer       user_data)
{
    LpMegaprojectPhase *phase;
    LpMegaprojectState state;
    gboolean result;

    (void)user_data;

    /* Add a phase and set unlock level to 0 (available immediately) */
    phase = lp_megaproject_phase_new ("Phase 1", 10);
    lp_megaproject_add_phase (fixture->project, phase);
    lp_megaproject_set_unlock_level (fixture->project, 0);

    /* Reset to make it available */
    lp_megaproject_reset (fixture->project);
    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_AVAILABLE);

    /* Can start with level >= unlock_level */
    g_assert_true (lp_megaproject_can_start (fixture->project, 0));

    /* Start the project */
    result = lp_megaproject_start (fixture->project);
    g_assert_true (result);
    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_ACTIVE);

    /* Pause the project */
    result = lp_megaproject_pause (fixture->project);
    g_assert_true (result);
    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_PAUSED);

    /* Resume the project */
    result = lp_megaproject_resume (fixture->project);
    g_assert_true (result);
    state = lp_megaproject_get_state (fixture->project);
    g_assert_cmpint (state, ==, LP_MEGAPROJECT_STATE_ACTIVE);
}

static void
test_megaproject_advance_years (MegaprojectFixture *fixture,
                                gconstpointer       user_data)
{
    LpMegaprojectPhase *phase1;
    LpMegaprojectPhase *phase2;
    guint invested;
    guint remaining;
    gfloat progress;

    (void)user_data;

    /* Set up project with 2 phases */
    phase1 = lp_megaproject_phase_new ("Phase 1", 10);
    phase2 = lp_megaproject_phase_new ("Phase 2", 20);
    lp_megaproject_add_phase (fixture->project, phase1);
    lp_megaproject_add_phase (fixture->project, phase2);
    lp_megaproject_set_unlock_level (fixture->project, 0);

    /* Make available and start */
    lp_megaproject_reset (fixture->project);
    lp_megaproject_start (fixture->project);

    /* Initial state */
    invested = lp_megaproject_get_years_invested (fixture->project);
    g_assert_cmpuint (invested, ==, 0);
    remaining = lp_megaproject_get_years_remaining (fixture->project);
    g_assert_cmpuint (remaining, ==, 30);

    /* Advance 5 years */
    lp_megaproject_advance_years (fixture->project, 5);
    invested = lp_megaproject_get_years_invested (fixture->project);
    g_assert_cmpuint (invested, ==, 5);
    remaining = lp_megaproject_get_years_remaining (fixture->project);
    g_assert_cmpuint (remaining, ==, 25);

    /* Check progress */
    progress = lp_megaproject_get_progress (fixture->project);
    g_assert_cmpfloat_with_epsilon (progress, 5.0f / 30.0f, 0.001f);

    /* Still in phase 1 */
    g_assert_cmpuint (lp_megaproject_get_current_phase_index (fixture->project), ==, 0);

    /* Advance to complete phase 1 */
    lp_megaproject_advance_years (fixture->project, 5);
    g_assert_cmpuint (lp_megaproject_get_current_phase_index (fixture->project), ==, 1);

    /* Complete the project */
    lp_megaproject_advance_years (fixture->project, 20);
    g_assert_true (lp_megaproject_is_complete (fixture->project));
    g_assert_cmpint (lp_megaproject_get_state (fixture->project), ==, LP_MEGAPROJECT_STATE_COMPLETE);
}

static void
test_megaproject_effects (MegaprojectFixture *fixture,
                          gconstpointer       user_data)
{
    LpMegaprojectPhase *phase1;
    LpMegaprojectPhase *phase2;
    gdouble bonus;

    (void)user_data;

    /* Set up project with effect phases */
    phase1 = lp_megaproject_phase_new ("Setup", 10);
    phase1->effect_type = g_strdup ("property_income_bonus");
    phase1->effect_value = 0.1;

    phase2 = lp_megaproject_phase_new ("Network", 10);
    phase2->effect_type = g_strdup ("agent_travel");
    phase2->effect_value = 1.0;

    lp_megaproject_add_phase (fixture->project, phase1);
    lp_megaproject_add_phase (fixture->project, phase2);
    lp_megaproject_set_unlock_level (fixture->project, 0);

    /* Start */
    lp_megaproject_reset (fixture->project);
    lp_megaproject_start (fixture->project);

    /* No effects initially */
    bonus = lp_megaproject_get_property_income_bonus (fixture->project);
    g_assert_cmpfloat (bonus, ==, 0.0);
    g_assert_false (lp_megaproject_has_agent_instant_travel (fixture->project));

    /* Complete phase 1 */
    lp_megaproject_advance_years (fixture->project, 10);
    bonus = lp_megaproject_get_property_income_bonus (fixture->project);
    g_assert_cmpfloat (bonus, ==, 0.1);

    /* Complete phase 2 */
    lp_megaproject_advance_years (fixture->project, 10);
    g_assert_true (lp_megaproject_has_agent_instant_travel (fixture->project));
}

static void
test_megaproject_destroy (MegaprojectFixture *fixture,
                          gconstpointer       user_data)
{
    LpMegaprojectPhase *phase;

    (void)user_data;

    phase = lp_megaproject_phase_new ("Phase", 10);
    lp_megaproject_add_phase (fixture->project, phase);
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);
    lp_megaproject_start (fixture->project);

    lp_megaproject_destroy (fixture->project);
    g_assert_cmpint (lp_megaproject_get_state (fixture->project), ==, LP_MEGAPROJECT_STATE_DESTROYED);
}

static void
test_megaproject_reset (MegaprojectFixture *fixture,
                        gconstpointer       user_data)
{
    LpMegaprojectPhase *phase;

    (void)user_data;

    phase = lp_megaproject_phase_new ("Phase", 10);
    lp_megaproject_add_phase (fixture->project, phase);
    lp_megaproject_set_unlock_level (fixture->project, 0);
    lp_megaproject_reset (fixture->project);
    lp_megaproject_start (fixture->project);
    lp_megaproject_advance_years (fixture->project, 5);

    g_assert_cmpuint (lp_megaproject_get_years_invested (fixture->project), ==, 5);

    lp_megaproject_reset (fixture->project);
    g_assert_cmpuint (lp_megaproject_get_years_invested (fixture->project), ==, 0);
    g_assert_cmpint (lp_megaproject_get_state (fixture->project), ==, LP_MEGAPROJECT_STATE_AVAILABLE);
}

/* ==========================================================================
 * Prestige Manager Tests
 * ========================================================================== */

static void
test_prestige_new (PrestigeFixture *fixture,
                   gconstpointer    user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_PRESTIGE_MANAGER (fixture->manager));
    g_assert_true (LRG_IS_SAVEABLE (fixture->manager));
}

static void
test_prestige_initial_echoes (PrestigeFixture *fixture,
                              gconstpointer    user_data)
{
    const LrgBigNumber *echoes;
    gdouble value;

    (void)user_data;

    echoes = lp_prestige_manager_get_echoes (fixture->manager);
    g_assert_nonnull (echoes);
    value = lrg_big_number_to_double (echoes);
    g_assert_cmpfloat (value, ==, 0.0);
}

static void
test_prestige_initial_count (PrestigeFixture *fixture,
                             gconstpointer    user_data)
{
    guint64 count;

    (void)user_data;

    count = lp_prestige_manager_get_times_prestiged (fixture->manager);
    g_assert_cmpuint (count, ==, 0);
}

static void
test_prestige_can_prestige_requirements (PrestigeFixture *fixture,
                                         gconstpointer    user_data)
{
    g_autoptr(LrgBigNumber) low_gold = NULL;
    g_autoptr(LrgBigNumber) high_gold = NULL;
    gboolean can;

    (void)user_data;

    /* Low gold, low years - cannot prestige */
    low_gold = lrg_big_number_new (1000.0);
    can = lp_prestige_manager_can_prestige (fixture->manager, low_gold, 50);
    g_assert_false (can);

    /* High gold, low years - cannot prestige */
    high_gold = lrg_big_number_new (10000000.0);
    can = lp_prestige_manager_can_prestige (fixture->manager, high_gold, 50);
    g_assert_false (can);

    /* High gold, high years - can prestige */
    can = lp_prestige_manager_can_prestige (fixture->manager, high_gold, 100);
    g_assert_true (can);
}

static void
test_prestige_calculate_reward (PrestigeFixture *fixture,
                                gconstpointer    user_data)
{
    g_autoptr(LrgBigNumber) gold = NULL;
    g_autoptr(LrgBigNumber) reward = NULL;
    gdouble value;

    (void)user_data;

    gold = lrg_big_number_new (1000000.0);
    reward = lp_prestige_manager_calculate_echo_reward (fixture->manager, gold, 100);
    g_assert_nonnull (reward);

    /* Reward should be positive */
    value = lrg_big_number_to_double (reward);
    g_assert_cmpfloat (value, >, 0.0);
}

static void
test_prestige_perform (PrestigeFixture *fixture,
                       gconstpointer    user_data)
{
    g_autoptr(LrgBigNumber) gold = NULL;
    g_autoptr(LrgBigNumber) reward = NULL;
    const LrgBigNumber *echoes;
    guint64 count;
    gdouble reward_value;
    gdouble echo_value;

    (void)user_data;

    gold = lrg_big_number_new (10000000.0);
    reward = lp_prestige_manager_perform_prestige (fixture->manager, gold, 100);
    g_assert_nonnull (reward);

    reward_value = lrg_big_number_to_double (reward);
    g_assert_cmpfloat (reward_value, >, 0.0);

    /* Check echoes were added */
    echoes = lp_prestige_manager_get_echoes (fixture->manager);
    echo_value = lrg_big_number_to_double (echoes);
    g_assert_cmpfloat_with_epsilon (echo_value, reward_value, 0.01);

    /* Check count increased */
    count = lp_prestige_manager_get_times_prestiged (fixture->manager);
    g_assert_cmpuint (count, ==, 1);
}

static void
test_prestige_bonus_multiplier (PrestigeFixture *fixture,
                                gconstpointer    user_data)
{
    g_autoptr(LrgBigNumber) gold = NULL;
    gdouble multiplier;

    (void)user_data;

    /* Initial multiplier should be 1.0 */
    multiplier = lp_prestige_manager_get_bonus_multiplier (fixture->manager);
    g_assert_cmpfloat (multiplier, >=, 1.0);

    /* After prestige, multiplier should increase */
    gold = lrg_big_number_new (10000000.0);
    lp_prestige_manager_perform_prestige (fixture->manager, gold, 100);

    multiplier = lp_prestige_manager_get_bonus_multiplier (fixture->manager);
    g_assert_cmpfloat (multiplier, >, 1.0);
}

static void
test_prestige_echo_trees (PrestigeFixture *fixture,
                          gconstpointer    user_data)
{
    LrgUnlockTree *tree;

    (void)user_data;

    /* Check all four trees exist */
    tree = lp_prestige_manager_get_echo_tree (fixture->manager, LP_ECHO_TREE_ECONOMIST);
    g_assert_nonnull (tree);

    tree = lp_prestige_manager_get_echo_tree (fixture->manager, LP_ECHO_TREE_MANIPULATOR);
    g_assert_nonnull (tree);

    tree = lp_prestige_manager_get_echo_tree (fixture->manager, LP_ECHO_TREE_SCHOLAR);
    g_assert_nonnull (tree);

    tree = lp_prestige_manager_get_echo_tree (fixture->manager, LP_ECHO_TREE_ARCHITECT);
    g_assert_nonnull (tree);
}

static void
test_prestige_reset (PrestigeFixture *fixture,
                     gconstpointer    user_data)
{
    g_autoptr(LrgBigNumber) gold = NULL;
    const LrgBigNumber *echoes;
    guint64 count;
    gdouble value;

    (void)user_data;

    /* Perform prestige first */
    gold = lrg_big_number_new (10000000.0);
    lp_prestige_manager_perform_prestige (fixture->manager, gold, 100);

    /* Reset */
    lp_prestige_manager_reset (fixture->manager);

    /* Echoes should be zero */
    echoes = lp_prestige_manager_get_echoes (fixture->manager);
    value = lrg_big_number_to_double (echoes);
    g_assert_cmpfloat (value, ==, 0.0);

    /* Count should be zero */
    count = lp_prestige_manager_get_times_prestiged (fixture->manager);
    g_assert_cmpuint (count, ==, 0);
}

/* ==========================================================================
 * Phylactery Tests
 * ========================================================================== */

static void
test_phylactery_new (PhylacteryFixture *fixture,
                     gconstpointer      user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_PHYLACTERY (fixture->phylactery));
    g_assert_true (LRG_IS_SAVEABLE (fixture->phylactery));
}

static void
test_phylactery_initial_points (PhylacteryFixture *fixture,
                                gconstpointer      user_data)
{
    guint64 points;

    (void)user_data;

    points = lp_phylactery_get_points (fixture->phylactery);
    g_assert_cmpuint (points, ==, 0);
}

static void
test_phylactery_add_points (PhylacteryFixture *fixture,
                            gconstpointer      user_data)
{
    guint64 points;
    guint64 total;

    (void)user_data;

    lp_phylactery_add_points (fixture->phylactery, 100);
    points = lp_phylactery_get_points (fixture->phylactery);
    g_assert_cmpuint (points, ==, 100);

    total = lp_phylactery_get_total_points_earned (fixture->phylactery);
    g_assert_cmpuint (total, ==, 100);

    lp_phylactery_add_points (fixture->phylactery, 50);
    points = lp_phylactery_get_points (fixture->phylactery);
    g_assert_cmpuint (points, ==, 150);
}

static void
test_phylactery_upgrade_trees (PhylacteryFixture *fixture,
                               gconstpointer      user_data)
{
    LrgUnlockTree *tree;

    (void)user_data;

    /* Check all five category trees exist */
    tree = lp_phylactery_get_upgrade_tree (fixture->phylactery, LP_UPGRADE_CATEGORY_TEMPORAL);
    g_assert_nonnull (tree);

    tree = lp_phylactery_get_upgrade_tree (fixture->phylactery, LP_UPGRADE_CATEGORY_NETWORK);
    g_assert_nonnull (tree);

    tree = lp_phylactery_get_upgrade_tree (fixture->phylactery, LP_UPGRADE_CATEGORY_DIVINATION);
    g_assert_nonnull (tree);

    tree = lp_phylactery_get_upgrade_tree (fixture->phylactery, LP_UPGRADE_CATEGORY_RESILIENCE);
    g_assert_nonnull (tree);

    tree = lp_phylactery_get_upgrade_tree (fixture->phylactery, LP_UPGRADE_CATEGORY_DARK_ARTS);
    g_assert_nonnull (tree);
}

static void
test_phylactery_initial_bonuses (PhylacteryFixture *fixture,
                                 gconstpointer      user_data)
{
    guint max_slumber;
    guint max_agents;
    gdouble time_efficiency;

    (void)user_data;

    /* Default values without upgrades */
    max_slumber = lp_phylactery_get_max_slumber_years (fixture->phylactery);
    g_assert_cmpuint (max_slumber, ==, 100);  /* base value */

    max_agents = lp_phylactery_get_max_agents (fixture->phylactery);
    g_assert_cmpuint (max_agents, ==, 3);  /* base value */

    time_efficiency = lp_phylactery_get_time_efficiency_bonus (fixture->phylactery);
    g_assert_cmpfloat (time_efficiency, ==, 1.0);  /* no bonus */
}

static void
test_phylactery_has_upgrade_initially (PhylacteryFixture *fixture,
                                       gconstpointer      user_data)
{
    gboolean has;

    (void)user_data;

    /* Should not have any upgrades initially */
    has = lp_phylactery_has_upgrade (fixture->phylactery, "extended-slumber-1");
    g_assert_false (has);
}

static void
test_phylactery_reset (PhylacteryFixture *fixture,
                       gconstpointer      user_data)
{
    guint64 points;

    (void)user_data;

    lp_phylactery_add_points (fixture->phylactery, 100);
    points = lp_phylactery_get_points (fixture->phylactery);
    g_assert_cmpuint (points, ==, 100);

    lp_phylactery_reset (fixture->phylactery);
    points = lp_phylactery_get_points (fixture->phylactery);
    g_assert_cmpuint (points, ==, 0);
}

/* ==========================================================================
 * Ledger Progress Tests
 * ========================================================================== */

static void
test_ledger_register_entry (LedgerProgressFixture *fixture,
                            gconstpointer          user_data)
{
    gboolean is_reg;

    (void)user_data;

    lp_ledger_register_entry (fixture->ledger, "test-entry",
                              LP_LEDGER_CATEGORY_ECONOMIC, 3);

    is_reg = lp_ledger_is_registered (fixture->ledger, "test-entry");
    g_assert_true (is_reg);

    is_reg = lp_ledger_is_registered (fixture->ledger, "nonexistent");
    g_assert_false (is_reg);
}

static void
test_ledger_progress_single (LedgerProgressFixture *fixture,
                             gconstpointer          user_data)
{
    guint progress;
    gboolean made_progress;
    gboolean discovered;

    (void)user_data;

    /* Register an entry that requires 1 occurrence */
    lp_ledger_register_entry (fixture->ledger, "simple-entry",
                              LP_LEDGER_CATEGORY_COMPETITOR, 1);

    progress = lp_ledger_get_progress (fixture->ledger, "simple-entry");
    g_assert_cmpuint (progress, ==, 0);

    /* Progress should discover immediately */
    made_progress = lp_ledger_progress_entry (fixture->ledger, "simple-entry",
                                               LP_DISCOVERY_METHOD_AGENT_REPORT);
    g_assert_true (made_progress);

    discovered = lp_ledger_has_discovered (fixture->ledger, "simple-entry");
    g_assert_true (discovered);
}

static void
test_ledger_progress_multiple (LedgerProgressFixture *fixture,
                               gconstpointer          user_data)
{
    guint progress;
    guint required;
    gfloat fraction;
    gboolean discovered;

    (void)user_data;

    /* Register an entry that requires 3 occurrences */
    lp_ledger_register_entry (fixture->ledger, "multi-entry",
                              LP_LEDGER_CATEGORY_HIDDEN, 3);

    required = lp_ledger_get_required_occurrences (fixture->ledger, "multi-entry");
    g_assert_cmpuint (required, ==, 3);

    /* First progress */
    lp_ledger_progress_entry (fixture->ledger, "multi-entry",
                               LP_DISCOVERY_METHOD_EVENT_SURVIVAL);
    progress = lp_ledger_get_progress (fixture->ledger, "multi-entry");
    g_assert_cmpuint (progress, ==, 1);

    discovered = lp_ledger_has_discovered (fixture->ledger, "multi-entry");
    g_assert_false (discovered);

    fraction = lp_ledger_get_progress_fraction (fixture->ledger, "multi-entry");
    g_assert_cmpfloat_with_epsilon (fraction, 1.0f / 3.0f, 0.001f);

    /* Second progress */
    lp_ledger_progress_entry (fixture->ledger, "multi-entry",
                               LP_DISCOVERY_METHOD_EVENT_SURVIVAL);
    progress = lp_ledger_get_progress (fixture->ledger, "multi-entry");
    g_assert_cmpuint (progress, ==, 2);

    /* Third progress - should complete discovery */
    lp_ledger_progress_entry (fixture->ledger, "multi-entry",
                               LP_DISCOVERY_METHOD_EVENT_SURVIVAL);
    discovered = lp_ledger_has_discovered (fixture->ledger, "multi-entry");
    g_assert_true (discovered);
}

static void
test_ledger_has_started (LedgerProgressFixture *fixture,
                         gconstpointer          user_data)
{
    gboolean started;

    (void)user_data;

    lp_ledger_register_entry (fixture->ledger, "started-entry",
                              LP_LEDGER_CATEGORY_AGENT, 3);

    started = lp_ledger_has_started (fixture->ledger, "started-entry");
    g_assert_false (started);

    lp_ledger_progress_entry (fixture->ledger, "started-entry",
                               LP_DISCOVERY_METHOD_COMPETITOR);
    started = lp_ledger_has_started (fixture->ledger, "started-entry");
    g_assert_true (started);
}

static void
test_ledger_in_progress_count (LedgerProgressFixture *fixture,
                               gconstpointer          user_data)
{
    guint count;

    (void)user_data;

    lp_ledger_register_entry (fixture->ledger, "entry-a",
                              LP_LEDGER_CATEGORY_ECONOMIC, 3);
    lp_ledger_register_entry (fixture->ledger, "entry-b",
                              LP_LEDGER_CATEGORY_ECONOMIC, 3);

    count = lp_ledger_get_in_progress_count (fixture->ledger);
    g_assert_cmpuint (count, ==, 0);

    /* Start both entries */
    lp_ledger_progress_entry (fixture->ledger, "entry-a",
                               LP_DISCOVERY_METHOD_MANUAL);
    lp_ledger_progress_entry (fixture->ledger, "entry-b",
                               LP_DISCOVERY_METHOD_MANUAL);

    count = lp_ledger_get_in_progress_count (fixture->ledger);
    g_assert_cmpuint (count, ==, 2);
}

static void
test_ledger_all_in_progress (LedgerProgressFixture *fixture,
                             gconstpointer          user_data)
{
    GList *in_progress;

    (void)user_data;

    lp_ledger_register_entry (fixture->ledger, "ip-entry",
                              LP_LEDGER_CATEGORY_HIDDEN, 5);
    lp_ledger_progress_entry (fixture->ledger, "ip-entry",
                               LP_DISCOVERY_METHOD_MILESTONE);

    in_progress = lp_ledger_get_all_in_progress (fixture->ledger);
    g_assert_cmpuint (g_list_length (in_progress), ==, 1);
    g_list_free (in_progress);
}

static void
test_ledger_auto_register (LedgerProgressFixture *fixture,
                           gconstpointer          user_data)
{
    gboolean discovered;

    (void)user_data;

    /* Progress on unregistered entry should auto-register with required=1 */
    lp_ledger_progress_entry (fixture->ledger, "auto-entry",
                               LP_DISCOVERY_METHOD_ACHIEVEMENT);

    discovered = lp_ledger_has_discovered (fixture->ledger, "auto-entry");
    g_assert_true (discovered);
}

static void
test_ledger_discover_immediate (LedgerProgressFixture *fixture,
                                gconstpointer          user_data)
{
    gboolean result;
    gboolean discovered;

    (void)user_data;

    /* Immediate discovery bypasses progress */
    result = lp_ledger_discover (fixture->ledger, "immediate-entry",
                                  LP_LEDGER_CATEGORY_COMPETITOR);
    g_assert_true (result);

    discovered = lp_ledger_has_discovered (fixture->ledger, "immediate-entry");
    g_assert_true (discovered);

    /* Second call should return FALSE (already discovered) */
    result = lp_ledger_discover (fixture->ledger, "immediate-entry",
                                  LP_LEDGER_CATEGORY_COMPETITOR);
    g_assert_false (result);
}

static void
test_ledger_clear_all (LedgerProgressFixture *fixture,
                       gconstpointer          user_data)
{
    guint count;

    (void)user_data;

    lp_ledger_discover (fixture->ledger, "entry-1", LP_LEDGER_CATEGORY_ECONOMIC);
    lp_ledger_discover (fixture->ledger, "entry-2", LP_LEDGER_CATEGORY_ECONOMIC);

    count = lp_ledger_get_discovered_count (fixture->ledger);
    g_assert_cmpuint (count, ==, 2);

    lp_ledger_clear_all (fixture->ledger);

    count = lp_ledger_get_discovered_count (fixture->ledger);
    g_assert_cmpuint (count, ==, 0);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Megaproject Phase tests */
    g_test_add_func ("/progression/megaproject/phase/new", test_megaproject_phase_new);
    g_test_add_func ("/progression/megaproject/phase/copy", test_megaproject_phase_copy);

    /* Megaproject tests */
    g_test_add ("/progression/megaproject/new",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_new, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/id",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_id, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/name",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_name, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/description",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_description, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/initial-state",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_initial_state, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/add-phase",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_add_phase, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/unlock-level",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_unlock_level, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/discovery-risk",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_discovery_risk, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/cost-per-year",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_cost_per_year, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/state-transitions",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_state_transitions, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/advance-years",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_advance_years, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/effects",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_effects, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/destroy",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_destroy, megaproject_fixture_tear_down);
    g_test_add ("/progression/megaproject/reset",
                MegaprojectFixture, NULL,
                megaproject_fixture_set_up, test_megaproject_reset, megaproject_fixture_tear_down);

    /* Prestige Manager tests */
    g_test_add ("/progression/prestige/new",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_new, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/initial-echoes",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_initial_echoes, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/initial-count",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_initial_count, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/can-prestige-requirements",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_can_prestige_requirements, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/calculate-reward",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_calculate_reward, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/perform",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_perform, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/bonus-multiplier",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_bonus_multiplier, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/echo-trees",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_echo_trees, prestige_fixture_tear_down);
    g_test_add ("/progression/prestige/reset",
                PrestigeFixture, NULL,
                prestige_fixture_set_up, test_prestige_reset, prestige_fixture_tear_down);

    /* Phylactery tests */
    g_test_add ("/progression/phylactery/new",
                PhylacteryFixture, NULL,
                phylactery_fixture_set_up, test_phylactery_new, phylactery_fixture_tear_down);
    g_test_add ("/progression/phylactery/initial-points",
                PhylacteryFixture, NULL,
                phylactery_fixture_set_up, test_phylactery_initial_points, phylactery_fixture_tear_down);
    g_test_add ("/progression/phylactery/add-points",
                PhylacteryFixture, NULL,
                phylactery_fixture_set_up, test_phylactery_add_points, phylactery_fixture_tear_down);
    g_test_add ("/progression/phylactery/upgrade-trees",
                PhylacteryFixture, NULL,
                phylactery_fixture_set_up, test_phylactery_upgrade_trees, phylactery_fixture_tear_down);
    g_test_add ("/progression/phylactery/initial-bonuses",
                PhylacteryFixture, NULL,
                phylactery_fixture_set_up, test_phylactery_initial_bonuses, phylactery_fixture_tear_down);
    g_test_add ("/progression/phylactery/has-upgrade-initially",
                PhylacteryFixture, NULL,
                phylactery_fixture_set_up, test_phylactery_has_upgrade_initially, phylactery_fixture_tear_down);
    g_test_add ("/progression/phylactery/reset",
                PhylacteryFixture, NULL,
                phylactery_fixture_set_up, test_phylactery_reset, phylactery_fixture_tear_down);

    /* Ledger Progress tests */
    g_test_add ("/progression/ledger/register-entry",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_register_entry, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/progress-single",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_progress_single, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/progress-multiple",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_progress_multiple, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/has-started",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_has_started, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/in-progress-count",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_in_progress_count, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/all-in-progress",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_all_in_progress, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/auto-register",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_auto_register, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/discover-immediate",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_discover_immediate, ledger_progress_fixture_tear_down);
    g_test_add ("/progression/ledger/clear-all",
                LedgerProgressFixture, NULL,
                ledger_progress_fixture_set_up, test_ledger_clear_all, ledger_progress_fixture_tear_down);

    return g_test_run ();
}
