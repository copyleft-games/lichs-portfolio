/* lp-state-pause.h - Pause Menu Overlay State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The pause state is a transparent overlay that pauses gameplay
 * and shows options to resume, save, load, settings, or quit.
 */

#ifndef LP_STATE_PAUSE_H
#define LP_STATE_PAUSE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_PAUSE (lp_state_pause_get_type ())

G_DECLARE_FINAL_TYPE (LpStatePause, lp_state_pause,
                      LP, STATE_PAUSE, LrgGameState)

/**
 * lp_state_pause_new:
 *
 * Creates a new pause menu overlay state.
 *
 * Returns: (transfer full): A new #LpStatePause
 */
LpStatePause *
lp_state_pause_new (void);

G_END_DECLS

#endif /* LP_STATE_PAUSE_H */
