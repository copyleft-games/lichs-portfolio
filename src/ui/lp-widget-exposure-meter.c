/* lp-widget-exposure-meter.c - Exposure Tracking Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-widget-exposure-meter.h"
#include "lp-theme.h"
#include "../lp-log.h"

/* ==========================================================================
 * Type Definition
 * ========================================================================== */

struct _LpWidgetExposureMeter
{
    LrgWidget       parent_instance;

    guint           value;
    LpExposureLevel level;
    gboolean        show_label;
    gboolean        show_percentage;
    LrgOrientation  orientation;

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpWidgetExposureMeter, lp_widget_exposure_meter, LRG_TYPE_WIDGET)

/* ==========================================================================
 * Properties
 * ========================================================================== */

enum
{
    PROP_0,
    PROP_VALUE,
    PROP_LEVEL,
    PROP_SHOW_LABEL,
    PROP_SHOW_PERCENTAGE,
    PROP_ORIENTATION,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* ==========================================================================
 * Signals
 * ========================================================================== */

enum
{
    SIGNAL_LEVEL_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/* ==========================================================================
 * Helper: Calculate Level from Value
 * ========================================================================== */

static LpExposureLevel
calculate_level_from_value (guint value)
{
    if (value >= 100)
        return LP_EXPOSURE_LEVEL_CRUSADE;
    else if (value >= 75)
        return LP_EXPOSURE_LEVEL_HUNT;
    else if (value >= 50)
        return LP_EXPOSURE_LEVEL_SUSPICION;
    else if (value >= 25)
        return LP_EXPOSURE_LEVEL_SCRUTINY;
    else
        return LP_EXPOSURE_LEVEL_HIDDEN;
}

/* ==========================================================================
 * Helper: Get Color for Level
 * ========================================================================== */

static const GrlColor *
get_color_for_level (LpExposureLevel level)
{
    switch (level)
    {
        case LP_EXPOSURE_LEVEL_HIDDEN:
            return lp_theme_get_hidden_color ();
        case LP_EXPOSURE_LEVEL_SCRUTINY:
            return lp_theme_get_scrutiny_color ();
        case LP_EXPOSURE_LEVEL_SUSPICION:
            return lp_theme_get_suspicion_color ();
        case LP_EXPOSURE_LEVEL_HUNT:
            return lp_theme_get_hunt_color ();
        case LP_EXPOSURE_LEVEL_CRUSADE:
            return lp_theme_get_crusade_color ();
        default:
            return lp_theme_get_inactive_color ();
    }
}

/* ==========================================================================
 * Helper: Get Label for Level
 * ========================================================================== */

static const gchar *
get_label_for_level (LpExposureLevel level)
{
    switch (level)
    {
        case LP_EXPOSURE_LEVEL_HIDDEN:
            return "Hidden";
        case LP_EXPOSURE_LEVEL_SCRUTINY:
            return "Scrutiny";
        case LP_EXPOSURE_LEVEL_SUSPICION:
            return "Suspicion";
        case LP_EXPOSURE_LEVEL_HUNT:
            return "Hunt";
        case LP_EXPOSURE_LEVEL_CRUSADE:
            return "Crusade!";
        default:
            return "Unknown";
    }
}

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
get_pool_label (LpWidgetExposureMeter *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpWidgetExposureMeter *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Virtual Method: draw
 * ========================================================================== */

static void
lp_widget_exposure_meter_draw (LrgWidget *widget)
{
    LpWidgetExposureMeter *self;
    LrgTheme *theme;
    GrlRectangle bar_rect;
    GrlRectangle fill_rect;
    gfloat x;
    gfloat y;
    gfloat w;
    gfloat h;
    gfloat fill_amount;
    const GrlColor *bg_color;
    const GrlColor *fill_color;
    const GrlColor *text_color;
    const GrlColor *border_color;

    self = LP_WIDGET_EXPOSURE_METER (widget);

    /* Reset label pool for this frame */
    reset_label_pool (self);

    theme = lrg_theme_get_default ();

    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget);
    w = lrg_widget_get_width (widget);
    h = lrg_widget_get_height (widget);

    /* Calculate fill amount (0.0 to 1.0) */
    fill_amount = (gfloat) self->value / 100.0f;

    /* Get colors */
    bg_color = lrg_theme_get_surface_color (theme);
    fill_color = get_color_for_level (self->level);
    text_color = lrg_theme_get_text_color (theme);
    border_color = lrg_theme_get_border_color (theme);

    /* Calculate bar dimensions */
    if (self->orientation == LRG_ORIENTATION_HORIZONTAL)
    {
        bar_rect.x = x;
        bar_rect.y = y + (self->show_label ? 20.0f : 0.0f);
        bar_rect.width = w;
        bar_rect.height = h - (self->show_label ? 20.0f : 0.0f) -
                          (self->show_percentage ? 16.0f : 0.0f);

        fill_rect.x = bar_rect.x + 2.0f;
        fill_rect.y = bar_rect.y + 2.0f;
        fill_rect.width = (bar_rect.width - 4.0f) * fill_amount;
        fill_rect.height = bar_rect.height - 4.0f;
    }
    else
    {
        bar_rect.x = x + (self->show_label ? 80.0f : 0.0f);
        bar_rect.y = y;
        bar_rect.width = w - (self->show_label ? 80.0f : 0.0f);
        bar_rect.height = h;

        fill_rect.x = bar_rect.x + 2.0f;
        fill_rect.y = bar_rect.y + bar_rect.height - 2.0f -
                      (bar_rect.height - 4.0f) * fill_amount;
        fill_rect.width = bar_rect.width - 4.0f;
        fill_rect.height = (bar_rect.height - 4.0f) * fill_amount;
    }

    /* Draw background */
    grl_draw_rectangle_rec (&bar_rect, bg_color);

    /* Draw fill */
    if (fill_amount > 0.0f)
        grl_draw_rectangle_rec (&fill_rect, fill_color);

    /* Draw border */
    grl_draw_rectangle_lines_ex (&bar_rect, 1.0f, border_color);

    /* Draw threshold markers */
    if (self->orientation == LRG_ORIENTATION_HORIZONTAL)
    {
        gfloat marker_y = bar_rect.y;
        gfloat marker_h = bar_rect.height;
        g_autoptr(GrlColor) marker_color = grl_color_new (0x50, 0x50, 0x50, 0xff);

        /* 25% marker */
        grl_draw_line (bar_rect.x + bar_rect.width * 0.25f, marker_y,
                       bar_rect.x + bar_rect.width * 0.25f, marker_y + marker_h,
                       marker_color);
        /* 50% marker */
        grl_draw_line (bar_rect.x + bar_rect.width * 0.50f, marker_y,
                       bar_rect.x + bar_rect.width * 0.50f, marker_y + marker_h,
                       marker_color);
        /* 75% marker */
        grl_draw_line (bar_rect.x + bar_rect.width * 0.75f, marker_y,
                       bar_rect.x + bar_rect.width * 0.75f, marker_y + marker_h,
                       marker_color);
    }

    /* Draw label if enabled */
    if (self->show_label)
    {
        const gchar *label_text;
        gfloat text_x;
        gfloat text_y;

        label_text = get_label_for_level (self->level);

        if (self->orientation == LRG_ORIENTATION_HORIZONTAL)
        {
            text_x = x;
            text_y = y;
        }
        else
        {
            text_x = x;
            text_y = y + h / 2.0f - 8.0f;
        }

        draw_label (get_pool_label (self), label_text, text_x, text_y,
                    lrg_theme_get_font_size_normal (theme), text_color);
    }

    /* Draw percentage if enabled */
    if (self->show_percentage)
    {
        gchar percent_str[8];
        gfloat percent_x;
        gfloat percent_y;

        g_snprintf (percent_str, sizeof (percent_str), "%u%%", self->value);

        if (self->orientation == LRG_ORIENTATION_HORIZONTAL)
        {
            percent_x = x + w / 2.0f - 16.0f;
            percent_y = bar_rect.y + bar_rect.height + 2.0f;
        }
        else
        {
            percent_x = bar_rect.x + bar_rect.width + 4.0f;
            percent_y = y + h / 2.0f - 8.0f;
        }

        draw_label (get_pool_label (self), percent_str, percent_x, percent_y,
                    lrg_theme_get_font_size_small (theme), text_color);
    }
}

/* ==========================================================================
 * Virtual Method: measure
 * ========================================================================== */

static void
lp_widget_exposure_meter_measure (LrgWidget *widget,
                                   gfloat    *preferred_width,
                                   gfloat    *preferred_height)
{
    LpWidgetExposureMeter *self = LP_WIDGET_EXPOSURE_METER (widget);

    if (self->orientation == LRG_ORIENTATION_HORIZONTAL)
    {
        *preferred_width = 200.0f;
        *preferred_height = 40.0f;
        if (self->show_label)
            *preferred_height += 20.0f;
        if (self->show_percentage)
            *preferred_height += 16.0f;
    }
    else
    {
        *preferred_width = 40.0f;
        *preferred_height = 200.0f;
        if (self->show_label)
            *preferred_width += 80.0f;
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_widget_exposure_meter_dispose (GObject *object)
{
    LpWidgetExposureMeter *self = LP_WIDGET_EXPOSURE_METER (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_widget_exposure_meter_parent_class)->dispose (object);
}

static void
lp_widget_exposure_meter_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
    LpWidgetExposureMeter *self = LP_WIDGET_EXPOSURE_METER (object);

    switch (prop_id)
    {
        case PROP_VALUE:
            g_value_set_uint (value, self->value);
            break;
        case PROP_LEVEL:
            g_value_set_enum (value, self->level);
            break;
        case PROP_SHOW_LABEL:
            g_value_set_boolean (value, self->show_label);
            break;
        case PROP_SHOW_PERCENTAGE:
            g_value_set_boolean (value, self->show_percentage);
            break;
        case PROP_ORIENTATION:
            g_value_set_enum (value, self->orientation);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_widget_exposure_meter_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
    LpWidgetExposureMeter *self = LP_WIDGET_EXPOSURE_METER (object);

    switch (prop_id)
    {
        case PROP_VALUE:
            lp_widget_exposure_meter_set_value (self, g_value_get_uint (value));
            break;
        case PROP_SHOW_LABEL:
            lp_widget_exposure_meter_set_show_label (self, g_value_get_boolean (value));
            break;
        case PROP_SHOW_PERCENTAGE:
            lp_widget_exposure_meter_set_show_percentage (self, g_value_get_boolean (value));
            break;
        case PROP_ORIENTATION:
            lp_widget_exposure_meter_set_orientation (self, g_value_get_enum (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_widget_exposure_meter_init (LpWidgetExposureMeter *self)
{
    guint i;

    self->value = 0;
    self->level = LP_EXPOSURE_LEVEL_HIDDEN;
    self->show_label = TRUE;
    self->show_percentage = TRUE;
    self->orientation = LRG_ORIENTATION_HORIZONTAL;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 5; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;

    lrg_widget_set_width (LRG_WIDGET (self), 200.0f);
    lrg_widget_set_height (LRG_WIDGET (self), 60.0f);
}

static void
lp_widget_exposure_meter_class_init (LpWidgetExposureMeterClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);

    object_class->get_property = lp_widget_exposure_meter_get_property;
    object_class->set_property = lp_widget_exposure_meter_set_property;
    object_class->dispose = lp_widget_exposure_meter_dispose;

    widget_class->draw = lp_widget_exposure_meter_draw;
    widget_class->measure = lp_widget_exposure_meter_measure;

    /**
     * LpWidgetExposureMeter:value:
     *
     * The current exposure value (0-100).
     */
    properties[PROP_VALUE] =
        g_param_spec_uint ("value",
                           "Value",
                           "Current exposure value (0-100)",
                           0, 100, 0,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * LpWidgetExposureMeter:level:
     *
     * The current exposure level (read-only, calculated from value).
     */
    properties[PROP_LEVEL] =
        g_param_spec_enum ("level",
                           "Level",
                           "Current exposure level",
                           LP_TYPE_EXPOSURE_LEVEL,
                           LP_EXPOSURE_LEVEL_HIDDEN,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * LpWidgetExposureMeter:show-label:
     *
     * Whether to show the exposure level label.
     */
    properties[PROP_SHOW_LABEL] =
        g_param_spec_boolean ("show-label",
                              "Show Label",
                              "Whether to show the level label",
                              TRUE,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * LpWidgetExposureMeter:show-percentage:
     *
     * Whether to show the exposure percentage.
     */
    properties[PROP_SHOW_PERCENTAGE] =
        g_param_spec_boolean ("show-percentage",
                              "Show Percentage",
                              "Whether to show the percentage",
                              TRUE,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * LpWidgetExposureMeter:orientation:
     *
     * The meter orientation (horizontal or vertical).
     */
    properties[PROP_ORIENTATION] =
        g_param_spec_enum ("orientation",
                           "Orientation",
                           "Meter orientation",
                           LRG_TYPE_ORIENTATION,
                           LRG_ORIENTATION_HORIZONTAL,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpWidgetExposureMeter::level-changed:
     * @self: the #LpWidgetExposureMeter
     * @old_level: the previous level
     * @new_level: the new level
     *
     * Emitted when the exposure level threshold is crossed.
     */
    signals[SIGNAL_LEVEL_CHANGED] =
        g_signal_new ("level-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      2,
                      LP_TYPE_EXPOSURE_LEVEL,
                      LP_TYPE_EXPOSURE_LEVEL);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

LpWidgetExposureMeter *
lp_widget_exposure_meter_new (void)
{
    return g_object_new (LP_TYPE_WIDGET_EXPOSURE_METER, NULL);
}

guint
lp_widget_exposure_meter_get_value (LpWidgetExposureMeter *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self), 0);

    return self->value;
}

void
lp_widget_exposure_meter_set_value (LpWidgetExposureMeter *self,
                                     guint                  value)
{
    LpExposureLevel old_level;
    LpExposureLevel new_level;

    g_return_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self));

    /* Clamp to 0-100 */
    if (value > 100)
        value = 100;

    if (self->value == value)
        return;

    old_level = self->level;
    self->value = value;
    new_level = calculate_level_from_value (value);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);

    if (old_level != new_level)
    {
        self->level = new_level;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LEVEL]);
        g_signal_emit (self, signals[SIGNAL_LEVEL_CHANGED], 0,
                       old_level, new_level);
    }
}

LpExposureLevel
lp_widget_exposure_meter_get_level (LpWidgetExposureMeter *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self), LP_EXPOSURE_LEVEL_HIDDEN);

    return self->level;
}

gboolean
lp_widget_exposure_meter_get_show_label (LpWidgetExposureMeter *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self), TRUE);

    return self->show_label;
}

void
lp_widget_exposure_meter_set_show_label (LpWidgetExposureMeter *self,
                                          gboolean               show)
{
    g_return_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self));

    if (self->show_label == show)
        return;

    self->show_label = show;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SHOW_LABEL]);
}

gboolean
lp_widget_exposure_meter_get_show_percentage (LpWidgetExposureMeter *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self), TRUE);

    return self->show_percentage;
}

void
lp_widget_exposure_meter_set_show_percentage (LpWidgetExposureMeter *self,
                                               gboolean               show)
{
    g_return_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self));

    if (self->show_percentage == show)
        return;

    self->show_percentage = show;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SHOW_PERCENTAGE]);
}

LrgOrientation
lp_widget_exposure_meter_get_orientation (LpWidgetExposureMeter *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self), LRG_ORIENTATION_HORIZONTAL);

    return self->orientation;
}

void
lp_widget_exposure_meter_set_orientation (LpWidgetExposureMeter *self,
                                           LrgOrientation         orientation)
{
    g_return_if_fail (LP_IS_WIDGET_EXPOSURE_METER (self));

    if (self->orientation == orientation)
        return;

    self->orientation = orientation;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ORIENTATION]);
}
