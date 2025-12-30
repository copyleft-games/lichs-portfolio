/* lp-screen-slumber.c - Slumber Configuration Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-screen-slumber.h"
#include "lp-theme.h"

/* Minimum slumber duration */
#define MIN_SLUMBER_YEARS 25

/* Slumber duration presets */
static const guint SLUMBER_PRESETS[] = {25, 50, 100, 250, 500};
#define N_PRESETS (sizeof(SLUMBER_PRESETS) / sizeof(SLUMBER_PRESETS[0]))

struct _LpScreenSlumber
{
    LrgContainer parent_instance;

    guint duration;
    gint  preset_index;
};

enum
{
    PROP_0,
    PROP_DURATION,
    N_PROPS
};

enum
{
    SIGNAL_SLUMBER_CONFIRMED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpScreenSlumber, lp_screen_slumber, LRG_TYPE_CONTAINER)

static void
lp_screen_slumber_draw (LrgWidget *widget)
{
    LpScreenSlumber *self;
    LrgTheme *theme;
    gfloat x, y, width, height;
    gfloat padding;
    gfloat font_size_large;
    gfloat font_size;
    gfloat font_size_small;
    gfloat header_height;
    const GrlColor *bg_color;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *border_color;
    const GrlColor *surface_color;
    const GrlColor *accent_color;
    gfloat content_y;
    guint i;

    self = LP_SCREEN_SLUMBER (widget);
    theme = lrg_theme_get_default ();

    /* Get widget position and size */
    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget);
    width = lrg_widget_get_width (widget);
    height = lrg_widget_get_height (widget);

    padding = lrg_theme_get_padding_normal (theme);
    font_size_large = lrg_theme_get_font_size_large (theme);
    font_size = lrg_theme_get_font_size_normal (theme);
    font_size_small = lrg_theme_get_font_size_small (theme);
    header_height = font_size_large + padding * 3;

    bg_color = lrg_theme_get_background_color (theme);
    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    border_color = lrg_theme_get_border_color (theme);
    surface_color = lrg_theme_get_surface_color (theme);
    accent_color = lrg_theme_get_accent_color (theme);

    (void)height; /* Silence unused variable warning for now */

    /* Draw background */
    grl_draw_rectangle (x, y, width, height, bg_color);

    /* Draw header */
    grl_draw_rectangle (x, y, width, header_height, surface_color);
    grl_draw_line (x, y + header_height,
                   x + width, y + header_height, border_color);

    grl_draw_text ("Slumber Configuration", x + padding, y + padding,
                   font_size_large, text_color);

    content_y = y + header_height + padding * 2;

    /* Draw duration selection */
    grl_draw_text ("Select slumber duration:", x + padding, content_y,
                   font_size, text_color);
    content_y += font_size + padding * 2;

    /* Draw preset buttons */
    {
        gfloat button_x = x + padding;
        gfloat button_height = font_size + padding * 2;

        for (i = 0; i < N_PRESETS; i++)
        {
            g_autofree gchar *label = g_strdup_printf ("%u years", SLUMBER_PRESETS[i]);
            gfloat button_width = grl_measure_text (label, font_size) + padding * 2;
            gboolean selected = ((gint)i == self->preset_index);

            if (selected)
            {
                grl_draw_rectangle (button_x, content_y, button_width, button_height,
                                     accent_color);
                grl_draw_text (label, button_x + padding, content_y + padding,
                               font_size, bg_color);
            }
            else
            {
                g_autoptr(GrlRectangle) btn_rect = grl_rectangle_new (
                    button_x, content_y, button_width, button_height);
                grl_draw_rectangle_lines_ex (btn_rect, 1.0f, border_color);
                grl_draw_text (label, button_x + padding, content_y + padding,
                               font_size, secondary_color);
            }

            button_x += button_width + padding;
        }
    }

    content_y += font_size + padding * 4;

    /* Draw current selection */
    {
        g_autofree gchar *duration_text = g_strdup_printf ("Slumber for %u years",
                                                            self->duration);
        grl_draw_text (duration_text, x + padding, content_y,
                       font_size_large, lp_theme_get_gold_color ());
    }

    content_y += font_size_large + padding * 4;

    /* Draw instructions */
    grl_draw_text ("[1-5] Select preset  [Enter] Confirm  [Esc] Cancel",
                   x + padding, content_y, font_size_small, secondary_color);

    LRG_WIDGET_CLASS (lp_screen_slumber_parent_class)->draw (widget);
}

static gboolean
lp_screen_slumber_handle_event (LrgWidget        *widget,
                                 const LrgUIEvent *event)
{
    LpScreenSlumber *self;
    LrgUIEventType event_type;
    GrlKey key;

    self = LP_SCREEN_SLUMBER (widget);
    event_type = lrg_ui_event_get_event_type (event);

    if (event_type != LRG_UI_EVENT_KEY_DOWN)
        return FALSE;

    key = lrg_ui_event_get_key (event);

    /* Number keys 1-5 for presets */
    if (key >= GRL_KEY_ONE && key <= GRL_KEY_FIVE)
    {
        gint index = key - GRL_KEY_ONE;
        if (index >= 0 && index < (gint)N_PRESETS)
        {
            self->preset_index = index;
            self->duration = SLUMBER_PRESETS[index];
            g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DURATION]);
            return TRUE;
        }
    }

    switch (key)
    {
        case GRL_KEY_LEFT:
            if (self->preset_index > 0)
            {
                self->preset_index--;
                self->duration = SLUMBER_PRESETS[self->preset_index];
                g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DURATION]);
            }
            return TRUE;

        case GRL_KEY_RIGHT:
            if (self->preset_index < (gint)N_PRESETS - 1)
            {
                self->preset_index++;
                self->duration = SLUMBER_PRESETS[self->preset_index];
                g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DURATION]);
            }
            return TRUE;

        case GRL_KEY_ENTER:
            g_signal_emit (self, signals[SIGNAL_SLUMBER_CONFIRMED], 0, self->duration);
            return TRUE;

        default:
            break;
    }

    return FALSE;
}

static void
lp_screen_slumber_layout_children (LrgContainer *container)
{
    /* No child widgets to layout yet */
    (void)container;
}

static void
lp_screen_slumber_get_property (GObject *object, guint prop_id,
                                 GValue *value, GParamSpec *pspec)
{
    LpScreenSlumber *self = LP_SCREEN_SLUMBER (object);

    switch (prop_id)
    {
        case PROP_DURATION:
            g_value_set_uint (value, self->duration);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_slumber_set_property (GObject *object, guint prop_id,
                                 const GValue *value, GParamSpec *pspec)
{
    LpScreenSlumber *self = LP_SCREEN_SLUMBER (object);

    switch (prop_id)
    {
        case PROP_DURATION:
            lp_screen_slumber_set_duration (self, g_value_get_uint (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_slumber_class_init (LpScreenSlumberClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_screen_slumber_get_property;
    object_class->set_property = lp_screen_slumber_set_property;

    widget_class->draw = lp_screen_slumber_draw;
    widget_class->handle_event = lp_screen_slumber_handle_event;
    container_class->layout_children = lp_screen_slumber_layout_children;

    properties[PROP_DURATION] =
        g_param_spec_uint ("duration", "Duration",
                           "The slumber duration in years",
                           MIN_SLUMBER_YEARS, G_MAXUINT, MIN_SLUMBER_YEARS,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    signals[SIGNAL_SLUMBER_CONFIRMED] =
        g_signal_new ("slumber-confirmed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void
lp_screen_slumber_init (LpScreenSlumber *self)
{
    self->duration = MIN_SLUMBER_YEARS;
    self->preset_index = 0;
}

LpScreenSlumber *
lp_screen_slumber_new (void)
{
    return g_object_new (LP_TYPE_SCREEN_SLUMBER, NULL);
}

guint
lp_screen_slumber_get_duration (LpScreenSlumber *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_SLUMBER (self), MIN_SLUMBER_YEARS);
    return self->duration;
}

void
lp_screen_slumber_set_duration (LpScreenSlumber *self,
                                 guint            years)
{
    guint i;

    g_return_if_fail (LP_IS_SCREEN_SLUMBER (self));

    years = MAX (years, MIN_SLUMBER_YEARS);

    if (self->duration != years)
    {
        self->duration = years;

        /* Find matching preset */
        self->preset_index = -1;
        for (i = 0; i < N_PRESETS; i++)
        {
            if (SLUMBER_PRESETS[i] == years)
            {
                self->preset_index = i;
                break;
            }
        }

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DURATION]);
    }
}
