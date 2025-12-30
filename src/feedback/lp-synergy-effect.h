/* lp-synergy-effect.h - Synergy Activation Effect
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Visual feedback when synergies activate - lines connect linked investments.
 */

#ifndef LP_SYNERGY_EFFECT_H
#define LP_SYNERGY_EFFECT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SYNERGY_EFFECT (lp_synergy_effect_get_type ())
G_DECLARE_FINAL_TYPE (LpSynergyEffect, lp_synergy_effect, LP, SYNERGY_EFFECT, LrgWidget)

/**
 * lp_synergy_effect_new:
 *
 * Creates a new synergy effect widget.
 *
 * Returns: (transfer full): a new #LpSynergyEffect
 */
LpSynergyEffect *lp_synergy_effect_new (void);

/**
 * lp_synergy_effect_play_activation:
 * @parent: the container to add the effect to
 * @source_x: X coordinate of source investment
 * @source_y: Y coordinate of source investment
 * @target_x: X coordinate of target investment
 * @target_y: Y coordinate of target investment
 *
 * Plays a synergy activation animation - a line draws from source
 * to target with a pulse traveling along it.
 */
void lp_synergy_effect_play_activation (LrgContainer *parent,
                                        gfloat        source_x,
                                        gfloat        source_y,
                                        gfloat        target_x,
                                        gfloat        target_y);

/**
 * lp_synergy_effect_play_completion:
 * @parent: the container to add the effect to
 * @center_x: X coordinate of synergy center
 * @center_y: Y coordinate of synergy center
 *
 * Plays a synergy completion flash at the center point.
 */
void lp_synergy_effect_play_completion (LrgContainer *parent,
                                        gfloat        center_x,
                                        gfloat        center_y);

/**
 * lp_synergy_effect_set_endpoints:
 * @self: a #LpSynergyEffect
 * @source_x: X coordinate of source
 * @source_y: Y coordinate of source
 * @target_x: X coordinate of target
 * @target_y: Y coordinate of target
 *
 * Sets the line endpoints for the synergy effect.
 */
void lp_synergy_effect_set_endpoints (LpSynergyEffect *self,
                                      gfloat           source_x,
                                      gfloat           source_y,
                                      gfloat           target_x,
                                      gfloat           target_y);

/**
 * lp_synergy_effect_is_complete:
 * @self: a #LpSynergyEffect
 *
 * Checks if the synergy effect animation has finished.
 *
 * Returns: %TRUE if the animation is complete
 */
gboolean lp_synergy_effect_is_complete (LpSynergyEffect *self);

/**
 * lp_synergy_effect_get_progress:
 * @self: a #LpSynergyEffect
 *
 * Gets the current animation progress.
 *
 * Returns: progress value 0.0 to 1.0
 */
gfloat lp_synergy_effect_get_progress (LpSynergyEffect *self);

/**
 * lp_synergy_effect_update:
 * @self: a #LpSynergyEffect
 * @delta: time elapsed since last update in seconds
 *
 * Updates the synergy effect animation.
 */
void lp_synergy_effect_update (LpSynergyEffect *self,
                               gfloat           delta);

G_END_DECLS

#endif /* LP_SYNERGY_EFFECT_H */
