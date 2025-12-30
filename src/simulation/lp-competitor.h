/* lp-competitor.h - Immortal Competitor
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Competitors represent rival immortals: dragons, vampires, liches,
 * fae lords, and demons. They expand territory, accumulate wealth,
 * and may ally or conflict with the player.
 */

#ifndef LP_COMPETITOR_H
#define LP_COMPETITOR_H

#include <glib-object.h>
#include "../lp-enums.h"
#include "lp-event.h"

G_BEGIN_DECLS

#define LP_TYPE_COMPETITOR (lp_competitor_get_type ())

G_DECLARE_FINAL_TYPE (LpCompetitor, lp_competitor,
                      LP, COMPETITOR, GObject)

/**
 * lp_competitor_new:
 * @id: unique identifier
 * @name: display name
 * @competitor_type: type of immortal
 *
 * Creates a new immortal competitor.
 *
 * Returns: (transfer full): A new #LpCompetitor
 */
LpCompetitor *
lp_competitor_new (const gchar      *id,
                   const gchar      *name,
                   LpCompetitorType  competitor_type);

/**
 * lp_competitor_get_id:
 * @self: an #LpCompetitor
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The competitor ID
 */
const gchar *
lp_competitor_get_id (LpCompetitor *self);

/**
 * lp_competitor_get_name:
 * @self: an #LpCompetitor
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The competitor name
 */
const gchar *
lp_competitor_get_name (LpCompetitor *self);

/**
 * lp_competitor_set_name:
 * @self: an #LpCompetitor
 * @name: the new name
 *
 * Sets the display name.
 */
void
lp_competitor_set_name (LpCompetitor *self,
                        const gchar  *name);

/**
 * lp_competitor_get_competitor_type:
 * @self: an #LpCompetitor
 *
 * Gets the type of immortal.
 *
 * Returns: The competitor type
 */
LpCompetitorType
lp_competitor_get_competitor_type (LpCompetitor *self);

/**
 * lp_competitor_get_stance:
 * @self: an #LpCompetitor
 *
 * Gets the attitude toward the player.
 *
 * Returns: The current stance
 */
LpCompetitorStance
lp_competitor_get_stance (LpCompetitor *self);

/**
 * lp_competitor_set_stance:
 * @self: an #LpCompetitor
 * @stance: the new stance
 *
 * Sets the attitude toward the player.
 */
void
lp_competitor_set_stance (LpCompetitor       *self,
                          LpCompetitorStance  stance);

/**
 * lp_competitor_get_power_level:
 * @self: an #LpCompetitor
 *
 * Gets the overall power level (0-100).
 *
 * Returns: Power level
 */
gint
lp_competitor_get_power_level (LpCompetitor *self);

/**
 * lp_competitor_set_power_level:
 * @self: an #LpCompetitor
 * @level: power level (0-100)
 *
 * Sets the power level.
 */
void
lp_competitor_set_power_level (LpCompetitor *self,
                               gint          level);

/**
 * lp_competitor_get_aggression:
 * @self: an #LpCompetitor
 *
 * Gets aggression trait (0-100).
 *
 * Returns: Aggression value
 */
gint
lp_competitor_get_aggression (LpCompetitor *self);

/**
 * lp_competitor_set_aggression:
 * @self: an #LpCompetitor
 * @aggression: aggression value (0-100)
 *
 * Sets the aggression trait.
 */
void
lp_competitor_set_aggression (LpCompetitor *self,
                              gint          aggression);

/**
 * lp_competitor_get_greed:
 * @self: an #LpCompetitor
 *
 * Gets greed trait (0-100).
 *
 * Returns: Greed value
 */
gint
lp_competitor_get_greed (LpCompetitor *self);

/**
 * lp_competitor_set_greed:
 * @self: an #LpCompetitor
 * @greed: greed value (0-100)
 *
 * Sets the greed trait.
 */
void
lp_competitor_set_greed (LpCompetitor *self,
                         gint          greed);

/**
 * lp_competitor_get_cunning:
 * @self: an #LpCompetitor
 *
 * Gets cunning trait (0-100).
 *
 * Returns: Cunning value
 */
gint
lp_competitor_get_cunning (LpCompetitor *self);

/**
 * lp_competitor_set_cunning:
 * @self: an #LpCompetitor
 * @cunning: cunning value (0-100)
 *
 * Sets the cunning trait.
 */
void
lp_competitor_set_cunning (LpCompetitor *self,
                           gint          cunning);

/**
 * lp_competitor_get_is_active:
 * @self: an #LpCompetitor
 *
 * Gets whether this competitor is actively participating.
 *
 * Returns: %TRUE if active
 */
gboolean
lp_competitor_get_is_active (LpCompetitor *self);

/**
 * lp_competitor_set_is_active:
 * @self: an #LpCompetitor
 * @active: whether active
 *
 * Sets whether the competitor is active.
 */
void
lp_competitor_set_is_active (LpCompetitor *self,
                             gboolean      active);

/**
 * lp_competitor_get_is_known:
 * @self: an #LpCompetitor
 *
 * Gets whether the player has discovered this competitor.
 *
 * Returns: %TRUE if known to player
 */
gboolean
lp_competitor_get_is_known (LpCompetitor *self);

/**
 * lp_competitor_set_is_known:
 * @self: an #LpCompetitor
 * @known: whether known
 *
 * Sets whether the player knows about this competitor.
 */
void
lp_competitor_set_is_known (LpCompetitor *self,
                            gboolean      known);

/**
 * lp_competitor_get_territory_region_ids:
 * @self: an #LpCompetitor
 *
 * Gets the IDs of regions controlled by this competitor.
 *
 * Returns: (transfer none) (element-type utf8): Array of region IDs
 */
GPtrArray *
lp_competitor_get_territory_region_ids (LpCompetitor *self);

/**
 * lp_competitor_add_territory:
 * @self: an #LpCompetitor
 * @region_id: the region to add
 *
 * Adds a region to this competitor's territory.
 */
void
lp_competitor_add_territory (LpCompetitor *self,
                             const gchar  *region_id);

/**
 * lp_competitor_remove_territory:
 * @self: an #LpCompetitor
 * @region_id: the region to remove
 *
 * Removes a region from this competitor's territory.
 *
 * Returns: %TRUE if removed
 */
gboolean
lp_competitor_remove_territory (LpCompetitor *self,
                                const gchar  *region_id);

/**
 * lp_competitor_has_territory:
 * @self: an #LpCompetitor
 * @region_id: the region to check
 *
 * Checks if the competitor controls a region.
 *
 * Returns: %TRUE if region is in territory
 */
gboolean
lp_competitor_has_territory (LpCompetitor *self,
                             const gchar  *region_id);

/**
 * lp_competitor_tick_year:
 * @self: an #LpCompetitor
 * @sim: the world simulation
 *
 * Advances the competitor by one year.
 * The AI makes decisions about expansion, wealth, and player relations.
 */
void
lp_competitor_tick_year (LpCompetitor      *self,
                         LpWorldSimulation *sim);

/**
 * lp_competitor_react_to_event:
 * @self: an #LpCompetitor
 * @event: the world event
 *
 * Responds to a world event.
 */
void
lp_competitor_react_to_event (LpCompetitor *self,
                              LpEvent      *event);

/**
 * lp_competitor_expand_territory:
 * @self: an #LpCompetitor
 * @sim: the world simulation
 *
 * Attempts to expand into new territory.
 *
 * Returns: %TRUE if expansion occurred
 */
gboolean
lp_competitor_expand_territory (LpCompetitor      *self,
                                LpWorldSimulation *sim);

/**
 * lp_competitor_get_player_threat_level:
 * @self: an #LpCompetitor
 *
 * Gets how threatening the player is perceived.
 *
 * Returns: Threat level (0-100)
 */
guint
lp_competitor_get_player_threat_level (LpCompetitor *self);

/**
 * lp_competitor_discover:
 * @self: an #LpCompetitor
 *
 * Marks this competitor as discovered by the player.
 * Emits the "discovered" signal.
 */
void
lp_competitor_discover (LpCompetitor *self);

/**
 * lp_competitor_destroy:
 * @self: an #LpCompetitor
 *
 * Destroys this competitor, removing them from the game.
 * Emits the "destroyed" signal.
 */
void
lp_competitor_destroy (LpCompetitor *self);

/**
 * lp_competitor_propose_alliance:
 * @self: an #LpCompetitor
 *
 * This competitor proposes an alliance with the player.
 * Emits the "alliance-proposed" signal.
 */
void
lp_competitor_propose_alliance (LpCompetitor *self);

/**
 * lp_competitor_declare_conflict:
 * @self: an #LpCompetitor
 *
 * This competitor declares conflict with the player.
 * Emits the "conflict-declared" signal.
 */
void
lp_competitor_declare_conflict (LpCompetitor *self);

G_END_DECLS

#endif /* LP_COMPETITOR_H */
