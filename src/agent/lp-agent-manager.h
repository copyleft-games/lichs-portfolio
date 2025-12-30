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
#include "lp-agent.h"

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
 * Agent Management
 * ========================================================================== */

/**
 * lp_agent_manager_add_agent:
 * @self: an #LpAgentManager
 * @agent: (transfer full): the #LpAgent to add
 *
 * Adds an agent to the manager. Takes ownership of the agent.
 */
void
lp_agent_manager_add_agent (LpAgentManager *self,
                            LpAgent        *agent);

/**
 * lp_agent_manager_remove_agent:
 * @self: an #LpAgentManager
 * @agent: the #LpAgent to remove
 *
 * Removes an agent from the manager.
 *
 * Returns: %TRUE if the agent was removed
 */
gboolean
lp_agent_manager_remove_agent (LpAgentManager *self,
                               LpAgent        *agent);

/**
 * lp_agent_manager_get_agent_by_id:
 * @self: an #LpAgentManager
 * @agent_id: the agent ID to find
 *
 * Finds an agent by ID.
 *
 * Returns: (transfer none) (nullable): The agent, or %NULL if not found
 */
LpAgent *
lp_agent_manager_get_agent_by_id (LpAgentManager *self,
                                  const gchar    *agent_id);

/* ==========================================================================
 * Agent Tracking
 * ========================================================================== */

/**
 * lp_agent_manager_get_agents:
 * @self: an #LpAgentManager
 *
 * Gets all agents.
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
 * Gets agents that are not currently assigned to investments.
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
 * Simulation
 * ========================================================================== */

/**
 * lp_agent_manager_advance_years:
 * @self: an #LpAgentManager
 * @years: number of years to advance
 *
 * Advances the agent simulation by the given number of years.
 * Handles aging, death, succession, loyalty changes, etc.
 */
void
lp_agent_manager_advance_years (LpAgentManager *self,
                                guint           years);

/**
 * lp_agent_manager_process_year:
 * @self: an #LpAgentManager
 *
 * Processes a single year for all agents.
 * Called internally by advance_years.
 */
void
lp_agent_manager_process_year (LpAgentManager *self);

/* ==========================================================================
 * Succession Handling
 * ========================================================================== */

/**
 * lp_agent_manager_process_succession:
 * @self: an #LpAgentManager
 * @dying_agent: the agent that died
 *
 * Handles succession when an individual agent dies.
 * The successor (if any) is added to the manager.
 *
 * Returns: (transfer none) (nullable): The successor who takes over
 */
LpAgent *
lp_agent_manager_process_succession (LpAgentManager *self,
                                     LpAgent        *dying_agent);

/* ==========================================================================
 * Statistics
 * ========================================================================== */

/**
 * lp_agent_manager_get_total_exposure:
 * @self: an #LpAgentManager
 *
 * Gets the total exposure contribution from all agents.
 *
 * Returns: Total exposure value
 */
guint
lp_agent_manager_get_total_exposure (LpAgentManager *self);

/**
 * lp_agent_manager_get_average_loyalty:
 * @self: an #LpAgentManager
 *
 * Gets the average loyalty across all agents.
 *
 * Returns: Average loyalty (0-100), or -1 if no agents
 */
gint
lp_agent_manager_get_average_loyalty (LpAgentManager *self);

/**
 * lp_agent_manager_get_average_competence:
 * @self: an #LpAgentManager
 *
 * Gets the average competence across all agents.
 *
 * Returns: Average competence (0-100), or -1 if no agents
 */
gint
lp_agent_manager_get_average_competence (LpAgentManager *self);

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
