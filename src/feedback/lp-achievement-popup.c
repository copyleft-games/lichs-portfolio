/* lp-achievement-popup.c - Achievement Notification Popup
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include <math.h>

#include "lp-achievement-popup.h"
#include "../ui/lp-theme.h"

/* Animation constants */
#define SLIDE_DURATION     0.3f
#define DEFAULT_TIMEOUT    5.0f
#define POPUP_WIDTH        280.0f
#define POPUP_HEIGHT       80.0f
#define POPUP_MARGIN       20.0f
#define BORDER_THICKNESS   2.0f
#define PULSE_SPEED        3.0f  /* Cycles per second */

typedef enum
{
    POPUP_STATE_HIDDEN,
    POPUP_STATE_SLIDING_IN,
    POPUP_STATE_VISIBLE,
    POPUP_STATE_SLIDING_OUT
} PopupState;

struct _LpAchievementPopup
{
    LrgContainer parent_instance;

    gchar      *name;
    gchar      *description;
    PopupState  state;
    gfloat      auto_dismiss_time;
    gfloat      elapsed;
    gfloat      visible_elapsed;
    gfloat      offset_x;
    gfloat      border_pulse;
};

enum
{
    PROP_0,
    PROP_NAME,
    PROP_DESCRIPTION,
    PROP_IS_VISIBLE,
    PROP_AUTO_DISMISS_TIME,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpAchievementPopup, lp_achievement_popup, LRG_TYPE_CONTAINER)

/*
 * ease_out_cubic:
 * @t: progress value 0.0 to 1.0
 *
 * Cubic ease-out function for smooth slide animation.
 */
static gfloat
ease_out_cubic (gfloat t)
{
    return 1.0f - powf (1.0f - t, 3.0f);
}

/*
 * ease_in_cubic:
 * @t: progress value 0.0 to 1.0
 *
 * Cubic ease-in function for smooth slide-out animation.
 */
static gfloat
ease_in_cubic (gfloat t)
{
    return t * t * t;
}

static void
lp_achievement_popup_draw (LrgWidget *widget)
{
    LpAchievementPopup *self;
    LrgTheme *theme;
    gfloat x, y, width, height;
    gfloat padding;
    gfloat font_size_large;
    gfloat font_size;
    gfloat font_size_small;
    const GrlColor *bg_color;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *gold_color;
    gfloat popup_x, popup_y;
    gfloat content_y;

    self = LP_ACHIEVEMENT_POPUP (widget);

    if (self->state == POPUP_STATE_HIDDEN)
        return;

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

    bg_color = lrg_theme_get_surface_color (theme);
    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    gold_color = lp_theme_get_gold_color ();

    (void)height;  /* May use later */

    /* Calculate popup position (top-right with offset) */
    popup_x = x + width - POPUP_WIDTH - POPUP_MARGIN + self->offset_x;
    popup_y = y + POPUP_MARGIN;

    /* Draw popup background */
    grl_draw_rectangle (popup_x, popup_y, POPUP_WIDTH, POPUP_HEIGHT, bg_color);

    /* Draw pulsing gold border */
    {
        guint8 border_alpha = (guint8)(180 + 75 * sinf (self->border_pulse));
        g_autoptr(GrlColor) border_color = grl_color_new (
            grl_color_get_r (gold_color),
            grl_color_get_g (gold_color),
            grl_color_get_b (gold_color),
            border_alpha);

        g_autoptr(GrlRectangle) border_rect = grl_rectangle_new (
            popup_x, popup_y, POPUP_WIDTH, POPUP_HEIGHT);
        grl_draw_rectangle_lines_ex (border_rect, BORDER_THICKNESS, border_color);
    }

    content_y = popup_y + padding;

    /* Draw "Achievement Unlocked!" header with star */
    {
        g_autofree gchar *header = g_strdup_printf ("★ Achievement Unlocked!");
        grl_draw_text (header, popup_x + padding, content_y,
                       font_size_small, gold_color);
    }
    content_y += font_size_small + padding / 2;

    /* Draw separator line */
    {
        gfloat line_y = content_y;
        g_autoptr(GrlColor) line_color = grl_color_new (
            grl_color_get_r (gold_color),
            grl_color_get_g (gold_color),
            grl_color_get_b (gold_color),
            128);
        grl_draw_line (popup_x + padding, line_y,
                       popup_x + POPUP_WIDTH - padding, line_y, line_color);
    }
    content_y += padding / 2;

    /* Draw achievement name */
    if (self->name != NULL)
    {
        grl_draw_text (self->name, popup_x + padding, content_y,
                       font_size_large, text_color);
        content_y += font_size_large + padding / 2;
    }

    /* Draw achievement description */
    if (self->description != NULL)
    {
        grl_draw_text (self->description, popup_x + padding, content_y,
                       font_size, secondary_color);
    }

    LRG_WIDGET_CLASS (lp_achievement_popup_parent_class)->draw (widget);
}

static gboolean
lp_achievement_popup_handle_event (LrgWidget        *widget,
                                   const LrgUIEvent *event)
{
    LpAchievementPopup *self;
    LrgUIEventType event_type;

    self = LP_ACHIEVEMENT_POPUP (widget);
    event_type = lrg_ui_event_get_event_type (event);

    /* Click anywhere to dismiss */
    if (event_type == LRG_UI_EVENT_MOUSE_BUTTON_DOWN)
    {
        if (self->state == POPUP_STATE_VISIBLE)
        {
            lp_achievement_popup_dismiss (self);
            return TRUE;
        }
    }

    return FALSE;
}

static void
lp_achievement_popup_layout_children (LrgContainer *container)
{
    /* No child widgets to layout */
    (void)container;
}

static void
lp_achievement_popup_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    LpAchievementPopup *self = LP_ACHIEVEMENT_POPUP (object);

    switch (prop_id)
    {
        case PROP_NAME:
            g_value_set_string (value, self->name);
            break;
        case PROP_DESCRIPTION:
            g_value_set_string (value, self->description);
            break;
        case PROP_IS_VISIBLE:
            g_value_set_boolean (value, self->state != POPUP_STATE_HIDDEN);
            break;
        case PROP_AUTO_DISMISS_TIME:
            g_value_set_float (value, self->auto_dismiss_time);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_achievement_popup_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    LpAchievementPopup *self = LP_ACHIEVEMENT_POPUP (object);

    switch (prop_id)
    {
        case PROP_AUTO_DISMISS_TIME:
            lp_achievement_popup_set_auto_dismiss_time (self, g_value_get_float (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_achievement_popup_dispose (GObject *object)
{
    LpAchievementPopup *self = LP_ACHIEVEMENT_POPUP (object);

    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->description, g_free);

    G_OBJECT_CLASS (lp_achievement_popup_parent_class)->dispose (object);
}

static void
lp_achievement_popup_class_init (LpAchievementPopupClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_achievement_popup_get_property;
    object_class->set_property = lp_achievement_popup_set_property;
    object_class->dispose = lp_achievement_popup_dispose;

    widget_class->draw = lp_achievement_popup_draw;
    widget_class->handle_event = lp_achievement_popup_handle_event;
    container_class->layout_children = lp_achievement_popup_layout_children;

    properties[PROP_NAME] =
        g_param_spec_string ("name", "Name",
                             "The achievement name",
                             NULL,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_DESCRIPTION] =
        g_param_spec_string ("description", "Description",
                             "The achievement description",
                             NULL,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_IS_VISIBLE] =
        g_param_spec_boolean ("is-visible", "Is Visible",
                              "Whether the popup is currently visible",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_AUTO_DISMISS_TIME] =
        g_param_spec_float ("auto-dismiss-time", "Auto Dismiss Time",
                            "Time in seconds before auto-dismiss (0 to disable)",
                            0.0f, G_MAXFLOAT, DEFAULT_TIMEOUT,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_achievement_popup_init (LpAchievementPopup *self)
{
    self->name = NULL;
    self->description = NULL;
    self->state = POPUP_STATE_HIDDEN;
    self->auto_dismiss_time = DEFAULT_TIMEOUT;
    self->elapsed = 0.0f;
    self->visible_elapsed = 0.0f;
    self->offset_x = POPUP_WIDTH + POPUP_MARGIN;  /* Start offscreen */
    self->border_pulse = 0.0f;
}

LpAchievementPopup *
lp_achievement_popup_new (void)
{
    return g_object_new (LP_TYPE_ACHIEVEMENT_POPUP, NULL);
}

void
lp_achievement_popup_show (LpAchievementPopup *self,
                           const gchar        *name,
                           const gchar        *description)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_POPUP (self));

    /* Set achievement info */
    g_free (self->name);
    g_free (self->description);
    self->name = g_strdup (name);
    self->description = g_strdup (description);

    /* Start slide-in animation */
    self->state = POPUP_STATE_SLIDING_IN;
    self->elapsed = 0.0f;
    self->visible_elapsed = 0.0f;
    self->offset_x = POPUP_WIDTH + POPUP_MARGIN;
    self->border_pulse = 0.0f;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESCRIPTION]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_VISIBLE]);
}

void
lp_achievement_popup_dismiss (LpAchievementPopup *self)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_POPUP (self));

    if (self->state == POPUP_STATE_HIDDEN ||
        self->state == POPUP_STATE_SLIDING_OUT)
        return;

    /* Start slide-out animation */
    self->state = POPUP_STATE_SLIDING_OUT;
    self->elapsed = 0.0f;
}

gboolean
lp_achievement_popup_is_visible (LpAchievementPopup *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_POPUP (self), FALSE);
    return self->state != POPUP_STATE_HIDDEN;
}

const gchar *
lp_achievement_popup_get_name (LpAchievementPopup *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_POPUP (self), NULL);
    return self->name;
}

const gchar *
lp_achievement_popup_get_description (LpAchievementPopup *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_POPUP (self), NULL);
    return self->description;
}

gfloat
lp_achievement_popup_get_auto_dismiss_time (LpAchievementPopup *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_POPUP (self), DEFAULT_TIMEOUT);
    return self->auto_dismiss_time;
}

void
lp_achievement_popup_set_auto_dismiss_time (LpAchievementPopup *self,
                                            gfloat              seconds)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_POPUP (self));

    if (self->auto_dismiss_time != seconds)
    {
        self->auto_dismiss_time = seconds;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AUTO_DISMISS_TIME]);
    }
}

void
lp_achievement_popup_update (LpAchievementPopup *self,
                             gfloat              delta)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_POPUP (self));

    if (self->state == POPUP_STATE_HIDDEN)
        return;

    /* Update border pulse animation */
    self->border_pulse += delta * PULSE_SPEED * 2.0f * G_PI;
    if (self->border_pulse > 2.0f * G_PI)
        self->border_pulse -= 2.0f * G_PI;

    switch (self->state)
    {
        case POPUP_STATE_SLIDING_IN:
            self->elapsed += delta;
            {
                gfloat progress = self->elapsed / SLIDE_DURATION;
                if (progress >= 1.0f)
                {
                    progress = 1.0f;
                    self->state = POPUP_STATE_VISIBLE;
                    self->visible_elapsed = 0.0f;
                }
                /* Slide from right (offset > 0) to position (offset = 0) */
                self->offset_x = (POPUP_WIDTH + POPUP_MARGIN) * (1.0f - ease_out_cubic (progress));
            }
            break;

        case POPUP_STATE_VISIBLE:
            self->visible_elapsed += delta;
            self->offset_x = 0.0f;

            /* Check for auto-dismiss */
            if (self->auto_dismiss_time > 0.0f &&
                self->visible_elapsed >= self->auto_dismiss_time)
            {
                lp_achievement_popup_dismiss (self);
            }
            break;

        case POPUP_STATE_SLIDING_OUT:
            self->elapsed += delta;
            {
                gfloat progress = self->elapsed / SLIDE_DURATION;
                if (progress >= 1.0f)
                {
                    progress = 1.0f;
                    self->state = POPUP_STATE_HIDDEN;
                    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_VISIBLE]);
                }
                /* Slide from position (offset = 0) to right (offset > 0) */
                self->offset_x = (POPUP_WIDTH + POPUP_MARGIN) * ease_in_cubic (progress);
            }
            break;

        case POPUP_STATE_HIDDEN:
        default:
            break;
    }
}
