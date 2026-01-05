/* lp-screen-world-map.c - World Map Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-screen-world-map.h"
#include "lp-theme.h"
#include "../simulation/lp-world-simulation.h"
#include "../simulation/lp-kingdom.h"
#include "../simulation/lp-region.h"

struct _LpScreenWorldMap
{
    LrgContainer parent_instance;

    /* Data binding */
    LpWorldSimulation *simulation;

    /* Selection state */
    LpKingdom *selected_kingdom;
    gint       selection_index;

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

enum
{
    PROP_0,
    PROP_SIMULATION,
    PROP_SELECTED_KINGDOM,
    N_PROPS
};

enum
{
    SIGNAL_KINGDOM_SELECTED,
    SIGNAL_REGION_CLICKED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpScreenWorldMap, lp_screen_world_map, LRG_TYPE_CONTAINER)

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
get_pool_label (LpScreenWorldMap *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpScreenWorldMap *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * LrgWidget Virtual Methods
 * ========================================================================== */

static void
lp_screen_world_map_draw (LrgWidget *widget)
{
    LpScreenWorldMap *self;
    LrgTheme *theme;
    const GrlColor *bg_color;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *border_color;
    const GrlColor *surface_color;
    gfloat x, y, width, height;
    gfloat padding;
    gfloat font_size_large;
    gfloat font_size;
    gfloat header_height;

    self = LP_SCREEN_WORLD_MAP (widget);

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

    /* Get colors */
    bg_color = lrg_theme_get_background_color (theme);
    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    border_color = lrg_theme_get_border_color (theme);
    surface_color = lrg_theme_get_surface_color (theme);

    /* Draw background */
    grl_draw_rectangle (x, y, width, height, bg_color);

    /* Draw header */
    grl_draw_rectangle (x, y, width, header_height, surface_color);
    grl_draw_line (x, y + header_height,
                   x + width, y + header_height, border_color);

    draw_label (get_pool_label (self), "World Map",
                x + padding, y + padding,
                font_size_large, text_color);

    /* Draw current year if simulation is set */
    if (self->simulation != NULL)
    {
        guint64 year = lp_world_simulation_get_current_year (self->simulation);
        g_autofree gchar *year_text = g_strdup_printf ("Year: %lu", (unsigned long)year);
        gfloat year_width = grl_measure_text (year_text, font_size);

        draw_label (get_pool_label (self), year_text,
                    x + width - year_width - padding,
                    y + padding + (font_size_large - font_size) / 2,
                    font_size, secondary_color);
    }

    /* Draw placeholder map area */
    {
        gfloat map_y = y + header_height + padding;
        gfloat map_h = height - header_height - padding * 2;
        g_autoptr(GrlRectangle) map_rect = grl_rectangle_new (
            x + padding, map_y, width - padding * 2, map_h);

        grl_draw_rectangle_lines_ex (map_rect, 1.0f, border_color);

        draw_label (get_pool_label (self), "Kingdom map visualization - coming soon",
                    x + width / 2 - 150,
                    map_y + map_h / 2,
                    font_size, secondary_color);
    }

    /* Draw child widgets */
    LRG_WIDGET_CLASS (lp_screen_world_map_parent_class)->draw (widget);
}

static void
lp_screen_world_map_layout_children (LrgContainer *container)
{
    /* No child widgets to layout yet */
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_screen_world_map_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    LpScreenWorldMap *self = LP_SCREEN_WORLD_MAP (object);

    switch (prop_id)
    {
        case PROP_SIMULATION:
            g_value_set_object (value, self->simulation);
            break;
        case PROP_SELECTED_KINGDOM:
            g_value_set_object (value, self->selected_kingdom);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_world_map_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    LpScreenWorldMap *self = LP_SCREEN_WORLD_MAP (object);

    switch (prop_id)
    {
        case PROP_SIMULATION:
            lp_screen_world_map_set_simulation (self, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_world_map_dispose (GObject *object)
{
    LpScreenWorldMap *self = LP_SCREEN_WORLD_MAP (object);

    g_clear_object (&self->simulation);
    self->selected_kingdom = NULL;

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_screen_world_map_parent_class)->dispose (object);
}

static void
lp_screen_world_map_class_init (LpScreenWorldMapClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_screen_world_map_get_property;
    object_class->set_property = lp_screen_world_map_set_property;
    object_class->dispose = lp_screen_world_map_dispose;

    widget_class->draw = lp_screen_world_map_draw;
    container_class->layout_children = lp_screen_world_map_layout_children;

    properties[PROP_SIMULATION] =
        g_param_spec_object ("simulation", "Simulation",
                             "The world simulation to display",
                             LP_TYPE_WORLD_SIMULATION,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_SELECTED_KINGDOM] =
        g_param_spec_object ("selected-kingdom", "Selected Kingdom",
                             "The currently selected kingdom",
                             LP_TYPE_KINGDOM,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    signals[SIGNAL_KINGDOM_SELECTED] =
        g_signal_new ("kingdom-selected",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, LP_TYPE_KINGDOM);

    signals[SIGNAL_REGION_CLICKED] =
        g_signal_new ("region-clicked",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, LP_TYPE_REGION);
}

static void
lp_screen_world_map_init (LpScreenWorldMap *self)
{
    guint i;

    self->simulation = NULL;
    self->selected_kingdom = NULL;
    self->selection_index = -1;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 10; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

LpScreenWorldMap *
lp_screen_world_map_new (void)
{
    return g_object_new (LP_TYPE_SCREEN_WORLD_MAP, NULL);
}

LpWorldSimulation *
lp_screen_world_map_get_simulation (LpScreenWorldMap *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_WORLD_MAP (self), NULL);
    return self->simulation;
}

void
lp_screen_world_map_set_simulation (LpScreenWorldMap  *self,
                                     LpWorldSimulation *simulation)
{
    g_return_if_fail (LP_IS_SCREEN_WORLD_MAP (self));

    if (g_set_object (&self->simulation, simulation))
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SIMULATION]);
}

LpKingdom *
lp_screen_world_map_get_selected_kingdom (LpScreenWorldMap *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_WORLD_MAP (self), NULL);
    return self->selected_kingdom;
}

void
lp_screen_world_map_select_kingdom (LpScreenWorldMap *self,
                                     LpKingdom        *kingdom)
{
    g_return_if_fail (LP_IS_SCREEN_WORLD_MAP (self));

    if (self->selected_kingdom != kingdom)
    {
        self->selected_kingdom = kingdom;
        g_signal_emit (self, signals[SIGNAL_KINGDOM_SELECTED], 0, kingdom);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SELECTED_KINGDOM]);
    }
}
