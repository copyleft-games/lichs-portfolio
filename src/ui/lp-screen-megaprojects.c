/* lp-screen-megaprojects.c - Megaprojects Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-screen-megaprojects.h"
#include "lp-theme.h"

struct _LpScreenMegaprojects
{
    LrgContainer parent_instance;
};

G_DEFINE_TYPE (LpScreenMegaprojects, lp_screen_megaprojects, LRG_TYPE_CONTAINER)

static void
lp_screen_megaprojects_draw (LrgWidget *widget)
{
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

    grl_draw_text ("Megaprojects", x + padding, y + padding,
                   font_size_large, text_color);

    /* Draw placeholder content */
    grl_draw_text ("Multi-century projects - coming soon",
                   x + padding,
                   y + header_height + padding,
                   font_size, secondary_color);

    LRG_WIDGET_CLASS (lp_screen_megaprojects_parent_class)->draw (widget);
}

static void
lp_screen_megaprojects_layout_children (LrgContainer *container)
{
    /* No child widgets to layout yet */
    (void)container;
}

static void
lp_screen_megaprojects_class_init (LpScreenMegaprojectsClass *klass)
{
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    widget_class->draw = lp_screen_megaprojects_draw;
    container_class->layout_children = lp_screen_megaprojects_layout_children;
}

static void
lp_screen_megaprojects_init (LpScreenMegaprojects *self)
{
    (void)self;
}

LpScreenMegaprojects *
lp_screen_megaprojects_new (void)
{
    return g_object_new (LP_TYPE_SCREEN_MEGAPROJECTS, NULL);
}

void
lp_screen_megaprojects_refresh (LpScreenMegaprojects *self)
{
    g_return_if_fail (LP_IS_SCREEN_MEGAPROJECTS (self));
    /* Redraw will happen automatically via property notifications */
}
