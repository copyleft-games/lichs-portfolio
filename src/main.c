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

#ifdef LP_MCP
#include "mcp/lp-mcp.h"
#endif

int
main (int    argc,
      char **argv)
{
    g_autoptr(LpGame) game = NULL;
    gint result;

#ifdef LP_MCP
    {
        g_autoptr(GError) error = NULL;

        if (!lp_mcp_initialize (&error))
        {
            g_warning ("MCP server failed to start: %s", error->message);
            /* Continue without MCP - it's optional */
        }
    }
#endif

    game = lp_game_new ();
    result = lrg_game_template_run (LRG_GAME_TEMPLATE (game), argc, argv);

#ifdef LP_MCP
    lp_mcp_shutdown ();
#endif

    return result;
}
