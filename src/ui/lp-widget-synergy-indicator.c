/* lp-widget-synergy-indicator.c - Synergy Bonus Display Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-widget-synergy-indicator.h"
#include "lp-theme.h"
#include "../core/lp-synergy-manager.h"

struct _LpWidgetSynergyIndicator
{
    LrgWidget parent_instance;

    /* Cached values from manager */
    guint    synergy_count;
    gdouble  total_bonus;

    /* Display options */
    gboolean show_details;
    gboolean compact;

    /* Manager connection */
    LpSynergyManager *manager;
    gulong            synergies_changed_handler;
};

enum
{
    PROP_0,
    PROP_SYNERGY_COUNT,
    PROP_TOTAL_BONUS,
    PROP_SHOW_DETAILS,
    PROP_COMPACT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpWidgetSynergyIndicator, lp_widget_synergy_indicator, LRG_TYPE_WIDGET)

/* ==========================================================================
 * Forward Declarations
 * ========================================================================== */

static void lp_widget_synergy_indicator_draw    (LrgWidget *widget);
static void lp_widget_synergy_indicator_measure (LrgWidget *widget,
                                                  gfloat    *preferred_width,
                                                  gfloat    *preferred_height);

/* ==========================================================================
 * Helper Functions
 * ========================================================================== */

/*
 * update_from_manager:
 *
 * Updates cached values from the synergy manager.
 */
static void
update_from_manager (LpWidgetSynergyIndicator *self)
{
    guint old_count;
    gdouble old_bonus;

    if (self->manager == NULL)
        return;

    old_count = self->synergy_count;
    old_bonus = self->total_bonus;

    self->synergy_count = lp_synergy_manager_get_synergy_count (self->manager);
    self->total_bonus = lp_synergy_manager_get_total_bonus (self->manager);

    /* Notify if changed */
    if (old_count != self->synergy_count)
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SYNERGY_COUNT]);

    if (old_bonus != self->total_bonus)
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TOTAL_BONUS]);
}

/*
 * on_synergies_changed:
 *
 * Signal handler for manager's synergies-changed signal.
 */
static void
on_synergies_changed (LpSynergyManager         *manager,
                      LpWidgetSynergyIndicator *self)
{
    update_from_manager (self);
}

/*
 * draw_diamond:
 *
 * Draws a diamond shape (rotated square) at the given position.
 */
static void
draw_diamond (gfloat          cx,
              gfloat          cy,
              gfloat          size,
              const GrlColor *color,
              gboolean        filled)
{
    g_autoptr(GrlVector2) center = grl_vector2_new (cx, cy);

    if (filled)
    {
        /* Draw a 4-sided polygon (diamond) */
        grl_draw_poly (center, 4, size, 45.0f, color);
    }
    else
    {
        /* Draw outline only */
        grl_draw_poly_lines (center, 4, size, 45.0f, color);
    }
}

/* ==========================================================================
 * LrgWidget Virtual Methods
 * ========================================================================== */

/*
 * lp_widget_synergy_indicator_draw:
 *
 * Draws the synergy indicator with icon, count, and bonus.
 */
static void
lp_widget_synergy_indicator_draw (LrgWidget *widget)
{
    LpWidgetSynergyIndicator *self;
    LrgTheme *theme;
    const GrlColor *synergy_color;
    const GrlColor *text_color;
    const GrlColor *inactive_color;
    const GrlColor *surface_color;
    const GrlColor *border_color;
    GrlRectangle bounds;
    gfloat x;
    gfloat y;
    gfloat width;
    gfloat height;
    gfloat padding;
    gfloat font_size;
    gfloat icon_size;
    g_autofree gchar *bonus_text = NULL;
    g_autofree gchar *count_text = NULL;
    gboolean has_synergies;

    self = LP_WIDGET_SYNERGY_INDICATOR (widget);
    theme = lrg_theme_get_default ();

    /* Get bounds */
    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget);
    width = lrg_widget_get_width (widget);
    height = lrg_widget_get_height (widget);

    /* Get theme values */
    padding = lrg_theme_get_padding_normal (theme);
    font_size = lrg_theme_get_font_size_normal (theme);
    icon_size = font_size * 1.5f;

    /* Get colors */
    synergy_color = lp_theme_get_synergy_color ();
    inactive_color = lp_theme_get_inactive_color ();
    text_color = lrg_theme_get_text_color (theme);
    surface_color = lrg_theme_get_surface_color (theme);
    border_color = lrg_theme_get_border_color (theme);

    has_synergies = (self->synergy_count > 0);

    /* Draw background panel */
    bounds.x = x;
    bounds.y = y;
    bounds.width = width;
    bounds.height = height;

    grl_draw_rectangle_rec (&bounds, surface_color);
    grl_draw_rectangle_lines_ex (&bounds, 1.0f,
                                  has_synergies ? synergy_color : border_color);

    if (self->compact)
    {
        /*
         * Compact mode: Just icon and bonus multiplier
         * [◇ x1.5]
         */
        gfloat text_x;
        gfloat text_y;

        /* Draw synergy icon (diamond shape) */
        gfloat icon_x = x + padding + icon_size / 2.0f;
        gfloat icon_y = y + height / 2.0f;

        draw_diamond (icon_x, icon_y, icon_size * 0.4f,
                      has_synergies ? synergy_color : inactive_color,
                      has_synergies);

        /* Draw bonus multiplier */
        bonus_text = g_strdup_printf ("x%.1f", self->total_bonus);
        text_x = x + padding * 2 + icon_size;
        text_y = y + height / 2.0f - font_size / 2.0f;

        grl_draw_text (bonus_text, (gint)text_x, (gint)text_y, (gint)font_size,
                       has_synergies ? text_color : inactive_color);
    }
    else
    {
        /*
         * Normal mode: Icon, label, count, and bonus
         * [◇ Synergies: 3 (x1.5)]
         */
        gfloat current_x;
        gfloat text_y;
        gint text_width;

        /* Draw synergy icon */
        gfloat icon_x = x + padding + icon_size / 2.0f;
        gfloat icon_y = y + height / 2.0f;

        draw_diamond (icon_x, icon_y, icon_size * 0.4f,
                      has_synergies ? synergy_color : inactive_color,
                      has_synergies);

        current_x = x + padding * 2 + icon_size;
        text_y = y + height / 2.0f - font_size / 2.0f;

        /* Draw label */
        grl_draw_text ("Synergies: ", (gint)current_x, (gint)text_y, (gint)font_size,
                       has_synergies ? text_color : inactive_color);
        text_width = grl_measure_text ("Synergies: ", (gint)font_size);
        current_x += text_width;

        /* Draw count */
        count_text = g_strdup_printf ("%u", self->synergy_count);
        grl_draw_text (count_text, (gint)current_x, (gint)text_y, (gint)font_size,
                       has_synergies ? synergy_color : inactive_color);
        text_width = grl_measure_text (count_text, (gint)font_size);
        current_x += text_width;

        /* Draw bonus in parentheses */
        bonus_text = g_strdup_printf (" (x%.1f)", self->total_bonus);
        grl_draw_text (bonus_text, (gint)current_x, (gint)text_y, (gint)font_size,
                       has_synergies ? synergy_color : inactive_color);

        /* Draw details if enabled and synergies exist */
        if (self->show_details && has_synergies)
        {
            GPtrArray *synergies;
            gfloat detail_y;
            gfloat detail_font_size;

            synergies = lp_synergy_manager_get_active_synergies (self->manager);
            detail_font_size = lrg_theme_get_font_size_small (theme);
            detail_y = y + height / 2.0f + font_size;

            /*
             * List individual synergies
             * Currently skeleton - will show synergy names when implemented
             */
            if (synergies != NULL && synergies->len > 0)
            {
                guint i;

                for (i = 0; i < synergies->len && i < 5; i++)
                {
                    /*
                     * LpSynergy type not yet implemented - this will be
                     * expanded in Phase 2+ to show actual synergy details
                     */
                    grl_draw_text ("* Active synergy",
                                   (gint)(x + padding * 3),
                                   (gint)(detail_y + i * (detail_font_size + 2.0f)),
                                   (gint)detail_font_size, synergy_color);
                }

                /* Show "and N more" if truncated */
                if (synergies->len > 5)
                {
                    g_autofree gchar *more_text = NULL;
                    more_text = g_strdup_printf ("  ...and %u more",
                                                 synergies->len - 5);
                    grl_draw_text (more_text,
                                   (gint)(x + padding * 3),
                                   (gint)(detail_y + 5 * (detail_font_size + 2.0f)),
                                   (gint)detail_font_size, inactive_color);
                }
            }
        }
    }
}

/*
 * lp_widget_synergy_indicator_measure:
 *
 * Calculates the widget's size requirements.
 */
static void
lp_widget_synergy_indicator_measure (LrgWidget *widget,
                                      gfloat    *preferred_width,
                                      gfloat    *preferred_height)
{
    LpWidgetSynergyIndicator *self;
    LrgTheme *theme;
    gfloat padding;
    gfloat font_size;
    gfloat icon_size;
    gint text_width;
    gfloat detail_height;

    self = LP_WIDGET_SYNERGY_INDICATOR (widget);
    theme = lrg_theme_get_default ();

    padding = lrg_theme_get_padding_normal (theme);
    font_size = lrg_theme_get_font_size_normal (theme);
    icon_size = font_size * 1.5f;

    if (self->compact)
    {
        /* Compact: icon + "x1.0" */
        text_width = grl_measure_text ("x9.9", (gint)font_size);

        *preferred_width = padding * 3 + icon_size + text_width;
        *preferred_height = padding * 2 + font_size;
    }
    else
    {
        /* Normal: icon + "Synergies: 99 (x9.9)" */
        text_width = grl_measure_text ("Synergies: 99 (x9.9)", (gint)font_size);

        *preferred_width = padding * 3 + icon_size + text_width;
        *preferred_height = padding * 2 + font_size;

        /* Add height for details if enabled */
        if (self->show_details && self->synergy_count > 0)
        {
            gfloat detail_font_size = lrg_theme_get_font_size_small (theme);
            guint detail_count = MIN (self->synergy_count, 6);  /* Max 5 + "more" */
            detail_height = detail_count * (detail_font_size + 2.0f);
            *preferred_height += detail_height + padding;
        }
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_widget_synergy_indicator_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
    LpWidgetSynergyIndicator *self = LP_WIDGET_SYNERGY_INDICATOR (object);

    switch (prop_id)
    {
        case PROP_SYNERGY_COUNT:
            g_value_set_uint (value, self->synergy_count);
            break;

        case PROP_TOTAL_BONUS:
            g_value_set_double (value, self->total_bonus);
            break;

        case PROP_SHOW_DETAILS:
            g_value_set_boolean (value, self->show_details);
            break;

        case PROP_COMPACT:
            g_value_set_boolean (value, self->compact);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_widget_synergy_indicator_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
    LpWidgetSynergyIndicator *self = LP_WIDGET_SYNERGY_INDICATOR (object);

    switch (prop_id)
    {
        case PROP_SHOW_DETAILS:
            lp_widget_synergy_indicator_set_show_details (self,
                g_value_get_boolean (value));
            break;

        case PROP_COMPACT:
            lp_widget_synergy_indicator_set_compact (self,
                g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_widget_synergy_indicator_dispose (GObject *object)
{
    LpWidgetSynergyIndicator *self = LP_WIDGET_SYNERGY_INDICATOR (object);

    /* Disconnect signal handler */
    if (self->manager != NULL && self->synergies_changed_handler != 0)
    {
        g_signal_handler_disconnect (self->manager,
                                     self->synergies_changed_handler);
        self->synergies_changed_handler = 0;
    }

    /* Don't unref manager - it's a singleton */
    self->manager = NULL;

    G_OBJECT_CLASS (lp_widget_synergy_indicator_parent_class)->dispose (object);
}

static void
lp_widget_synergy_indicator_class_init (LpWidgetSynergyIndicatorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);

    object_class->get_property = lp_widget_synergy_indicator_get_property;
    object_class->set_property = lp_widget_synergy_indicator_set_property;
    object_class->dispose = lp_widget_synergy_indicator_dispose;

    widget_class->draw = lp_widget_synergy_indicator_draw;
    widget_class->measure = lp_widget_synergy_indicator_measure;

    /**
     * LpWidgetSynergyIndicator:synergy-count:
     *
     * The number of currently active synergies.
     */
    properties[PROP_SYNERGY_COUNT] =
        g_param_spec_uint ("synergy-count",
                           "Synergy Count",
                           "Number of active synergies",
                           0, G_MAXUINT, 0,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * LpWidgetSynergyIndicator:total-bonus:
     *
     * The total synergy bonus multiplier (1.0 = no bonus).
     */
    properties[PROP_TOTAL_BONUS] =
        g_param_spec_double ("total-bonus",
                             "Total Bonus",
                             "Total synergy bonus multiplier",
                             0.0, G_MAXDOUBLE, 1.0,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * LpWidgetSynergyIndicator:show-details:
     *
     * Whether to show detailed synergy list.
     */
    properties[PROP_SHOW_DETAILS] =
        g_param_spec_boolean ("show-details",
                              "Show Details",
                              "Whether to show synergy details",
                              FALSE,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * LpWidgetSynergyIndicator:compact:
     *
     * Whether to use compact display mode.
     */
    properties[PROP_COMPACT] =
        g_param_spec_boolean ("compact",
                              "Compact",
                              "Whether to use compact mode",
                              FALSE,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_widget_synergy_indicator_init (LpWidgetSynergyIndicator *self)
{
    self->synergy_count = 0;
    self->total_bonus = 1.0;
    self->show_details = FALSE;
    self->compact = FALSE;

    /* Connect to the default synergy manager */
    self->manager = lp_synergy_manager_get_default ();
    self->synergies_changed_handler =
        g_signal_connect (self->manager, "synergies-changed",
                          G_CALLBACK (on_synergies_changed), self);

    /* Initial sync from manager */
    update_from_manager (self);

    /* Set default size */
    lrg_widget_set_width (LRG_WIDGET (self), 200.0f);
    lrg_widget_set_height (LRG_WIDGET (self), 32.0f);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_widget_synergy_indicator_new:
 *
 * Creates a new synergy indicator widget.
 *
 * Returns: (transfer full): A new #LpWidgetSynergyIndicator
 */
LpWidgetSynergyIndicator *
lp_widget_synergy_indicator_new (void)
{
    return g_object_new (LP_TYPE_WIDGET_SYNERGY_INDICATOR, NULL);
}

/**
 * lp_widget_synergy_indicator_get_synergy_count:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets the number of currently active synergies.
 *
 * Returns: The number of active synergies
 */
guint
lp_widget_synergy_indicator_get_synergy_count (LpWidgetSynergyIndicator *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_SYNERGY_INDICATOR (self), 0);

    return self->synergy_count;
}

/**
 * lp_widget_synergy_indicator_get_total_bonus:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets the total synergy bonus multiplier.
 *
 * Returns: The bonus multiplier (1.0 = no bonus)
 */
gdouble
lp_widget_synergy_indicator_get_total_bonus (LpWidgetSynergyIndicator *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_SYNERGY_INDICATOR (self), 1.0);

    return self->total_bonus;
}

/**
 * lp_widget_synergy_indicator_get_show_details:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets whether detailed synergy list is shown.
 *
 * Returns: %TRUE if showing details
 */
gboolean
lp_widget_synergy_indicator_get_show_details (LpWidgetSynergyIndicator *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_SYNERGY_INDICATOR (self), FALSE);

    return self->show_details;
}

/**
 * lp_widget_synergy_indicator_set_show_details:
 * @self: an #LpWidgetSynergyIndicator
 * @show: whether to show details
 *
 * Sets whether to display the detailed synergy list.
 */
void
lp_widget_synergy_indicator_set_show_details (LpWidgetSynergyIndicator *self,
                                               gboolean                  show)
{
    g_return_if_fail (LP_IS_WIDGET_SYNERGY_INDICATOR (self));

    show = !!show;

    if (self->show_details != show)
    {
        self->show_details = show;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SHOW_DETAILS]);
    }
}

/**
 * lp_widget_synergy_indicator_get_compact:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets whether the widget is in compact mode.
 *
 * Returns: %TRUE if in compact mode
 */
gboolean
lp_widget_synergy_indicator_get_compact (LpWidgetSynergyIndicator *self)
{
    g_return_val_if_fail (LP_IS_WIDGET_SYNERGY_INDICATOR (self), FALSE);

    return self->compact;
}

/**
 * lp_widget_synergy_indicator_set_compact:
 * @self: an #LpWidgetSynergyIndicator
 * @compact: whether to use compact mode
 *
 * Sets compact mode. In compact mode, only shows icon and bonus.
 */
void
lp_widget_synergy_indicator_set_compact (LpWidgetSynergyIndicator *self,
                                          gboolean                  compact)
{
    g_return_if_fail (LP_IS_WIDGET_SYNERGY_INDICATOR (self));

    compact = !!compact;

    if (self->compact != compact)
    {
        self->compact = compact;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMPACT]);
    }
}

/**
 * lp_widget_synergy_indicator_refresh:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Forces a refresh of the synergy display from the manager.
 */
void
lp_widget_synergy_indicator_refresh (LpWidgetSynergyIndicator *self)
{
    g_return_if_fail (LP_IS_WIDGET_SYNERGY_INDICATOR (self));

    update_from_manager (self);
}
