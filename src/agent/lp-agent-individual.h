/* lp-agent-individual.h - Individual Mortal Agent
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Individual agents are single mortals who serve the lich.
 * They can train a successor before death to preserve some
 * of their skills and knowledge.
 */

#ifndef LP_AGENT_INDIVIDUAL_H
#define LP_AGENT_INDIVIDUAL_H

#include <glib-object.h>
#include "lp-agent.h"

G_BEGIN_DECLS

#define LP_TYPE_AGENT_INDIVIDUAL (lp_agent_individual_get_type ())

G_DECLARE_FINAL_TYPE (LpAgentIndividual, lp_agent_individual, LP, AGENT_INDIVIDUAL, LpAgent)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_agent_individual_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new individual agent with default stats.
 *
 * Returns: (transfer full): A new #LpAgentIndividual
 */
LpAgentIndividual *
lp_agent_individual_new (const gchar *id,
                         const gchar *name);

/**
 * lp_agent_individual_new_full:
 * @id: unique identifier
 * @name: display name
 * @age: starting age
 * @max_age: maximum lifespan
 * @loyalty: initial loyalty (0-100)
 * @competence: initial competence (0-100)
 *
 * Creates a new individual agent with specified stats.
 *
 * Returns: (transfer full): A new #LpAgentIndividual
 */
LpAgentIndividual *
lp_agent_individual_new_full (const gchar *id,
                              const gchar *name,
                              guint        age,
                              guint        max_age,
                              gint         loyalty,
                              gint         competence);

/* ==========================================================================
 * Successor Management
 * ========================================================================== */

/**
 * lp_agent_individual_get_successor:
 * @self: an #LpAgentIndividual
 *
 * Gets the designated successor.
 *
 * Returns: (transfer none) (nullable): The successor agent
 */
LpAgentIndividual *
lp_agent_individual_get_successor (LpAgentIndividual *self);

/**
 * lp_agent_individual_set_successor:
 * @self: an #LpAgentIndividual
 * @successor: (nullable): the successor agent
 *
 * Sets the designated successor.
 */
void
lp_agent_individual_set_successor (LpAgentIndividual *self,
                                   LpAgentIndividual *successor);

/**
 * lp_agent_individual_get_training_progress:
 * @self: an #LpAgentIndividual
 *
 * Gets the successor training progress (0.0-1.0).
 *
 * Returns: Training progress
 */
gfloat
lp_agent_individual_get_training_progress (LpAgentIndividual *self);

/**
 * lp_agent_individual_set_training_progress:
 * @self: an #LpAgentIndividual
 * @progress: training progress (0.0-1.0)
 *
 * Sets the training progress.
 */
void
lp_agent_individual_set_training_progress (LpAgentIndividual *self,
                                           gfloat             progress);

/**
 * lp_agent_individual_has_trained_successor:
 * @self: an #LpAgentIndividual
 *
 * Checks if the successor is fully trained (progress >= 1.0).
 *
 * Returns: %TRUE if successor is trained
 */
gboolean
lp_agent_individual_has_trained_successor (LpAgentIndividual *self);

/* ==========================================================================
 * Training Methods
 * ========================================================================== */

/**
 * lp_agent_individual_train_successor:
 * @self: an #LpAgentIndividual
 * @years: number of years of training
 *
 * Advances successor training. Each year of training adds progress
 * based on the mentor's competence.
 */
void
lp_agent_individual_train_successor (LpAgentIndividual *self,
                                     guint              years);

/**
 * lp_agent_individual_recruit_successor:
 * @self: an #LpAgentIndividual
 *
 * Recruits a new untrained successor with random stats.
 * Fails if the agent cannot recruit (see lp_agent_can_recruit()).
 *
 * Returns: (transfer none) (nullable): The new successor, or %NULL
 */
LpAgentIndividual *
lp_agent_individual_recruit_successor (LpAgentIndividual *self);

/* ==========================================================================
 * Succession Methods
 * ========================================================================== */

/**
 * lp_agent_individual_process_succession:
 * @self: an #LpAgentIndividual
 *
 * Called when this agent dies. Transfers knowledge and investments
 * to the successor based on training progress.
 *
 * If no successor exists or is not trained:
 * - 25% skill retention
 * - Loyalty reset to base
 *
 * If trained successor exists:
 * - 75% skill retention
 * - Base loyalty preserved
 *
 * Returns: (transfer none) (nullable): The successor who takes over, or %NULL
 */
LpAgentIndividual *
lp_agent_individual_process_succession (LpAgentIndividual *self);

/**
 * lp_agent_individual_get_skill_retention:
 * @self: an #LpAgentIndividual
 *
 * Gets the skill retention multiplier based on training.
 *
 * Returns: Skill retention (0.25 to 0.75)
 */
gfloat
lp_agent_individual_get_skill_retention (LpAgentIndividual *self);

G_END_DECLS

#endif /* LP_AGENT_INDIVIDUAL_H */
