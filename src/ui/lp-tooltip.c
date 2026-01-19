/* lp-tooltip.c - Contextual Help Tooltip Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-tooltip.h"
#include <libregnum.h>
#include <graylib.h>

/* Default values */
#define DEFAULT_MAX_WIDTH   (300.0f)
#define DEFAULT_DELAY_MS    (500)
#define TOOLTIP_PADDING     (12.0f)
#define TOOLTIP_MARGIN      (8.0f)
#define LINE_SPACING        (4.0f)
#define CORNER_RADIUS       (6.0f)

struct _LpTooltip
{
    LrgWidget parent_instance;

    /* Content */
    gchar *title;
    gchar *text;
    gchar *hint;

    /* Display state */
    gboolean visible;
    gfloat   target_x;
    gfloat   target_y;
    LpTooltipPosition position;

    /* Appearance */
    gfloat max_width;
    guint  delay_ms;

    /* Internal labels */
    LrgLabel *label_title;
    LrgLabel *label_text;
    LrgLabel *label_hint;
};

G_DEFINE_TYPE (LpTooltip, lp_tooltip, LRG_TYPE_WIDGET)

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

static void
calculate_tooltip_size (LpTooltip *self,
                        gfloat    *out_width,
                        gfloat    *out_height)
{
    LrgTheme *theme;
    gfloat width = 0.0f;
    gfloat height = TOOLTIP_PADDING * 2;
    gfloat title_size, text_size, hint_size;
    gint text_width;
    gfloat line_height;

    theme = lrg_theme_get_default ();
    title_size = lrg_theme_get_font_size_large (theme);
    text_size = lrg_theme_get_font_size_normal (theme);
    hint_size = lrg_theme_get_font_size_small (theme);

    if (self->title != NULL && self->title[0] != '\0')
    {
        text_width = grl_measure_text (self->title, (gint)title_size);
        line_height = title_size * 1.2f;
        width = MAX (width, (gfloat)text_width);
        height += line_height + LINE_SPACING;
    }

    if (self->text != NULL && self->text[0] != '\0')
    {
        text_width = grl_measure_text (self->text, (gint)text_size);
        line_height = text_size * 1.2f;
        if (self->max_width > 0 && (gfloat)text_width > self->max_width - TOOLTIP_PADDING * 2)
        {
            /* Estimate wrapped height */
            gfloat available_width = self->max_width - TOOLTIP_PADDING * 2;
            gint lines = (gint)(((gfloat)text_width / available_width) + 1);
            text_width = (gint)available_width;
            line_height *= lines;
        }
        width = MAX (width, (gfloat)text_width);
        height += line_height + LINE_SPACING;
    }

    if (self->hint != NULL && self->hint[0] != '\0')
    {
        text_width = grl_measure_text (self->hint, (gint)hint_size);
        line_height = hint_size * 1.2f;
        width = MAX (width, (gfloat)text_width);
        height += line_height + LINE_SPACING;
    }

    width += TOOLTIP_PADDING * 2;

    /* Apply max width */
    if (self->max_width > 0 && width > self->max_width)
        width = self->max_width;

    *out_width = width;
    *out_height = height;
}

static void
position_tooltip (LpTooltip *self,
                  gfloat     target_x,
                  gfloat     target_y,
                  gfloat     tooltip_w,
                  gfloat     tooltip_h,
                  gfloat    *out_x,
                  gfloat    *out_y)
{
    LrgEngine *engine;
    LrgWindow *window;
    gint screen_w, screen_h;
    LpTooltipPosition pos;

    engine = lrg_engine_get_default ();
    window = lrg_engine_get_window (engine);

    /* Default fallback dimensions */
    screen_w = 1280;
    screen_h = 720;

    if (window != NULL)
    {
        screen_w = lrg_window_get_width (window);
        screen_h = lrg_window_get_height (window);
    }

    pos = self->position;

    /* Auto-position: prefer below, then above, then right, then left */
    if (pos == LP_TOOLTIP_POSITION_AUTO)
    {
        if (target_y + TOOLTIP_MARGIN + tooltip_h < screen_h)
            pos = LP_TOOLTIP_POSITION_BELOW;
        else if (target_y - TOOLTIP_MARGIN - tooltip_h > 0)
            pos = LP_TOOLTIP_POSITION_ABOVE;
        else if (target_x + TOOLTIP_MARGIN + tooltip_w < screen_w)
            pos = LP_TOOLTIP_POSITION_RIGHT;
        else
            pos = LP_TOOLTIP_POSITION_LEFT;
    }

    switch (pos)
    {
    case LP_TOOLTIP_POSITION_ABOVE:
        *out_x = target_x - tooltip_w / 2;
        *out_y = target_y - TOOLTIP_MARGIN - tooltip_h;
        break;
    case LP_TOOLTIP_POSITION_BELOW:
        *out_x = target_x - tooltip_w / 2;
        *out_y = target_y + TOOLTIP_MARGIN;
        break;
    case LP_TOOLTIP_POSITION_LEFT:
        *out_x = target_x - TOOLTIP_MARGIN - tooltip_w;
        *out_y = target_y - tooltip_h / 2;
        break;
    case LP_TOOLTIP_POSITION_RIGHT:
        *out_x = target_x + TOOLTIP_MARGIN;
        *out_y = target_y - tooltip_h / 2;
        break;
    default:
        *out_x = target_x;
        *out_y = target_y + TOOLTIP_MARGIN;
        break;
    }

    /* Clamp to screen bounds */
    *out_x = CLAMP (*out_x, TOOLTIP_MARGIN, screen_w - tooltip_w - TOOLTIP_MARGIN);
    *out_y = CLAMP (*out_y, TOOLTIP_MARGIN, screen_h - tooltip_h - TOOLTIP_MARGIN);
}

/* ==========================================================================
 * LrgWidget Virtual Methods
 * ========================================================================== */

static void
lp_tooltip_draw_impl (LrgWidget *widget)
{
    LpTooltip *self = LP_TOOLTIP (widget);
    LrgTheme *theme;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) border_color = NULL;
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) hint_color = NULL;
    gfloat tooltip_w, tooltip_h;
    gfloat tooltip_x, tooltip_y;
    gfloat content_x, content_y;
    gfloat title_size, text_size, hint_size;

    if (!self->visible)
        return;

    theme = lrg_theme_get_default ();
    title_size = lrg_theme_get_font_size_large (theme);
    text_size = lrg_theme_get_font_size_normal (theme);
    hint_size = lrg_theme_get_font_size_small (theme);

    /* Calculate size and position */
    calculate_tooltip_size (self, &tooltip_w, &tooltip_h);
    position_tooltip (self, self->target_x, self->target_y,
                      tooltip_w, tooltip_h, &tooltip_x, &tooltip_y);

    /* Colors - dark theme with purple accent */
    bg_color = grl_color_new (25, 25, 35, 245);
    border_color = grl_color_new (100, 80, 140, 255);
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    hint_color = grl_color_new (120, 120, 140, 255);

    /* Draw background with rounded corners */
    {
        GrlRectangle rect = { tooltip_x, tooltip_y, tooltip_w, tooltip_h };
        grl_draw_rectangle_rounded (&rect, CORNER_RADIUS, 8, bg_color);
        grl_draw_rectangle_rounded_lines_ex (&rect, CORNER_RADIUS, 8, 1.0f, border_color);
    }

    /* Draw content */
    content_x = tooltip_x + TOOLTIP_PADDING;
    content_y = tooltip_y + TOOLTIP_PADDING;

    if (self->title != NULL && self->title[0] != '\0')
    {
        lrg_label_set_text (self->label_title, self->title);
        lrg_widget_set_position (LRG_WIDGET (self->label_title), content_x, content_y);
        lrg_label_set_font_size (self->label_title, title_size);
        lrg_label_set_color (self->label_title, title_color);
        lrg_widget_draw (LRG_WIDGET (self->label_title));

        content_y += title_size * 1.2f + LINE_SPACING;
    }

    if (self->text != NULL && self->text[0] != '\0')
    {
        lrg_label_set_text (self->label_text, self->text);
        lrg_widget_set_position (LRG_WIDGET (self->label_text), content_x, content_y);
        lrg_label_set_font_size (self->label_text, text_size);
        lrg_label_set_color (self->label_text, text_color);
        lrg_widget_draw (LRG_WIDGET (self->label_text));

        content_y += text_size * 1.2f + LINE_SPACING;
    }

    if (self->hint != NULL && self->hint[0] != '\0')
    {
        lrg_label_set_text (self->label_hint, self->hint);
        lrg_widget_set_position (LRG_WIDGET (self->label_hint), content_x, content_y);
        lrg_label_set_font_size (self->label_hint, hint_size);
        lrg_label_set_color (self->label_hint, hint_color);
        lrg_widget_draw (LRG_WIDGET (self->label_hint));
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_tooltip_dispose (GObject *object)
{
    LpTooltip *self = LP_TOOLTIP (object);

    g_clear_object (&self->label_title);
    g_clear_object (&self->label_text);
    g_clear_object (&self->label_hint);

    G_OBJECT_CLASS (lp_tooltip_parent_class)->dispose (object);
}

static void
lp_tooltip_finalize (GObject *object)
{
    LpTooltip *self = LP_TOOLTIP (object);

    g_free (self->title);
    g_free (self->text);
    g_free (self->hint);

    G_OBJECT_CLASS (lp_tooltip_parent_class)->finalize (object);
}

static void
lp_tooltip_class_init (LpTooltipClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);

    object_class->dispose = lp_tooltip_dispose;
    object_class->finalize = lp_tooltip_finalize;

    widget_class->draw = lp_tooltip_draw_impl;
}

static void
lp_tooltip_init (LpTooltip *self)
{
    self->title = NULL;
    self->text = NULL;
    self->hint = NULL;
    self->visible = FALSE;
    self->target_x = 0.0f;
    self->target_y = 0.0f;
    self->position = LP_TOOLTIP_POSITION_AUTO;
    self->max_width = DEFAULT_MAX_WIDTH;
    self->delay_ms = DEFAULT_DELAY_MS;

    /* Create internal labels */
    self->label_title = lrg_label_new (NULL);
    self->label_text = lrg_label_new (NULL);
    self->label_hint = lrg_label_new (NULL);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

LpTooltip *
lp_tooltip_new (void)
{
    return g_object_new (LP_TYPE_TOOLTIP, NULL);
}

void
lp_tooltip_set_title (LpTooltip   *self,
                      const gchar *title)
{
    g_return_if_fail (LP_IS_TOOLTIP (self));

    g_free (self->title);
    self->title = g_strdup (title);
}

const gchar *
lp_tooltip_get_title (LpTooltip *self)
{
    g_return_val_if_fail (LP_IS_TOOLTIP (self), NULL);
    return self->title;
}

void
lp_tooltip_set_text (LpTooltip   *self,
                     const gchar *text)
{
    g_return_if_fail (LP_IS_TOOLTIP (self));

    g_free (self->text);
    self->text = g_strdup (text);
}

const gchar *
lp_tooltip_get_text (LpTooltip *self)
{
    g_return_val_if_fail (LP_IS_TOOLTIP (self), NULL);
    return self->text;
}

void
lp_tooltip_set_hint (LpTooltip   *self,
                     const gchar *hint)
{
    g_return_if_fail (LP_IS_TOOLTIP (self));

    g_free (self->hint);
    self->hint = g_strdup (hint);
}

const gchar *
lp_tooltip_get_hint (LpTooltip *self)
{
    g_return_val_if_fail (LP_IS_TOOLTIP (self), NULL);
    return self->hint;
}

void
lp_tooltip_show_at (LpTooltip          *self,
                    gfloat              x,
                    gfloat              y,
                    LpTooltipPosition   position)
{
    g_return_if_fail (LP_IS_TOOLTIP (self));

    self->target_x = x;
    self->target_y = y;
    self->position = position;
    self->visible = TRUE;
}

void
lp_tooltip_show_for_widget (LpTooltip          *self,
                            LrgWidget          *widget,
                            LpTooltipPosition   position)
{
    gfloat x, y, w, h;

    g_return_if_fail (LP_IS_TOOLTIP (self));
    g_return_if_fail (LRG_IS_WIDGET (widget));

    x = lrg_widget_get_x (widget);
    y = lrg_widget_get_y (widget);
    w = lrg_widget_get_width (widget);
    h = lrg_widget_get_height (widget);

    /* Center on widget */
    lp_tooltip_show_at (self, x + w / 2, y + h / 2, position);
}

void
lp_tooltip_hide (LpTooltip *self)
{
    g_return_if_fail (LP_IS_TOOLTIP (self));
    self->visible = FALSE;
}

gboolean
lp_tooltip_is_visible (LpTooltip *self)
{
    g_return_val_if_fail (LP_IS_TOOLTIP (self), FALSE);
    return self->visible;
}

void
lp_tooltip_set_max_width (LpTooltip *self,
                          gfloat     width)
{
    g_return_if_fail (LP_IS_TOOLTIP (self));
    self->max_width = width;
}

void
lp_tooltip_set_delay (LpTooltip *self,
                      guint      delay_ms)
{
    g_return_if_fail (LP_IS_TOOLTIP (self));
    self->delay_ms = delay_ms;
}
