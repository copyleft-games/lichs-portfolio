/* lp-growth-particles.h - Growth Particle Effects
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Visual celebration of portfolio growth using particle effects.
 */

#ifndef LP_GROWTH_PARTICLES_H
#define LP_GROWTH_PARTICLES_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_GROWTH_PARTICLES (lp_growth_particles_get_type ())
G_DECLARE_FINAL_TYPE (LpGrowthParticles, lp_growth_particles, LP, GROWTH_PARTICLES, LrgWidget)

/**
 * lp_growth_particles_new:
 *
 * Creates a new growth particles widget.
 *
 * Returns: (transfer full): a new #LpGrowthParticles
 */
LpGrowthParticles *lp_growth_particles_new (void);

/**
 * lp_growth_particles_spawn:
 * @self: a #LpGrowthParticles
 * @x: X position to spawn particles
 * @y: Y position to spawn particles
 * @intensity: the growth intensity level
 *
 * Spawns a burst of particles at the given position.
 * The number and appearance of particles depends on intensity.
 */
void lp_growth_particles_spawn (LpGrowthParticles *self,
                                gfloat             x,
                                gfloat             y,
                                LpGrowthIntensity  intensity);

/**
 * lp_growth_particles_update:
 * @self: a #LpGrowthParticles
 * @delta: time elapsed since last update in seconds
 *
 * Updates the particle simulation.
 */
void lp_growth_particles_update (LpGrowthParticles *self,
                                 gfloat             delta);

/**
 * lp_growth_particles_is_alive:
 * @self: a #LpGrowthParticles
 *
 * Checks if the particle system has any active particles.
 *
 * Returns: %TRUE if particles are still active
 */
gboolean lp_growth_particles_is_alive (LpGrowthParticles *self);

/**
 * lp_growth_particles_clear:
 * @self: a #LpGrowthParticles
 *
 * Immediately kills all active particles.
 */
void lp_growth_particles_clear (LpGrowthParticles *self);

/**
 * lp_growth_particles_get_intensity:
 * @self: a #LpGrowthParticles
 *
 * Gets the current intensity level.
 *
 * Returns: the intensity level
 */
LpGrowthIntensity lp_growth_particles_get_intensity (LpGrowthParticles *self);

G_END_DECLS

#endif /* LP_GROWTH_PARTICLES_H */
