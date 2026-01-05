/* lp-screen-intelligence.c - Intelligence Reports Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-screen-intelligence.h"
#include "lp-theme.h"

struct _LpScreenIntelligence
{
    LrgContainer parent_instance;

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpScreenIntelligence, lp_screen_intelligence, LRG_TYPE_CONTAINER)

/* ==========================================================================
 * Label Helpers
 * ========================================================================== */

static void
draw_label (LrgLabel       *label,
            const gchar    *text,
            gfloat          x,
            gfloat          y,
            gfloat          font_size,
            const GrlColor *color)
{
    lrg_label_set_text (label, text);
    lrg_widget_set_position (LRG_WIDGET (label), x, y);
    lrg_label_set_font_size (label, font_size);
    lrg_label_set_color (label, color);
    lrg_widget_draw (LRG_WIDGET (label));
}

static LrgLabel *
get_pool_label (LpScreenIntelligence *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpScreenIntelligence *self)
{
    self->label_pool_index = 0;
}

static void
lp_screen_intelligence_draw (LrgWidget *widget)
{
    LpScreenIntelligence *self;
    LrgTheme *theme;
    gfloat x, y, width, height;
    gfloat padding;
    gfloat font_size_large;
    gfloat font_size;
    gfloat header_height;
    const GrlColor *bg_color;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *border_color;
    const GrlColor *surface_color;

    self = LP_SCREEN_INTELLIGENCE (widget);

    /* Reset label pool for this frame */
    reset_label_pool (self);

    theme = lrg_theme_get_default ();

    /* Get widget position and size */
    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget);
    width = lrg_widget_get_width (widget);
    height = lrg_widget_get_height (widget);

    padding = lrg_theme_get_padding_normal (theme);
    font_size_large = lrg_theme_get_font_size_large (theme);
    font_size = lrg_theme_get_font_size_normal (theme);
    header_height = font_size_large + padding * 3;

    bg_color = lrg_theme_get_background_color (theme);
    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    border_color = lrg_theme_get_border_color (theme);
    surface_color = lrg_theme_get_surface_color (theme);

    (void)height; /* Silence unused variable warning for now */

    /* Draw background */
    grl_draw_rectangle (x, y, width, height, bg_color);

    /* Draw header */
    grl_draw_rectangle (x, y, width, header_height, surface_color);
    grl_draw_line (x, y + header_height,
                   x + width, y + header_height, border_color);

    draw_label (get_pool_label (self), "Intelligence",
                x + padding, y + padding,
                font_size_large, text_color);

    /* Draw placeholder content */
    draw_label (get_pool_label (self), "Intelligence reports - coming soon",
                x + padding,
                y + header_height + padding,
                font_size, secondary_color);

    LRG_WIDGET_CLASS (lp_screen_intelligence_parent_class)->draw (widget);
}

static void
lp_screen_intelligence_layout_children (LrgContainer *container)
{
    /* No child widgets to layout yet */
    (void)container;
}

static void
lp_screen_intelligence_dispose (GObject *object)
{
    LpScreenIntelligence *self = LP_SCREEN_INTELLIGENCE (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_screen_intelligence_parent_class)->dispose (object);
}

static void
lp_screen_intelligence_class_init (LpScreenIntelligenceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->dispose = lp_screen_intelligence_dispose;

    widget_class->draw = lp_screen_intelligence_draw;
    container_class->layout_children = lp_screen_intelligence_layout_children;
}

static void
lp_screen_intelligence_init (LpScreenIntelligence *self)
{
    guint i;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 5; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

LpScreenIntelligence *
lp_screen_intelligence_new (void)
{
    return g_object_new (LP_TYPE_SCREEN_INTELLIGENCE, NULL);
}

void
lp_screen_intelligence_refresh (LpScreenIntelligence *self)
{
    g_return_if_fail (LP_IS_SCREEN_INTELLIGENCE (self));
    /* Redraw will happen automatically via property notifications */
}
