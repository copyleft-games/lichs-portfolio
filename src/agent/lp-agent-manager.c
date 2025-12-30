/* lp-agent-manager.c - Agent Lifecycle Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_AGENT
#include "../lp-log.h"

#include "lp-agent-manager.h"
#include "lp-agent-individual.h"
#include "lp-agent-family.h"
#include "lp-trait.h"

struct _LpAgentManager
{
    GObject parent_instance;

    GPtrArray *agents;  /* Array of LpAgent* */
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
 * Helper Functions
 * ========================================================================== */

/*
 * Creates an agent of the appropriate type based on agent_type enum.
 * Used during loading to reconstruct the correct subclass.
 */
static LpAgent *
create_agent_for_type (LpAgentType agent_type)
{
    switch (agent_type)
    {
    case LP_AGENT_TYPE_INDIVIDUAL:
        return LP_AGENT (lp_agent_individual_new ("temp", "Temp"));

    case LP_AGENT_TYPE_FAMILY:
        return LP_AGENT (lp_agent_family_new ("temp", "Temp", 847));

    default:
        lp_log_warning ("Unknown agent type %d, creating base agent", agent_type);
        return lp_agent_new ("temp", "Temp");
    }
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_agent_manager_get_save_id (LrgSaveable *saveable)
{
    (void)saveable;
    return "agent-manager";
}

static gboolean
lp_agent_manager_save (LrgSaveable    *saveable,
                       LrgSaveContext *context,
                       GError        **error)
{
    LpAgentManager *self = LP_AGENT_MANAGER (saveable);
    guint i;

    lrg_save_context_write_uint (context, "agent-count", self->agents->len);

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);
        g_autofree gchar *key = g_strdup_printf ("agent-%u", i);

        lrg_save_context_begin_section (context, key);
        lrg_saveable_save (LRG_SAVEABLE (agent), context, error);
        lrg_save_context_end_section (context);
    }

    lp_log_debug ("Saved %u agents", self->agents->len);

    return TRUE;
}

static gboolean
lp_agent_manager_load (LrgSaveable    *saveable,
                       LrgSaveContext *context,
                       GError        **error)
{
    LpAgentManager *self = LP_AGENT_MANAGER (saveable);
    guint agent_count;
    guint i;

    /* Clear existing agents */
    g_ptr_array_set_size (self->agents, 0);

    agent_count = lrg_save_context_read_uint (context, "agent-count", 0);

    for (i = 0; i < agent_count; i++)
    {
        g_autofree gchar *key = g_strdup_printf ("agent-%u", i);
        LpAgent *agent;
        LpAgentType agent_type;

        if (lrg_save_context_enter_section (context, key))
        {
            /* Read agent type first to create correct subclass */
            agent_type = lrg_save_context_read_int (context, "agent-type",
                                                    LP_AGENT_TYPE_INDIVIDUAL);

            agent = create_agent_for_type (agent_type);
            lrg_saveable_load (LRG_SAVEABLE (agent), context, error);

            g_ptr_array_add (self->agents, agent);
            lrg_save_context_leave_section (context);
        }
    }

    lp_log_debug ("Loaded %u agents", self->agents->len);

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
                      LP_TYPE_AGENT);

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
                      LP_TYPE_AGENT);

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
                      LP_TYPE_AGENT,
                      LP_TYPE_AGENT);
}

static void
lp_agent_manager_init (LpAgentManager *self)
{
    self->agents = g_ptr_array_new_with_free_func (g_object_unref);
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpAgentManager *
lp_agent_manager_new (void)
{
    return g_object_new (LP_TYPE_AGENT_MANAGER, NULL);
}

/* ==========================================================================
 * Agent Management
 * ========================================================================== */

void
lp_agent_manager_add_agent (LpAgentManager *self,
                            LpAgent        *agent)
{
    g_return_if_fail (LP_IS_AGENT_MANAGER (self));
    g_return_if_fail (LP_IS_AGENT (agent));

    /* Check if already exists */
    if (lp_agent_manager_get_agent_by_id (self, lp_agent_get_id (agent)) != NULL)
    {
        lp_log_warning ("Agent %s already exists in manager",
                        lp_agent_get_id (agent));
        return;
    }

    g_ptr_array_add (self->agents, agent);

    lp_log_debug ("Added agent %s (%s)",
                  lp_agent_get_name (agent), lp_agent_get_id (agent));

    g_signal_emit (self, signals[SIGNAL_AGENT_ADDED], 0, agent);
}

gboolean
lp_agent_manager_remove_agent (LpAgentManager *self,
                               LpAgent        *agent)
{
    gboolean removed;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_AGENT (agent), FALSE);

    /* Hold a reference during signal emission */
    g_object_ref (agent);

    removed = g_ptr_array_remove (self->agents, agent);

    if (removed)
    {
        lp_log_debug ("Removed agent %s", lp_agent_get_id (agent));
        g_signal_emit (self, signals[SIGNAL_AGENT_REMOVED], 0, agent);
    }

    g_object_unref (agent);

    return removed;
}

LpAgent *
lp_agent_manager_get_agent_by_id (LpAgentManager *self,
                                  const gchar    *agent_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);
    g_return_val_if_fail (agent_id != NULL, NULL);

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);

        if (g_strcmp0 (lp_agent_get_id (agent), agent_id) == 0)
            return agent;
    }

    return NULL;
}

/* ==========================================================================
 * Agent Tracking
 * ========================================================================== */

GPtrArray *
lp_agent_manager_get_agents (LpAgentManager *self)
{
    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);

    return self->agents;
}

guint
lp_agent_manager_get_agent_count (LpAgentManager *self)
{
    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), 0);

    return self->agents->len;
}

GList *
lp_agent_manager_get_available_agents (LpAgentManager *self)
{
    GList *available = NULL;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);
        GPtrArray *investments = lp_agent_get_assigned_investments (agent);

        /* Agent is available if not assigned to any investments */
        if (investments == NULL || investments->len == 0)
        {
            available = g_list_prepend (available, agent);
        }
    }

    return g_list_reverse (available);
}

GList *
lp_agent_manager_get_agents_by_type (LpAgentManager *self,
                                     LpAgentType     agent_type)
{
    GList *filtered = NULL;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);

        if (lp_agent_get_agent_type (agent) == agent_type)
        {
            filtered = g_list_prepend (filtered, agent);
        }
    }

    return g_list_reverse (filtered);
}

/* ==========================================================================
 * Simulation
 * ========================================================================== */

void
lp_agent_manager_advance_years (LpAgentManager *self,
                                guint           years)
{
    guint year;

    g_return_if_fail (LP_IS_AGENT_MANAGER (self));

    lp_log_debug ("Advancing agents by %u years", years);

    for (year = 0; year < years; year++)
    {
        lp_agent_manager_process_year (self);
    }
}

void
lp_agent_manager_process_year (LpAgentManager *self)
{
    GPtrArray *agents_to_process;
    guint i;

    g_return_if_fail (LP_IS_AGENT_MANAGER (self));

    /*
     * Copy agent list since processing might modify it
     * (e.g., adding successors, removing dead agents)
     */
    agents_to_process = g_ptr_array_new_with_free_func (g_object_unref);

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);
        g_ptr_array_add (agents_to_process, g_object_ref (agent));
    }

    /* Process each agent */
    for (i = 0; i < agents_to_process->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (agents_to_process, i);
        gboolean was_alive = lp_agent_is_alive (agent);

        /* Call the agent's year processing */
        lp_agent_on_year_passed (agent);

        /* Check if agent died this year */
        if (was_alive && !lp_agent_is_alive (agent))
        {
            /* For individual agents, process succession */
            if (LP_IS_AGENT_INDIVIDUAL (agent))
            {
                LpAgent *successor;

                successor = lp_agent_manager_process_succession (self, agent);
                g_signal_emit (self, signals[SIGNAL_AGENT_DIED], 0, agent, successor);

                /* Remove the dead agent (family agents handle this internally) */
                lp_agent_manager_remove_agent (self, agent);
            }
            /* Family agents don't die - they advance generations */
        }
    }

    g_ptr_array_unref (agents_to_process);
}

/* ==========================================================================
 * Succession Handling
 * ========================================================================== */

LpAgent *
lp_agent_manager_process_succession (LpAgentManager *self,
                                     LpAgent        *dying_agent)
{
    LpAgentIndividual *individual;
    LpAgentIndividual *successor;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), NULL);
    g_return_val_if_fail (LP_IS_AGENT (dying_agent), NULL);

    /* Only individual agents have explicit succession */
    if (!LP_IS_AGENT_INDIVIDUAL (dying_agent))
        return NULL;

    individual = LP_AGENT_INDIVIDUAL (dying_agent);
    successor = lp_agent_individual_get_successor (individual);

    if (successor != NULL)
    {
        /* Add successor to manager if not already present */
        if (lp_agent_manager_get_agent_by_id (self, lp_agent_get_id (LP_AGENT (successor))) == NULL)
        {
            lp_agent_manager_add_agent (self, g_object_ref (LP_AGENT (successor)));
        }

        lp_log_info ("Succession: %s -> %s",
                     lp_agent_get_name (dying_agent),
                     lp_agent_get_name (LP_AGENT (successor)));

        return LP_AGENT (successor);
    }

    lp_log_info ("Agent %s died with no successor",
                 lp_agent_get_name (dying_agent));

    return NULL;
}

/* ==========================================================================
 * Statistics
 * ========================================================================== */

guint
lp_agent_manager_get_total_exposure (LpAgentManager *self)
{
    guint total = 0;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), 0);

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);
        total += lp_agent_get_exposure_contribution (agent);
    }

    return total;
}

gint
lp_agent_manager_get_average_loyalty (LpAgentManager *self)
{
    gint total = 0;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), -1);

    if (self->agents->len == 0)
        return -1;

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);
        total += lp_agent_get_loyalty (agent);
    }

    return total / (gint)self->agents->len;
}

gint
lp_agent_manager_get_average_competence (LpAgentManager *self)
{
    gint total = 0;
    guint i;

    g_return_val_if_fail (LP_IS_AGENT_MANAGER (self), -1);

    if (self->agents->len == 0)
        return -1;

    for (i = 0; i < self->agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (self->agents, i);
        total += lp_agent_get_competence (agent);
    }

    return total / (gint)self->agents->len;
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

void
lp_agent_manager_reset (LpAgentManager *self)
{
    g_return_if_fail (LP_IS_AGENT_MANAGER (self));

    lp_log_debug ("Resetting agent manager");

    g_ptr_array_set_size (self->agents, 0);
}
