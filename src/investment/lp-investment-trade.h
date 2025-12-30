/* lp-investment-trade.h - Trade Investment
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Trade investments represent commercial ventures: trade routes,
 * commodity holdings, guild memberships, and shipping interests.
 * They offer medium risk with variable returns that depend on
 * market conditions and route safety.
 *
 * Trade investments are particularly affected by:
 * - Route disruption (wars, bandits)
 * - Market cycles (boom/bust)
 * - Political relations between kingdoms
 */

#ifndef LP_INVESTMENT_TRADE_H
#define LP_INVESTMENT_TRADE_H

#include <glib-object.h>
#include "lp-investment.h"

G_BEGIN_DECLS

#define LP_TYPE_INVESTMENT_TRADE (lp_investment_trade_get_type ())

G_DECLARE_FINAL_TYPE (LpInvestmentTrade, lp_investment_trade,
                      LP, INVESTMENT_TRADE, LpInvestment)

/**
 * LpTradeType:
 * @LP_TRADE_TYPE_ROUTE: Trade route between regions
 * @LP_TRADE_TYPE_COMMODITY: Commodity holdings (grain, metals, etc.)
 * @LP_TRADE_TYPE_GUILD: Guild membership and influence
 * @LP_TRADE_TYPE_SHIPPING: Ships and maritime trade
 * @LP_TRADE_TYPE_CARAVAN: Land-based caravan operations
 *
 * Subtypes of trade investments.
 */
typedef enum
{
    LP_TRADE_TYPE_ROUTE,
    LP_TRADE_TYPE_COMMODITY,
    LP_TRADE_TYPE_GUILD,
    LP_TRADE_TYPE_SHIPPING,
    LP_TRADE_TYPE_CARAVAN
} LpTradeType;

GType lp_trade_type_get_type (void) G_GNUC_CONST;
#define LP_TYPE_TRADE_TYPE (lp_trade_type_get_type ())

/**
 * LpRouteStatus:
 * @LP_ROUTE_STATUS_OPEN: Route is open and operating normally
 * @LP_ROUTE_STATUS_DISRUPTED: Route is disrupted (reduced returns)
 * @LP_ROUTE_STATUS_CLOSED: Route is closed (no returns)
 *
 * Status of a trade route.
 */
typedef enum
{
    LP_ROUTE_STATUS_OPEN,
    LP_ROUTE_STATUS_DISRUPTED,
    LP_ROUTE_STATUS_CLOSED
} LpRouteStatus;

GType lp_route_status_get_type (void) G_GNUC_CONST;
#define LP_TYPE_ROUTE_STATUS (lp_route_status_get_type ())

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_investment_trade_new:
 * @id: unique identifier
 * @name: display name
 * @trade_type: the #LpTradeType
 *
 * Creates a new trade investment.
 *
 * Returns: (transfer full): A new #LpInvestmentTrade
 */
LpInvestmentTrade *
lp_investment_trade_new (const gchar *id,
                         const gchar *name,
                         LpTradeType  trade_type);

/**
 * lp_investment_trade_new_with_value:
 * @id: unique identifier
 * @name: display name
 * @trade_type: the #LpTradeType
 * @value: (transfer full): initial value
 *
 * Creates a new trade investment with specified value.
 *
 * Returns: (transfer full): A new #LpInvestmentTrade
 */
LpInvestmentTrade *
lp_investment_trade_new_with_value (const gchar  *id,
                                    const gchar  *name,
                                    LpTradeType   trade_type,
                                    LrgBigNumber *value);

/* ==========================================================================
 * Trade-Specific Methods
 * ========================================================================== */

/**
 * lp_investment_trade_get_trade_type:
 * @self: an #LpInvestmentTrade
 *
 * Gets the trade subtype.
 *
 * Returns: The #LpTradeType
 */
LpTradeType
lp_investment_trade_get_trade_type (LpInvestmentTrade *self);

/**
 * lp_investment_trade_get_route_status:
 * @self: an #LpInvestmentTrade
 *
 * Gets the current route status (for route/shipping/caravan types).
 *
 * Returns: The #LpRouteStatus
 */
LpRouteStatus
lp_investment_trade_get_route_status (LpInvestmentTrade *self);

/**
 * lp_investment_trade_set_route_status:
 * @self: an #LpInvestmentTrade
 * @status: the new #LpRouteStatus
 *
 * Sets the route status.
 */
void
lp_investment_trade_set_route_status (LpInvestmentTrade *self,
                                      LpRouteStatus      status);

/**
 * lp_investment_trade_get_market_modifier:
 * @self: an #LpInvestmentTrade
 *
 * Gets the current market modifier. This reflects boom/bust cycles
 * and affects returns.
 *
 * Returns: Market modifier (1.0 = normal, >1.0 = boom, <1.0 = bust)
 */
gdouble
lp_investment_trade_get_market_modifier (LpInvestmentTrade *self);

/**
 * lp_investment_trade_set_market_modifier:
 * @self: an #LpInvestmentTrade
 * @modifier: the market modifier
 *
 * Sets the market modifier.
 */
void
lp_investment_trade_set_market_modifier (LpInvestmentTrade *self,
                                         gdouble            modifier);

/**
 * lp_investment_trade_get_source_region:
 * @self: an #LpInvestmentTrade
 *
 * Gets the source region ID (for route types).
 *
 * Returns: (transfer none) (nullable): Source region ID
 */
const gchar *
lp_investment_trade_get_source_region (LpInvestmentTrade *self);

/**
 * lp_investment_trade_set_source_region:
 * @self: an #LpInvestmentTrade
 * @region_id: (nullable): the source region ID
 *
 * Sets the source region.
 */
void
lp_investment_trade_set_source_region (LpInvestmentTrade *self,
                                       const gchar       *region_id);

/**
 * lp_investment_trade_get_destination_region:
 * @self: an #LpInvestmentTrade
 *
 * Gets the destination region ID (for route types).
 *
 * Returns: (transfer none) (nullable): Destination region ID
 */
const gchar *
lp_investment_trade_get_destination_region (LpInvestmentTrade *self);

/**
 * lp_investment_trade_set_destination_region:
 * @self: an #LpInvestmentTrade
 * @region_id: (nullable): the destination region ID
 *
 * Sets the destination region.
 */
void
lp_investment_trade_set_destination_region (LpInvestmentTrade *self,
                                            const gchar       *region_id);

/**
 * lp_investment_trade_get_commodity_type:
 * @self: an #LpInvestmentTrade
 *
 * Gets the commodity type being traded (for commodity/route types).
 *
 * Returns: (transfer none) (nullable): Commodity type string
 */
const gchar *
lp_investment_trade_get_commodity_type (LpInvestmentTrade *self);

/**
 * lp_investment_trade_set_commodity_type:
 * @self: an #LpInvestmentTrade
 * @commodity: (nullable): the commodity type
 *
 * Sets the commodity type.
 */
void
lp_investment_trade_set_commodity_type (LpInvestmentTrade *self,
                                        const gchar       *commodity);

/**
 * lp_investment_trade_is_disrupted:
 * @self: an #LpInvestmentTrade
 *
 * Checks if this trade investment is currently disrupted.
 *
 * Returns: %TRUE if disrupted or closed
 */
gboolean
lp_investment_trade_is_disrupted (LpInvestmentTrade *self);

G_END_DECLS

#endif /* LP_INVESTMENT_TRADE_H */
