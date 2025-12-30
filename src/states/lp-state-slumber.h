/* lp-state-slumber.h - Slumber Configuration Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The slumber state allows the player to configure how long
 * to sleep and what standing orders to give to agents.
 */

#ifndef LP_STATE_SLUMBER_H
#define LP_STATE_SLUMBER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_SLUMBER (lp_state_slumber_get_type ())

G_DECLARE_FINAL_TYPE (LpStateSlumber, lp_state_slumber,
                      LP, STATE_SLUMBER, LrgGameState)

/**
 * lp_state_slumber_new:
 *
 * Creates a new slumber configuration state.
 *
 * Returns: (transfer full): A new #LpStateSlumber
 */
LpStateSlumber *
lp_state_slumber_new (void);

/**
 * lp_state_slumber_get_years:
 * @self: an #LpStateSlumber
 *
 * Gets the configured slumber duration.
 *
 * Returns: Number of years to slumber
 */
guint
lp_state_slumber_get_years (LpStateSlumber *self);

G_END_DECLS

#endif /* LP_STATE_SLUMBER_H */
