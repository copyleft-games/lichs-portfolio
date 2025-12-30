/* test-portfolio.c - Portfolio Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include <libregnum.h>
#include "investment/lp-portfolio.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpPortfolio *portfolio;
} PortfolioFixture;

static void
fixture_set_up (PortfolioFixture *fixture,
                gconstpointer     user_data)
{
    (void)user_data;

    fixture->portfolio = lp_portfolio_new ();
    g_assert_nonnull (fixture->portfolio);
}

static void
fixture_tear_down (PortfolioFixture *fixture,
                   gconstpointer     user_data)
{
    (void)user_data;

    g_clear_object (&fixture->portfolio);
}

/* ==========================================================================
 * Tests
 * ========================================================================== */

static void
test_portfolio_new (PortfolioFixture *fixture,
                    gconstpointer     user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_PORTFOLIO (fixture->portfolio));
}

static void
test_portfolio_default_gold (PortfolioFixture *fixture,
                             gconstpointer     user_data)
{
    LrgBigNumber *gold;
    gdouble       value;

    (void)user_data;

    /* lp_portfolio_get_gold returns (transfer none) - don't free it */
    gold = lp_portfolio_get_gold (fixture->portfolio);
    g_assert_nonnull (gold);

    value = lrg_big_number_to_double (gold);
    g_assert_cmpfloat (value, ==, 1000.0);
}

static void
test_portfolio_set_gold (PortfolioFixture *fixture,
                         gconstpointer     user_data)
{
    g_autoptr(LrgBigNumber) new_gold = NULL;
    LrgBigNumber           *retrieved_gold;
    gdouble                 value;

    (void)user_data;

    new_gold = lrg_big_number_new (5000.0);
    /* set_gold takes (transfer full), so use g_steal_pointer */
    lp_portfolio_set_gold (fixture->portfolio, g_steal_pointer (&new_gold));

    /* get_gold returns (transfer none), don't free it */
    retrieved_gold = lp_portfolio_get_gold (fixture->portfolio);
    g_assert_nonnull (retrieved_gold);

    value = lrg_big_number_to_double (retrieved_gold);
    g_assert_cmpfloat (value, ==, 5000.0);
}

static void
test_portfolio_add_gold (PortfolioFixture *fixture,
                         gconstpointer     user_data)
{
    g_autoptr(LrgBigNumber) amount = NULL;
    LrgBigNumber           *gold;
    gdouble                 value;

    (void)user_data;

    /* Start with 1000, add 500 */
    amount = lrg_big_number_new (500.0);
    lp_portfolio_add_gold (fixture->portfolio, amount);

    gold = lp_portfolio_get_gold (fixture->portfolio);
    value = lrg_big_number_to_double (gold);
    g_assert_cmpfloat (value, ==, 1500.0);
}

static void
test_portfolio_spend_gold_success (PortfolioFixture *fixture,
                                   gconstpointer     user_data)
{
    g_autoptr(LrgBigNumber) amount = NULL;
    LrgBigNumber           *gold;
    gdouble                 value;
    gboolean                result;

    (void)user_data;

    /* Start with 1000, spend 300 */
    amount = lrg_big_number_new (300.0);
    result = lp_portfolio_subtract_gold (fixture->portfolio, amount);
    g_assert_true (result);

    gold = lp_portfolio_get_gold (fixture->portfolio);
    value = lrg_big_number_to_double (gold);
    g_assert_cmpfloat (value, ==, 700.0);
}

static void
test_portfolio_spend_gold_insufficient (PortfolioFixture *fixture,
                                        gconstpointer     user_data)
{
    g_autoptr(LrgBigNumber) amount = NULL;
    LrgBigNumber           *gold;
    gdouble                 value;
    gboolean                result;

    (void)user_data;

    /* Start with 1000, try to spend 2000 */
    amount = lrg_big_number_new (2000.0);
    result = lp_portfolio_subtract_gold (fixture->portfolio, amount);
    g_assert_false (result);

    /* Gold should be unchanged */
    gold = lp_portfolio_get_gold (fixture->portfolio);
    value = lrg_big_number_to_double (gold);
    g_assert_cmpfloat (value, ==, 1000.0);
}

static void
test_portfolio_investment_count (PortfolioFixture *fixture,
                                 gconstpointer     user_data)
{
    guint count;

    (void)user_data;

    /* Portfolio starts empty */
    count = lp_portfolio_get_investment_count (fixture->portfolio);
    g_assert_cmpuint (count, ==, 0);
}

static void
test_portfolio_saveable_interface (PortfolioFixture *fixture,
                                   gconstpointer     user_data)
{
    (void)user_data;

    /* Verify it implements LrgSaveable */
    g_assert_true (LRG_IS_SAVEABLE (fixture->portfolio));
}

static void
test_portfolio_save_id (PortfolioFixture *fixture,
                        gconstpointer     user_data)
{
    const gchar *save_id;

    (void)user_data;

    save_id = lrg_saveable_get_save_id (LRG_SAVEABLE (fixture->portfolio));
    g_assert_cmpstr (save_id, ==, "portfolio");
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/portfolio/new",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_new, fixture_tear_down);

    g_test_add ("/portfolio/default-gold",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_default_gold, fixture_tear_down);

    g_test_add ("/portfolio/set-gold",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_set_gold, fixture_tear_down);

    g_test_add ("/portfolio/add-gold",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_add_gold, fixture_tear_down);

    g_test_add ("/portfolio/spend-gold-success",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_spend_gold_success, fixture_tear_down);

    g_test_add ("/portfolio/spend-gold-insufficient",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_spend_gold_insufficient, fixture_tear_down);

    g_test_add ("/portfolio/investment-count",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_investment_count, fixture_tear_down);

    g_test_add ("/portfolio/saveable-interface",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_saveable_interface, fixture_tear_down);

    g_test_add ("/portfolio/save-id",
                PortfolioFixture, NULL,
                fixture_set_up, test_portfolio_save_id, fixture_tear_down);

    return g_test_run ();
}
