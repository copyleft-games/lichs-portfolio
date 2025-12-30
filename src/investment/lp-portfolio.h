/* lp-portfolio.h - Investment Portfolio Container
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The Portfolio holds all of the player's investments and gold.
 * It tracks total value, manages buy/sell operations, and calculates
 * returns during slumber periods.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_PORTFOLIO_H
#define LP_PORTFOLIO_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_PORTFOLIO (lp_portfolio_get_type ())

G_DECLARE_FINAL_TYPE (LpPortfolio, lp_portfolio, LP, PORTFOLIO, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_portfolio_new:
 *
 * Creates a new portfolio with default starting gold.
 *
 * Returns: (transfer full): A new #LpPortfolio
 */
LpPortfolio *
lp_portfolio_new (void);

/**
 * lp_portfolio_new_with_gold:
 * @initial_gold: (transfer full): Initial gold amount
 *
 * Creates a new portfolio with specified starting gold.
 *
 * Returns: (transfer full): A new #LpPortfolio
 */
LpPortfolio *
lp_portfolio_new_with_gold (LrgBigNumber *initial_gold);

/* ==========================================================================
 * Gold Management
 * ========================================================================== */

/**
 * lp_portfolio_get_gold:
 * @self: an #LpPortfolio
 *
 * Gets the current gold amount.
 *
 * Returns: (transfer none): The current gold as #LrgBigNumber
 */
LrgBigNumber *
lp_portfolio_get_gold (LpPortfolio *self);

/**
 * lp_portfolio_set_gold:
 * @self: an #LpPortfolio
 * @gold: (transfer full): The new gold amount
 *
 * Sets the gold amount directly.
 */
void
lp_portfolio_set_gold (LpPortfolio  *self,
                       LrgBigNumber *gold);

/**
 * lp_portfolio_add_gold:
 * @self: an #LpPortfolio
 * @amount: (transfer none): Amount to add
 *
 * Adds gold to the portfolio.
 */
void
lp_portfolio_add_gold (LpPortfolio        *self,
                       const LrgBigNumber *amount);

/**
 * lp_portfolio_subtract_gold:
 * @self: an #LpPortfolio
 * @amount: (transfer none): Amount to subtract
 *
 * Subtracts gold from the portfolio.
 * Will not go below zero.
 *
 * Returns: %TRUE if enough gold was available
 */
gboolean
lp_portfolio_subtract_gold (LpPortfolio        *self,
                            const LrgBigNumber *amount);

/**
 * lp_portfolio_can_afford:
 * @self: an #LpPortfolio
 * @cost: (transfer none): Cost to check
 *
 * Checks if the portfolio has enough gold.
 *
 * Returns: %TRUE if gold >= cost
 */
gboolean
lp_portfolio_can_afford (LpPortfolio        *self,
                         const LrgBigNumber *cost);

/* ==========================================================================
 * Investment Management
 * ========================================================================== */

/**
 * lp_portfolio_get_investments:
 * @self: an #LpPortfolio
 *
 * Gets the list of investments.
 *
 * Returns: (transfer none) (element-type LpInvestment): Array of investments
 */
GPtrArray *
lp_portfolio_get_investments (LpPortfolio *self);

/**
 * lp_portfolio_get_investment_count:
 * @self: an #LpPortfolio
 *
 * Gets the number of investments.
 *
 * Returns: Number of investments
 */
guint
lp_portfolio_get_investment_count (LpPortfolio *self);

/**
 * lp_portfolio_add_investment:
 * @self: an #LpPortfolio
 * @investment: (transfer full): Investment to add
 *
 * Adds an investment to the portfolio. The portfolio takes ownership.
 *
 * Emits: #LpPortfolio::investment-added
 */
void
lp_portfolio_add_investment (LpPortfolio  *self,
                             LpInvestment *investment);

/**
 * lp_portfolio_remove_investment:
 * @self: an #LpPortfolio
 * @investment: Investment to remove
 *
 * Removes an investment from the portfolio.
 * The investment is unreffed when removed.
 *
 * Emits: #LpPortfolio::investment-removed
 *
 * Returns: %TRUE if the investment was found and removed
 */
gboolean
lp_portfolio_remove_investment (LpPortfolio  *self,
                                LpInvestment *investment);

/**
 * lp_portfolio_remove_investment_by_id:
 * @self: an #LpPortfolio
 * @investment_id: ID of investment to remove
 *
 * Removes an investment by its ID.
 *
 * Emits: #LpPortfolio::investment-removed
 *
 * Returns: %TRUE if the investment was found and removed
 */
gboolean
lp_portfolio_remove_investment_by_id (LpPortfolio *self,
                                      const gchar *investment_id);

/**
 * lp_portfolio_get_investment_by_id:
 * @self: an #LpPortfolio
 * @investment_id: ID to search for
 *
 * Finds an investment by its ID.
 *
 * Returns: (transfer none) (nullable): The investment, or %NULL if not found
 */
LpInvestment *
lp_portfolio_get_investment_by_id (LpPortfolio *self,
                                   const gchar *investment_id);

/**
 * lp_portfolio_get_investments_by_class:
 * @self: an #LpPortfolio
 * @asset_class: The #LpAssetClass to filter by
 *
 * Gets all investments of a specific asset class.
 *
 * Returns: (transfer container) (element-type LpInvestment): Array of matching investments
 */
GPtrArray *
lp_portfolio_get_investments_by_class (LpPortfolio  *self,
                                       LpAssetClass  asset_class);

/**
 * lp_portfolio_get_investments_by_risk:
 * @self: an #LpPortfolio
 * @risk_level: The #LpRiskLevel to filter by
 *
 * Gets all investments of a specific risk level.
 *
 * Returns: (transfer container) (element-type LpInvestment): Array of matching investments
 */
GPtrArray *
lp_portfolio_get_investments_by_risk (LpPortfolio *self,
                                      LpRiskLevel  risk_level);

/**
 * lp_portfolio_get_total_value:
 * @self: an #LpPortfolio
 *
 * Gets the total value of all investments plus gold.
 *
 * Returns: (transfer full): Total portfolio value
 */
LrgBigNumber *
lp_portfolio_get_total_value (LpPortfolio *self);

/**
 * lp_portfolio_get_investment_value:
 * @self: an #LpPortfolio
 *
 * Gets the total value of investments only (excluding gold).
 *
 * Returns: (transfer full): Total investment value
 */
LrgBigNumber *
lp_portfolio_get_investment_value (LpPortfolio *self);

/**
 * lp_portfolio_calculate_income:
 * @self: an #LpPortfolio
 * @years: Number of years to calculate
 *
 * Calculates the expected income from all investments over the
 * specified number of years. Does not modify the portfolio.
 *
 * Returns: (transfer full): Expected income as #LrgBigNumber
 */
LrgBigNumber *
lp_portfolio_calculate_income (LpPortfolio *self,
                               guint        years);

/**
 * lp_portfolio_apply_slumber:
 * @self: an #LpPortfolio
 * @years: Number of years slumbered
 *
 * Applies the effects of slumber to all investments.
 * Updates investment values and adds income to gold.
 *
 * Returns: (transfer full): Total income earned during slumber
 */
LrgBigNumber *
lp_portfolio_apply_slumber (LpPortfolio *self,
                            guint        years);

/**
 * lp_portfolio_apply_event:
 * @self: an #LpPortfolio
 * @event: The #LpEvent to apply
 *
 * Applies an event to all investments in the portfolio.
 * Events may affect investment values based on their type.
 */
void
lp_portfolio_apply_event (LpPortfolio *self,
                          LpEvent     *event);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_portfolio_reset:
 * @self: an #LpPortfolio
 * @starting_gold: (transfer full) (nullable): New starting gold, or %NULL for default
 *
 * Resets the portfolio to initial state.
 * Called when starting a new game or after prestige.
 */
void
lp_portfolio_reset (LpPortfolio  *self,
                    LrgBigNumber *starting_gold);

G_END_DECLS

#endif /* LP_PORTFOLIO_H */
