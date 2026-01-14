/* lp-state-agents.c - Agent Management Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-agents.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../investment/lp-portfolio.h"
#include "../agent/lp-agent-manager.h"
#include "../agent/lp-agent.h"
#include "../agent/lp-agent-individual.h"
#include "../lp-input-helpers.h"
#include <graylib.h>
#include <libregnum.h>

/* Agent recruitment costs */
#define RECRUIT_BASE_COST (200.0)

/* Maximum items visible in list */
#define MAX_VISIBLE_ITEMS (6)

/* Names for random agent generation */
static const gchar *first_names[] = {
    "Marcus", "Helena", "Aldric", "Beatrice", "Conrad",
    "Diana", "Edmund", "Fiona", "Gerald", "Isolde",
    "Julian", "Katrina", "Leopold", "Miriam", "Nikolai",
    "Ophelia", "Percival", "Rosalind", "Sebastian", "Theodora"
};

static const gchar *surnames[] = {
    "Blackwood", "Thornton", "Ashworth", "Greymoor", "Silverton",
    "Ironforge", "Nightshade", "Stormwind", "Goldwater", "Darkholme",
    "Ravenscroft", "Whitmore", "Coldwell", "Brightstone", "Shadowmere"
};

#define NUM_FIRST_NAMES (G_N_ELEMENTS(first_names))
#define NUM_SURNAMES (G_N_ELEMENTS(surnames))

typedef enum
{
    VIEW_MODE_AGENTS,       /* Viewing current agents */
    VIEW_MODE_RECRUIT       /* Recruiting new agents */
} ViewMode;

struct _LpStateAgents
{
    LrgGameState parent_instance;

    ViewMode view_mode;       /* Current view mode */
    gint     selected_index;  /* Currently selected item */
    gint     scroll_offset;   /* Scroll offset for long lists */

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpStateAgents, lp_state_agents, LRG_TYPE_GAME_STATE)

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
get_pool_label (LpStateAgents *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateAgents *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_agents_dispose (GObject *object)
{
    LpStateAgents *self = LP_STATE_AGENTS (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_agents_parent_class)->dispose (object);
}

/* ==========================================================================
 * Helper Functions
 * ========================================================================== */

static const gchar *
agent_type_to_string (LpAgentType agent_type)
{
    switch (agent_type)
    {
    case LP_AGENT_TYPE_INDIVIDUAL:
        return "Individual";
    case LP_AGENT_TYPE_FAMILY:
        return "Family";
    case LP_AGENT_TYPE_CULT:
        return "Cult";
    case LP_AGENT_TYPE_BOUND:
        return "Bound";
    default:
        return "Unknown";
    }
}

static const gchar *
cover_status_to_string (LpCoverStatus status)
{
    switch (status)
    {
    case LP_COVER_STATUS_SECURE:
        return "Secure";
    case LP_COVER_STATUS_SUSPICIOUS:
        return "Suspicious";
    case LP_COVER_STATUS_COMPROMISED:
        return "Compromised";
    case LP_COVER_STATUS_EXPOSED:
        return "Exposed";
    default:
        return "Unknown";
    }
}

static gchar *
generate_random_name (void)
{
    const gchar *first = first_names[g_random_int_range (0, NUM_FIRST_NAMES)];
    const gchar *last = surnames[g_random_int_range (0, NUM_SURNAMES)];
    return g_strdup_printf ("%s %s", first, last);
}

static LpAgentIndividual *
create_random_agent (void)
{
    g_autofree gchar *name = NULL;
    g_autofree gchar *id = NULL;
    guint age, max_age;
    gint loyalty, competence;

    name = generate_random_name ();
    id = g_strdup_printf ("agent-%d", g_random_int_range (10000, 99999));

    /* Generate random stats */
    age = g_random_int_range (20, 40);          /* Starting age 20-39 */
    max_age = g_random_int_range (55, 80);      /* Max age 55-79 */
    loyalty = g_random_int_range (40, 80);      /* Loyalty 40-79 */
    competence = g_random_int_range (30, 70);   /* Competence 30-69 */

    return lp_agent_individual_new_full (id, name, age, max_age, loyalty, competence);
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_agents_enter (LrgGameState *state)
{
    LpStateAgents *self = LP_STATE_AGENTS (state);

    lp_log_info ("Entering agents state");

    self->view_mode = VIEW_MODE_AGENTS;
    self->selected_index = 0;
    self->scroll_offset = 0;
}

static void
lp_state_agents_exit (LrgGameState *state)
{
    lp_log_info ("Exiting agents state");
}

static void
lp_state_agents_update (LrgGameState *state,
                        gdouble       delta)
{
    LpStateAgents *self = LP_STATE_AGENTS (state);
    LpGame *game = lp_game_get_from_state (state);
    LpGameData *game_data = lp_game_get_game_data (game);
    LpAgentManager *agent_manager = lp_game_data_get_agent_manager (game_data);
    LpPortfolio *portfolio = lp_game_data_get_portfolio (game_data);
    LrgGameStateManager *manager;
    guint max_items;

    (void)delta;

    /* Determine max items based on view mode */
    if (self->view_mode == VIEW_MODE_AGENTS)
    {
        max_items = lp_agent_manager_get_agent_count (agent_manager);
    }
    else
    {
        max_items = 3; /* Number of recruit options */
    }

    /* Navigation: Up/Down (including vim keys and gamepad D-pad) */
    if (LP_INPUT_NAV_UP_PRESSED ())
    {
        if (self->selected_index > 0)
        {
            self->selected_index--;
            if (self->selected_index < self->scroll_offset)
            {
                self->scroll_offset = self->selected_index;
            }
        }
    }

    if (LP_INPUT_NAV_DOWN_PRESSED ())
    {
        if (max_items > 0 && self->selected_index < (gint)(max_items - 1))
        {
            self->selected_index++;
            if (self->selected_index >= self->scroll_offset + MAX_VISIBLE_ITEMS)
            {
                self->scroll_offset = self->selected_index - MAX_VISIBLE_ITEMS + 1;
            }
        }
    }

    /* Tab/H/L/LB/RB to switch views */
    if (grl_input_is_key_pressed (GRL_KEY_TAB) ||
        grl_input_is_key_pressed (GRL_KEY_H) ||
        grl_input_is_key_pressed (GRL_KEY_L) ||
        LP_INPUT_TAB_NEXT_PRESSED () ||
        LP_INPUT_TAB_PREV_PRESSED ())
    {
        if (self->view_mode == VIEW_MODE_AGENTS)
        {
            self->view_mode = VIEW_MODE_RECRUIT;
        }
        else
        {
            self->view_mode = VIEW_MODE_AGENTS;
        }
        self->selected_index = 0;
        self->scroll_offset = 0;
        lp_log_info ("Switched to %s view",
                     self->view_mode == VIEW_MODE_AGENTS ? "agents" : "recruit");
    }

    /* Enter/A button to recruit or view details */
    if (LP_INPUT_CONFIRM_PRESSED ())
    {
        if (self->view_mode == VIEW_MODE_RECRUIT)
        {
            /* Recruit new agent */
            gdouble cost = RECRUIT_BASE_COST;
            g_autoptr(LrgBigNumber) cost_bn = lrg_big_number_new (cost);

            if (lp_portfolio_can_afford (portfolio, cost_bn))
            {
                LpAgentIndividual *new_agent = create_random_agent ();

                if (new_agent != NULL)
                {
                    lp_portfolio_subtract_gold (portfolio, cost_bn);
                    lp_agent_manager_add_agent (agent_manager, LP_AGENT (new_agent));
                    lp_log_info ("Recruited new agent: %s for %.0f gold",
                                 lp_agent_get_name (LP_AGENT (new_agent)), cost);
                }
            }
            else
            {
                lp_log_info ("Cannot afford to recruit (cost: %.0f gold)", cost);
            }
        }
        else
        {
            /* View agent details (just log for now) */
            GPtrArray *agents = lp_agent_manager_get_agents (agent_manager);
            if (agents != NULL &&
                self->selected_index >= 0 &&
                self->selected_index < (gint)agents->len)
            {
                LpAgent *agent = g_ptr_array_index (agents, self->selected_index);
                lp_log_info ("Selected agent: %s (Loyalty: %d, Competence: %d)",
                             lp_agent_get_name (agent),
                             lp_agent_get_loyalty (agent),
                             lp_agent_get_competence (agent));
            }
        }
    }

    /* ESC/B button to return to analyze */
    if (LP_INPUT_CANCEL_PRESSED ())
    {
        lp_log_info ("Returning to analyze state");

        manager = lrg_game_template_get_state_manager (LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_pop (manager);
    }
}

static void
lp_state_agents_draw (LrgGameState *state)
{
    LpStateAgents *self = LP_STATE_AGENTS (state);
    LpGame *game = lp_game_get_from_state (state);
    LpGameData *game_data = lp_game_get_game_data (game);
    LpAgentManager *agent_manager = lp_game_data_get_agent_manager (game_data);
    LpPortfolio *portfolio = lp_game_data_get_portfolio (game_data);
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) gold_color = NULL;
    g_autoptr(GrlColor) panel_color = NULL;
    g_autoptr(GrlColor) selected_color = NULL;
    g_autoptr(GrlColor) tab_active_color = NULL;
    g_autoptr(GrlColor) tab_inactive_color = NULL;
    g_autoptr(GrlColor) loyalty_high_color = NULL;
    g_autoptr(GrlColor) loyalty_low_color = NULL;
    gint screen_w, screen_h;
    gint center_x;
    gint margin, header_h;
    gint panel_x, panel_y, panel_w, panel_h;
    gint list_y, item_h;
    gchar str_buf[128];
    guint count;
    gint avg_loyalty, avg_competence;
    g_autoptr(LrgBigNumber) cost_bn = NULL;
    gboolean can_afford;
    gint option_y;
    gboolean is_selected;

    /* Reset label pool for this frame */
    reset_label_pool (self);

    /* Get virtual resolution */
    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
    center_x = screen_w / 2;

    /* Layout */
    margin = 30;
    header_h = 80;
    panel_x = margin;
    panel_y = header_h + margin;
    panel_w = screen_w - (margin * 2);
    panel_h = screen_h - header_h - (margin * 3);
    list_y = panel_y + 100;
    item_h = 60;

    /* Colors */
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    gold_color = grl_color_new (255, 215, 0, 255);
    panel_color = grl_color_new (25, 25, 35, 255);
    selected_color = grl_color_new (60, 50, 80, 255);
    tab_active_color = grl_color_new (100, 80, 140, 255);
    tab_inactive_color = grl_color_new (40, 40, 50, 255);
    loyalty_high_color = grl_color_new (100, 200, 100, 255);
    loyalty_low_color = grl_color_new (200, 100, 100, 255);

    /* Draw header */
    draw_label (self->label_title, "AGENT NETWORK", center_x - 150, 30, 36, title_color);

    /* Draw gold display */
    if (portfolio != NULL)
    {
        LrgBigNumber *gold = lp_portfolio_get_gold (portfolio);
        g_snprintf (str_buf, sizeof (str_buf), "Gold: %.0f gp",
                    lrg_big_number_to_double (gold));
    }
    else
    {
        g_snprintf (str_buf, sizeof (str_buf), "Gold: -- gp");
    }
    draw_label (get_pool_label (self), str_buf, screen_w - 250, 35, 20, gold_color);

    /* Draw main panel */
    grl_draw_rectangle (panel_x, panel_y, panel_w, panel_h, panel_color);

    /* Draw tabs */
    grl_draw_rectangle (panel_x + 10, panel_y + 10, 150, 35,
                        self->view_mode == VIEW_MODE_AGENTS ? tab_active_color : tab_inactive_color);
    draw_label (get_pool_label (self), "My Agents", panel_x + 35, panel_y + 17, 18, text_color);

    grl_draw_rectangle (panel_x + 170, panel_y + 10, 150, 35,
                        self->view_mode == VIEW_MODE_RECRUIT ? tab_active_color : tab_inactive_color);
    draw_label (get_pool_label (self), "Recruit", panel_x + 210, panel_y + 17, 18, text_color);

    /* Draw stats summary */
    if (agent_manager != NULL)
    {
        count = lp_agent_manager_get_agent_count (agent_manager);
        avg_loyalty = lp_agent_manager_get_average_loyalty (agent_manager);
        avg_competence = lp_agent_manager_get_average_competence (agent_manager);

        g_snprintf (str_buf, sizeof (str_buf), "Total Agents: %u", count);
        draw_label (get_pool_label (self), str_buf, panel_x + 400, panel_y + 20, 16, text_color);

        if (avg_loyalty >= 0)
        {
            g_snprintf (str_buf, sizeof (str_buf), "Avg Loyalty: %d", avg_loyalty);
            draw_label (get_pool_label (self), str_buf, panel_x + 550, panel_y + 20, 16,
                        avg_loyalty >= 50 ? loyalty_high_color : loyalty_low_color);
        }

        if (avg_competence >= 0)
        {
            g_snprintf (str_buf, sizeof (str_buf), "Avg Competence: %d", avg_competence);
            draw_label (get_pool_label (self), str_buf, panel_x + 700, panel_y + 20, 16, text_color);
        }
    }

    if (self->view_mode == VIEW_MODE_AGENTS)
    {
        GPtrArray *agents;
        guint i, idx, visible_count;

        /* Draw column headers */
        draw_label (get_pool_label (self), "Name", panel_x + 20, panel_y + 65, 16, dim_color);
        draw_label (get_pool_label (self), "Type", panel_x + 250, panel_y + 65, 16, dim_color);
        draw_label (get_pool_label (self), "Age", panel_x + 370, panel_y + 65, 16, dim_color);
        draw_label (get_pool_label (self), "Loyalty", panel_x + 450, panel_y + 65, 16, dim_color);
        draw_label (get_pool_label (self), "Competence", panel_x + 550, panel_y + 65, 16, dim_color);
        draw_label (get_pool_label (self), "Cover", panel_x + 680, panel_y + 65, 16, dim_color);

        /* Draw agents list */
        agents = (agent_manager != NULL) ? lp_agent_manager_get_agents (agent_manager) : NULL;
        count = (agents != NULL) ? agents->len : 0;

        if (count == 0)
        {
            draw_label (get_pool_label (self), "No agents in your network. Press TAB to recruit.",
                        panel_x + 50, list_y + 50, 18, dim_color);
        }
        else
        {
            visible_count = MIN (count - self->scroll_offset, MAX_VISIBLE_ITEMS);
            for (i = 0; i < visible_count; i++)
            {
                LpAgent *agent;
                gint agent_item_y;
                gboolean agent_selected;
                gint loyalty;
                guint years_left;

                idx = self->scroll_offset + i;
                agent = g_ptr_array_index (agents, idx);
                agent_item_y = list_y + (i * item_h);
                agent_selected = ((gint)idx == self->selected_index);
                loyalty = lp_agent_get_loyalty (agent);

                /* Draw selection highlight */
                if (agent_selected)
                {
                    grl_draw_rectangle (panel_x + 10, agent_item_y - 3,
                                        panel_w - 20, item_h - 2, selected_color);
                }

                /* Draw agent details */
                draw_label (get_pool_label (self), lp_agent_get_name (agent),
                            panel_x + 20, agent_item_y, 18,
                            agent_selected ? gold_color : text_color);

                draw_label (get_pool_label (self), agent_type_to_string (lp_agent_get_agent_type (agent)),
                            panel_x + 250, agent_item_y, 16, text_color);

                g_snprintf (str_buf, sizeof (str_buf), "%u/%u",
                            lp_agent_get_age (agent), lp_agent_get_max_age (agent));
                draw_label (get_pool_label (self), str_buf, panel_x + 370, agent_item_y, 16, text_color);

                g_snprintf (str_buf, sizeof (str_buf), "%d", loyalty);
                draw_label (get_pool_label (self), str_buf, panel_x + 450, agent_item_y, 16,
                            loyalty >= 50 ? loyalty_high_color : loyalty_low_color);

                g_snprintf (str_buf, sizeof (str_buf), "%d", lp_agent_get_competence (agent));
                draw_label (get_pool_label (self), str_buf, panel_x + 550, agent_item_y, 16, text_color);

                draw_label (get_pool_label (self), cover_status_to_string (lp_agent_get_cover_status (agent)),
                            panel_x + 680, agent_item_y, 16, dim_color);

                /* Second line: years remaining */
                years_left = lp_agent_get_years_remaining (agent);
                g_snprintf (str_buf, sizeof (str_buf), "%u years of service remaining", years_left);
                draw_label (get_pool_label (self), str_buf, panel_x + 40, agent_item_y + 22, 14, dim_color);
            }
        }
    }
    else
    {
        /* Draw recruitment options */
        cost_bn = lrg_big_number_new (RECRUIT_BASE_COST);
        can_afford = portfolio != NULL && lp_portfolio_can_afford (portfolio, cost_bn);

        draw_label (get_pool_label (self), "Recruit new agents to manage your investments and gather intelligence.",
                    panel_x + 20, panel_y + 70, 16, text_color);

        /* Recruitment option */
        option_y = list_y;
        is_selected = (self->selected_index == 0);

        if (is_selected)
        {
            grl_draw_rectangle (panel_x + 10, option_y - 3,
                                panel_w - 20, item_h - 2, selected_color);
        }

        draw_label (get_pool_label (self), "Recruit Individual Agent",
                    panel_x + 20, option_y, 20,
                    is_selected ? gold_color : (can_afford ? text_color : dim_color));

        draw_label (get_pool_label (self), "A mortal servant who will manage investments and can train successors.",
                    panel_x + 40, option_y + 25, 14, dim_color);

        g_snprintf (str_buf, sizeof (str_buf), "Cost: %.0f gp", RECRUIT_BASE_COST);
        draw_label (get_pool_label (self), str_buf, panel_x + 600, option_y, 16,
                    can_afford ? gold_color : dim_color);

        /* Future options (disabled) */
        option_y += item_h + 10;
        draw_label (get_pool_label (self), "Recruit Noble Family (Locked)", panel_x + 20, option_y, 20, dim_color);
        draw_label (get_pool_label (self), "Establish a bloodline of loyal servants. Requires: 5000 gp, Phylactery Upgrade",
                    panel_x + 40, option_y + 25, 14, dim_color);

        option_y += item_h + 10;
        draw_label (get_pool_label (self), "Found Secret Cult (Locked)", panel_x + 20, option_y, 20, dim_color);
        draw_label (get_pool_label (self), "Create an organization devoted to your will. Requires: 10000 gp, Dark Arts Mastery",
                    panel_x + 40, option_y + 25, 14, dim_color);
    }

    /* Draw instructions */
    draw_label (get_pool_label (self), "[UP/DOWN] Select    [TAB] Switch View    [ENTER] Select/Recruit    [ESC] Back",
                panel_x + 20, panel_y + panel_h - 35, 14, dim_color);

    /* Malachar hint */
    draw_label (get_pool_label (self), "\"Mortals are fleeting, but their service can span generations with proper planning...\"",
                panel_x + 20, panel_y + panel_h - 60, 14, gold_color);
}

static gboolean
lp_state_agents_handle_input (LrgGameState *state,
                              gpointer      event)
{
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_agents_class_init (LpStateAgentsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_agents_dispose;

    state_class->enter = lp_state_agents_enter;
    state_class->exit = lp_state_agents_exit;
    state_class->update = lp_state_agents_update;
    state_class->draw = lp_state_agents_draw;
    state_class->handle_input = lp_state_agents_handle_input;
}

static void
lp_state_agents_init (LpStateAgents *self)
{
    guint i;

    lrg_game_state_set_name (LRG_GAME_STATE (self), "Agents");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->view_mode = VIEW_MODE_AGENTS;
    self->selected_index = 0;
    self->scroll_offset = 0;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text (6 items * 8 columns + headers/static) */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 70; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_agents_new:
 *
 * Creates a new agent management state.
 *
 * Returns: (transfer full): A new #LpStateAgents
 */
LpStateAgents *
lp_state_agents_new (void)
{
    return g_object_new (LP_TYPE_STATE_AGENTS, NULL);
}
