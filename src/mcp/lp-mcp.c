/* lp-mcp.c - MCP Server Integration
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Initializes MCP server with both libregnum's default providers
 * and Lich's Portfolio game-specific debugging tools.
 */

#ifdef LP_MCP

#include "lp-mcp.h"
#include "lp-mcp-game-tools.h"

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_MCP
#include "../lp-log.h"

gboolean
lp_mcp_initialize (GError **error)
{
    LrgMcpServer *server;
    LpMcpGameTools *game_tools;

    server = lrg_mcp_server_get_default ();

    /* Configure server identity */
    lrg_mcp_server_set_server_name (server, "lichs-portfolio");

    /* Register libregnum built-in tools (screenshot, input, etc.) */
    lrg_mcp_server_register_default_providers (server);

    /* Register game-specific debugging tools */
    game_tools = lp_mcp_game_tools_new ();
    lrg_mcp_server_add_tool_provider (server,
                                      LRG_MCP_TOOL_PROVIDER (game_tools));

    lp_log_debug ("Starting MCP server...");

    if (!lrg_mcp_server_start (server, error))
    {
        lp_log_warning ("Failed to start MCP server");
        return FALSE;
    }

    lp_log_debug ("MCP server started successfully");
    return TRUE;
}

void
lp_mcp_shutdown (void)
{
    LrgMcpServer *server;

    server = lrg_mcp_server_get_default ();
    lrg_mcp_server_stop (server);

    lp_log_debug ("MCP server stopped");
}

#endif /* LP_MCP */
