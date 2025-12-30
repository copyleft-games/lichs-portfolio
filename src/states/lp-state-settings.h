/* lp-state-settings.h - Settings Menu Overlay State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The settings state is a transparent overlay for configuring
 * game options including graphics, audio, and controls.
 */

#ifndef LP_STATE_SETTINGS_H
#define LP_STATE_SETTINGS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_SETTINGS (lp_state_settings_get_type ())

G_DECLARE_FINAL_TYPE (LpStateSettings, lp_state_settings,
                      LP, STATE_SETTINGS, LrgGameState)

/**
 * lp_state_settings_new:
 *
 * Creates a new settings menu overlay state.
 *
 * Returns: (transfer full): A new #LpStateSettings
 */
LpStateSettings *
lp_state_settings_new (void);

G_END_DECLS

#endif /* LP_STATE_SETTINGS_H */
