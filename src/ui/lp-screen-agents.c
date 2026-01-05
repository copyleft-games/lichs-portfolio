/* lp-screen-agents.c - Agent Management Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-screen-agents.h"
#include "lp-theme.h"
#include "../agent/lp-agent-manager.h"
#include "../agent/lp-agent.h"

struct _LpScreenAgents
{
    LrgContainer parent_instance;

    LpAgentManager *manager;
    LpAgent        *selected_agent;
    gint            selection_index;
    GPtrArray      *displayed_agents;

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

enum
{
    PROP_0,
    PROP_MANAGER,
    PROP_SELECTED_AGENT,
    N_PROPS
};

enum
{
    SIGNAL_AGENT_SELECTED,
    SIGNAL_RECRUIT_REQUESTED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpScreenAgents, lp_screen_agents, LRG_TYPE_CONTAINER)

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
get_pool_label (LpScreenAgents *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpScreenAgents *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Widget Draw
 * ========================================================================== */

static void
lp_screen_agents_draw (LrgWidget *widget)
{
    LpScreenAgents *self;
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

    self = LP_SCREEN_AGENTS (widget);

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

    draw_label (get_pool_label (self), "Agents",
                x + padding, y + padding,
                font_size_large, text_color);

    /* Draw agent count */
    if (self->manager != NULL)
    {
        guint count = lp_agent_manager_get_agent_count (self->manager);
        g_autofree gchar *count_text = g_strdup_printf ("Active: %u", count);
        gfloat text_width = grl_measure_text (count_text, font_size);

        draw_label (get_pool_label (self), count_text,
                    x + width - text_width - padding,
                    y + padding + (font_size_large - font_size) / 2,
                    font_size, secondary_color);
    }

    /* Draw placeholder list */
    draw_label (get_pool_label (self), "Agent management - coming soon",
                x + padding,
                y + header_height + padding,
                font_size, secondary_color);

    LRG_WIDGET_CLASS (lp_screen_agents_parent_class)->draw (widget);
}

static void
lp_screen_agents_layout_children (LrgContainer *container)
{
    /* No child widgets to layout yet */
    (void)container;
}

static void
lp_screen_agents_get_property (GObject *object, guint prop_id,
                                GValue *value, GParamSpec *pspec)
{
    LpScreenAgents *self = LP_SCREEN_AGENTS (object);

    switch (prop_id)
    {
        case PROP_MANAGER:
            g_value_set_object (value, self->manager);
            break;
        case PROP_SELECTED_AGENT:
            g_value_set_object (value, self->selected_agent);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_agents_set_property (GObject *object, guint prop_id,
                                const GValue *value, GParamSpec *pspec)
{
    LpScreenAgents *self = LP_SCREEN_AGENTS (object);

    switch (prop_id)
    {
        case PROP_MANAGER:
            lp_screen_agents_set_manager (self, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_agents_dispose (GObject *object)
{
    LpScreenAgents *self = LP_SCREEN_AGENTS (object);

    g_clear_object (&self->manager);
    self->selected_agent = NULL;

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_screen_agents_parent_class)->dispose (object);
}

static void
lp_screen_agents_finalize (GObject *object)
{
    LpScreenAgents *self = LP_SCREEN_AGENTS (object);

    g_clear_pointer (&self->displayed_agents, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_screen_agents_parent_class)->finalize (object);
}

static void
lp_screen_agents_class_init (LpScreenAgentsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_screen_agents_get_property;
    object_class->set_property = lp_screen_agents_set_property;
    object_class->dispose = lp_screen_agents_dispose;
    object_class->finalize = lp_screen_agents_finalize;

    widget_class->draw = lp_screen_agents_draw;
    container_class->layout_children = lp_screen_agents_layout_children;

    properties[PROP_MANAGER] =
        g_param_spec_object ("manager", "Manager",
                             "The agent manager to display",
                             LP_TYPE_AGENT_MANAGER,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_SELECTED_AGENT] =
        g_param_spec_object ("selected-agent", "Selected Agent",
                             "The currently selected agent",
                             LP_TYPE_AGENT,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    signals[SIGNAL_AGENT_SELECTED] =
        g_signal_new ("agent-selected",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, LP_TYPE_AGENT);

    signals[SIGNAL_RECRUIT_REQUESTED] =
        g_signal_new ("recruit-requested",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_screen_agents_init (LpScreenAgents *self)
{
    guint i;

    self->manager = NULL;
    self->selected_agent = NULL;
    self->selection_index = -1;
    self->displayed_agents = g_ptr_array_new ();

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 10; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

LpScreenAgents *
lp_screen_agents_new (void)
{
    return g_object_new (LP_TYPE_SCREEN_AGENTS, NULL);
}

LpAgentManager *
lp_screen_agents_get_manager (LpScreenAgents *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_AGENTS (self), NULL);
    return self->manager;
}

void
lp_screen_agents_set_manager (LpScreenAgents *self,
                               LpAgentManager *manager)
{
    g_return_if_fail (LP_IS_SCREEN_AGENTS (self));

    if (g_set_object (&self->manager, manager))
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MANAGER]);
}

LpAgent *
lp_screen_agents_get_selected_agent (LpScreenAgents *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_AGENTS (self), NULL);
    return self->selected_agent;
}

void
lp_screen_agents_refresh (LpScreenAgents *self)
{
    g_return_if_fail (LP_IS_SCREEN_AGENTS (self));
    /* Redraw will happen automatically via property notifications */
}
