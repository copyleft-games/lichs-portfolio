/* main.c - Entry Point for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Main entry point for the game. Creates the game instance
 * and runs the template-managed game loop.
 */

#include <glib.h>
#include "core/lp-game.h"

int
main (int    argc,
      char **argv)
{
    g_autoptr(LpGame) game = NULL;

    game = lp_game_new ();

    return lrg_game_template_run (LRG_GAME_TEMPLATE (game), argc, argv);
}
