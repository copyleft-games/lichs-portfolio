/* lp-floating-text.c - Floating Text Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-floating-text.h"
#include "../ui/lp-theme.h"

/* Default animation values */
#define DEFAULT_LIFETIME   2.0f
#define DEFAULT_VELOCITY_Y -50.0f
#define DEFAULT_FONT_SIZE  18.0f

struct _LpFloatingText
{
    LrgWidget parent_instance;

    gchar   *text;
    gfloat   start_x;
    gfloat   start_y;
    gfloat   current_y;
    gfloat   lifetime;
    gfloat   velocity_y;
    gfloat   elapsed;
    gfloat   alpha;
    gfloat   font_size;

    GrlColor *color;
};

enum
{
    PROP_0,
    PROP_TEXT,
    PROP_LIFETIME,
    PROP_VELOCITY_Y,
    PROP_ALPHA,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpFloatingText, lp_floating_text, LRG_TYPE_WIDGET)

static void
lp_floating_text_draw (LrgWidget *widget)
{
    LpFloatingText *self;
    gfloat x, y;

    self = LP_FLOATING_TEXT (widget);

    if (self->text == NULL || self->alpha <= 0.0f)
        return;

    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget) + (self->current_y - self->start_y);

    /* Apply alpha to color */
    if (self->color != NULL)
    {
        g_autoptr(GrlColor) draw_color = grl_color_new (
            grl_color_get_r (self->color),
            grl_color_get_g (self->color),
            grl_color_get_b (self->color),
            (guint8)(self->alpha * 255.0f));

        grl_draw_text (self->text, x, y, self->font_size, draw_color);
    }

    LRG_WIDGET_CLASS (lp_floating_text_parent_class)->draw (widget);
}

static void
lp_floating_text_measure (LrgWidget *widget,
                          gfloat    *preferred_width,
                          gfloat    *preferred_height)
{
    LpFloatingText *self = LP_FLOATING_TEXT (widget);
    gfloat text_width = 0.0f;

    if (self->text != NULL)
        text_width = grl_measure_text (self->text, self->font_size);

    if (preferred_width != NULL)
        *preferred_width = text_width;
    if (preferred_height != NULL)
        *preferred_height = self->font_size;
}

static void
lp_floating_text_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    LpFloatingText *self = LP_FLOATING_TEXT (object);

    switch (prop_id)
    {
        case PROP_TEXT:
            g_value_set_string (value, self->text);
            break;
        case PROP_LIFETIME:
            g_value_set_float (value, self->lifetime);
            break;
        case PROP_VELOCITY_Y:
            g_value_set_float (value, self->velocity_y);
            break;
        case PROP_ALPHA:
            g_value_set_float (value, self->alpha);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_floating_text_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    LpFloatingText *self = LP_FLOATING_TEXT (object);

    switch (prop_id)
    {
        case PROP_TEXT:
            lp_floating_text_set_text (self, g_value_get_string (value));
            break;
        case PROP_LIFETIME:
            lp_floating_text_set_lifetime (self, g_value_get_float (value));
            break;
        case PROP_VELOCITY_Y:
            lp_floating_text_set_velocity_y (self, g_value_get_float (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_floating_text_dispose (GObject *object)
{
    LpFloatingText *self = LP_FLOATING_TEXT (object);

    g_clear_pointer (&self->text, g_free);
    g_clear_pointer (&self->color, grl_color_free);

    G_OBJECT_CLASS (lp_floating_text_parent_class)->dispose (object);
}

static void
lp_floating_text_class_init (LpFloatingTextClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);

    object_class->get_property = lp_floating_text_get_property;
    object_class->set_property = lp_floating_text_set_property;
    object_class->dispose = lp_floating_text_dispose;

    widget_class->draw = lp_floating_text_draw;
    widget_class->measure = lp_floating_text_measure;

    properties[PROP_TEXT] =
        g_param_spec_string ("text", "Text",
                             "The text to display",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_LIFETIME] =
        g_param_spec_float ("lifetime", "Lifetime",
                            "Total lifetime in seconds",
                            0.0f, G_MAXFLOAT, DEFAULT_LIFETIME,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_VELOCITY_Y] =
        g_param_spec_float ("velocity-y", "Velocity Y",
                            "Vertical velocity in pixels per second",
                            -G_MAXFLOAT, G_MAXFLOAT, DEFAULT_VELOCITY_Y,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_ALPHA] =
        g_param_spec_float ("alpha", "Alpha",
                            "Current opacity 0.0-1.0",
                            0.0f, 1.0f, 1.0f,
                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_floating_text_init (LpFloatingText *self)
{
    self->text = NULL;
    self->start_x = 0.0f;
    self->start_y = 0.0f;
    self->current_y = 0.0f;
    self->lifetime = DEFAULT_LIFETIME;
    self->velocity_y = DEFAULT_VELOCITY_Y;
    self->elapsed = 0.0f;
    self->alpha = 1.0f;
    self->font_size = DEFAULT_FONT_SIZE;
    self->color = NULL;
}

LpFloatingText *
lp_floating_text_new (const gchar *text,
                      gfloat       x,
                      gfloat       y,
                      GrlColor    *color)
{
    LpFloatingText *self;

    self = g_object_new (LP_TYPE_FLOATING_TEXT, NULL);

    self->text = g_strdup (text);
    self->start_x = x;
    self->start_y = y;
    self->current_y = y;

    if (color != NULL)
        self->color = grl_color_copy (color);

    /* Set widget position */
    lrg_widget_set_x (LRG_WIDGET (self), x);
    lrg_widget_set_y (LRG_WIDGET (self), y);

    return self;
}

void
lp_floating_text_spawn_gold (LrgContainer *parent,
                             LrgBigNumber *amount,
                             gboolean      positive,
                             gfloat        x,
                             gfloat        y)
{
    g_autofree gchar *text = NULL;
    g_autofree gchar *formatted = NULL;
    const GrlColor *color;
    LpFloatingText *floating;

    g_return_if_fail (LRG_IS_CONTAINER (parent));
    g_return_if_fail (amount != NULL);

    /* Format the amount */
    formatted = lrg_big_number_format_short (amount);

    if (positive)
    {
        text = g_strdup_printf ("+%s gp", formatted);
        color = lp_theme_get_gold_color ();
    }
    else
    {
        text = g_strdup_printf ("-%s gp", formatted);
        color = lp_theme_get_danger_color ();
    }

    floating = lp_floating_text_new (text, x, y, (GrlColor *)color);

    /* Add to parent container */
    lrg_container_add_child (parent, LRG_WIDGET (floating));
}

const gchar *
lp_floating_text_get_text (LpFloatingText *self)
{
    g_return_val_if_fail (LP_IS_FLOATING_TEXT (self), NULL);
    return self->text;
}

void
lp_floating_text_set_text (LpFloatingText *self,
                           const gchar    *text)
{
    g_return_if_fail (LP_IS_FLOATING_TEXT (self));

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

gfloat
lp_floating_text_get_lifetime (LpFloatingText *self)
{
    g_return_val_if_fail (LP_IS_FLOATING_TEXT (self), DEFAULT_LIFETIME);
    return self->lifetime;
}

void
lp_floating_text_set_lifetime (LpFloatingText *self,
                               gfloat          lifetime)
{
    g_return_if_fail (LP_IS_FLOATING_TEXT (self));

    if (self->lifetime != lifetime)
    {
        self->lifetime = lifetime;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LIFETIME]);
    }
}

gfloat
lp_floating_text_get_velocity_y (LpFloatingText *self)
{
    g_return_val_if_fail (LP_IS_FLOATING_TEXT (self), DEFAULT_VELOCITY_Y);
    return self->velocity_y;
}

void
lp_floating_text_set_velocity_y (LpFloatingText *self,
                                 gfloat          velocity_y)
{
    g_return_if_fail (LP_IS_FLOATING_TEXT (self));

    if (self->velocity_y != velocity_y)
    {
        self->velocity_y = velocity_y;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VELOCITY_Y]);
    }
}

gfloat
lp_floating_text_get_alpha (LpFloatingText *self)
{
    g_return_val_if_fail (LP_IS_FLOATING_TEXT (self), 0.0f);
    return self->alpha;
}

gboolean
lp_floating_text_is_finished (LpFloatingText *self)
{
    g_return_val_if_fail (LP_IS_FLOATING_TEXT (self), TRUE);
    return self->elapsed >= self->lifetime;
}

void
lp_floating_text_update (LpFloatingText *self,
                         gfloat          delta)
{
    gfloat progress;

    g_return_if_fail (LP_IS_FLOATING_TEXT (self));

    if (lp_floating_text_is_finished (self))
        return;

    self->elapsed += delta;

    /* Update position */
    self->current_y += self->velocity_y * delta;

    /* Calculate alpha fade (start fading after 50% of lifetime) */
    progress = self->elapsed / self->lifetime;
    if (progress > 0.5f)
    {
        /* Fade from 1.0 to 0.0 over remaining 50% */
        gfloat fade_progress = (progress - 0.5f) / 0.5f;
        self->alpha = 1.0f - fade_progress;
    }
    else
    {
        self->alpha = 1.0f;
    }

    /* Clamp alpha */
    if (self->alpha < 0.0f)
        self->alpha = 0.0f;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ALPHA]);
}
