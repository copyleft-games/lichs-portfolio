/* lp-dialog-event.c - Event Dialog
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-dialog-event.h"
#include "lp-theme.h"
#include "../simulation/lp-event.h"

struct _LpDialogEvent
{
    LrgContainer parent_instance;

    LpEvent *event;
    gint     selected_choice;
    guint    choice_count;
};

enum
{
    PROP_0,
    PROP_EVENT,
    PROP_SELECTED_CHOICE,
    N_PROPS
};

enum
{
    SIGNAL_CHOICE_CONFIRMED,
    SIGNAL_DISMISSED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpDialogEvent, lp_dialog_event, LRG_TYPE_CONTAINER)

static void
lp_dialog_event_draw (LrgWidget *widget)
{
    LpDialogEvent *self;
    LrgTheme *theme;
    gfloat x, y, width, height;
    gfloat padding;
    gfloat font_size_large;
    gfloat font_size;
    gfloat font_size_small;
    const GrlColor *bg_color;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *border_color;
    const GrlColor *accent_color;
    gfloat dialog_width, dialog_height, dialog_x, dialog_y;
    gfloat content_y;

    self = LP_DIALOG_EVENT (widget);
    theme = lrg_theme_get_default ();

    /* Get widget position and size */
    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget);
    width = lrg_widget_get_width (widget);
    height = lrg_widget_get_height (widget);

    padding = lrg_theme_get_padding_large (theme);
    font_size_large = lrg_theme_get_font_size_large (theme);
    font_size = lrg_theme_get_font_size_normal (theme);
    font_size_small = lrg_theme_get_font_size_small (theme);

    bg_color = lrg_theme_get_surface_color (theme);
    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    border_color = lrg_theme_get_border_color (theme);
    accent_color = lrg_theme_get_accent_color (theme);

    dialog_width = MIN (width * 0.8f, 600.0f);
    dialog_height = MIN (height * 0.7f, 400.0f);
    dialog_x = x + (width - dialog_width) / 2;
    dialog_y = y + (height - dialog_height) / 2;

    (void)border_color; /* May be used later */

    /* Draw dimmed background */
    {
        g_autoptr(GrlColor) dim_color = grl_color_new (0, 0, 0, 180);
        grl_draw_rectangle (x, y, width, height, dim_color);
    }

    /* Draw dialog box */
    grl_draw_rectangle (dialog_x, dialog_y, dialog_width, dialog_height, bg_color);
    {
        g_autoptr(GrlRectangle) dlg_rect = grl_rectangle_new (
            dialog_x, dialog_y, dialog_width, dialog_height);
        grl_draw_rectangle_lines_ex (dlg_rect, 2.0f, accent_color);
    }

    content_y = dialog_y + padding;

    if (self->event == NULL)
    {
        grl_draw_text ("No event to display",
                       dialog_x + padding, content_y,
                       font_size, secondary_color);
    }
    else
    {
        const gchar *name = lp_event_get_name (self->event);
        const gchar *description = lp_event_get_description (self->event);
        GPtrArray *choices;

        /* Draw event title */
        grl_draw_text (name, dialog_x + padding, content_y,
                       font_size_large, text_color);
        content_y += font_size_large + padding;

        /* Draw separator */
        grl_draw_line (dialog_x + padding, content_y,
                       dialog_x + dialog_width - padding, content_y, accent_color);
        content_y += padding;

        /* Draw description */
        grl_draw_text (description, dialog_x + padding, content_y,
                       font_size, secondary_color);
        content_y += font_size * 3 + padding * 2;

        /* Draw choices */
        choices = lp_event_get_choices (self->event);
        if (choices != NULL && choices->len > 0)
        {
            guint i;

            grl_draw_text ("Choose your response:", dialog_x + padding, content_y,
                           font_size, text_color);
            content_y += font_size + padding;

            for (i = 0; i < choices->len && i < 4; i++)
            {
                LpEventChoice *choice = g_ptr_array_index (choices, i);
                gboolean selected = ((gint)i == self->selected_choice);
                g_autofree gchar *label = g_strdup_printf ("[%u] %s", i + 1, choice->text);

                if (selected)
                {
                    grl_draw_rectangle (dialog_x + padding / 2, content_y,
                                         dialog_width - padding, font_size + padding,
                                         accent_color);
                    grl_draw_text (label, dialog_x + padding, content_y + padding / 2,
                                   font_size, bg_color);
                }
                else
                {
                    grl_draw_text (label, dialog_x + padding, content_y + padding / 2,
                                   font_size, text_color);
                }

                content_y += font_size + padding;
            }
        }
        else
        {
            /* No choices - just acknowledgement */
            grl_draw_text ("[Enter] Acknowledge", dialog_x + padding, content_y,
                           font_size, secondary_color);
        }
    }

    /* Draw instructions at bottom */
    grl_draw_text ("[1-4] Select  [Enter] Confirm  [Esc] Dismiss",
                   dialog_x + padding,
                   dialog_y + dialog_height - font_size_small - padding,
                   font_size_small, secondary_color);

    LRG_WIDGET_CLASS (lp_dialog_event_parent_class)->draw (widget);
}

static gboolean
lp_dialog_event_handle_event (LrgWidget        *widget,
                               const LrgUIEvent *event)
{
    LpDialogEvent *self;
    LrgUIEventType event_type;
    GrlKey key;

    self = LP_DIALOG_EVENT (widget);
    event_type = lrg_ui_event_get_event_type (event);

    if (event_type != LRG_UI_EVENT_KEY_DOWN)
        return FALSE;

    key = lrg_ui_event_get_key (event);

    /* Number keys 1-4 for choice selection */
    if (key >= GRL_KEY_ONE && key <= GRL_KEY_FOUR)
    {
        gint index = key - GRL_KEY_ONE;
        if (index < (gint)self->choice_count)
        {
            self->selected_choice = index;
            g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SELECTED_CHOICE]);
            return TRUE;
        }
    }

    switch (key)
    {
        case GRL_KEY_UP:
            if (self->selected_choice > 0)
            {
                self->selected_choice--;
                g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SELECTED_CHOICE]);
            }
            return TRUE;

        case GRL_KEY_DOWN:
            if (self->selected_choice < (gint)self->choice_count - 1)
            {
                self->selected_choice++;
                g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SELECTED_CHOICE]);
            }
            return TRUE;

        case GRL_KEY_ENTER:
            g_signal_emit (self, signals[SIGNAL_CHOICE_CONFIRMED], 0, self->selected_choice);
            return TRUE;

        case GRL_KEY_ESCAPE:
            g_signal_emit (self, signals[SIGNAL_DISMISSED], 0);
            return TRUE;

        default:
            break;
    }

    return FALSE;
}

static void
lp_dialog_event_layout_children (LrgContainer *container)
{
    /* No child widgets */
    (void)container;
}

static void
lp_dialog_event_get_property (GObject *object, guint prop_id,
                               GValue *value, GParamSpec *pspec)
{
    LpDialogEvent *self = LP_DIALOG_EVENT (object);

    switch (prop_id)
    {
        case PROP_EVENT:
            g_value_set_object (value, self->event);
            break;
        case PROP_SELECTED_CHOICE:
            g_value_set_int (value, self->selected_choice);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_dialog_event_set_property (GObject *object, guint prop_id,
                               const GValue *value, GParamSpec *pspec)
{
    LpDialogEvent *self = LP_DIALOG_EVENT (object);

    switch (prop_id)
    {
        case PROP_EVENT:
            lp_dialog_event_set_event (self, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_dialog_event_dispose (GObject *object)
{
    LpDialogEvent *self = LP_DIALOG_EVENT (object);

    g_clear_object (&self->event);

    G_OBJECT_CLASS (lp_dialog_event_parent_class)->dispose (object);
}

static void
lp_dialog_event_class_init (LpDialogEventClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_dialog_event_get_property;
    object_class->set_property = lp_dialog_event_set_property;
    object_class->dispose = lp_dialog_event_dispose;

    widget_class->draw = lp_dialog_event_draw;
    widget_class->handle_event = lp_dialog_event_handle_event;
    container_class->layout_children = lp_dialog_event_layout_children;

    properties[PROP_EVENT] =
        g_param_spec_object ("event", "Event",
                             "The event to display",
                             LP_TYPE_EVENT,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_SELECTED_CHOICE] =
        g_param_spec_int ("selected-choice", "Selected Choice",
                          "The selected choice index",
                          -1, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    signals[SIGNAL_CHOICE_CONFIRMED] =
        g_signal_new ("choice-confirmed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_INT);

    signals[SIGNAL_DISMISSED] =
        g_signal_new ("dismissed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_dialog_event_init (LpDialogEvent *self)
{
    self->event = NULL;
    self->selected_choice = 0;
    self->choice_count = 0;
}

LpDialogEvent *
lp_dialog_event_new (void)
{
    return g_object_new (LP_TYPE_DIALOG_EVENT, NULL);
}

LpEvent *
lp_dialog_event_get_event (LpDialogEvent *self)
{
    g_return_val_if_fail (LP_IS_DIALOG_EVENT (self), NULL);
    return self->event;
}

void
lp_dialog_event_set_event (LpDialogEvent *self,
                            LpEvent       *event)
{
    GPtrArray *choices;

    g_return_if_fail (LP_IS_DIALOG_EVENT (self));

    if (g_set_object (&self->event, event))
    {
        /* Update choice count */
        if (event != NULL)
        {
            choices = lp_event_get_choices (event);
            self->choice_count = (choices != NULL) ? choices->len : 0;
        }
        else
        {
            self->choice_count = 0;
        }

        self->selected_choice = 0;

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EVENT]);
    }
}

gint
lp_dialog_event_get_selected_choice (LpDialogEvent *self)
{
    g_return_val_if_fail (LP_IS_DIALOG_EVENT (self), -1);
    return self->selected_choice;
}

void
lp_dialog_event_select_choice (LpDialogEvent *self,
                                gint           index)
{
    g_return_if_fail (LP_IS_DIALOG_EVENT (self));

    if (index >= 0 && index < (gint)self->choice_count)
    {
        self->selected_choice = index;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SELECTED_CHOICE]);
    }
}

void
lp_dialog_event_confirm_choice (LpDialogEvent *self)
{
    g_return_if_fail (LP_IS_DIALOG_EVENT (self));

    g_signal_emit (self, signals[SIGNAL_CHOICE_CONFIRMED], 0, self->selected_choice);
}
