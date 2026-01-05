/* lp-screen-ledger.c - Discovery Ledger Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-screen-ledger.h"
#include "lp-theme.h"
#include "../core/lp-ledger.h"

struct _LpScreenLedger
{
    LrgContainer parent_instance;

    LpLedger *ledger;

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

enum
{
    PROP_0,
    PROP_LEDGER,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpScreenLedger, lp_screen_ledger, LRG_TYPE_CONTAINER)

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
get_pool_label (LpScreenLedger *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpScreenLedger *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Widget Draw
 * ========================================================================== */

static void
lp_screen_ledger_draw (LrgWidget *widget)
{
    LpScreenLedger *self;
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

    self = LP_SCREEN_LEDGER (widget);

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

    draw_label (get_pool_label (self), "Ledger of Secrets",
                x + padding, y + padding,
                font_size_large, text_color);

    /* Draw discovery count if ledger is set */
    if (self->ledger != NULL)
    {
        guint discovered = lp_ledger_get_discovered_count (self->ledger);
        guint in_progress = lp_ledger_get_in_progress_count (self->ledger);
        g_autofree gchar *count_text = g_strdup_printf ("Discovered: %u  In Progress: %u",
                                                         discovered, in_progress);
        gfloat text_width = grl_measure_text (count_text, font_size);

        draw_label (get_pool_label (self), count_text,
                    x + width - text_width - padding,
                    y + padding + (font_size_large - font_size) / 2,
                    font_size, lp_theme_get_hidden_color ());
    }

    /* Draw placeholder content */
    draw_label (get_pool_label (self), "Hidden knowledge and discoveries - coming soon",
                x + padding,
                y + header_height + padding,
                font_size, secondary_color);

    LRG_WIDGET_CLASS (lp_screen_ledger_parent_class)->draw (widget);
}

static void
lp_screen_ledger_layout_children (LrgContainer *container)
{
    /* No child widgets to layout yet */
    (void)container;
}

static void
lp_screen_ledger_get_property (GObject *object, guint prop_id,
                                GValue *value, GParamSpec *pspec)
{
    LpScreenLedger *self = LP_SCREEN_LEDGER (object);

    switch (prop_id)
    {
        case PROP_LEDGER:
            g_value_set_object (value, self->ledger);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_ledger_set_property (GObject *object, guint prop_id,
                                const GValue *value, GParamSpec *pspec)
{
    LpScreenLedger *self = LP_SCREEN_LEDGER (object);

    switch (prop_id)
    {
        case PROP_LEDGER:
            lp_screen_ledger_set_ledger (self, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_ledger_dispose (GObject *object)
{
    LpScreenLedger *self = LP_SCREEN_LEDGER (object);

    g_clear_object (&self->ledger);
    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_screen_ledger_parent_class)->dispose (object);
}

static void
lp_screen_ledger_class_init (LpScreenLedgerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_screen_ledger_get_property;
    object_class->set_property = lp_screen_ledger_set_property;
    object_class->dispose = lp_screen_ledger_dispose;

    widget_class->draw = lp_screen_ledger_draw;
    container_class->layout_children = lp_screen_ledger_layout_children;

    properties[PROP_LEDGER] =
        g_param_spec_object ("ledger", "Ledger",
                             "The ledger to display",
                             LP_TYPE_LEDGER,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_screen_ledger_init (LpScreenLedger *self)
{
    guint i;

    self->ledger = NULL;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 10; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

LpScreenLedger *
lp_screen_ledger_new (void)
{
    return g_object_new (LP_TYPE_SCREEN_LEDGER, NULL);
}

LpLedger *
lp_screen_ledger_get_ledger (LpScreenLedger *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_LEDGER (self), NULL);
    return self->ledger;
}

void
lp_screen_ledger_set_ledger (LpScreenLedger *self,
                              LpLedger       *ledger)
{
    g_return_if_fail (LP_IS_SCREEN_LEDGER (self));

    if (g_set_object (&self->ledger, ledger))
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LEDGER]);
}

void
lp_screen_ledger_refresh (LpScreenLedger *self)
{
    g_return_if_fail (LP_IS_SCREEN_LEDGER (self));
    /* Redraw will happen automatically via property notifications */
}
