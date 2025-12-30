/* lp-screen-agents.h - Agent Management Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Screen for managing mortal agents - individuals, families, cults.
 * Shows agent list, assignments, and allows recruitment.
 */

#ifndef LP_SCREEN_AGENTS_H
#define LP_SCREEN_AGENTS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SCREEN_AGENTS (lp_screen_agents_get_type ())

G_DECLARE_FINAL_TYPE (LpScreenAgents, lp_screen_agents,
                      LP, SCREEN_AGENTS, LrgContainer)

/**
 * lp_screen_agents_new:
 *
 * Creates a new agent management screen.
 *
 * Returns: (transfer full): A new #LpScreenAgents
 */
LpScreenAgents * lp_screen_agents_new (void);

/**
 * lp_screen_agents_get_manager:
 * @self: an #LpScreenAgents
 *
 * Gets the agent manager being displayed.
 *
 * Returns: (transfer none) (nullable): The #LpAgentManager
 */
LpAgentManager * lp_screen_agents_get_manager (LpScreenAgents *self);

/**
 * lp_screen_agents_set_manager:
 * @self: an #LpScreenAgents
 * @manager: (nullable): the agent manager
 *
 * Sets the agent manager to display.
 */
void lp_screen_agents_set_manager (LpScreenAgents *self,
                                    LpAgentManager *manager);

/**
 * lp_screen_agents_get_selected_agent:
 * @self: an #LpScreenAgents
 *
 * Gets the currently selected agent.
 *
 * Returns: (transfer none) (nullable): The selected #LpAgent
 */
LpAgent * lp_screen_agents_get_selected_agent (LpScreenAgents *self);

/**
 * lp_screen_agents_refresh:
 * @self: an #LpScreenAgents
 *
 * Refreshes the agent list from the manager.
 */
void lp_screen_agents_refresh (LpScreenAgents *self);

G_END_DECLS

#endif /* LP_SCREEN_AGENTS_H */
