/* lp-state-wake.h - Wake Report Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The wake state shows the report of what happened during slumber.
 * Displays events, portfolio changes, agent updates, etc.
 */

#ifndef LP_STATE_WAKE_H
#define LP_STATE_WAKE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_WAKE (lp_state_wake_get_type ())

G_DECLARE_FINAL_TYPE (LpStateWake, lp_state_wake,
                      LP, STATE_WAKE, LrgGameState)

/**
 * lp_state_wake_new:
 *
 * Creates a new wake report state.
 *
 * Returns: (transfer full): A new #LpStateWake
 */
LpStateWake *
lp_state_wake_new (void);

/**
 * lp_state_wake_set_events:
 * @self: an #LpStateWake
 * @events: (transfer full) (element-type LpEvent): List of events to display
 *
 * Sets the events to display in the wake report.
 */
void
lp_state_wake_set_events (LpStateWake *self,
                          GList       *events);

G_END_DECLS

#endif /* LP_STATE_WAKE_H */
