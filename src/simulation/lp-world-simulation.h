/* lp-world-simulation.h - World State Simulation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The World Simulation manages the state of kingdoms, regions,
 * and generates world events during slumber periods.
 *
 * In Phase 1, this is a skeleton that tracks only the current year
 * and provides basic tick functionality.
 */

#ifndef LP_WORLD_SIMULATION_H
#define LP_WORLD_SIMULATION_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_WORLD_SIMULATION (lp_world_simulation_get_type ())

G_DECLARE_FINAL_TYPE (LpWorldSimulation, lp_world_simulation,
                      LP, WORLD_SIMULATION, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_world_simulation_new:
 *
 * Creates a new world simulation.
 *
 * Returns: (transfer full): A new #LpWorldSimulation
 */
LpWorldSimulation *
lp_world_simulation_new (void);

/* ==========================================================================
 * Simulation Control
 * ========================================================================== */

/**
 * lp_world_simulation_advance_year:
 * @self: an #LpWorldSimulation
 *
 * Advances the simulation by one year.
 * Processes all yearly events, economic changes, etc.
 *
 * Returns: (transfer full) (element-type LpEvent): List of events that occurred
 */
GList *
lp_world_simulation_advance_year (LpWorldSimulation *self);

/**
 * lp_world_simulation_advance_years:
 * @self: an #LpWorldSimulation
 * @years: number of years to advance
 *
 * Advances the simulation by multiple years.
 * Used during slumber periods.
 *
 * Returns: (transfer full) (element-type LpEvent): List of all events that occurred
 */
GList *
lp_world_simulation_advance_years (LpWorldSimulation *self,
                                   guint              years);

/**
 * lp_world_simulation_get_current_year:
 * @self: an #LpWorldSimulation
 *
 * Gets the current simulation year.
 *
 * Returns: The current year
 */
guint64
lp_world_simulation_get_current_year (LpWorldSimulation *self);

/**
 * lp_world_simulation_set_current_year:
 * @self: an #LpWorldSimulation
 * @year: the year to set
 *
 * Sets the current simulation year directly.
 * Used during game loading.
 */
void
lp_world_simulation_set_current_year (LpWorldSimulation *self,
                                      guint64            year);

/* ==========================================================================
 * Kingdom/Region Access (Skeleton - Phase 4+)
 * ========================================================================== */

/**
 * lp_world_simulation_get_kingdoms:
 * @self: an #LpWorldSimulation
 *
 * Gets all kingdoms in the world.
 *
 * Note: Skeleton implementation returns empty array in Phase 1.
 *
 * Returns: (transfer none) (element-type LpKingdom): Array of kingdoms
 */
GPtrArray *
lp_world_simulation_get_kingdoms (LpWorldSimulation *self);

/**
 * lp_world_simulation_get_kingdom_count:
 * @self: an #LpWorldSimulation
 *
 * Gets the number of kingdoms.
 *
 * Returns: Number of kingdoms
 */
guint
lp_world_simulation_get_kingdom_count (LpWorldSimulation *self);

/* ==========================================================================
 * Economic State (Skeleton - Phase 4+)
 * ========================================================================== */

/**
 * lp_world_simulation_get_economic_cycle_phase:
 * @self: an #LpWorldSimulation
 *
 * Gets the current economic cycle phase (0-3).
 * 0 = Expansion, 1 = Peak, 2 = Contraction, 3 = Trough
 *
 * Returns: Economic cycle phase
 */
guint
lp_world_simulation_get_economic_cycle_phase (LpWorldSimulation *self);

/**
 * lp_world_simulation_get_base_growth_rate:
 * @self: an #LpWorldSimulation
 *
 * Gets the current base economic growth rate.
 * Modified by cycle phase and events.
 *
 * Returns: Growth rate as multiplier (1.0 = neutral)
 */
gdouble
lp_world_simulation_get_base_growth_rate (LpWorldSimulation *self);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_world_simulation_reset:
 * @self: an #LpWorldSimulation
 * @starting_year: the year to start at
 *
 * Resets the world simulation to initial state.
 */
void
lp_world_simulation_reset (LpWorldSimulation *self,
                           guint64            starting_year);

G_END_DECLS

#endif /* LP_WORLD_SIMULATION_H */
