/* lp-mcp.h - MCP Server Integration
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Master header for MCP (Model Context Protocol) integration.
 * Enables Claude Code to debug and test the game via:
 * - Screenshots of the game viewport
 * - Controller/keyboard input injection
 * - Game-specific inspection and manipulation tools
 *
 * Only compiled when LP_MCP is defined (make MCP=1).
 */

#ifndef LP_MCP_H
#define LP_MCP_H

#ifdef LP_MCP

#include <glib.h>
#include <libregnum.h>

G_BEGIN_DECLS

/**
 * lp_mcp_initialize:
 * @error: (nullable): return location for error
 *
 * Initializes the MCP server with game-specific tools.
 * Registers libregnum's default providers (screenshot, input, etc.)
 * plus Lich's Portfolio game tools for debugging.
 *
 * Call this before entering the game loop.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_mcp_initialize (GError **error);

/**
 * lp_mcp_shutdown:
 *
 * Shuts down the MCP server.
 * Call this before exiting the application.
 */
void
lp_mcp_shutdown (void);

G_END_DECLS

#endif /* LP_MCP */

#endif /* LP_MCP_H */
