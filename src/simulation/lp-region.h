/* lp-region.h - Geographic Region
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Regions are geographic areas that make up kingdoms.
 * They have specific geography types that affect resources and trade.
 */

#ifndef LP_REGION_H
#define LP_REGION_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_REGION (lp_region_get_type ())

G_DECLARE_FINAL_TYPE (LpRegion, lp_region, LP, REGION, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_region_new:
 * @id: unique identifier
 * @name: display name
 * @geography_type: the geography type
 *
 * Creates a new region with default values.
 *
 * Returns: (transfer full): A new #LpRegion
 */
LpRegion *
lp_region_new (const gchar    *id,
               const gchar    *name,
               LpGeographyType geography_type);

/**
 * lp_region_new_full:
 * @id: unique identifier
 * @name: display name
 * @geography_type: the geography type
 * @population: initial population
 * @resource_modifier: base resource productivity
 *
 * Creates a new region with specified values.
 *
 * Returns: (transfer full): A new #LpRegion
 */
LpRegion *
lp_region_new_full (const gchar    *id,
                    const gchar    *name,
                    LpGeographyType geography_type,
                    guint           population,
                    gdouble         resource_modifier);

/* ==========================================================================
 * Property Accessors
 * ========================================================================== */

/**
 * lp_region_get_id:
 * @self: an #LpRegion
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The region ID
 */
const gchar *
lp_region_get_id (LpRegion *self);

/**
 * lp_region_get_name:
 * @self: an #LpRegion
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The region name
 */
const gchar *
lp_region_get_name (LpRegion *self);

/**
 * lp_region_set_name:
 * @self: an #LpRegion
 * @name: the new name
 *
 * Sets the display name.
 */
void
lp_region_set_name (LpRegion    *self,
                    const gchar *name);

/**
 * lp_region_get_geography_type:
 * @self: an #LpRegion
 *
 * Gets the geography type.
 *
 * Returns: The geography type
 */
LpGeographyType
lp_region_get_geography_type (LpRegion *self);

/**
 * lp_region_get_owning_kingdom_id:
 * @self: an #LpRegion
 *
 * Gets the ID of the kingdom that owns this region.
 *
 * Returns: (transfer none) (nullable): The owning kingdom ID, or %NULL
 */
const gchar *
lp_region_get_owning_kingdom_id (LpRegion *self);

/**
 * lp_region_set_owning_kingdom_id:
 * @self: an #LpRegion
 * @kingdom_id: (nullable): the new owning kingdom ID
 *
 * Sets the owning kingdom. Emits ::ownership-changed signal.
 */
void
lp_region_set_owning_kingdom_id (LpRegion    *self,
                                 const gchar *kingdom_id);

/**
 * lp_region_get_population:
 * @self: an #LpRegion
 *
 * Gets the population.
 *
 * Returns: The population count
 */
guint
lp_region_get_population (LpRegion *self);

/**
 * lp_region_set_population:
 * @self: an #LpRegion
 * @population: the new population
 *
 * Sets the population.
 */
void
lp_region_set_population (LpRegion *self,
                          guint     population);

/**
 * lp_region_get_resource_modifier:
 * @self: an #LpRegion
 *
 * Gets the resource productivity modifier.
 *
 * Returns: The resource modifier (1.0 = baseline)
 */
gdouble
lp_region_get_resource_modifier (LpRegion *self);

/**
 * lp_region_set_resource_modifier:
 * @self: an #LpRegion
 * @modifier: the resource modifier
 *
 * Sets the resource productivity modifier.
 */
void
lp_region_set_resource_modifier (LpRegion *self,
                                 gdouble   modifier);

/**
 * lp_region_get_trade_connected:
 * @self: an #LpRegion
 *
 * Gets whether the region has trade connections.
 *
 * Returns: %TRUE if trade connected
 */
gboolean
lp_region_get_trade_connected (LpRegion *self);

/**
 * lp_region_set_trade_connected:
 * @self: an #LpRegion
 * @connected: whether trade connected
 *
 * Sets the trade connection status.
 */
void
lp_region_set_trade_connected (LpRegion *self,
                               gboolean  connected);

/* ==========================================================================
 * Trade Routes
 * ========================================================================== */

/**
 * lp_region_get_trade_route_ids:
 * @self: an #LpRegion
 *
 * Gets the IDs of regions connected via trade routes.
 *
 * Returns: (transfer none) (element-type utf8): Array of region IDs
 */
GPtrArray *
lp_region_get_trade_route_ids (LpRegion *self);

/**
 * lp_region_add_trade_route:
 * @self: an #LpRegion
 * @region_id: ID of the connected region
 *
 * Adds a trade route to another region.
 */
void
lp_region_add_trade_route (LpRegion    *self,
                           const gchar *region_id);

/**
 * lp_region_remove_trade_route:
 * @self: an #LpRegion
 * @region_id: ID of the region to disconnect
 *
 * Removes a trade route to another region.
 *
 * Returns: %TRUE if the route was removed
 */
gboolean
lp_region_remove_trade_route (LpRegion    *self,
                              const gchar *region_id);

/**
 * lp_region_has_trade_route_to:
 * @self: an #LpRegion
 * @region_id: ID of the region to check
 *
 * Checks if there is a trade route to the specified region.
 *
 * Returns: %TRUE if a trade route exists
 */
gboolean
lp_region_has_trade_route_to (LpRegion    *self,
                              const gchar *region_id);

/* ==========================================================================
 * Geography Bonuses
 * ========================================================================== */

/**
 * lp_region_get_geography_trade_bonus:
 * @self: an #LpRegion
 *
 * Gets the trade bonus from geography (coastal gets bonus).
 *
 * Returns: Trade bonus multiplier (1.0 = no bonus)
 */
gdouble
lp_region_get_geography_trade_bonus (LpRegion *self);

/**
 * lp_region_get_geography_resource_bonus:
 * @self: an #LpRegion
 *
 * Gets the resource bonus from geography.
 *
 * Returns: Resource bonus multiplier (1.0 = no bonus)
 */
gdouble
lp_region_get_geography_resource_bonus (LpRegion *self);

/**
 * lp_region_get_geography_concealment_bonus:
 * @self: an #LpRegion
 *
 * Gets the concealment bonus from geography (swamp/forest gets bonus).
 *
 * Returns: Concealment bonus multiplier (1.0 = no bonus)
 */
gdouble
lp_region_get_geography_concealment_bonus (LpRegion *self);

/* ==========================================================================
 * Events
 * ========================================================================== */

/**
 * lp_region_devastate:
 * @self: an #LpRegion
 * @severity: how severe the devastation (0.0-1.0)
 *
 * Devastates the region, reducing population and resources.
 * Emits ::devastated signal if severe.
 */
void
lp_region_devastate (LpRegion *self,
                     gdouble   severity);

G_END_DECLS

#endif /* LP_REGION_H */
