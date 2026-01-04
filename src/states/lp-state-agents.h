/* lp-state-agents.h - Agent Management Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The agents state allows the player to view, recruit, and manage
 * their network of mortal agents.
 */

#ifndef LP_STATE_AGENTS_H
#define LP_STATE_AGENTS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_AGENTS (lp_state_agents_get_type ())

G_DECLARE_FINAL_TYPE (LpStateAgents, lp_state_agents,
                      LP, STATE_AGENTS, LrgGameState)

/**
 * lp_state_agents_new:
 *
 * Creates a new agent management state.
 *
 * Returns: (transfer full): A new #LpStateAgents
 */
LpStateAgents *
lp_state_agents_new (void);

G_END_DECLS

#endif /* LP_STATE_AGENTS_H */
