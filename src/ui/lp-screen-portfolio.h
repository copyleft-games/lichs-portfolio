/* lp-screen-portfolio.h - Portfolio Management Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Main portfolio screen showing current investments, asset allocation,
 * synergies, and total wealth. The core screen for managing the lich's
 * financial empire.
 */

#ifndef LP_SCREEN_PORTFOLIO_H
#define LP_SCREEN_PORTFOLIO_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SCREEN_PORTFOLIO (lp_screen_portfolio_get_type ())

G_DECLARE_FINAL_TYPE (LpScreenPortfolio, lp_screen_portfolio,
                      LP, SCREEN_PORTFOLIO, LrgContainer)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_screen_portfolio_new:
 *
 * Creates a new portfolio screen.
 *
 * Returns: (transfer full): A new #LpScreenPortfolio
 */
LpScreenPortfolio * lp_screen_portfolio_new (void);

/* ==========================================================================
 * Portfolio Binding
 * ========================================================================== */

/**
 * lp_screen_portfolio_get_portfolio:
 * @self: an #LpScreenPortfolio
 *
 * Gets the portfolio being displayed.
 *
 * Returns: (transfer none) (nullable): The #LpPortfolio, or %NULL
 */
LpPortfolio * lp_screen_portfolio_get_portfolio (LpScreenPortfolio *self);

/**
 * lp_screen_portfolio_set_portfolio:
 * @self: an #LpScreenPortfolio
 * @portfolio: (nullable): the portfolio to display
 *
 * Sets the portfolio to display. The screen will update to show
 * investments from this portfolio.
 */
void lp_screen_portfolio_set_portfolio (LpScreenPortfolio *self,
                                         LpPortfolio       *portfolio);

/* ==========================================================================
 * View Modes
 * ========================================================================== */

/**
 * LpPortfolioViewMode:
 * @LP_PORTFOLIO_VIEW_LIST: List view of all investments
 * @LP_PORTFOLIO_VIEW_ALLOCATION: Pie chart of asset allocation
 * @LP_PORTFOLIO_VIEW_PERFORMANCE: Performance history graph
 *
 * View modes for the portfolio screen.
 */
typedef enum
{
    LP_PORTFOLIO_VIEW_LIST,
    LP_PORTFOLIO_VIEW_ALLOCATION,
    LP_PORTFOLIO_VIEW_PERFORMANCE
} LpPortfolioViewMode;

/**
 * lp_screen_portfolio_get_view_mode:
 * @self: an #LpScreenPortfolio
 *
 * Gets the current view mode.
 *
 * Returns: The current #LpPortfolioViewMode
 */
LpPortfolioViewMode lp_screen_portfolio_get_view_mode (LpScreenPortfolio *self);

/**
 * lp_screen_portfolio_set_view_mode:
 * @self: an #LpScreenPortfolio
 * @mode: the view mode
 *
 * Sets the view mode for the portfolio display.
 */
void lp_screen_portfolio_set_view_mode (LpScreenPortfolio   *self,
                                         LpPortfolioViewMode  mode);

/* ==========================================================================
 * Selection
 * ========================================================================== */

/**
 * lp_screen_portfolio_get_selected_investment:
 * @self: an #LpScreenPortfolio
 *
 * Gets the currently selected investment.
 *
 * Returns: (transfer none) (nullable): The selected #LpInvestment, or %NULL
 */
LpInvestment * lp_screen_portfolio_get_selected_investment (LpScreenPortfolio *self);

/**
 * lp_screen_portfolio_select_investment:
 * @self: an #LpScreenPortfolio
 * @investment: (nullable): the investment to select
 *
 * Selects an investment in the list view.
 */
void lp_screen_portfolio_select_investment (LpScreenPortfolio *self,
                                             LpInvestment      *investment);

/* ==========================================================================
 * Actions
 * ========================================================================== */

/**
 * lp_screen_portfolio_show_buy_dialog:
 * @self: an #LpScreenPortfolio
 *
 * Shows the buy investment dialog.
 */
void lp_screen_portfolio_show_buy_dialog (LpScreenPortfolio *self);

/**
 * lp_screen_portfolio_sell_selected:
 * @self: an #LpScreenPortfolio
 *
 * Initiates a sell action for the currently selected investment.
 */
void lp_screen_portfolio_sell_selected (LpScreenPortfolio *self);

/* ==========================================================================
 * Refresh
 * ========================================================================== */

/**
 * lp_screen_portfolio_refresh:
 * @self: an #LpScreenPortfolio
 *
 * Refreshes the portfolio display from current data.
 */
void lp_screen_portfolio_refresh (LpScreenPortfolio *self);

/* ==========================================================================
 * Signals
 * ========================================================================== */

/**
 * LpScreenPortfolio::investment-selected:
 * @self: the #LpScreenPortfolio
 * @investment: (nullable): the selected investment
 *
 * Emitted when an investment is selected in the list.
 */

/**
 * LpScreenPortfolio::buy-requested:
 * @self: the #LpScreenPortfolio
 *
 * Emitted when the user requests to buy an investment.
 */

/**
 * LpScreenPortfolio::sell-requested:
 * @self: the #LpScreenPortfolio
 * @investment: the investment to sell
 *
 * Emitted when the user requests to sell an investment.
 */

G_END_DECLS

#endif /* LP_SCREEN_PORTFOLIO_H */
