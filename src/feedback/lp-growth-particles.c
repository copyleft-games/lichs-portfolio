/* lp-growth-particles.c - Growth Particle Effects
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-growth-particles.h"

/* Gold color values (normalized 0.0-1.0) */
#define GOLD_R (0.79f)  /* 201/255 = 0.788 (from #c9a227) */
#define GOLD_G (0.64f)  /* 162/255 = 0.635 */
#define GOLD_B (0.15f)  /* 39/255 = 0.153 */

/* Particle counts by intensity */
#define PARTICLES_MINOR     10
#define PARTICLES_MODERATE  30
#define PARTICLES_MAJOR     75
#define PARTICLES_LEGENDARY 200

/* Max particles the system can handle */
#define MAX_PARTICLES 300

struct _LpGrowthParticles
{
    LrgWidget parent_instance;

    LrgParticleSystem  *particle_system;
    LrgParticleEmitter *emitter;
    LpGrowthIntensity   intensity;
};

enum
{
    PROP_0,
    PROP_INTENSITY,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpGrowthParticles, lp_growth_particles, LRG_TYPE_WIDGET)

/*
 * configure_emitter_for_intensity:
 * @emitter: the particle emitter to configure
 * @intensity: the growth intensity level
 *
 * Configures the emitter settings based on intensity level.
 */
static void
configure_emitter_for_intensity (LrgParticleEmitter *emitter,
                                 LpGrowthIntensity   intensity)
{
    gfloat size_min, size_max;
    gfloat lifetime_min, lifetime_max;
    gfloat speed_min, speed_max;
    gfloat spread_angle;

    switch (intensity)
    {
        case LP_GROWTH_INTENSITY_MINOR:
            size_min = 2.0f;
            size_max = 4.0f;
            lifetime_min = 0.5f;
            lifetime_max = 1.0f;
            speed_min = 30.0f;
            speed_max = 60.0f;
            spread_angle = G_PI / 4.0f;  /* 45 degrees */
            break;

        case LP_GROWTH_INTENSITY_MODERATE:
            size_min = 3.0f;
            size_max = 6.0f;
            lifetime_min = 1.0f;
            lifetime_max = 1.5f;
            speed_min = 50.0f;
            speed_max = 100.0f;
            spread_angle = G_PI / 3.0f;  /* 60 degrees */
            break;

        case LP_GROWTH_INTENSITY_MAJOR:
            size_min = 4.0f;
            size_max = 8.0f;
            lifetime_min = 1.5f;
            lifetime_max = 2.0f;
            speed_min = 75.0f;
            speed_max = 150.0f;
            spread_angle = G_PI / 2.0f;  /* 90 degrees */
            break;

        case LP_GROWTH_INTENSITY_LEGENDARY:
        default:
            size_min = 5.0f;
            size_max = 10.0f;
            lifetime_min = 2.0f;
            lifetime_max = 3.0f;
            speed_min = 100.0f;
            speed_max = 200.0f;
            spread_angle = G_PI;  /* 180 degrees - full hemisphere */
            break;
    }

    lrg_particle_emitter_set_initial_size (emitter, size_min, size_max);
    lrg_particle_emitter_set_initial_lifetime (emitter, lifetime_min, lifetime_max);
    lrg_particle_emitter_set_initial_speed (emitter, speed_min, speed_max);
    lrg_particle_emitter_set_spread_angle (emitter, spread_angle);

    /* Upward burst direction */
    lrg_particle_emitter_set_direction (emitter, 0.0f, -1.0f, 0.0f);

    /* Gold color that fades out */
    lrg_particle_emitter_set_start_color (emitter, GOLD_R, GOLD_G, GOLD_B, 1.0f);
    lrg_particle_emitter_set_end_color (emitter, GOLD_R, GOLD_G, GOLD_B, 0.0f);
}

/*
 * get_particle_count_for_intensity:
 * @intensity: the growth intensity level
 *
 * Returns the number of particles to emit for the given intensity.
 */
static guint
get_particle_count_for_intensity (LpGrowthIntensity intensity)
{
    switch (intensity)
    {
        case LP_GROWTH_INTENSITY_MINOR:
            return PARTICLES_MINOR;
        case LP_GROWTH_INTENSITY_MODERATE:
            return PARTICLES_MODERATE;
        case LP_GROWTH_INTENSITY_MAJOR:
            return PARTICLES_MAJOR;
        case LP_GROWTH_INTENSITY_LEGENDARY:
        default:
            return PARTICLES_LEGENDARY;
    }
}

static void
lp_growth_particles_draw (LrgWidget *widget)
{
    LpGrowthParticles *self;

    self = LP_GROWTH_PARTICLES (widget);

    if (self->particle_system != NULL)
        lrg_particle_system_draw (self->particle_system);

    LRG_WIDGET_CLASS (lp_growth_particles_parent_class)->draw (widget);
}

static void
lp_growth_particles_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    LpGrowthParticles *self = LP_GROWTH_PARTICLES (object);

    switch (prop_id)
    {
        case PROP_INTENSITY:
            g_value_set_enum (value, self->intensity);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_growth_particles_dispose (GObject *object)
{
    LpGrowthParticles *self = LP_GROWTH_PARTICLES (object);

    g_clear_object (&self->particle_system);
    g_clear_object (&self->emitter);

    G_OBJECT_CLASS (lp_growth_particles_parent_class)->dispose (object);
}

static void
lp_growth_particles_class_init (LpGrowthParticlesClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);

    object_class->get_property = lp_growth_particles_get_property;
    object_class->dispose = lp_growth_particles_dispose;

    widget_class->draw = lp_growth_particles_draw;

    properties[PROP_INTENSITY] =
        g_param_spec_enum ("intensity", "Intensity",
                           "The current growth intensity level",
                           LP_TYPE_GROWTH_INTENSITY,
                           LP_GROWTH_INTENSITY_MINOR,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_growth_particles_init (LpGrowthParticles *self)
{
    self->particle_system = lrg_particle_system_new (MAX_PARTICLES);
    self->emitter = lrg_particle_emitter_new ();
    self->intensity = LP_GROWTH_INTENSITY_MINOR;

    /* Configure emitter defaults */
    configure_emitter_for_intensity (self->emitter, self->intensity);

    /* Add emitter to system but disable continuous emission */
    lrg_particle_emitter_set_enabled (self->emitter, FALSE);
    lrg_particle_emitter_set_emission_rate (self->emitter, 0.0f);
    lrg_particle_system_add_emitter (self->particle_system, self->emitter);

    /* Don't loop - one-shot bursts */
    lrg_particle_system_set_loop (self->particle_system, FALSE);
    lrg_particle_system_play (self->particle_system);
}

LpGrowthParticles *
lp_growth_particles_new (void)
{
    return g_object_new (LP_TYPE_GROWTH_PARTICLES, NULL);
}

void
lp_growth_particles_spawn (LpGrowthParticles *self,
                           gfloat             x,
                           gfloat             y,
                           LpGrowthIntensity  intensity)
{
    guint count;

    g_return_if_fail (LP_IS_GROWTH_PARTICLES (self));

    self->intensity = intensity;

    /* Configure emitter for this intensity */
    configure_emitter_for_intensity (self->emitter, intensity);

    /* Get particle count for intensity */
    count = get_particle_count_for_intensity (intensity);

    /* Emit burst at position */
    lrg_particle_system_emit_at (self->particle_system, x, y, 0.0f, count);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INTENSITY]);
}

void
lp_growth_particles_update (LpGrowthParticles *self,
                            gfloat             delta)
{
    g_return_if_fail (LP_IS_GROWTH_PARTICLES (self));

    if (self->particle_system != NULL)
        lrg_particle_system_update (self->particle_system, delta);
}

gboolean
lp_growth_particles_is_alive (LpGrowthParticles *self)
{
    g_return_val_if_fail (LP_IS_GROWTH_PARTICLES (self), FALSE);

    if (self->particle_system == NULL)
        return FALSE;

    /* Only consider alive if there are active particles */
    return lrg_particle_system_get_active_count (self->particle_system) > 0;
}

void
lp_growth_particles_clear (LpGrowthParticles *self)
{
    g_return_if_fail (LP_IS_GROWTH_PARTICLES (self));

    if (self->particle_system != NULL)
        lrg_particle_system_clear (self->particle_system);
}

LpGrowthIntensity
lp_growth_particles_get_intensity (LpGrowthParticles *self)
{
    g_return_val_if_fail (LP_IS_GROWTH_PARTICLES (self), LP_GROWTH_INTENSITY_MINOR);
    return self->intensity;
}
