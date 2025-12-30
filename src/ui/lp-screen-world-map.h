/* lp-screen-world-map.h - World Map Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Interactive map showing kingdoms, regions, and world state.
 * Displays stability, prosperity, and other kingdom attributes.
 */

#ifndef LP_SCREEN_WORLD_MAP_H
#define LP_SCREEN_WORLD_MAP_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SCREEN_WORLD_MAP (lp_screen_world_map_get_type ())

G_DECLARE_FINAL_TYPE (LpScreenWorldMap, lp_screen_world_map,
                      LP, SCREEN_WORLD_MAP, LrgContainer)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_screen_world_map_new:
 *
 * Creates a new world map screen.
 *
 * Returns: (transfer full): A new #LpScreenWorldMap
 */
LpScreenWorldMap * lp_screen_world_map_new (void);

/* ==========================================================================
 * World Binding
 * ========================================================================== */

/**
 * lp_screen_world_map_get_simulation:
 * @self: an #LpScreenWorldMap
 *
 * Gets the world simulation being displayed.
 *
 * Returns: (transfer none) (nullable): The #LpWorldSimulation, or %NULL
 */
LpWorldSimulation * lp_screen_world_map_get_simulation (LpScreenWorldMap *self);

/**
 * lp_screen_world_map_set_simulation:
 * @self: an #LpScreenWorldMap
 * @simulation: (nullable): the world simulation to display
 *
 * Sets the world simulation to display.
 */
void lp_screen_world_map_set_simulation (LpScreenWorldMap  *self,
                                          LpWorldSimulation *simulation);

/* ==========================================================================
 * Selection
 * ========================================================================== */

/**
 * lp_screen_world_map_get_selected_kingdom:
 * @self: an #LpScreenWorldMap
 *
 * Gets the currently selected kingdom.
 *
 * Returns: (transfer none) (nullable): The selected #LpKingdom, or %NULL
 */
LpKingdom * lp_screen_world_map_get_selected_kingdom (LpScreenWorldMap *self);

/**
 * lp_screen_world_map_select_kingdom:
 * @self: an #LpScreenWorldMap
 * @kingdom: (nullable): the kingdom to select
 *
 * Selects a kingdom on the map.
 */
void lp_screen_world_map_select_kingdom (LpScreenWorldMap *self,
                                          LpKingdom        *kingdom);

/* ==========================================================================
 * Signals
 * ========================================================================== */

/**
 * LpScreenWorldMap::kingdom-selected:
 * @self: the #LpScreenWorldMap
 * @kingdom: (nullable): the selected kingdom
 *
 * Emitted when a kingdom is selected.
 */

/**
 * LpScreenWorldMap::region-clicked:
 * @self: the #LpScreenWorldMap
 * @region: the clicked region
 *
 * Emitted when a region is clicked.
 */

G_END_DECLS

#endif /* LP_SCREEN_WORLD_MAP_H */
