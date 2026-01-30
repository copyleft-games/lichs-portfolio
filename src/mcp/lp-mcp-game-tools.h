/* lp-mcp-game-tools.h - Game-Specific MCP Debugging Tools
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * MCP tools for inspecting and manipulating Lich's Portfolio game state.
 * Extends LrgMcpToolGroup to provide game-specific debugging capabilities.
 *
 * Available tools:
 * - lp_inspect_portfolio: Get portfolio state (gold, investments, value)
 * - lp_inspect_agents: List agents with stats (type filter optional)
 * - lp_inspect_exposure: Get exposure level and status
 * - lp_inspect_synergies: List active synergies
 * - lp_advance_years: Simulate N years (slumber)
 * - lp_set_gold: Set gold amount (debug)
 * - lp_unlock_phylactery: Unlock an upgrade node
 * - lp_trigger_event: Force a specific event
 * - lp_get_game_state: Full game state summary
 */

#ifndef LP_MCP_GAME_TOOLS_H
#define LP_MCP_GAME_TOOLS_H

#ifdef LP_MCP

#include <glib-object.h>
#include <libregnum.h>

G_BEGIN_DECLS

#define LP_TYPE_MCP_GAME_TOOLS (lp_mcp_game_tools_get_type ())

G_DECLARE_FINAL_TYPE (LpMcpGameTools, lp_mcp_game_tools,
                      LP, MCP_GAME_TOOLS, LrgMcpToolGroup)

/**
 * lp_mcp_game_tools_new:
 *
 * Creates a new game tools provider.
 *
 * Returns: (transfer full): A new #LpMcpGameTools
 */
LpMcpGameTools *
lp_mcp_game_tools_new (void);

G_END_DECLS

#endif /* LP_MCP */

#endif /* LP_MCP_GAME_TOOLS_H */
