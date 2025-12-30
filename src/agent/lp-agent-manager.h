/* lp-agent-manager.h - Agent Lifecycle Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Manages the lifecycle of all agents serving the lich.
 * Handles recruitment, aging, death, succession, and assignment
 * of agents to investments.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_AGENT_MANAGER_H
#define LP_AGENT_MANAGER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_AGENT_MANAGER (lp_agent_manager_get_type ())

G_DECLARE_FINAL_TYPE (LpAgentManager, lp_agent_manager,
                      LP, AGENT_MANAGER, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_agent_manager_new:
 *
 * Creates a new agent manager.
 *
 * Returns: (transfer full): A new #LpAgentManager
 */
LpAgentManager *
lp_agent_manager_new (void);

/* ==========================================================================
 * Agent Tracking (Skeleton - Phase 3+)
 * ========================================================================== */

/**
 * lp_agent_manager_get_agents:
 * @self: an #LpAgentManager
 *
 * Gets all agents.
 *
 * Note: This is a skeleton implementation. Returns empty array in Phase 1.
 *
 * Returns: (transfer none) (element-type LpAgent): Array of agents
 */
GPtrArray *
lp_agent_manager_get_agents (LpAgentManager *self);

/**
 * lp_agent_manager_get_agent_count:
 * @self: an #LpAgentManager
 *
 * Gets the total number of agents.
 *
 * Returns: Number of agents
 */
guint
lp_agent_manager_get_agent_count (LpAgentManager *self);

/**
 * lp_agent_manager_get_available_agents:
 * @self: an #LpAgentManager
 *
 * Gets agents that are not currently assigned.
 *
 * Returns: (transfer container) (element-type LpAgent): List of unassigned agents
 */
GList *
lp_agent_manager_get_available_agents (LpAgentManager *self);

/**
 * lp_agent_manager_get_agents_by_type:
 * @self: an #LpAgentManager
 * @agent_type: the type of agent to filter
 *
 * Gets agents of a specific type.
 *
 * Returns: (transfer container) (element-type LpAgent): List of agents of the type
 */
GList *
lp_agent_manager_get_agents_by_type (LpAgentManager *self,
                                     LpAgentType     agent_type);

/* ==========================================================================
 * Simulation (Skeleton - Phase 3+)
 * ========================================================================== */

/**
 * lp_agent_manager_advance_years:
 * @self: an #LpAgentManager
 * @years: number of years to advance
 *
 * Advances the agent simulation by the given number of years.
 * Handles aging, death, succession, loyalty changes, etc.
 *
 * Note: Skeleton implementation - no-op in Phase 1.
 */
void
lp_agent_manager_advance_years (LpAgentManager *self,
                                guint           years);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_agent_manager_reset:
 * @self: an #LpAgentManager
 *
 * Resets the agent manager to initial state.
 * Called when starting a new game or after prestige.
 */
void
lp_agent_manager_reset (LpAgentManager *self);

G_END_DECLS

#endif /* LP_AGENT_MANAGER_H */
