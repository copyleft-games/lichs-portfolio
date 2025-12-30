/* lp-world-simulation.h - World State Simulation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The World Simulation manages the state of kingdoms, regions,
 * competitors, and generates world events during slumber periods.
 *
 * Phase 4 implements full kingdom/region management, event generation,
 * and immortal competitor AI.
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
 * Kingdom Management
 * ========================================================================== */

/**
 * lp_world_simulation_get_kingdoms:
 * @self: an #LpWorldSimulation
 *
 * Gets all kingdoms in the world.
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

/**
 * lp_world_simulation_add_kingdom:
 * @self: an #LpWorldSimulation
 * @kingdom: (transfer full): the kingdom to add
 *
 * Adds a kingdom to the simulation.
 */
void
lp_world_simulation_add_kingdom (LpWorldSimulation *self,
                                 LpKingdom         *kingdom);

/**
 * lp_world_simulation_get_kingdom_by_id:
 * @self: an #LpWorldSimulation
 * @id: the kingdom ID
 *
 * Gets a kingdom by its ID.
 *
 * Returns: (transfer none) (nullable): The kingdom, or %NULL if not found
 */
LpKingdom *
lp_world_simulation_get_kingdom_by_id (LpWorldSimulation *self,
                                       const gchar       *id);

/**
 * lp_world_simulation_remove_kingdom:
 * @self: an #LpWorldSimulation
 * @id: the kingdom ID
 *
 * Removes a kingdom from the simulation.
 *
 * Returns: %TRUE if removed
 */
gboolean
lp_world_simulation_remove_kingdom (LpWorldSimulation *self,
                                    const gchar       *id);

/* ==========================================================================
 * Region Management
 * ========================================================================== */

/**
 * lp_world_simulation_get_regions:
 * @self: an #LpWorldSimulation
 *
 * Gets all regions in the world.
 *
 * Returns: (transfer none) (element-type LpRegion): Array of regions
 */
GPtrArray *
lp_world_simulation_get_regions (LpWorldSimulation *self);

/**
 * lp_world_simulation_get_region_count:
 * @self: an #LpWorldSimulation
 *
 * Gets the number of regions.
 *
 * Returns: Number of regions
 */
guint
lp_world_simulation_get_region_count (LpWorldSimulation *self);

/**
 * lp_world_simulation_add_region:
 * @self: an #LpWorldSimulation
 * @region: (transfer full): the region to add
 *
 * Adds a region to the simulation.
 */
void
lp_world_simulation_add_region (LpWorldSimulation *self,
                                LpRegion          *region);

/**
 * lp_world_simulation_get_region_by_id:
 * @self: an #LpWorldSimulation
 * @id: the region ID
 *
 * Gets a region by its ID.
 *
 * Returns: (transfer none) (nullable): The region, or %NULL if not found
 */
LpRegion *
lp_world_simulation_get_region_by_id (LpWorldSimulation *self,
                                      const gchar       *id);

/* ==========================================================================
 * Competitor Management
 * ========================================================================== */

/**
 * lp_world_simulation_get_competitors:
 * @self: an #LpWorldSimulation
 *
 * Gets all immortal competitors in the world.
 *
 * Returns: (transfer none) (element-type LpCompetitor): Array of competitors
 */
GPtrArray *
lp_world_simulation_get_competitors (LpWorldSimulation *self);

/**
 * lp_world_simulation_get_competitor_count:
 * @self: an #LpWorldSimulation
 *
 * Gets the number of competitors.
 *
 * Returns: Number of competitors
 */
guint
lp_world_simulation_get_competitor_count (LpWorldSimulation *self);

/**
 * lp_world_simulation_add_competitor:
 * @self: an #LpWorldSimulation
 * @competitor: (transfer full): the competitor to add
 *
 * Adds a competitor to the simulation.
 */
void
lp_world_simulation_add_competitor (LpWorldSimulation *self,
                                    LpCompetitor      *competitor);

/**
 * lp_world_simulation_get_competitor_by_id:
 * @self: an #LpWorldSimulation
 * @id: the competitor ID
 *
 * Gets a competitor by its ID.
 *
 * Returns: (transfer none) (nullable): The competitor, or %NULL if not found
 */
LpCompetitor *
lp_world_simulation_get_competitor_by_id (LpWorldSimulation *self,
                                          const gchar       *id);

/**
 * lp_world_simulation_get_known_competitors:
 * @self: an #LpWorldSimulation
 *
 * Gets competitors that have been discovered by the player.
 *
 * Returns: (transfer container) (element-type LpCompetitor): List of known competitors
 */
GList *
lp_world_simulation_get_known_competitors (LpWorldSimulation *self);

/* ==========================================================================
 * Event Management
 * ========================================================================== */

/**
 * lp_world_simulation_get_active_events:
 * @self: an #LpWorldSimulation
 *
 * Gets currently active (ongoing) events.
 *
 * Returns: (transfer none) (element-type LpEvent): Array of active events
 */
GPtrArray *
lp_world_simulation_get_active_events (LpWorldSimulation *self);

/**
 * lp_world_simulation_get_event_generator:
 * @self: an #LpWorldSimulation
 *
 * Gets the event generator.
 *
 * Returns: (transfer none): The event generator
 */
LpEventGenerator *
lp_world_simulation_get_event_generator (LpWorldSimulation *self);

/* ==========================================================================
 * Economic State
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
