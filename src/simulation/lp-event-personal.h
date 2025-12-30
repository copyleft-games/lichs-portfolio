/* lp-event-personal.h - Personal Events
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Personal events affect individual agents.
 * Examples: agent deaths, betrayal attempts, investigations.
 */

#ifndef LP_EVENT_PERSONAL_H
#define LP_EVENT_PERSONAL_H

#include <glib-object.h>
#include "lp-event.h"

G_BEGIN_DECLS

#define LP_TYPE_EVENT_PERSONAL (lp_event_personal_get_type ())

G_DECLARE_FINAL_TYPE (LpEventPersonal, lp_event_personal,
                      LP, EVENT_PERSONAL, LpEvent)

/**
 * lp_event_personal_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new personal event.
 *
 * Returns: (transfer full): A new #LpEventPersonal
 */
LpEventPersonal *
lp_event_personal_new (const gchar *id,
                       const gchar *name);

/**
 * lp_event_personal_get_target_agent_id:
 * @self: an #LpEventPersonal
 *
 * Gets the ID of the targeted agent.
 *
 * Returns: (transfer none) (nullable): The agent ID
 */
const gchar *
lp_event_personal_get_target_agent_id (LpEventPersonal *self);

/**
 * lp_event_personal_set_target_agent_id:
 * @self: an #LpEventPersonal
 * @agent_id: (nullable): the agent ID
 *
 * Sets the targeted agent.
 */
void
lp_event_personal_set_target_agent_id (LpEventPersonal *self,
                                       const gchar     *agent_id);

/**
 * lp_event_personal_get_is_betrayal:
 * @self: an #LpEventPersonal
 *
 * Gets whether this event is a betrayal.
 *
 * Returns: %TRUE if betrayal event
 */
gboolean
lp_event_personal_get_is_betrayal (LpEventPersonal *self);

/**
 * lp_event_personal_set_is_betrayal:
 * @self: an #LpEventPersonal
 * @is_betrayal: whether betrayal
 *
 * Sets whether this is a betrayal event.
 */
void
lp_event_personal_set_is_betrayal (LpEventPersonal *self,
                                   gboolean         is_betrayal);

/**
 * lp_event_personal_get_is_death:
 * @self: an #LpEventPersonal
 *
 * Gets whether this event is an agent death.
 *
 * Returns: %TRUE if death event
 */
gboolean
lp_event_personal_get_is_death (LpEventPersonal *self);

/**
 * lp_event_personal_set_is_death:
 * @self: an #LpEventPersonal
 * @is_death: whether death
 *
 * Sets whether this is a death event.
 */
void
lp_event_personal_set_is_death (LpEventPersonal *self,
                                gboolean         is_death);

G_END_DECLS

#endif /* LP_EVENT_PERSONAL_H */
