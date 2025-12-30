/* lp-slumber-visualization.h - Slumber Phase Visualization
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Makes slumber phase visually interesting with year counter and event timeline.
 */

#ifndef LP_SLUMBER_VISUALIZATION_H
#define LP_SLUMBER_VISUALIZATION_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SLUMBER_VISUALIZATION (lp_slumber_visualization_get_type ())
G_DECLARE_FINAL_TYPE (LpSlumberVisualization, lp_slumber_visualization, LP, SLUMBER_VISUALIZATION, LrgContainer)

/**
 * lp_slumber_visualization_new:
 *
 * Creates a new slumber visualization widget.
 *
 * Returns: (transfer full): a new #LpSlumberVisualization
 */
LpSlumberVisualization *lp_slumber_visualization_new (void);

/**
 * lp_slumber_visualization_start:
 * @self: a #LpSlumberVisualization
 * @start_year: the starting year
 * @end_year: the target wake year
 *
 * Starts the slumber visualization from start_year to end_year.
 */
void lp_slumber_visualization_start (LpSlumberVisualization *self,
                                     guint64                 start_year,
                                     guint64                 end_year);

/**
 * lp_slumber_visualization_stop:
 * @self: a #LpSlumberVisualization
 *
 * Stops the slumber visualization.
 */
void lp_slumber_visualization_stop (LpSlumberVisualization *self);

/**
 * lp_slumber_visualization_add_event:
 * @self: a #LpSlumberVisualization
 * @event: (transfer none): the event to add to the timeline
 *
 * Adds an event to the timeline display.
 */
void lp_slumber_visualization_add_event (LpSlumberVisualization *self,
                                         LpEvent                *event);

/**
 * lp_slumber_visualization_set_year:
 * @self: a #LpSlumberVisualization
 * @year: the current year to display
 *
 * Updates the displayed year counter.
 */
void lp_slumber_visualization_set_year (LpSlumberVisualization *self,
                                        guint64                 year);

/**
 * lp_slumber_visualization_get_current_year:
 * @self: a #LpSlumberVisualization
 *
 * Gets the currently displayed year.
 *
 * Returns: the current year
 */
guint64 lp_slumber_visualization_get_current_year (LpSlumberVisualization *self);

/**
 * lp_slumber_visualization_get_target_year:
 * @self: a #LpSlumberVisualization
 *
 * Gets the target wake year.
 *
 * Returns: the target year
 */
guint64 lp_slumber_visualization_get_target_year (LpSlumberVisualization *self);

/**
 * lp_slumber_visualization_accelerate:
 * @self: a #LpSlumberVisualization
 * @accelerate: %TRUE to enable acceleration
 *
 * Enables or disables fast-forward mode (5x speed).
 */
void lp_slumber_visualization_accelerate (LpSlumberVisualization *self,
                                          gboolean                accelerate);

/**
 * lp_slumber_visualization_is_accelerating:
 * @self: a #LpSlumberVisualization
 *
 * Checks if acceleration is currently active.
 *
 * Returns: %TRUE if accelerating
 */
gboolean lp_slumber_visualization_is_accelerating (LpSlumberVisualization *self);

/**
 * lp_slumber_visualization_get_simulation_speed:
 * @self: a #LpSlumberVisualization
 *
 * Gets the current simulation speed multiplier.
 *
 * Returns: speed multiplier (1.0 = normal, 5.0 = accelerated)
 */
gfloat lp_slumber_visualization_get_simulation_speed (LpSlumberVisualization *self);

/**
 * lp_slumber_visualization_is_active:
 * @self: a #LpSlumberVisualization
 *
 * Checks if the visualization is currently running.
 *
 * Returns: %TRUE if active
 */
gboolean lp_slumber_visualization_is_active (LpSlumberVisualization *self);

/**
 * lp_slumber_visualization_clear_events:
 * @self: a #LpSlumberVisualization
 *
 * Clears all events from the timeline.
 */
void lp_slumber_visualization_clear_events (LpSlumberVisualization *self);

/**
 * lp_slumber_visualization_update:
 * @self: a #LpSlumberVisualization
 * @delta: time elapsed since last update in seconds
 *
 * Updates the visualization animation.
 */
void lp_slumber_visualization_update (LpSlumberVisualization *self,
                                      gfloat                  delta);

G_END_DECLS

#endif /* LP_SLUMBER_VISUALIZATION_H */
