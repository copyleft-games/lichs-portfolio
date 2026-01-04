/* lp-state-investments.h - Investment Management Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The investments state allows the player to view, buy, and sell
 * investments in their portfolio.
 */

#ifndef LP_STATE_INVESTMENTS_H
#define LP_STATE_INVESTMENTS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_INVESTMENTS (lp_state_investments_get_type ())

G_DECLARE_FINAL_TYPE (LpStateInvestments, lp_state_investments,
                      LP, STATE_INVESTMENTS, LrgGameState)

/**
 * lp_state_investments_new:
 *
 * Creates a new investment management state.
 *
 * Returns: (transfer full): A new #LpStateInvestments
 */
LpStateInvestments *
lp_state_investments_new (void);

G_END_DECLS

#endif /* LP_STATE_INVESTMENTS_H */
