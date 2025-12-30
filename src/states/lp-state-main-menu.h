/* lp-state-main-menu.h - Main Menu Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The main menu state shown at game startup.
 * Provides options to start new game, continue, settings, and quit.
 */

#ifndef LP_STATE_MAIN_MENU_H
#define LP_STATE_MAIN_MENU_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_MAIN_MENU (lp_state_main_menu_get_type ())

G_DECLARE_FINAL_TYPE (LpStateMainMenu, lp_state_main_menu,
                      LP, STATE_MAIN_MENU, LrgGameState)

/**
 * lp_state_main_menu_new:
 *
 * Creates a new main menu state.
 *
 * Returns: (transfer full): A new #LpStateMainMenu
 */
LpStateMainMenu *
lp_state_main_menu_new (void);

G_END_DECLS

#endif /* LP_STATE_MAIN_MENU_H */
