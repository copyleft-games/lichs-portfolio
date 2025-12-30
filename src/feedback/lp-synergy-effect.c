/* lp-synergy-effect.c - Synergy Activation Effect
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-synergy-effect.h"
#include "../ui/lp-theme.h"

/* Animation constants */
#define ACTIVATION_DURATION 1.0f
#define COMPLETION_DURATION 0.5f
#define LINE_THICKNESS      2.0f
#define PULSE_RADIUS        6.0f

typedef enum
{
    SYNERGY_MODE_ACTIVATION,
    SYNERGY_MODE_COMPLETION
} SynergyMode;

struct _LpSynergyEffect
{
    LrgWidget parent_instance;

    SynergyMode mode;
    gfloat      source_x;
    gfloat      source_y;
    gfloat      target_x;
    gfloat      target_y;
    gfloat      elapsed;
    gfloat      duration;
    gfloat      progress;
    gfloat      line_alpha;
    gboolean    is_complete;
};

enum
{
    PROP_0,
    PROP_PROGRESS,
    PROP_IS_COMPLETE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpSynergyEffect, lp_synergy_effect, LRG_TYPE_WIDGET)

/*
 * ease_out_quad:
 * @t: progress value 0.0 to 1.0
 *
 * Quadratic ease-out function for smooth animation.
 */
static gfloat
ease_out_quad (gfloat t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static void
lp_synergy_effect_draw (LrgWidget *widget)
{
    LpSynergyEffect *self;
    const GrlColor *synergy_color;
    gfloat eased_progress;

    self = LP_SYNERGY_EFFECT (widget);

    if (self->is_complete || self->line_alpha <= 0.0f)
        return;

    synergy_color = lp_theme_get_synergy_color ();
    eased_progress = ease_out_quad (self->progress);

    if (self->mode == SYNERGY_MODE_ACTIVATION)
    {
        gfloat line_end_x, line_end_y;
        gfloat pulse_x, pulse_y;

        /*
         * Draw the line from source toward target, growing with progress.
         * At progress 0, line length is 0. At progress 1, full length.
         */
        line_end_x = self->source_x + (self->target_x - self->source_x) * eased_progress;
        line_end_y = self->source_y + (self->target_y - self->source_y) * eased_progress;

        /* Draw line with current alpha */
        {
            g_autoptr(GrlColor) line_color = grl_color_new (
                grl_color_get_r (synergy_color),
                grl_color_get_g (synergy_color),
                grl_color_get_b (synergy_color),
                (guint8)(self->line_alpha * 255.0f));
            g_autoptr(GrlVector2) start_pos = grl_vector2_new (self->source_x, self->source_y);
            g_autoptr(GrlVector2) end_pos = grl_vector2_new (line_end_x, line_end_y);

            grl_draw_line_ex (start_pos, end_pos, LINE_THICKNESS, line_color);
        }

        /*
         * Draw pulse dot traveling along the line.
         * The pulse is ahead of the line drawing progress.
         */
        if (eased_progress > 0.1f)
        {
            gfloat pulse_progress = MIN (1.0f, eased_progress + 0.1f);
            pulse_x = self->source_x + (self->target_x - self->source_x) * pulse_progress;
            pulse_y = self->source_y + (self->target_y - self->source_y) * pulse_progress;

            /* Bright pulse dot */
            {
                g_autoptr(GrlColor) pulse_color = grl_color_new (255, 255, 255,
                    (guint8)(self->line_alpha * 200.0f));
                g_autoptr(GrlVector2) pulse_pos = grl_vector2_new (pulse_x, pulse_y);

                grl_draw_circle_v (pulse_pos, PULSE_RADIUS, pulse_color);
            }
        }
    }
    else if (self->mode == SYNERGY_MODE_COMPLETION)
    {
        /*
         * Completion mode: expanding circle flash at center.
         * Radius grows, alpha fades.
         */
        gfloat radius = 10.0f + eased_progress * 40.0f;
        guint8 alpha = (guint8)((1.0f - eased_progress) * self->line_alpha * 255.0f);

        {
            g_autoptr(GrlColor) flash_color = grl_color_new (
                grl_color_get_r (synergy_color),
                grl_color_get_g (synergy_color),
                grl_color_get_b (synergy_color),
                alpha);
            g_autoptr(GrlVector2) center_pos = grl_vector2_new (self->source_x, self->source_y);

            /* Draw expanding ring */
            grl_draw_circle_lines_v (center_pos, radius, flash_color);
        }
    }

    LRG_WIDGET_CLASS (lp_synergy_effect_parent_class)->draw (widget);
}

static void
lp_synergy_effect_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    LpSynergyEffect *self = LP_SYNERGY_EFFECT (object);

    switch (prop_id)
    {
        case PROP_PROGRESS:
            g_value_set_float (value, self->progress);
            break;
        case PROP_IS_COMPLETE:
            g_value_set_boolean (value, self->is_complete);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_synergy_effect_class_init (LpSynergyEffectClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);

    object_class->get_property = lp_synergy_effect_get_property;

    widget_class->draw = lp_synergy_effect_draw;

    properties[PROP_PROGRESS] =
        g_param_spec_float ("progress", "Progress",
                            "Animation progress 0.0 to 1.0",
                            0.0f, 1.0f, 0.0f,
                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_IS_COMPLETE] =
        g_param_spec_boolean ("is-complete", "Is Complete",
                              "Whether the animation has finished",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_synergy_effect_init (LpSynergyEffect *self)
{
    self->mode = SYNERGY_MODE_ACTIVATION;
    self->source_x = 0.0f;
    self->source_y = 0.0f;
    self->target_x = 0.0f;
    self->target_y = 0.0f;
    self->elapsed = 0.0f;
    self->duration = ACTIVATION_DURATION;
    self->progress = 0.0f;
    self->line_alpha = 1.0f;
    self->is_complete = FALSE;
}

LpSynergyEffect *
lp_synergy_effect_new (void)
{
    return g_object_new (LP_TYPE_SYNERGY_EFFECT, NULL);
}

void
lp_synergy_effect_play_activation (LrgContainer *parent,
                                   gfloat        source_x,
                                   gfloat        source_y,
                                   gfloat        target_x,
                                   gfloat        target_y)
{
    LpSynergyEffect *effect;

    g_return_if_fail (LRG_IS_CONTAINER (parent));

    effect = lp_synergy_effect_new ();
    effect->mode = SYNERGY_MODE_ACTIVATION;
    effect->source_x = source_x;
    effect->source_y = source_y;
    effect->target_x = target_x;
    effect->target_y = target_y;
    effect->duration = ACTIVATION_DURATION;

    lrg_container_add_child (parent, LRG_WIDGET (effect));
}

void
lp_synergy_effect_play_completion (LrgContainer *parent,
                                   gfloat        center_x,
                                   gfloat        center_y)
{
    LpSynergyEffect *effect;

    g_return_if_fail (LRG_IS_CONTAINER (parent));

    effect = lp_synergy_effect_new ();
    effect->mode = SYNERGY_MODE_COMPLETION;
    effect->source_x = center_x;  /* Center point stored in source_x/y */
    effect->source_y = center_y;
    effect->duration = COMPLETION_DURATION;

    lrg_container_add_child (parent, LRG_WIDGET (effect));
}

void
lp_synergy_effect_set_endpoints (LpSynergyEffect *self,
                                 gfloat           source_x,
                                 gfloat           source_y,
                                 gfloat           target_x,
                                 gfloat           target_y)
{
    g_return_if_fail (LP_IS_SYNERGY_EFFECT (self));

    self->source_x = source_x;
    self->source_y = source_y;
    self->target_x = target_x;
    self->target_y = target_y;
}

gboolean
lp_synergy_effect_is_complete (LpSynergyEffect *self)
{
    g_return_val_if_fail (LP_IS_SYNERGY_EFFECT (self), TRUE);
    return self->is_complete;
}

gfloat
lp_synergy_effect_get_progress (LpSynergyEffect *self)
{
    g_return_val_if_fail (LP_IS_SYNERGY_EFFECT (self), 0.0f);
    return self->progress;
}

void
lp_synergy_effect_update (LpSynergyEffect *self,
                          gfloat           delta)
{
    g_return_if_fail (LP_IS_SYNERGY_EFFECT (self));

    if (self->is_complete)
        return;

    self->elapsed += delta;
    self->progress = self->elapsed / self->duration;

    if (self->progress >= 1.0f)
    {
        self->progress = 1.0f;
        self->is_complete = TRUE;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_COMPLETE]);
    }

    /*
     * Fade out the line during the last 30% of animation.
     */
    if (self->progress > 0.7f)
    {
        gfloat fade_progress = (self->progress - 0.7f) / 0.3f;
        self->line_alpha = 1.0f - fade_progress;
    }
    else
    {
        self->line_alpha = 1.0f;
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROGRESS]);
}
