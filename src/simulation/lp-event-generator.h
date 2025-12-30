/* lp-event-generator.h - Event Generator
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The event generator creates world events based on
 * weighted probabilities influenced by the current world state.
 * Events are generated at three scales: yearly, decade, and era.
 */

#ifndef LP_EVENT_GENERATOR_H
#define LP_EVENT_GENERATOR_H

#include <glib-object.h>
#include "lp-event.h"

G_BEGIN_DECLS

#define LP_TYPE_EVENT_GENERATOR (lp_event_generator_get_type ())

G_DECLARE_FINAL_TYPE (LpEventGenerator, lp_event_generator,
                      LP, EVENT_GENERATOR, GObject)

/**
 * lp_event_generator_get_default:
 *
 * Gets the singleton event generator instance.
 *
 * Returns: (transfer none): The default #LpEventGenerator
 */
LpEventGenerator *
lp_event_generator_get_default (void);

/**
 * lp_event_generator_get_base_yearly_event_chance:
 * @self: an #LpEventGenerator
 *
 * Gets the base probability of a yearly event occurring.
 *
 * Returns: Probability (0.0 to 1.0)
 */
gdouble
lp_event_generator_get_base_yearly_event_chance (LpEventGenerator *self);

/**
 * lp_event_generator_set_base_yearly_event_chance:
 * @self: an #LpEventGenerator
 * @chance: probability (0.0 to 1.0)
 *
 * Sets the base probability of yearly events.
 */
void
lp_event_generator_set_base_yearly_event_chance (LpEventGenerator *self,
                                                 gdouble           chance);

/**
 * lp_event_generator_get_base_decade_event_chance:
 * @self: an #LpEventGenerator
 *
 * Gets the base probability of a decade event occurring.
 *
 * Returns: Probability (0.0 to 1.0)
 */
gdouble
lp_event_generator_get_base_decade_event_chance (LpEventGenerator *self);

/**
 * lp_event_generator_set_base_decade_event_chance:
 * @self: an #LpEventGenerator
 * @chance: probability (0.0 to 1.0)
 *
 * Sets the base probability of decade events.
 */
void
lp_event_generator_set_base_decade_event_chance (LpEventGenerator *self,
                                                 gdouble           chance);

/**
 * lp_event_generator_get_base_era_event_chance:
 * @self: an #LpEventGenerator
 *
 * Gets the base probability of an era event occurring.
 *
 * Returns: Probability (0.0 to 1.0)
 */
gdouble
lp_event_generator_get_base_era_event_chance (LpEventGenerator *self);

/**
 * lp_event_generator_set_base_era_event_chance:
 * @self: an #LpEventGenerator
 * @chance: probability (0.0 to 1.0)
 *
 * Sets the base probability of era events.
 */
void
lp_event_generator_set_base_era_event_chance (LpEventGenerator *self,
                                              gdouble           chance);

/**
 * lp_event_generator_generate_yearly_events:
 * @self: an #LpEventGenerator
 * @sim: the #LpWorldSimulation for context
 *
 * Generates events for the current year based on world state.
 * Yearly events are typically minor or moderate in severity.
 *
 * Returns: (transfer full) (element-type LpEvent): List of generated events
 */
GList *
lp_event_generator_generate_yearly_events (LpEventGenerator  *self,
                                           LpWorldSimulation *sim);

/**
 * lp_event_generator_generate_decade_events:
 * @self: an #LpEventGenerator
 * @sim: the #LpWorldSimulation for context
 *
 * Generates events for the current decade.
 * Decade events are typically moderate or major in severity.
 *
 * Returns: (transfer full) (element-type LpEvent): List of generated events
 */
GList *
lp_event_generator_generate_decade_events (LpEventGenerator  *self,
                                           LpWorldSimulation *sim);

/**
 * lp_event_generator_generate_era_events:
 * @self: an #LpEventGenerator
 * @sim: the #LpWorldSimulation for context
 *
 * Generates events for the current era (century).
 * Era events are typically major or catastrophic.
 *
 * Returns: (transfer full) (element-type LpEvent): List of generated events
 */
GList *
lp_event_generator_generate_era_events (LpEventGenerator  *self,
                                        LpWorldSimulation *sim);

/**
 * lp_event_generator_create_economic_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random economic event of the given severity.
 *
 * Returns: (transfer full): A new economic event
 */
LpEvent *
lp_event_generator_create_economic_event (LpEventGenerator *self,
                                          LpEventSeverity   severity);

/**
 * lp_event_generator_create_political_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random political event of the given severity.
 *
 * Returns: (transfer full): A new political event
 */
LpEvent *
lp_event_generator_create_political_event (LpEventGenerator *self,
                                           LpEventSeverity   severity);

/**
 * lp_event_generator_create_magical_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random magical event of the given severity.
 *
 * Returns: (transfer full): A new magical event
 */
LpEvent *
lp_event_generator_create_magical_event (LpEventGenerator *self,
                                         LpEventSeverity   severity);

/**
 * lp_event_generator_create_personal_event:
 * @self: an #LpEventGenerator
 * @severity: the event severity
 *
 * Creates a random personal event of the given severity.
 *
 * Returns: (transfer full): A new personal event
 */
LpEvent *
lp_event_generator_create_personal_event (LpEventGenerator *self,
                                          LpEventSeverity   severity);

G_END_DECLS

#endif /* LP_EVENT_GENERATOR_H */
