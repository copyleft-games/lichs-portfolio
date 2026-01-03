/* lp-state-welcome-back.h - Offline Progress Notification State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpStateWelcomeBack displays the offline progress earned while the
 * game was closed. Shows time away and gold earned, with a continue
 * button to proceed to the main game.
 */

#ifndef LP_STATE_WELCOME_BACK_H
#define LP_STATE_WELCOME_BACK_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_WELCOME_BACK (lp_state_welcome_back_get_type ())

G_DECLARE_FINAL_TYPE (LpStateWelcomeBack, lp_state_welcome_back,
                      LP, STATE_WELCOME_BACK, LrgGameState)

/**
 * lp_state_welcome_back_new:
 *
 * Creates a new welcome back state for showing offline progress.
 *
 * Returns: (transfer full): A new #LpStateWelcomeBack
 */
LpStateWelcomeBack *
lp_state_welcome_back_new (void);

/**
 * lp_state_welcome_back_set_offline_data:
 * @self: an #LpStateWelcomeBack
 * @seconds_offline: how long the player was away
 * @gold_earned: (transfer none): gold earned while offline
 *
 * Sets the offline progress data to display.
 */
void
lp_state_welcome_back_set_offline_data (LpStateWelcomeBack *self,
                                         gdouble             seconds_offline,
                                         const LrgBigNumber *gold_earned);

G_END_DECLS

#endif /* LP_STATE_WELCOME_BACK_H */
