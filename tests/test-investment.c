/* test-investment.c - Investment System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests the investment base class and all subclasses:
 * - LpInvestment (base class)
 * - LpInvestmentProperty
 * - LpInvestmentTrade
 * - LpInvestmentFinancial
 */

#include <glib.h>
#include <libregnum.h>
#include "investment/lp-investment.h"
#include "investment/lp-investment-property.h"
#include "investment/lp-investment-trade.h"
#include "investment/lp-investment-financial.h"
#include "investment/lp-portfolio.h"

/* ==========================================================================
 * Property Investment Fixture
 * ========================================================================== */

typedef struct
{
    LpInvestmentProperty *property;
} PropertyFixture;

static void
property_fixture_set_up (PropertyFixture *fixture,
                         gconstpointer    user_data)
{
    (void)user_data;

    fixture->property = lp_investment_property_new ("prop-001", "Test Farm",
                                                     LP_PROPERTY_TYPE_AGRICULTURAL);
    g_assert_nonnull (fixture->property);
}

static void
property_fixture_tear_down (PropertyFixture *fixture,
                            gconstpointer    user_data)
{
    (void)user_data;

    g_clear_object (&fixture->property);
}

/* ==========================================================================
 * Trade Investment Fixture
 * ========================================================================== */

typedef struct
{
    LpInvestmentTrade *trade;
} TradeFixture;

static void
trade_fixture_set_up (TradeFixture *fixture,
                      gconstpointer user_data)
{
    (void)user_data;

    fixture->trade = lp_investment_trade_new ("trade-001", "Silk Road",
                                               LP_TRADE_TYPE_ROUTE);
    g_assert_nonnull (fixture->trade);
}

static void
trade_fixture_tear_down (TradeFixture *fixture,
                         gconstpointer user_data)
{
    (void)user_data;

    g_clear_object (&fixture->trade);
}

/* ==========================================================================
 * Financial Investment Fixture
 * ========================================================================== */

typedef struct
{
    LpInvestmentFinancial *financial;
} FinancialFixture;

static void
financial_fixture_set_up (FinancialFixture *fixture,
                          gconstpointer     user_data)
{
    (void)user_data;

    fixture->financial = lp_investment_financial_new ("fin-001", "Crown Bond",
                                                       LP_FINANCIAL_TYPE_CROWN_BOND);
    g_assert_nonnull (fixture->financial);
}

static void
financial_fixture_tear_down (FinancialFixture *fixture,
                             gconstpointer     user_data)
{
    (void)user_data;

    g_clear_object (&fixture->financial);
}

/* ==========================================================================
 * Portfolio Investment Fixture
 * ========================================================================== */

typedef struct
{
    LpPortfolio *portfolio;
} PortfolioInvFixture;

static void
portfolio_inv_fixture_set_up (PortfolioInvFixture *fixture,
                              gconstpointer        user_data)
{
    (void)user_data;

    fixture->portfolio = lp_portfolio_new ();
    g_assert_nonnull (fixture->portfolio);
}

static void
portfolio_inv_fixture_tear_down (PortfolioInvFixture *fixture,
                                 gconstpointer        user_data)
{
    (void)user_data;

    g_clear_object (&fixture->portfolio);
}

/* ==========================================================================
 * Property Investment Tests
 * ========================================================================== */

static void
test_property_new (PropertyFixture *fixture,
                   gconstpointer    user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_INVESTMENT_PROPERTY (fixture->property));
    g_assert_true (LP_IS_INVESTMENT (fixture->property));
}

static void
test_property_type (PropertyFixture *fixture,
                    gconstpointer    user_data)
{
    LpPropertyType type;

    (void)user_data;

    type = lp_investment_property_get_property_type (fixture->property);
    g_assert_cmpint (type, ==, LP_PROPERTY_TYPE_AGRICULTURAL);
}

static void
test_property_asset_class (PropertyFixture *fixture,
                           gconstpointer    user_data)
{
    LpAssetClass asset_class;

    (void)user_data;

    asset_class = lp_investment_get_asset_class (LP_INVESTMENT (fixture->property));
    g_assert_cmpint (asset_class, ==, LP_ASSET_CLASS_PROPERTY);
}

static void
test_property_risk_level (PropertyFixture *fixture,
                          gconstpointer    user_data)
{
    LpRiskLevel risk;

    (void)user_data;

    risk = lp_investment_get_risk_level (LP_INVESTMENT (fixture->property));
    g_assert_cmpint (risk, ==, LP_RISK_LEVEL_LOW);
}

static void
test_property_stability_bonus (PropertyFixture *fixture,
                               gconstpointer    user_data)
{
    gdouble stability;

    (void)user_data;

    stability = lp_investment_property_get_stability_bonus (fixture->property);
    g_assert_cmpfloat (stability, >=, 1.0);
}

static void
test_property_returns (PropertyFixture *fixture,
                       gconstpointer    user_data)
{
    g_autoptr(LrgBigNumber) value = NULL;
    g_autoptr(LrgBigNumber) returns = NULL;
    gdouble initial;
    gdouble final;

    (void)user_data;

    /* Set initial value */
    value = lrg_big_number_new (1000.0);
    lp_investment_set_current_value (LP_INVESTMENT (fixture->property),
                                     g_steal_pointer (&value));

    /* Calculate returns over 10 years */
    returns = lp_investment_calculate_returns (LP_INVESTMENT (fixture->property), 10);
    g_assert_nonnull (returns);

    initial = 1000.0;
    final = lrg_big_number_to_double (returns);

    /* Property should have grown (3% base rate + improvements) */
    g_assert_cmpfloat (final, >, initial);

    /* With 3% rate over 10 years: 1000 * 1.03^10 ~ 1344 */
    g_assert_cmpfloat (final, >, 1300.0);
    g_assert_cmpfloat (final, <, 1400.0);
}

static void
test_property_improvements (PropertyFixture *fixture,
                            gconstpointer    user_data)
{
    g_autoptr(LrgBigNumber) value = NULL;
    g_autoptr(LrgBigNumber) cost = NULL;
    guint improvements;
    gboolean result;

    (void)user_data;

    /* Set initial value */
    value = lrg_big_number_new (1000.0);
    lp_investment_set_current_value (LP_INVESTMENT (fixture->property),
                                     g_steal_pointer (&value));

    /* Add an improvement */
    cost = lrg_big_number_new (200.0);
    result = lp_investment_property_add_improvement (fixture->property,
                                                      g_steal_pointer (&cost));
    g_assert_true (result);

    improvements = lp_investment_property_get_improvements (fixture->property);
    g_assert_cmpuint (improvements, ==, 1);
}

static void
test_property_saveable (PropertyFixture *fixture,
                        gconstpointer    user_data)
{
    (void)user_data;

    g_assert_true (LRG_IS_SAVEABLE (fixture->property));
}

/* ==========================================================================
 * Trade Investment Tests
 * ========================================================================== */

static void
test_trade_new (TradeFixture *fixture,
                gconstpointer user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_INVESTMENT_TRADE (fixture->trade));
    g_assert_true (LP_IS_INVESTMENT (fixture->trade));
}

static void
test_trade_type (TradeFixture *fixture,
                 gconstpointer user_data)
{
    LpTradeType type;

    (void)user_data;

    type = lp_investment_trade_get_trade_type (fixture->trade);
    g_assert_cmpint (type, ==, LP_TRADE_TYPE_ROUTE);
}

static void
test_trade_asset_class (TradeFixture *fixture,
                        gconstpointer user_data)
{
    LpAssetClass asset_class;

    (void)user_data;

    asset_class = lp_investment_get_asset_class (LP_INVESTMENT (fixture->trade));
    g_assert_cmpint (asset_class, ==, LP_ASSET_CLASS_TRADE);
}

static void
test_trade_risk_level (TradeFixture *fixture,
                       gconstpointer user_data)
{
    LpRiskLevel risk;

    (void)user_data;

    risk = lp_investment_get_risk_level (LP_INVESTMENT (fixture->trade));
    g_assert_cmpint (risk, ==, LP_RISK_LEVEL_MEDIUM);
}

static void
test_trade_route_status (TradeFixture *fixture,
                         gconstpointer user_data)
{
    LpRouteStatus status;

    (void)user_data;

    /* Default status should be OPEN */
    status = lp_investment_trade_get_route_status (fixture->trade);
    g_assert_cmpint (status, ==, LP_ROUTE_STATUS_OPEN);

    /* Set to disrupted */
    lp_investment_trade_set_route_status (fixture->trade, LP_ROUTE_STATUS_DISRUPTED);
    status = lp_investment_trade_get_route_status (fixture->trade);
    g_assert_cmpint (status, ==, LP_ROUTE_STATUS_DISRUPTED);
}

static void
test_trade_market_modifier (TradeFixture *fixture,
                            gconstpointer user_data)
{
    gdouble modifier;

    (void)user_data;

    /* Default modifier should be 1.0 */
    modifier = lp_investment_trade_get_market_modifier (fixture->trade);
    g_assert_cmpfloat (modifier, ==, 1.0);

    /* Set boom modifier */
    lp_investment_trade_set_market_modifier (fixture->trade, 1.5);
    modifier = lp_investment_trade_get_market_modifier (fixture->trade);
    g_assert_cmpfloat (modifier, ==, 1.5);
}

static void
test_trade_returns (TradeFixture *fixture,
                    gconstpointer user_data)
{
    g_autoptr(LrgBigNumber) value = NULL;
    g_autoptr(LrgBigNumber) returns = NULL;
    gdouble initial;
    gdouble final;

    (void)user_data;

    /* Set initial value */
    value = lrg_big_number_new (1000.0);
    lp_investment_set_current_value (LP_INVESTMENT (fixture->trade),
                                     g_steal_pointer (&value));

    /* Calculate returns over 10 years */
    returns = lp_investment_calculate_returns (LP_INVESTMENT (fixture->trade), 10);
    g_assert_nonnull (returns);

    initial = 1000.0;
    final = lrg_big_number_to_double (returns);

    /* Trade should have grown (5-8% base rate) */
    g_assert_cmpfloat (final, >, initial);
}

static void
test_trade_disrupted_returns (TradeFixture *fixture,
                              gconstpointer user_data)
{
    g_autoptr(LrgBigNumber) value = NULL;
    g_autoptr(LrgBigNumber) returns_open = NULL;
    g_autoptr(LrgBigNumber) returns_disrupted = NULL;
    gdouble open_val;
    gdouble disrupted_val;

    (void)user_data;

    /* Set initial value */
    value = lrg_big_number_new (1000.0);
    lp_investment_set_current_value (LP_INVESTMENT (fixture->trade),
                                     g_steal_pointer (&value));

    /* Calculate returns when open */
    returns_open = lp_investment_calculate_returns (LP_INVESTMENT (fixture->trade), 10);
    open_val = lrg_big_number_to_double (returns_open);

    /* Set to disrupted and recalculate */
    lp_investment_trade_set_route_status (fixture->trade, LP_ROUTE_STATUS_DISRUPTED);
    returns_disrupted = lp_investment_calculate_returns (LP_INVESTMENT (fixture->trade), 10);
    disrupted_val = lrg_big_number_to_double (returns_disrupted);

    /* Disrupted returns should be less than open returns */
    g_assert_cmpfloat (disrupted_val, <, open_val);
}

/* ==========================================================================
 * Financial Investment Tests
 * ========================================================================== */

static void
test_financial_new (FinancialFixture *fixture,
                    gconstpointer     user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_INVESTMENT_FINANCIAL (fixture->financial));
    g_assert_true (LP_IS_INVESTMENT (fixture->financial));
}

static void
test_financial_type (FinancialFixture *fixture,
                     gconstpointer     user_data)
{
    LpFinancialType type;

    (void)user_data;

    type = lp_investment_financial_get_financial_type (fixture->financial);
    g_assert_cmpint (type, ==, LP_FINANCIAL_TYPE_CROWN_BOND);
}

static void
test_financial_asset_class (FinancialFixture *fixture,
                            gconstpointer     user_data)
{
    LpAssetClass asset_class;

    (void)user_data;

    asset_class = lp_investment_get_asset_class (LP_INVESTMENT (fixture->financial));
    g_assert_cmpint (asset_class, ==, LP_ASSET_CLASS_FINANCIAL);
}

static void
test_financial_debt_status (FinancialFixture *fixture,
                            gconstpointer     user_data)
{
    LpDebtStatus status;

    (void)user_data;

    /* Default status should be PERFORMING */
    status = lp_investment_financial_get_debt_status (fixture->financial);
    g_assert_cmpint (status, ==, LP_DEBT_STATUS_PERFORMING);

    /* Set to delinquent */
    lp_investment_financial_set_debt_status (fixture->financial, LP_DEBT_STATUS_DELINQUENT);
    status = lp_investment_financial_get_debt_status (fixture->financial);
    g_assert_cmpint (status, ==, LP_DEBT_STATUS_DELINQUENT);
}

static void
test_financial_interest_rate (FinancialFixture *fixture,
                              gconstpointer     user_data)
{
    gdouble rate;

    (void)user_data;

    /* Crown bonds have ~4% rate */
    rate = lp_investment_financial_get_interest_rate (fixture->financial);
    g_assert_cmpfloat (rate, >=, 0.03);
    g_assert_cmpfloat (rate, <=, 0.05);
}

static void
test_financial_face_value (FinancialFixture *fixture,
                           gconstpointer     user_data)
{
    g_autoptr(LrgBigNumber) face_value = NULL;
    LrgBigNumber *retrieved;

    (void)user_data;

    /* Set face value */
    face_value = lrg_big_number_new (1000.0);
    lp_investment_financial_set_face_value (fixture->financial,
                                             g_steal_pointer (&face_value));

    retrieved = lp_investment_financial_get_face_value (fixture->financial);
    g_assert_nonnull (retrieved);
    g_assert_cmpfloat (lrg_big_number_to_double (retrieved), ==, 1000.0);
}

static void
test_financial_returns (FinancialFixture *fixture,
                        gconstpointer     user_data)
{
    g_autoptr(LrgBigNumber) value = NULL;
    g_autoptr(LrgBigNumber) face_value = NULL;
    g_autoptr(LrgBigNumber) returns = NULL;
    gdouble initial;
    gdouble final;

    (void)user_data;

    /* Set initial and face value */
    value = lrg_big_number_new (1000.0);
    face_value = lrg_big_number_new (1000.0);
    lp_investment_set_current_value (LP_INVESTMENT (fixture->financial),
                                     g_steal_pointer (&value));
    lp_investment_financial_set_face_value (fixture->financial,
                                             g_steal_pointer (&face_value));

    /* Calculate returns over 10 years */
    returns = lp_investment_calculate_returns (LP_INVESTMENT (fixture->financial), 10);
    g_assert_nonnull (returns);

    initial = 1000.0;
    final = lrg_big_number_to_double (returns);

    /* Financial should have grown (5% simple interest for crown bonds) */
    g_assert_cmpfloat (final, >, initial);

    /* With 5% simple interest over 10 years: 1000 + (1000 * 0.05 * 10) = 1500 */
    g_assert_cmpfloat (final, >=, 1450.0);
    g_assert_cmpfloat (final, <=, 1550.0);
}

static void
test_financial_default_check (FinancialFixture *fixture,
                              gconstpointer     user_data)
{
    gboolean defaulted;

    (void)user_data;

    /* Not defaulted initially */
    defaulted = lp_investment_financial_is_defaulted (fixture->financial);
    g_assert_false (defaulted);

    /* Set to default status */
    lp_investment_financial_set_debt_status (fixture->financial, LP_DEBT_STATUS_DEFAULT);
    defaulted = lp_investment_financial_is_defaulted (fixture->financial);
    g_assert_true (defaulted);
}

/* ==========================================================================
 * Portfolio Investment Management Tests
 * ========================================================================== */

static void
test_portfolio_add_investment (PortfolioInvFixture *fixture,
                               gconstpointer        user_data)
{
    LpInvestmentProperty *property;
    guint count;

    (void)user_data;

    property = lp_investment_property_new ("prop-001", "Test Farm",
                                           LP_PROPERTY_TYPE_AGRICULTURAL);

    /* Add investment (takes ownership) */
    lp_portfolio_add_investment (fixture->portfolio, LP_INVESTMENT (property));

    count = lp_portfolio_get_investment_count (fixture->portfolio);
    g_assert_cmpuint (count, ==, 1);
}

static void
test_portfolio_get_by_id (PortfolioInvFixture *fixture,
                          gconstpointer        user_data)
{
    LpInvestmentProperty *property;
    LpInvestment *found;

    (void)user_data;

    property = lp_investment_property_new ("prop-unique", "Unique Farm",
                                           LP_PROPERTY_TYPE_AGRICULTURAL);
    lp_portfolio_add_investment (fixture->portfolio, LP_INVESTMENT (property));

    found = lp_portfolio_get_investment_by_id (fixture->portfolio, "prop-unique");
    g_assert_nonnull (found);
    g_assert_true (LP_INVESTMENT (property) == found);

    /* Not found case */
    found = lp_portfolio_get_investment_by_id (fixture->portfolio, "nonexistent");
    g_assert_null (found);
}

static void
test_portfolio_get_by_class (PortfolioInvFixture *fixture,
                             gconstpointer        user_data)
{
    LpInvestmentProperty *property;
    LpInvestmentTrade *trade;
    g_autoptr(GPtrArray) properties = NULL;
    g_autoptr(GPtrArray) trades = NULL;

    (void)user_data;

    property = lp_investment_property_new ("prop-001", "Farm",
                                           LP_PROPERTY_TYPE_AGRICULTURAL);
    trade = lp_investment_trade_new ("trade-001", "Route",
                                     LP_TRADE_TYPE_ROUTE);

    lp_portfolio_add_investment (fixture->portfolio, LP_INVESTMENT (property));
    lp_portfolio_add_investment (fixture->portfolio, LP_INVESTMENT (trade));

    properties = lp_portfolio_get_investments_by_class (fixture->portfolio,
                                                         LP_ASSET_CLASS_PROPERTY);
    g_assert_cmpuint (properties->len, ==, 1);

    trades = lp_portfolio_get_investments_by_class (fixture->portfolio,
                                                     LP_ASSET_CLASS_TRADE);
    g_assert_cmpuint (trades->len, ==, 1);
}

static void
test_portfolio_remove_investment (PortfolioInvFixture *fixture,
                                  gconstpointer        user_data)
{
    LpInvestmentProperty *property;
    gboolean result;
    guint count;

    (void)user_data;

    property = lp_investment_property_new ("prop-remove", "To Remove",
                                           LP_PROPERTY_TYPE_AGRICULTURAL);
    lp_portfolio_add_investment (fixture->portfolio, LP_INVESTMENT (property));

    count = lp_portfolio_get_investment_count (fixture->portfolio);
    g_assert_cmpuint (count, ==, 1);

    /* Remove by ID */
    result = lp_portfolio_remove_investment_by_id (fixture->portfolio, "prop-remove");
    g_assert_true (result);

    count = lp_portfolio_get_investment_count (fixture->portfolio);
    g_assert_cmpuint (count, ==, 0);
}

static void
test_portfolio_total_value (PortfolioInvFixture *fixture,
                            gconstpointer        user_data)
{
    LpInvestmentProperty *property;
    g_autoptr(LrgBigNumber) value = NULL;
    g_autoptr(LrgBigNumber) total = NULL;
    gdouble total_val;

    (void)user_data;

    /* Portfolio starts with 1000 gold */
    property = lp_investment_property_new_with_value ("prop-001", "Farm",
                                                       LP_PROPERTY_TYPE_AGRICULTURAL,
                                                       lrg_big_number_new (500.0));
    lp_portfolio_add_investment (fixture->portfolio, LP_INVESTMENT (property));

    total = lp_portfolio_get_total_value (fixture->portfolio);
    total_val = lrg_big_number_to_double (total);

    /* 1000 gold + 500 investment = 1500 */
    g_assert_cmpfloat (total_val, ==, 1500.0);
}

static void
test_portfolio_calculate_income (PortfolioInvFixture *fixture,
                                 gconstpointer        user_data)
{
    LpInvestmentProperty *property;
    g_autoptr(LrgBigNumber) income = NULL;
    gdouble income_val;

    (void)user_data;

    property = lp_investment_property_new_with_value ("prop-001", "Farm",
                                                       LP_PROPERTY_TYPE_AGRICULTURAL,
                                                       lrg_big_number_new (1000.0));
    lp_portfolio_add_investment (fixture->portfolio, LP_INVESTMENT (property));

    /* Calculate income over 10 years */
    income = lp_portfolio_calculate_income (fixture->portfolio, 10);
    income_val = lrg_big_number_to_double (income);

    /* Should have positive income from the investment */
    g_assert_cmpfloat (income_val, >, 0.0);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Property tests */
    g_test_add ("/investment/property/new",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_new, property_fixture_tear_down);
    g_test_add ("/investment/property/type",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_type, property_fixture_tear_down);
    g_test_add ("/investment/property/asset-class",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_asset_class, property_fixture_tear_down);
    g_test_add ("/investment/property/risk-level",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_risk_level, property_fixture_tear_down);
    g_test_add ("/investment/property/stability-bonus",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_stability_bonus, property_fixture_tear_down);
    g_test_add ("/investment/property/returns",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_returns, property_fixture_tear_down);
    g_test_add ("/investment/property/improvements",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_improvements, property_fixture_tear_down);
    g_test_add ("/investment/property/saveable",
                PropertyFixture, NULL,
                property_fixture_set_up, test_property_saveable, property_fixture_tear_down);

    /* Trade tests */
    g_test_add ("/investment/trade/new",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_new, trade_fixture_tear_down);
    g_test_add ("/investment/trade/type",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_type, trade_fixture_tear_down);
    g_test_add ("/investment/trade/asset-class",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_asset_class, trade_fixture_tear_down);
    g_test_add ("/investment/trade/risk-level",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_risk_level, trade_fixture_tear_down);
    g_test_add ("/investment/trade/route-status",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_route_status, trade_fixture_tear_down);
    g_test_add ("/investment/trade/market-modifier",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_market_modifier, trade_fixture_tear_down);
    g_test_add ("/investment/trade/returns",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_returns, trade_fixture_tear_down);
    g_test_add ("/investment/trade/disrupted-returns",
                TradeFixture, NULL,
                trade_fixture_set_up, test_trade_disrupted_returns, trade_fixture_tear_down);

    /* Financial tests */
    g_test_add ("/investment/financial/new",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_new, financial_fixture_tear_down);
    g_test_add ("/investment/financial/type",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_type, financial_fixture_tear_down);
    g_test_add ("/investment/financial/asset-class",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_asset_class, financial_fixture_tear_down);
    g_test_add ("/investment/financial/debt-status",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_debt_status, financial_fixture_tear_down);
    g_test_add ("/investment/financial/interest-rate",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_interest_rate, financial_fixture_tear_down);
    g_test_add ("/investment/financial/face-value",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_face_value, financial_fixture_tear_down);
    g_test_add ("/investment/financial/returns",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_returns, financial_fixture_tear_down);
    g_test_add ("/investment/financial/default-check",
                FinancialFixture, NULL,
                financial_fixture_set_up, test_financial_default_check, financial_fixture_tear_down);

    /* Portfolio investment management tests */
    g_test_add ("/portfolio/investment/add",
                PortfolioInvFixture, NULL,
                portfolio_inv_fixture_set_up, test_portfolio_add_investment, portfolio_inv_fixture_tear_down);
    g_test_add ("/portfolio/investment/get-by-id",
                PortfolioInvFixture, NULL,
                portfolio_inv_fixture_set_up, test_portfolio_get_by_id, portfolio_inv_fixture_tear_down);
    g_test_add ("/portfolio/investment/get-by-class",
                PortfolioInvFixture, NULL,
                portfolio_inv_fixture_set_up, test_portfolio_get_by_class, portfolio_inv_fixture_tear_down);
    g_test_add ("/portfolio/investment/remove",
                PortfolioInvFixture, NULL,
                portfolio_inv_fixture_set_up, test_portfolio_remove_investment, portfolio_inv_fixture_tear_down);
    g_test_add ("/portfolio/investment/total-value",
                PortfolioInvFixture, NULL,
                portfolio_inv_fixture_set_up, test_portfolio_total_value, portfolio_inv_fixture_tear_down);
    g_test_add ("/portfolio/investment/calculate-income",
                PortfolioInvFixture, NULL,
                portfolio_inv_fixture_set_up, test_portfolio_calculate_income, portfolio_inv_fixture_tear_down);

    return g_test_run ();
}
