/* lp-state-decide.h - Decision Making Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The decide state handles investment decisions, agent assignments,
 * and other player actions before entering slumber.
 */

#ifndef LP_STATE_DECIDE_H
#define LP_STATE_DECIDE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_DECIDE (lp_state_decide_get_type ())

G_DECLARE_FINAL_TYPE (LpStateDecide, lp_state_decide,
                      LP, STATE_DECIDE, LrgGameState)

/**
 * lp_state_decide_new:
 *
 * Creates a new decision making state.
 *
 * Returns: (transfer full): A new #LpStateDecide
 */
LpStateDecide *
lp_state_decide_new (void);

G_END_DECLS

#endif /* LP_STATE_DECIDE_H */
