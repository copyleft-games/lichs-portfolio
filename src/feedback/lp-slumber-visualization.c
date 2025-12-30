/* lp-slumber-visualization.c - Slumber Phase Visualization
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include <math.h>

#include "lp-slumber-visualization.h"
#include "../ui/lp-theme.h"
#include "../simulation/lp-event.h"

/* Speed constants */
#define NORMAL_SPEED      1.0f
#define ACCELERATED_SPEED 5.0f

/* Display constants */
#define MAX_VISIBLE_EVENTS 8
#define EVENT_FADE_TIME    2.0f
#define YEAR_FONT_SIZE     48.0f

/* Timeline event entry */
typedef struct
{
    guint64  year;
    gchar   *text;
    gfloat   age;  /* Time since added */
    gboolean is_key_event;
} TimelineEntry;

static void
timeline_entry_free (gpointer data)
{
    TimelineEntry *entry = data;
    g_free (entry->text);
    g_free (entry);
}

struct _LpSlumberVisualization
{
    LrgContainer parent_instance;

    guint64      start_year;
    guint64      current_year;
    guint64      target_year;
    gfloat       simulation_speed;
    gboolean     is_accelerating;
    gboolean     is_active;

    GPtrArray   *events;        /* TimelineEntry* */
    gfloat       timeline_scroll;
    gfloat       year_pulse;    /* For year counter animation */
};

enum
{
    PROP_0,
    PROP_CURRENT_YEAR,
    PROP_TARGET_YEAR,
    PROP_SIMULATION_SPEED,
    PROP_IS_ACCELERATING,
    PROP_IS_ACTIVE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpSlumberVisualization, lp_slumber_visualization, LRG_TYPE_CONTAINER)

static void
lp_slumber_visualization_draw (LrgWidget *widget)
{
    LpSlumberVisualization *self;
    LrgTheme *theme;
    gfloat x, y, width, height;
    gfloat padding;
    gfloat font_size;
    gfloat font_size_small;
    const GrlColor *bg_color;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *gold_color;
    const GrlColor *synergy_color;
    gfloat center_x, center_y;
    gfloat timeline_y;

    self = LP_SLUMBER_VISUALIZATION (widget);

    if (!self->is_active)
        return;

    theme = lrg_theme_get_default ();

    /* Get widget position and size */
    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget);
    width = lrg_widget_get_width (widget);
    height = lrg_widget_get_height (widget);

    padding = lrg_theme_get_padding_large (theme);
    font_size = lrg_theme_get_font_size_normal (theme);
    font_size_small = lrg_theme_get_font_size_small (theme);

    bg_color = lrg_theme_get_background_color (theme);
    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    gold_color = lp_theme_get_gold_color ();
    synergy_color = lp_theme_get_synergy_color ();

    /* Draw background */
    grl_draw_rectangle (x, y, width, height, bg_color);

    center_x = x + width / 2;
    center_y = y + height / 3;

    /* Draw year counter with pulse effect */
    {
        g_autofree gchar *year_text = g_strdup_printf ("Year %" G_GUINT64_FORMAT,
                                                        self->current_year);
        gfloat text_width = grl_measure_text (year_text, YEAR_FONT_SIZE);
        gfloat year_x = center_x - text_width / 2;

        /* Pulse effect on the year */
        guint8 pulse_alpha = (guint8)(200 + 55 * sinf (self->year_pulse));
        g_autoptr(GrlColor) year_color = grl_color_new (
            grl_color_get_r (gold_color),
            grl_color_get_g (gold_color),
            grl_color_get_b (gold_color),
            pulse_alpha);

        grl_draw_text (year_text, year_x, center_y - YEAR_FONT_SIZE / 2,
                       YEAR_FONT_SIZE, year_color);
    }

    /* Draw progress bar */
    {
        gfloat bar_width = width * 0.6f;
        gfloat bar_height = 8.0f;
        gfloat bar_x = center_x - bar_width / 2;
        gfloat bar_y = center_y + YEAR_FONT_SIZE / 2 + padding;
        gdouble total_years = (gdouble)(self->target_year - self->start_year);
        gdouble elapsed_years = (gdouble)(self->current_year - self->start_year);
        gfloat progress = (total_years > 0) ? (gfloat)(elapsed_years / total_years) : 0.0f;

        /* Background bar */
        {
            g_autoptr(GrlColor) bar_bg = grl_color_new (40, 40, 40, 255);
            grl_draw_rectangle (bar_x, bar_y, bar_width, bar_height, bar_bg);
        }

        /* Progress fill */
        if (progress > 0.0f)
        {
            grl_draw_rectangle (bar_x, bar_y, bar_width * progress, bar_height, gold_color);
        }

        /* Progress text */
        {
            g_autofree gchar *progress_text = g_strdup_printf (
                "%" G_GUINT64_FORMAT " / %" G_GUINT64_FORMAT " years",
                self->current_year - self->start_year,
                self->target_year - self->start_year);
            gfloat text_width = grl_measure_text (progress_text, font_size_small);

            grl_draw_text (progress_text, center_x - text_width / 2,
                           bar_y + bar_height + padding / 2,
                           font_size_small, secondary_color);
        }
    }

    /* Draw event timeline */
    timeline_y = center_y + YEAR_FONT_SIZE + padding * 4;

    /* Timeline header */
    {
        gfloat header_width = grl_measure_text ("Event Timeline", font_size);
        grl_draw_text ("Event Timeline", center_x - header_width / 2, timeline_y,
                       font_size, text_color);
    }
    timeline_y += font_size + padding;

    /* Draw separator */
    grl_draw_line (x + padding * 2, timeline_y, x + width - padding * 2, timeline_y,
                   secondary_color);
    timeline_y += padding;

    /* Draw events */
    if (self->events != NULL && self->events->len > 0)
    {
        guint start_idx = 0;
        guint i;

        /* Only show last MAX_VISIBLE_EVENTS */
        if (self->events->len > MAX_VISIBLE_EVENTS)
            start_idx = self->events->len - MAX_VISIBLE_EVENTS;

        for (i = start_idx; i < self->events->len; i++)
        {
            TimelineEntry *entry = g_ptr_array_index (self->events, i);
            g_autofree gchar *line_text = g_strdup_printf (
                "• %" G_GUINT64_FORMAT ": %s", entry->year, entry->text);

            /* Calculate alpha based on age (newer events are brighter) */
            gfloat age_factor = 1.0f - (entry->age / EVENT_FADE_TIME);
            if (age_factor < 0.3f)
                age_factor = 0.3f;

            /* Key events use gold/cyan, normal events use secondary */
            if (entry->is_key_event)
            {
                guint8 alpha = (guint8)(age_factor * 255.0f);
                g_autoptr(GrlColor) event_color = grl_color_new (
                    grl_color_get_r (synergy_color),
                    grl_color_get_g (synergy_color),
                    grl_color_get_b (synergy_color),
                    alpha);
                grl_draw_text (line_text, x + padding * 2, timeline_y,
                               font_size, event_color);
            }
            else
            {
                guint8 alpha = (guint8)(age_factor * 200.0f);
                g_autoptr(GrlColor) event_color = grl_color_new (
                    grl_color_get_r (secondary_color),
                    grl_color_get_g (secondary_color),
                    grl_color_get_b (secondary_color),
                    alpha);
                grl_draw_text (line_text, x + padding * 2, timeline_y,
                               font_size, event_color);
            }

            timeline_y += font_size + padding / 2;
        }
    }
    else
    {
        grl_draw_text ("Awaiting events...", x + padding * 2, timeline_y,
                       font_size, secondary_color);
    }

    /* Draw acceleration hint at bottom */
    {
        const gchar *hint_text = self->is_accelerating
            ? "[Accelerating - Release to slow]"
            : "[Hold SPACE to accelerate]";
        gfloat hint_width = grl_measure_text (hint_text, font_size_small);
        const GrlColor *hint_color = self->is_accelerating ? gold_color : secondary_color;

        grl_draw_text (hint_text, center_x - hint_width / 2,
                       y + height - font_size_small - padding,
                       font_size_small, hint_color);
    }

    LRG_WIDGET_CLASS (lp_slumber_visualization_parent_class)->draw (widget);
}

static gboolean
lp_slumber_visualization_handle_event (LrgWidget        *widget,
                                       const LrgUIEvent *event)
{
    LpSlumberVisualization *self;
    LrgUIEventType event_type;
    GrlKey key;

    self = LP_SLUMBER_VISUALIZATION (widget);
    event_type = lrg_ui_event_get_event_type (event);

    if (!self->is_active)
        return FALSE;

    key = lrg_ui_event_get_key (event);

    /* Handle SPACE/Enter for acceleration */
    if (key == GRL_KEY_SPACE || key == GRL_KEY_ENTER)
    {
        if (event_type == LRG_UI_EVENT_KEY_DOWN)
        {
            lp_slumber_visualization_accelerate (self, TRUE);
            return TRUE;
        }
        else if (event_type == LRG_UI_EVENT_KEY_UP)
        {
            lp_slumber_visualization_accelerate (self, FALSE);
            return TRUE;
        }
    }

    return FALSE;
}

static void
lp_slumber_visualization_layout_children (LrgContainer *container)
{
    /* No child widgets to layout */
    (void)container;
}

static void
lp_slumber_visualization_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
    LpSlumberVisualization *self = LP_SLUMBER_VISUALIZATION (object);

    switch (prop_id)
    {
        case PROP_CURRENT_YEAR:
            g_value_set_uint64 (value, self->current_year);
            break;
        case PROP_TARGET_YEAR:
            g_value_set_uint64 (value, self->target_year);
            break;
        case PROP_SIMULATION_SPEED:
            g_value_set_float (value, self->simulation_speed);
            break;
        case PROP_IS_ACCELERATING:
            g_value_set_boolean (value, self->is_accelerating);
            break;
        case PROP_IS_ACTIVE:
            g_value_set_boolean (value, self->is_active);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_slumber_visualization_dispose (GObject *object)
{
    LpSlumberVisualization *self = LP_SLUMBER_VISUALIZATION (object);

    g_clear_pointer (&self->events, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_slumber_visualization_parent_class)->dispose (object);
}

static void
lp_slumber_visualization_class_init (LpSlumberVisualizationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_slumber_visualization_get_property;
    object_class->dispose = lp_slumber_visualization_dispose;

    widget_class->draw = lp_slumber_visualization_draw;
    widget_class->handle_event = lp_slumber_visualization_handle_event;
    container_class->layout_children = lp_slumber_visualization_layout_children;

    properties[PROP_CURRENT_YEAR] =
        g_param_spec_uint64 ("current-year", "Current Year",
                             "The currently displayed year",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_TARGET_YEAR] =
        g_param_spec_uint64 ("target-year", "Target Year",
                             "The target wake year",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_SIMULATION_SPEED] =
        g_param_spec_float ("simulation-speed", "Simulation Speed",
                            "Current simulation speed multiplier",
                            0.0f, G_MAXFLOAT, NORMAL_SPEED,
                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_IS_ACCELERATING] =
        g_param_spec_boolean ("is-accelerating", "Is Accelerating",
                              "Whether acceleration is active",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_IS_ACTIVE] =
        g_param_spec_boolean ("is-active", "Is Active",
                              "Whether the visualization is running",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_slumber_visualization_init (LpSlumberVisualization *self)
{
    self->start_year = 0;
    self->current_year = 0;
    self->target_year = 0;
    self->simulation_speed = NORMAL_SPEED;
    self->is_accelerating = FALSE;
    self->is_active = FALSE;
    self->events = g_ptr_array_new_with_free_func (timeline_entry_free);
    self->timeline_scroll = 0.0f;
    self->year_pulse = 0.0f;
}

LpSlumberVisualization *
lp_slumber_visualization_new (void)
{
    return g_object_new (LP_TYPE_SLUMBER_VISUALIZATION, NULL);
}

void
lp_slumber_visualization_start (LpSlumberVisualization *self,
                                guint64                 start_year,
                                guint64                 end_year)
{
    g_return_if_fail (LP_IS_SLUMBER_VISUALIZATION (self));

    self->start_year = start_year;
    self->current_year = start_year;
    self->target_year = end_year;
    self->is_active = TRUE;
    self->simulation_speed = NORMAL_SPEED;
    self->is_accelerating = FALSE;

    /* Clear old events */
    g_ptr_array_set_size (self->events, 0);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_YEAR]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_YEAR]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_ACTIVE]);
}

void
lp_slumber_visualization_stop (LpSlumberVisualization *self)
{
    g_return_if_fail (LP_IS_SLUMBER_VISUALIZATION (self));

    self->is_active = FALSE;
    self->is_accelerating = FALSE;
    self->simulation_speed = NORMAL_SPEED;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_ACTIVE]);
}

void
lp_slumber_visualization_add_event (LpSlumberVisualization *self,
                                    LpEvent                *event)
{
    TimelineEntry *entry;
    const gchar *name;

    g_return_if_fail (LP_IS_SLUMBER_VISUALIZATION (self));
    g_return_if_fail (LP_IS_EVENT (event));

    name = lp_event_get_name (event);

    entry = g_new0 (TimelineEntry, 1);
    entry->year = self->current_year;
    entry->text = g_strdup (name);
    entry->age = 0.0f;

    /* Mark major events as key events */
    entry->is_key_event = (lp_event_get_severity (event) >= LP_EVENT_SEVERITY_MAJOR);

    g_ptr_array_add (self->events, entry);
}

void
lp_slumber_visualization_set_year (LpSlumberVisualization *self,
                                   guint64                 year)
{
    g_return_if_fail (LP_IS_SLUMBER_VISUALIZATION (self));

    if (self->current_year != year)
    {
        self->current_year = year;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_YEAR]);
    }
}

guint64
lp_slumber_visualization_get_current_year (LpSlumberVisualization *self)
{
    g_return_val_if_fail (LP_IS_SLUMBER_VISUALIZATION (self), 0);
    return self->current_year;
}

guint64
lp_slumber_visualization_get_target_year (LpSlumberVisualization *self)
{
    g_return_val_if_fail (LP_IS_SLUMBER_VISUALIZATION (self), 0);
    return self->target_year;
}

void
lp_slumber_visualization_accelerate (LpSlumberVisualization *self,
                                     gboolean                accelerate)
{
    g_return_if_fail (LP_IS_SLUMBER_VISUALIZATION (self));

    if (self->is_accelerating != accelerate)
    {
        self->is_accelerating = accelerate;
        self->simulation_speed = accelerate ? ACCELERATED_SPEED : NORMAL_SPEED;

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_ACCELERATING]);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SIMULATION_SPEED]);
    }
}

gboolean
lp_slumber_visualization_is_accelerating (LpSlumberVisualization *self)
{
    g_return_val_if_fail (LP_IS_SLUMBER_VISUALIZATION (self), FALSE);
    return self->is_accelerating;
}

gfloat
lp_slumber_visualization_get_simulation_speed (LpSlumberVisualization *self)
{
    g_return_val_if_fail (LP_IS_SLUMBER_VISUALIZATION (self), NORMAL_SPEED);
    return self->simulation_speed;
}

gboolean
lp_slumber_visualization_is_active (LpSlumberVisualization *self)
{
    g_return_val_if_fail (LP_IS_SLUMBER_VISUALIZATION (self), FALSE);
    return self->is_active;
}

void
lp_slumber_visualization_clear_events (LpSlumberVisualization *self)
{
    g_return_if_fail (LP_IS_SLUMBER_VISUALIZATION (self));

    g_ptr_array_set_size (self->events, 0);
}

void
lp_slumber_visualization_update (LpSlumberVisualization *self,
                                 gfloat                  delta)
{
    guint i;

    g_return_if_fail (LP_IS_SLUMBER_VISUALIZATION (self));

    if (!self->is_active)
        return;

    /* Update year pulse animation */
    self->year_pulse += delta * 2.0f * G_PI;
    if (self->year_pulse > 2.0f * G_PI)
        self->year_pulse -= 2.0f * G_PI;

    /* Age all events */
    for (i = 0; i < self->events->len; i++)
    {
        TimelineEntry *entry = g_ptr_array_index (self->events, i);
        entry->age += delta;
    }
}
