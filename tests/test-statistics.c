/* test-statistics.c - Statistics Tracking Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include <libregnum.h>
#include "core/lp-statistics.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpStatistics *stats;
} StatisticsFixture;

static void
fixture_set_up (StatisticsFixture *fixture,
                gconstpointer      user_data)
{
    (void)user_data;

    /* Get the singleton */
    fixture->stats = lp_statistics_get_default ();
    g_assert_nonnull (fixture->stats);

    /* Reset to known state */
    lp_statistics_reset (fixture->stats);
}

static void
fixture_tear_down (StatisticsFixture *fixture,
                   gconstpointer      user_data)
{
    (void)user_data;
    (void)fixture;

    /* Don't unref the singleton */
}

/* ==========================================================================
 * Singleton Tests
 * ========================================================================== */

static void
test_statistics_singleton (StatisticsFixture *fixture,
                           gconstpointer      user_data)
{
    LpStatistics *stats2;

    (void)user_data;

    /* Singleton should return the same instance */
    stats2 = lp_statistics_get_default ();
    g_assert_true (fixture->stats == stats2);
}

static void
test_statistics_type (StatisticsFixture *fixture,
                      gconstpointer      user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_STATISTICS (fixture->stats));
    g_assert_true (LRG_IS_SAVEABLE (fixture->stats));
}

/* ==========================================================================
 * Wealth Statistics Tests
 * ========================================================================== */

static void
test_statistics_gold_earned (StatisticsFixture *fixture,
                             gconstpointer      user_data)
{
    g_autoptr(LrgBigNumber) amount1 = NULL;
    g_autoptr(LrgBigNumber) amount2 = NULL;
    g_autoptr(LrgBigNumber) total = NULL;

    (void)user_data;

    /* Add some gold earned */
    amount1 = lrg_big_number_new (1000.0);
    lp_statistics_on_gold_earned (fixture->stats, amount1);

    amount2 = lrg_big_number_new (500.0);
    lp_statistics_on_gold_earned (fixture->stats, amount2);

    /* Check total */
    total = lp_statistics_get_lifetime_gold_earned (fixture->stats);
    g_assert_nonnull (total);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (total), 1500.0, 0.01);
}

static void
test_statistics_gold_spent (StatisticsFixture *fixture,
                            gconstpointer      user_data)
{
    g_autoptr(LrgBigNumber) amount = NULL;
    g_autoptr(LrgBigNumber) total = NULL;

    (void)user_data;

    amount = lrg_big_number_new (250.0);
    lp_statistics_on_gold_spent (fixture->stats, amount);

    total = lp_statistics_get_lifetime_gold_spent (fixture->stats);
    g_assert_nonnull (total);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (total), 250.0, 0.01);
}

static void
test_statistics_peak_net_worth (StatisticsFixture *fixture,
                                gconstpointer      user_data)
{
    g_autoptr(LrgBigNumber) net1 = NULL;
    g_autoptr(LrgBigNumber) net2 = NULL;
    g_autoptr(LrgBigNumber) net3 = NULL;
    g_autoptr(LrgBigNumber) peak = NULL;

    (void)user_data;

    /* Set increasing net worth */
    net1 = lrg_big_number_new (1000.0);
    lp_statistics_on_net_worth_changed (fixture->stats, net1, 847);

    net2 = lrg_big_number_new (5000.0);
    lp_statistics_on_net_worth_changed (fixture->stats, net2, 850);

    /* Peak should be 5000 */
    peak = lp_statistics_get_peak_net_worth (fixture->stats);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (peak), 5000.0, 0.01);
    g_assert_cmpuint (lp_statistics_get_peak_net_worth_year (fixture->stats), ==, 850);

    /* Lower net worth shouldn't change peak */
    lrg_big_number_free (peak);
    net3 = lrg_big_number_new (3000.0);
    lp_statistics_on_net_worth_changed (fixture->stats, net3, 855);

    peak = lp_statistics_get_peak_net_worth (fixture->stats);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (peak), 5000.0, 0.01);
    g_assert_cmpuint (lp_statistics_get_peak_net_worth_year (fixture->stats), ==, 850);
}

/* ==========================================================================
 * Investment Statistics Tests
 * ========================================================================== */

static void
test_statistics_investments (StatisticsFixture *fixture,
                             gconstpointer      user_data)
{
    g_autoptr(LrgBigNumber) returns = NULL;
    g_autoptr(LrgBigNumber) total_returns = NULL;

    (void)user_data;

    /* Purchase some investments */
    lp_statistics_on_investment_purchased (fixture->stats);
    lp_statistics_on_investment_purchased (fixture->stats);
    lp_statistics_on_investment_purchased (fixture->stats);

    g_assert_cmpuint (lp_statistics_get_investments_purchased (fixture->stats), ==, 3);

    /* Sell one with returns */
    returns = lrg_big_number_new (500.0);
    lp_statistics_on_investment_sold (fixture->stats, returns);

    g_assert_cmpuint (lp_statistics_get_investments_sold (fixture->stats), ==, 1);

    total_returns = lp_statistics_get_total_investment_returns (fixture->stats);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (total_returns), 500.0, 0.01);

    /* Lose one */
    lp_statistics_on_investment_lost (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_investments_lost (fixture->stats), ==, 1);
}

static void
test_statistics_longest_investment (StatisticsFixture *fixture,
                                    gconstpointer      user_data)
{
    (void)user_data;

    lp_statistics_on_investment_held (fixture->stats, 50);
    g_assert_cmpuint (lp_statistics_get_longest_investment_held (fixture->stats), ==, 50);

    lp_statistics_on_investment_held (fixture->stats, 30);
    g_assert_cmpuint (lp_statistics_get_longest_investment_held (fixture->stats), ==, 50);

    lp_statistics_on_investment_held (fixture->stats, 100);
    g_assert_cmpuint (lp_statistics_get_longest_investment_held (fixture->stats), ==, 100);
}

/* ==========================================================================
 * Agent Statistics Tests
 * ========================================================================== */

static void
test_statistics_agents (StatisticsFixture *fixture,
                        gconstpointer      user_data)
{
    (void)user_data;

    lp_statistics_on_agent_recruited (fixture->stats);
    lp_statistics_on_agent_recruited (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_agents_recruited (fixture->stats), ==, 2);

    lp_statistics_on_agent_death (fixture->stats, 25);
    g_assert_cmpuint (lp_statistics_get_agent_deaths (fixture->stats), ==, 1);
    g_assert_cmpuint (lp_statistics_get_total_agent_years_served (fixture->stats), ==, 25);

    lp_statistics_on_agent_betrayal (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_agent_betrayals (fixture->stats), ==, 1);
}

static void
test_statistics_family_generation (StatisticsFixture *fixture,
                                   gconstpointer      user_data)
{
    (void)user_data;

    lp_statistics_on_family_succession (fixture->stats, 2);
    g_assert_cmpuint (lp_statistics_get_highest_family_generation (fixture->stats), ==, 2);

    lp_statistics_on_family_succession (fixture->stats, 5);
    g_assert_cmpuint (lp_statistics_get_highest_family_generation (fixture->stats), ==, 5);

    /* Lower generation shouldn't change record */
    lp_statistics_on_family_succession (fixture->stats, 3);
    g_assert_cmpuint (lp_statistics_get_highest_family_generation (fixture->stats), ==, 5);
}

/* ==========================================================================
 * Time Statistics Tests
 * ========================================================================== */

static void
test_statistics_slumber (StatisticsFixture *fixture,
                         gconstpointer      user_data)
{
    (void)user_data;

    lp_statistics_on_slumber_complete (fixture->stats, 50);
    g_assert_cmpuint (lp_statistics_get_total_years_slumbered (fixture->stats), ==, 50);
    g_assert_cmpuint (lp_statistics_get_longest_slumber (fixture->stats), ==, 50);
    g_assert_cmpuint (lp_statistics_get_total_awakenings (fixture->stats), ==, 1);

    lp_statistics_on_slumber_complete (fixture->stats, 100);
    g_assert_cmpuint (lp_statistics_get_total_years_slumbered (fixture->stats), ==, 150);
    g_assert_cmpuint (lp_statistics_get_longest_slumber (fixture->stats), ==, 100);
    g_assert_cmpuint (lp_statistics_get_total_awakenings (fixture->stats), ==, 2);

    /* Shorter slumber shouldn't change longest */
    lp_statistics_on_slumber_complete (fixture->stats, 30);
    g_assert_cmpuint (lp_statistics_get_longest_slumber (fixture->stats), ==, 100);
}

/* ==========================================================================
 * World Statistics Tests
 * ========================================================================== */

static void
test_statistics_world_events (StatisticsFixture *fixture,
                              gconstpointer      user_data)
{
    (void)user_data;

    lp_statistics_on_event_witnessed (fixture->stats);
    lp_statistics_on_event_witnessed (fixture->stats);
    lp_statistics_on_event_witnessed (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_events_witnessed (fixture->stats), ==, 3);

    lp_statistics_on_kingdom_collapsed (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_kingdoms_collapsed (fixture->stats), ==, 1);

    lp_statistics_on_crusade_survived (fixture->stats);
    lp_statistics_on_crusade_survived (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_crusades_survived (fixture->stats), ==, 2);

    lp_statistics_on_competitor_defeated (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_competitors_defeated (fixture->stats), ==, 1);
}

/* ==========================================================================
 * Prestige Statistics Tests
 * ========================================================================== */

static void
test_statistics_prestige (StatisticsFixture *fixture,
                          gconstpointer      user_data)
{
    g_autoptr(LrgBigNumber) points1 = NULL;
    g_autoptr(LrgBigNumber) points2 = NULL;
    g_autoptr(LrgBigNumber) total = NULL;
    g_autoptr(LrgBigNumber) best = NULL;

    (void)user_data;

    points1 = lrg_big_number_new (100.0);
    lp_statistics_on_prestige (fixture->stats, points1);
    g_assert_cmpuint (lp_statistics_get_prestige_count (fixture->stats), ==, 1);

    points2 = lrg_big_number_new (200.0);
    lp_statistics_on_prestige (fixture->stats, points2);
    g_assert_cmpuint (lp_statistics_get_prestige_count (fixture->stats), ==, 2);

    total = lp_statistics_get_total_phylactery_points_earned (fixture->stats);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (total), 300.0, 0.01);

    best = lp_statistics_get_best_prestige_run (fixture->stats);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (best), 200.0, 0.01);
}

/* ==========================================================================
 * Session Statistics Tests
 * ========================================================================== */

static void
test_statistics_sessions (StatisticsFixture *fixture,
                          gconstpointer      user_data)
{
    (void)user_data;

    lp_statistics_on_session_start (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_session_count (fixture->stats), ==, 1);
    g_assert_true (lp_statistics_get_first_play_timestamp (fixture->stats) > 0);

    lp_statistics_on_session_end (fixture->stats, 3600);
    g_assert_cmpuint (lp_statistics_get_total_play_time_seconds (fixture->stats), ==, 3600);

    lp_statistics_on_session_start (fixture->stats);
    lp_statistics_on_session_end (fixture->stats, 1800);
    g_assert_cmpuint (lp_statistics_get_session_count (fixture->stats), ==, 2);
    g_assert_cmpuint (lp_statistics_get_total_play_time_seconds (fixture->stats), ==, 5400);
}

/* ==========================================================================
 * Dark Arts Statistics Tests
 * ========================================================================== */

static void
test_statistics_dark_arts (StatisticsFixture *fixture,
                           gconstpointer      user_data)
{
    (void)user_data;

    lp_statistics_on_soul_trade (fixture->stats);
    lp_statistics_on_soul_trade (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_soul_trades_completed (fixture->stats), ==, 2);

    lp_statistics_on_dark_investment (fixture->stats);
    g_assert_cmpuint (lp_statistics_get_dark_investments_owned (fixture->stats), ==, 1);
}

/* ==========================================================================
 * Reset Test
 * ========================================================================== */

static void
test_statistics_reset (StatisticsFixture *fixture,
                       gconstpointer      user_data)
{
    g_autoptr(LrgBigNumber) amount = NULL;
    g_autoptr(LrgBigNumber) gold = NULL;

    (void)user_data;

    /* Add some data */
    amount = lrg_big_number_new (1000.0);
    lp_statistics_on_gold_earned (fixture->stats, amount);
    lp_statistics_on_investment_purchased (fixture->stats);
    lp_statistics_on_slumber_complete (fixture->stats, 100);

    /* Reset */
    lp_statistics_reset (fixture->stats);

    /* Verify everything is zero */
    gold = lp_statistics_get_lifetime_gold_earned (fixture->stats);
    g_assert_cmpfloat_with_epsilon (lrg_big_number_to_double (gold), 0.0, 0.01);
    g_assert_cmpuint (lp_statistics_get_investments_purchased (fixture->stats), ==, 0);
    g_assert_cmpuint (lp_statistics_get_total_years_slumbered (fixture->stats), ==, 0);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Singleton tests */
    g_test_add ("/statistics/singleton",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_singleton, fixture_tear_down);

    g_test_add ("/statistics/type",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_type, fixture_tear_down);

    /* Wealth tests */
    g_test_add ("/statistics/wealth/gold-earned",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_gold_earned, fixture_tear_down);

    g_test_add ("/statistics/wealth/gold-spent",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_gold_spent, fixture_tear_down);

    g_test_add ("/statistics/wealth/peak-net-worth",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_peak_net_worth, fixture_tear_down);

    /* Investment tests */
    g_test_add ("/statistics/investments/counts",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_investments, fixture_tear_down);

    g_test_add ("/statistics/investments/longest-held",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_longest_investment, fixture_tear_down);

    /* Agent tests */
    g_test_add ("/statistics/agents/counts",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_agents, fixture_tear_down);

    g_test_add ("/statistics/agents/family-generation",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_family_generation, fixture_tear_down);

    /* Time tests */
    g_test_add ("/statistics/time/slumber",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_slumber, fixture_tear_down);

    /* World tests */
    g_test_add ("/statistics/world/events",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_world_events, fixture_tear_down);

    /* Prestige tests */
    g_test_add ("/statistics/prestige/counts",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_prestige, fixture_tear_down);

    /* Session tests */
    g_test_add ("/statistics/sessions/tracking",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_sessions, fixture_tear_down);

    /* Dark arts tests */
    g_test_add ("/statistics/dark-arts/counts",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_dark_arts, fixture_tear_down);

    /* Reset test */
    g_test_add ("/statistics/reset",
                StatisticsFixture, NULL,
                fixture_set_up, test_statistics_reset, fixture_tear_down);

    return g_test_run ();
}
