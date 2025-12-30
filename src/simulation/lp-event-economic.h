/* lp-event-economic.h - Economic Events
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Economic events affect markets, trade, and investment returns.
 * Examples: market crashes, trade route discoveries, resource booms.
 */

#ifndef LP_EVENT_ECONOMIC_H
#define LP_EVENT_ECONOMIC_H

#include <glib-object.h>
#include "lp-event.h"

G_BEGIN_DECLS

#define LP_TYPE_EVENT_ECONOMIC (lp_event_economic_get_type ())

G_DECLARE_FINAL_TYPE (LpEventEconomic, lp_event_economic,
                      LP, EVENT_ECONOMIC, LpEvent)

/**
 * lp_event_economic_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new economic event.
 *
 * Returns: (transfer full): A new #LpEventEconomic
 */
LpEventEconomic *
lp_event_economic_new (const gchar *id,
                       const gchar *name);

/**
 * lp_event_economic_get_market_modifier:
 * @self: an #LpEventEconomic
 *
 * Gets the market-wide modifier from this event.
 *
 * Returns: Market modifier (1.0 = no change)
 */
gdouble
lp_event_economic_get_market_modifier (LpEventEconomic *self);

/**
 * lp_event_economic_set_market_modifier:
 * @self: an #LpEventEconomic
 * @modifier: the market modifier
 *
 * Sets the market-wide modifier.
 */
void
lp_event_economic_set_market_modifier (LpEventEconomic *self,
                                       gdouble          modifier);

/**
 * lp_event_economic_get_affected_asset_class:
 * @self: an #LpEventEconomic
 *
 * Gets the asset class primarily affected by this event.
 *
 * Returns: The #LpAssetClass, or -1 if all classes affected
 */
gint
lp_event_economic_get_affected_asset_class (LpEventEconomic *self);

/**
 * lp_event_economic_set_affected_asset_class:
 * @self: an #LpEventEconomic
 * @asset_class: the #LpAssetClass, or -1 for all
 *
 * Sets which asset class is affected.
 */
void
lp_event_economic_set_affected_asset_class (LpEventEconomic *self,
                                            gint             asset_class);

G_END_DECLS

#endif /* LP_EVENT_ECONOMIC_H */
