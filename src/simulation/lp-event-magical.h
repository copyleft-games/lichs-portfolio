/* lp-event-magical.h - Magical Events
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Magical events involve supernatural occurrences.
 * Examples: artifact discoveries, divine intervention, magical plagues.
 */

#ifndef LP_EVENT_MAGICAL_H
#define LP_EVENT_MAGICAL_H

#include <glib-object.h>
#include "lp-event.h"

G_BEGIN_DECLS

#define LP_TYPE_EVENT_MAGICAL (lp_event_magical_get_type ())

G_DECLARE_FINAL_TYPE (LpEventMagical, lp_event_magical,
                      LP, EVENT_MAGICAL, LpEvent)

/**
 * lp_event_magical_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new magical event.
 *
 * Returns: (transfer full): A new #LpEventMagical
 */
LpEventMagical *
lp_event_magical_new (const gchar *id,
                      const gchar *name);

/**
 * lp_event_magical_get_exposure_impact:
 * @self: an #LpEventMagical
 *
 * Gets the exposure impact from this event.
 *
 * Returns: Exposure change (can be negative)
 */
gint
lp_event_magical_get_exposure_impact (LpEventMagical *self);

/**
 * lp_event_magical_set_exposure_impact:
 * @self: an #LpEventMagical
 * @impact: exposure change
 *
 * Sets the exposure impact.
 */
void
lp_event_magical_set_exposure_impact (LpEventMagical *self,
                                      gint            impact);

/**
 * lp_event_magical_get_affects_dark_investments:
 * @self: an #LpEventMagical
 *
 * Gets whether this event affects dark investments specifically.
 *
 * Returns: %TRUE if affects dark investments
 */
gboolean
lp_event_magical_get_affects_dark_investments (LpEventMagical *self);

/**
 * lp_event_magical_set_affects_dark_investments:
 * @self: an #LpEventMagical
 * @affects: whether affects dark investments
 *
 * Sets whether this event affects dark investments.
 */
void
lp_event_magical_set_affects_dark_investments (LpEventMagical *self,
                                               gboolean        affects);

G_END_DECLS

#endif /* LP_EVENT_MAGICAL_H */
