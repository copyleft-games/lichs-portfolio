/* lp-event-political.h - Political Events
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Political events affect kingdoms, regions, and stability.
 * Examples: wars, succession crises, revolutions, treaties.
 */

#ifndef LP_EVENT_POLITICAL_H
#define LP_EVENT_POLITICAL_H

#include <glib-object.h>
#include "lp-event.h"

G_BEGIN_DECLS

#define LP_TYPE_EVENT_POLITICAL (lp_event_political_get_type ())

G_DECLARE_FINAL_TYPE (LpEventPolitical, lp_event_political,
                      LP, EVENT_POLITICAL, LpEvent)

/**
 * lp_event_political_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new political event.
 *
 * Returns: (transfer full): A new #LpEventPolitical
 */
LpEventPolitical *
lp_event_political_new (const gchar *id,
                        const gchar *name);

/**
 * lp_event_political_get_stability_impact:
 * @self: an #LpEventPolitical
 *
 * Gets the stability impact on affected kingdoms.
 *
 * Returns: Stability change (can be negative)
 */
gint
lp_event_political_get_stability_impact (LpEventPolitical *self);

/**
 * lp_event_political_set_stability_impact:
 * @self: an #LpEventPolitical
 * @impact: stability change
 *
 * Sets the stability impact.
 */
void
lp_event_political_set_stability_impact (LpEventPolitical *self,
                                         gint              impact);

/**
 * lp_event_political_get_causes_war:
 * @self: an #LpEventPolitical
 *
 * Gets whether this event causes a war.
 *
 * Returns: %TRUE if causes war
 */
gboolean
lp_event_political_get_causes_war (LpEventPolitical *self);

/**
 * lp_event_political_set_causes_war:
 * @self: an #LpEventPolitical
 * @causes_war: whether causes war
 *
 * Sets whether this event causes a war.
 */
void
lp_event_political_set_causes_war (LpEventPolitical *self,
                                   gboolean          causes_war);

G_END_DECLS

#endif /* LP_EVENT_POLITICAL_H */
