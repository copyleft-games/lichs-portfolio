/* lp-state-simulating.h - Slumber Simulation Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The simulating state shows the passage of time during slumber.
 * Displays year progression and key events as they happen.
 */

#ifndef LP_STATE_SIMULATING_H
#define LP_STATE_SIMULATING_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_SIMULATING (lp_state_simulating_get_type ())

G_DECLARE_FINAL_TYPE (LpStateSimulating, lp_state_simulating,
                      LP, STATE_SIMULATING, LrgGameState)

/**
 * lp_state_simulating_new:
 *
 * Creates a new slumber simulation state.
 *
 * Returns: (transfer full): A new #LpStateSimulating
 */
LpStateSimulating *
lp_state_simulating_new (void);

/**
 * lp_state_simulating_set_years:
 * @self: an #LpStateSimulating
 * @years: number of years to simulate
 *
 * Sets the number of years to simulate.
 */
void
lp_state_simulating_set_years (LpStateSimulating *self,
                               guint              years);

G_END_DECLS

#endif /* LP_STATE_SIMULATING_H */
