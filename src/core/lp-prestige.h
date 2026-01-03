/* lp-prestige.h - Custom Prestige Layer (LrgPrestige Subclass)
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpPrestige customizes the prestige mechanics for Lich's Portfolio.
 * It extends LrgPrestige with a custom reward formula and integration
 * with the LpPhylactery upgrade tree.
 *
 * ## Prestige Formula
 *
 * Points = log10(portfolio_value) - 3
 *
 * So:
 * - 1,000 gold = 0 points (minimum threshold)
 * - 10,000 gold = 1 point
 * - 100,000 gold = 2 points
 * - 1,000,000 gold = 3 points
 *
 * ## Integration with Phylactery
 *
 * When prestige is performed:
 * 1. Prestige points are calculated using the formula above
 * 2. Points are added to the phylactery via lp_phylactery_add_points()
 * 3. Portfolio is reset (gold and investments cleared)
 * 4. World simulation continues (agents, events persist)
 */

#ifndef LP_PRESTIGE_H
#define LP_PRESTIGE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_PRESTIGE (lp_prestige_get_type ())

G_DECLARE_FINAL_TYPE (LpPrestige, lp_prestige, LP, PRESTIGE, LrgPrestige)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_prestige_new:
 *
 * Creates a new prestige layer with Lich's Portfolio settings.
 *
 * Returns: (transfer full): A new #LpPrestige
 */
LpPrestige *
lp_prestige_new (void);

/* ==========================================================================
 * Configuration
 * ========================================================================== */

/**
 * lp_prestige_set_phylactery:
 * @self: an #LpPrestige
 * @phylactery: (nullable): the phylactery to integrate with
 *
 * Sets the phylactery that receives prestige points.
 * Call this after creating/loading game data.
 */
void
lp_prestige_set_phylactery (LpPrestige   *self,
                            LpPhylactery *phylactery);

/**
 * lp_prestige_get_phylactery:
 * @self: an #LpPrestige
 *
 * Gets the associated phylactery.
 *
 * Returns: (transfer none) (nullable): The #LpPhylactery
 */
LpPhylactery *
lp_prestige_get_phylactery (LpPrestige *self);

/**
 * lp_prestige_set_portfolio:
 * @self: an #LpPrestige
 * @portfolio: (nullable): the portfolio to reset on prestige
 *
 * Sets the portfolio that gets reset when prestige is performed.
 */
void
lp_prestige_set_portfolio (LpPrestige  *self,
                           LpPortfolio *portfolio);

/**
 * lp_prestige_get_portfolio:
 * @self: an #LpPrestige
 *
 * Gets the associated portfolio.
 *
 * Returns: (transfer none) (nullable): The #LpPortfolio
 */
LpPortfolio *
lp_prestige_get_portfolio (LpPrestige *self);

/* ==========================================================================
 * Prestige Information
 * ========================================================================== */

/**
 * lp_prestige_get_pending_points:
 * @self: an #LpPrestige
 * @portfolio_value: current portfolio total value
 *
 * Calculates how many points would be earned if prestige is performed now.
 *
 * Returns: Pending prestige points (may be 0)
 */
guint64
lp_prestige_get_pending_points (LpPrestige         *self,
                                const LrgBigNumber *portfolio_value);

/**
 * lp_prestige_can_perform:
 * @self: an #LpPrestige
 * @portfolio_value: current portfolio total value
 *
 * Checks if prestige requirements are met.
 * Requires at least 10,000 gold (1+ point).
 *
 * Returns: %TRUE if prestige is available
 */
gboolean
lp_prestige_can_perform (LpPrestige         *self,
                         const LrgBigNumber *portfolio_value);

G_END_DECLS

#endif /* LP_PRESTIGE_H */
