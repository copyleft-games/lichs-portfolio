/* lp-agent-manager.c - Agent Lifecycle Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_AGENT
#include "../lp-log.h"

#include "lp-agent-manager.h"

struct _LpAgentManager
{
    GObject parent_instance;

    GPtrArray *agents;  /* Array of LpAgent (empty in Phase 1) */
};

enum
{
    SIGNAL_AGENT_ADDED,
    SIGNAL_AGENT_REMOVED,
    SIGNAL_AGENT_DIED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_agent_manager_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpAgentManager, lp_agent_manager, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_agent_manager_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_agent_manager_get_save_id (LrgSaveable *saveable)
{
    return "agent-manager";
}

static gboolean
lp_agent_manager_save (LrgSaveable    *saveable,
                       LrgSaveContext *context,
                       GError        **error)
{
    /* Phase 1 skeleton: No agents to save */
    lrg_save_context_write_uint (context, "agent-count", 0);

    return TRUE;
}

static gboolean
lp_agent_manager_load (LrgSaveable    *saveable,
                       LrgSaveContext *context,
                       GError        **error)
{
    LpAgentManager *self = LP_AGENT_MANAGER (saveable);

    /* Clear existing agents */
    g_ptr_array_set_size (self->agents, 0);

    /* Phase 1 skeleton: No agents to load */
    lp_log_debug ("Agent manager loaded (skeleton)");

    return TRUE;
}

static void
lp_agent_manager_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_agent_manager_get_save_id;
    iface->save = lp_agent_manager_save;
    iface->load = lp_agent_manager_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_agent_manager_finalize (GObject *object)
{
    LpAgentManager *self = LP_AGENT_MANAGER (object);

    lp_log_debug ("Finalizing agent manager");

    g_clear_pointer (&self->agents, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_agent_manager_parent_class)->finalize (object);
}

static void
lp_agent_manager_class_init (LpAgentManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_agent_manager_finalize;

    /**
     * LpAgentManager::agent-added:
     * @self: the #LpAgentManager
     * @agent: (transfer none): the added agent
     *
     * Emitted when a new agent is added.
     */
    signals[SIGNAL_AGENT_ADDED] =
        g_signal_new ("agent-added",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_OBJECT);

    /**
     * LpAgentManager::agent-removed:
     * @self: the #LpAgentManager
     * @agent: (transfer none): the removed agent
     *
     * Emitted when an agent is removed.
     */
    signals[SIGNAL_AGENT_REMOVED] =
        g_signal_new ("agent-removed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_OBJECT);

    /**
     * LpAgentManager::agent-died:
     * @self: the #LpAgentManager
     * @agent: (transfer none): the agent that died
     * @successor: (transfer none) (nullable): the successor, if any
     *
     * Emitted when an agent dies.
     */
    signals[SIGNAL_AGENT_DIED] =
        g_signal_new ("agent-died",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_OBJECT,
                      G_TYPE_OBJECT);
}

static void
lp_agent_manager_init (LpAgentManager *self)
{
    self->agents = g_ptr_array_new_with_free_func (g_object_unref);
}

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
lp_agent_manager_new (void)
{
    return g_object_new (LP_TYPE_AGENT_MANAGER, NULL);
}

/* ==========================================================================
 * Agent Tracking (Skeleton)
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
lp_agent_manager_get_agents (LpAgentManager *self)
{
    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);

    return self->agents;
}

/**
 * lp_agent_manager_get_agent_count:
 * @self: an #LpAgentManager
 *
 * Gets the total number of agents.
 *
 * Returns: Number of agents
 */
guint
lp_agent_manager_get_agent_count (LpAgentManager *self)
{
    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), 0);

    return self->agents->len;
}

/**
 * lp_agent_manager_get_available_agents:
 * @self: an #LpAgentManager
 *
 * Gets agents that are not currently assigned.
 *
 * Returns: (transfer container) (element-type LpAgent): List of unassigned agents
 */
GList *
lp_agent_manager_get_available_agents (LpAgentManager *self)
{
    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);

    /*
     * Phase 1 skeleton: No agents, return empty list.
     * Phase 3+: Filter agents by assignment status.
     */
    return NULL;
}

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
                                     LpAgentType     agent_type)
{
    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);

    /*
     * Phase 1 skeleton: No agents, return empty list.
     * Phase 3+: Filter agents by type.
     */
    return NULL;
}

/* ==========================================================================
 * Simulation (Skeleton)
 * ========================================================================== */

/**
 * lp_agent_manager_advance_years:
 * @self: an #LpAgentManager
 * @years: number of years to advance
 *
 * Advances the agent simulation.
 */
void
lp_agent_manager_advance_years (LpAgentManager *self,
                                guint           years)
{
    g_return_if_fail (LP_IS_AGENT_MANAGER (self));

    /*
     * Phase 1 skeleton: No-op.
     * Phase 3+: Process agent aging, death, succession, etc.
     */
    lp_log_debug ("Advancing agents by %u years (skeleton - no-op)", years);
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_agent_manager_reset:
 * @self: an #LpAgentManager
 *
 * Resets the agent manager to initial state.
 */
void
lp_agent_manager_reset (LpAgentManager *self)
{
    g_return_if_fail (LP_IS_AGENT_MANAGER (self));

    lp_log_debug ("Resetting agent manager");

    g_ptr_array_set_size (self->agents, 0);
}
