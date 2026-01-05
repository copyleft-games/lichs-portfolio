/* lp-state-investments.c - Investment Management Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-investments.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../investment/lp-portfolio.h"
#include "../investment/lp-investment.h"
#include "../investment/lp-investment-property.h"
#include "../investment/lp-investment-trade.h"
#include "../investment/lp-investment-financial.h"
#include <graylib.h>
#include <libregnum.h>

/* Maximum items visible in list */
#define MAX_VISIBLE_ITEMS (8)

/* Investment option for buying */
typedef struct
{
    const gchar   *name;
    const gchar   *description;
    LpAssetClass   asset_class;
    gdouble        base_cost;
    gint           subtype;  /* Property/Trade/Financial type */
} InvestmentOption;

static const InvestmentOption available_investments[] = {
    { "Manor House", "A stately property in the countryside", LP_ASSET_CLASS_PROPERTY, 2000.0, LP_PROPERTY_TYPE_URBAN },
    { "City Warehouse", "Storage facility for trade goods", LP_ASSET_CLASS_PROPERTY, 1500.0, LP_PROPERTY_TYPE_URBAN },
    { "Farmland", "Productive agricultural land", LP_ASSET_CLASS_PROPERTY, 800.0, LP_PROPERTY_TYPE_AGRICULTURAL },
    { "Tavern", "A popular drinking establishment", LP_ASSET_CLASS_PROPERTY, 600.0, LP_PROPERTY_TYPE_URBAN },
    { "Silk Trade Route", "Exotic goods from the east", LP_ASSET_CLASS_TRADE, 1200.0, LP_TRADE_TYPE_ROUTE },
    { "Spice Caravan", "Valuable spices and herbs", LP_ASSET_CLASS_TRADE, 900.0, LP_TRADE_TYPE_CARAVAN },
    { "Wool Merchant", "Domestic textile trade", LP_ASSET_CLASS_TRADE, 400.0, LP_TRADE_TYPE_COMMODITY },
    { "Salt License", "Essential commodity trading", LP_ASSET_CLASS_TRADE, 300.0, LP_TRADE_TYPE_COMMODITY },
    { "Royal Bond", "Low-risk crown debt", LP_ASSET_CLASS_FINANCIAL, 500.0, LP_FINANCIAL_TYPE_CROWN_BOND },
    { "Merchant Guild Note", "Trade guild financing", LP_ASSET_CLASS_FINANCIAL, 350.0, LP_FINANCIAL_TYPE_MERCHANT_NOTE },
    { "Mining Shares", "Stake in ore extraction", LP_ASSET_CLASS_FINANCIAL, 250.0, LP_FINANCIAL_TYPE_NOBLE_DEBT },
};

#define NUM_AVAILABLE_INVESTMENTS (G_N_ELEMENTS(available_investments))

typedef enum
{
    VIEW_MODE_PORTFOLIO,    /* Viewing owned investments */
    VIEW_MODE_MARKET        /* Viewing market for buying */
} ViewMode;

struct _LpStateInvestments
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

G_DEFINE_TYPE (LpStateInvestments, lp_state_investments, LRG_TYPE_GAME_STATE)

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
get_pool_label (LpStateInvestments *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateInvestments *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_investments_dispose (GObject *object)
{
    LpStateInvestments *self = LP_STATE_INVESTMENTS (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_investments_parent_class)->dispose (object);
}

/* ==========================================================================
 * Helper Functions
 * ========================================================================== */

static const gchar *
asset_class_to_string (LpAssetClass asset_class)
{
    switch (asset_class)
    {
    case LP_ASSET_CLASS_PROPERTY:
        return "Property";
    case LP_ASSET_CLASS_TRADE:
        return "Trade";
    case LP_ASSET_CLASS_FINANCIAL:
        return "Financial";
    case LP_ASSET_CLASS_MAGICAL:
        return "Magical";
    case LP_ASSET_CLASS_POLITICAL:
        return "Political";
    case LP_ASSET_CLASS_DARK:
        return "Dark";
    default:
        return "Unknown";
    }
}

static LpInvestment *
create_investment_from_option (const InvestmentOption *option,
                               guint64                 current_year)
{
    LpInvestment *investment = NULL;
    g_autoptr(LrgBigNumber) price = NULL;
    g_autofree gchar *id = NULL;

    /* Generate unique ID */
    id = g_strdup_printf ("%s-%lu-%d",
                          option->name,
                          (gulong)current_year,
                          g_random_int_range (1000, 9999));

    price = lrg_big_number_new (option->base_cost);

    switch (option->asset_class)
    {
    case LP_ASSET_CLASS_PROPERTY:
        investment = LP_INVESTMENT (lp_investment_property_new (id, option->name,
                                                                 (LpPropertyType)option->subtype));
        break;
    case LP_ASSET_CLASS_TRADE:
        investment = LP_INVESTMENT (lp_investment_trade_new (id, option->name,
                                                              (LpTradeType)option->subtype));
        break;
    case LP_ASSET_CLASS_FINANCIAL:
        investment = LP_INVESTMENT (lp_investment_financial_new (id, option->name,
                                                                  (LpFinancialType)option->subtype));
        break;
    default:
        /* For unsupported types, create generic investment */
        investment = lp_investment_new (id, option->name, option->asset_class);
        break;
    }

    if (investment != NULL)
    {
        lp_investment_set_purchase_price (investment, lrg_big_number_copy (price));
        lp_investment_set_current_value (investment, lrg_big_number_new (option->base_cost));
        lp_investment_set_purchase_year (investment, current_year);
        lp_investment_set_description (investment, option->description);
    }

    return investment;
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_investments_enter (LrgGameState *state)
{
    LpStateInvestments *self = LP_STATE_INVESTMENTS (state);

    lp_log_info ("Entering investments state");

    self->view_mode = VIEW_MODE_PORTFOLIO;
    self->selected_index = 0;
    self->scroll_offset = 0;
}

static void
lp_state_investments_exit (LrgGameState *state)
{
    lp_log_info ("Exiting investments state");
}

static void
lp_state_investments_update (LrgGameState *state,
                             gdouble       delta)
{
    LpStateInvestments *self = LP_STATE_INVESTMENTS (state);
    LpGame *game = lp_game_get_from_state (state);
    LpGameData *game_data = lp_game_get_game_data (game);
    LpPortfolio *portfolio = lp_game_data_get_portfolio (game_data);
    LrgGameStateManager *manager;
    guint max_items;

    (void)delta;

    /* Determine max items based on view mode */
    if (self->view_mode == VIEW_MODE_PORTFOLIO)
    {
        max_items = lp_portfolio_get_investment_count (portfolio);
    }
    else
    {
        max_items = NUM_AVAILABLE_INVESTMENTS;
    }

    /* Navigation: Up/Down (including vim keys) */
    if (grl_input_is_key_pressed (GRL_KEY_UP) ||
        grl_input_is_key_pressed (GRL_KEY_K))
    {
        if (self->selected_index > 0)
        {
            self->selected_index--;
            /* Adjust scroll if needed */
            if (self->selected_index < self->scroll_offset)
            {
                self->scroll_offset = self->selected_index;
            }
        }
    }

    if (grl_input_is_key_pressed (GRL_KEY_DOWN) ||
        grl_input_is_key_pressed (GRL_KEY_J))
    {
        if (max_items > 0 && self->selected_index < (gint)(max_items - 1))
        {
            self->selected_index++;
            /* Adjust scroll if needed */
            if (self->selected_index >= self->scroll_offset + MAX_VISIBLE_ITEMS)
            {
                self->scroll_offset = self->selected_index - MAX_VISIBLE_ITEMS + 1;
            }
        }
    }

    /* Tab/H/L to switch between portfolio and market views */
    if (grl_input_is_key_pressed (GRL_KEY_TAB) ||
        grl_input_is_key_pressed (GRL_KEY_H) ||
        grl_input_is_key_pressed (GRL_KEY_L))
    {
        if (self->view_mode == VIEW_MODE_PORTFOLIO)
        {
            self->view_mode = VIEW_MODE_MARKET;
        }
        else
        {
            self->view_mode = VIEW_MODE_PORTFOLIO;
        }
        self->selected_index = 0;
        self->scroll_offset = 0;
        lp_log_info ("Switched to %s view",
                     self->view_mode == VIEW_MODE_PORTFOLIO ? "portfolio" : "market");
    }

    /* Enter to buy or sell */
    if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        if (self->view_mode == VIEW_MODE_MARKET)
        {
            /* Buy selected investment */
            if (self->selected_index >= 0 &&
                self->selected_index < (gint)NUM_AVAILABLE_INVESTMENTS)
            {
                const InvestmentOption *option = &available_investments[self->selected_index];
                g_autoptr(LrgBigNumber) cost = lrg_big_number_new (option->base_cost);

                if (lp_portfolio_can_afford (portfolio, cost))
                {
                    guint64 current_year = lp_game_data_get_current_year (game_data);
                    LpInvestment *investment = create_investment_from_option (option, current_year);

                    if (investment != NULL)
                    {
                        lp_portfolio_subtract_gold (portfolio, cost);
                        lp_portfolio_add_investment (portfolio, investment);
                        lp_log_info ("Purchased %s for %.0f gold",
                                     option->name, option->base_cost);
                    }
                }
                else
                {
                    lp_log_info ("Cannot afford %s (cost: %.0f gold)",
                                 option->name, option->base_cost);
                }
            }
        }
        else
        {
            /* Sell selected investment */
            GPtrArray *investments = lp_portfolio_get_investments (portfolio);
            if (investments != NULL &&
                self->selected_index >= 0 &&
                self->selected_index < (gint)investments->len)
            {
                LpInvestment *investment = g_ptr_array_index (investments, self->selected_index);

                if (lp_investment_can_sell (investment))
                {
                    LrgBigNumber *value = lp_investment_get_current_value (investment);
                    const gchar *name = lp_investment_get_name (investment);
                    guint new_count;

                    lp_portfolio_add_gold (portfolio, value);
                    lp_portfolio_remove_investment (portfolio, investment);
                    lp_log_info ("Sold %s for %.0f gold",
                                 name, lrg_big_number_to_double (value));

                    /* Adjust selection if needed */
                    new_count = lp_portfolio_get_investment_count (portfolio);
                    if (new_count > 0 && self->selected_index >= (gint)new_count)
                    {
                        self->selected_index = (gint)new_count - 1;
                    }
                }
                else
                {
                    lp_log_info ("Cannot sell this investment right now");
                }
            }
        }
    }

    /* ESC to return to analyze */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
    {
        lp_log_info ("Returning to analyze state");

        manager = lrg_game_template_get_state_manager (LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_pop (manager);
    }
}

static void
lp_state_investments_draw (LrgGameState *state)
{
    LpStateInvestments *self = LP_STATE_INVESTMENTS (state);
    LpGame *game = lp_game_get_from_state (state);
    LpGameData *game_data = lp_game_get_game_data (game);
    LpPortfolio *portfolio = lp_game_data_get_portfolio (game_data);
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) gold_color = NULL;
    g_autoptr(GrlColor) panel_color = NULL;
    g_autoptr(GrlColor) selected_color = NULL;
    g_autoptr(GrlColor) tab_active_color = NULL;
    g_autoptr(GrlColor) tab_inactive_color = NULL;
    gint screen_w, screen_h;
    gint center_x;
    gint margin, header_h;
    gint panel_x, panel_y, panel_w, panel_h;
    gint list_y, item_h;
    gchar str_buf[128];
    guint i, idx;
    guint count, visible_count;

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
    item_h = 35;

    /* Colors */
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    gold_color = grl_color_new (255, 215, 0, 255);
    panel_color = grl_color_new (25, 25, 35, 255);
    selected_color = grl_color_new (60, 50, 80, 255);
    tab_active_color = grl_color_new (100, 80, 140, 255);
    tab_inactive_color = grl_color_new (40, 40, 50, 255);

    /* Draw header */
    draw_label (self->label_title, "INVESTMENT MANAGEMENT", center_x - 200, 30, 36, title_color);

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
                        self->view_mode == VIEW_MODE_PORTFOLIO ? tab_active_color : tab_inactive_color);
    draw_label (get_pool_label (self), "My Portfolio", panel_x + 25, panel_y + 17, 18, text_color);

    grl_draw_rectangle (panel_x + 170, panel_y + 10, 150, 35,
                        self->view_mode == VIEW_MODE_MARKET ? tab_active_color : tab_inactive_color);
    draw_label (get_pool_label (self), "Market", panel_x + 210, panel_y + 17, 18, text_color);

    /* Draw column headers */
    draw_label (get_pool_label (self), "Name", panel_x + 20, panel_y + 65, 16, dim_color);
    draw_label (get_pool_label (self), "Type", panel_x + 300, panel_y + 65, 16, dim_color);
    draw_label (get_pool_label (self), "Value", panel_x + 450, panel_y + 65, 16, dim_color);

    if (self->view_mode == VIEW_MODE_PORTFOLIO)
    {
        GPtrArray *investments;
        draw_label (get_pool_label (self), "Return", panel_x + 580, panel_y + 65, 16, dim_color);

        /* Draw owned investments */
        investments = (portfolio != NULL) ? lp_portfolio_get_investments (portfolio) : NULL;
        count = (investments != NULL) ? investments->len : 0;

        if (count == 0)
        {
            draw_label (get_pool_label (self), "No investments owned. Press TAB to browse market.",
                        panel_x + 50, list_y + 50, 18, dim_color);
        }
        else
        {
            visible_count = MIN (count - self->scroll_offset, MAX_VISIBLE_ITEMS);
            for (i = 0; i < visible_count; i++)
            {
                LpInvestment *inv;
                gint item_y;
                gboolean is_selected;
                LrgBigNumber *value;
                gdouble ret_pct;

                idx = self->scroll_offset + i;
                inv = g_ptr_array_index (investments, idx);
                item_y = list_y + (i * item_h);
                is_selected = ((gint)idx == self->selected_index);

                /* Draw selection highlight */
                if (is_selected)
                {
                    grl_draw_rectangle (panel_x + 10, item_y - 3,
                                        panel_w - 20, item_h - 2, selected_color);
                }

                /* Draw investment details */
                draw_label (get_pool_label (self), lp_investment_get_name (inv),
                            panel_x + 20, item_y, 16,
                            is_selected ? gold_color : text_color);

                draw_label (get_pool_label (self), asset_class_to_string (lp_investment_get_asset_class (inv)),
                            panel_x + 300, item_y, 16, text_color);

                value = lp_investment_get_current_value (inv);
                g_snprintf (str_buf, sizeof (str_buf), "%.0f gp",
                            lrg_big_number_to_double (value));
                draw_label (get_pool_label (self), str_buf, panel_x + 450, item_y, 16, gold_color);

                ret_pct = lp_investment_get_return_percentage (inv);
                g_snprintf (str_buf, sizeof (str_buf), "%+.1f%%", ret_pct);
                draw_label (get_pool_label (self), str_buf, panel_x + 580, item_y, 16,
                            ret_pct >= 0 ? text_color : dim_color);
            }
        }
    }
    else
    {
        draw_label (get_pool_label (self), "Cost", panel_x + 450, panel_y + 65, 16, dim_color);

        /* Draw available investments */
        visible_count = MIN (NUM_AVAILABLE_INVESTMENTS - self->scroll_offset, MAX_VISIBLE_ITEMS);
        for (i = 0; i < visible_count; i++)
        {
            const InvestmentOption *option;
            gint item_y;
            gboolean is_selected;
            g_autoptr(LrgBigNumber) cost = NULL;
            gboolean can_afford;

            idx = self->scroll_offset + i;
            option = &available_investments[idx];
            item_y = list_y + (i * item_h);
            is_selected = ((gint)idx == self->selected_index);

            /* Check if player can afford */
            cost = lrg_big_number_new (option->base_cost);
            can_afford = portfolio != NULL && lp_portfolio_can_afford (portfolio, cost);

            /* Draw selection highlight */
            if (is_selected)
            {
                grl_draw_rectangle (panel_x + 10, item_y - 3,
                                    panel_w - 20, item_h - 2, selected_color);
            }

            /* Draw option details */
            draw_label (get_pool_label (self), option->name, panel_x + 20, item_y, 16,
                        is_selected ? gold_color : (can_afford ? text_color : dim_color));

            draw_label (get_pool_label (self), asset_class_to_string (option->asset_class),
                        panel_x + 300, item_y, 16,
                        can_afford ? text_color : dim_color);

            g_snprintf (str_buf, sizeof (str_buf), "%.0f gp", option->base_cost);
            draw_label (get_pool_label (self), str_buf, panel_x + 450, item_y, 16,
                        can_afford ? gold_color : dim_color);
        }

        /* Draw selected item description below the list */
        if (self->selected_index >= 0 &&
            self->selected_index < (gint)NUM_AVAILABLE_INVESTMENTS)
        {
            const InvestmentOption *selected = &available_investments[self->selected_index];
            draw_label (get_pool_label (self), selected->description,
                        panel_x + 20, list_y + (MAX_VISIBLE_ITEMS * item_h) + 20,
                        16, dim_color);
        }
    }

    /* Draw instructions */
    draw_label (get_pool_label (self), "[UP/DOWN] Select    [TAB] Switch View    [ENTER] Buy/Sell    [ESC] Back",
                panel_x + 20, panel_y + panel_h - 35, 14, dim_color);

    /* Malachar hint */
    draw_label (get_pool_label (self), "\"Choose wisely, my lord. These assets will generate wealth while you slumber...\"",
                panel_x + 20, panel_y + panel_h - 60, 14, gold_color);
}

static gboolean
lp_state_investments_handle_input (LrgGameState *state,
                                   gpointer      event)
{
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_investments_class_init (LpStateInvestmentsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_investments_dispose;

    state_class->enter = lp_state_investments_enter;
    state_class->exit = lp_state_investments_exit;
    state_class->update = lp_state_investments_update;
    state_class->draw = lp_state_investments_draw;
    state_class->handle_input = lp_state_investments_handle_input;
}

static void
lp_state_investments_init (LpStateInvestments *self)
{
    guint i;

    lrg_game_state_set_name (LRG_GAME_STATE (self), "Investments");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->view_mode = VIEW_MODE_PORTFOLIO;
    self->selected_index = 0;
    self->scroll_offset = 0;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text (8 items * 4 columns + headers/static) */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 50; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_investments_new:
 *
 * Creates a new investment management state.
 *
 * Returns: (transfer full): A new #LpStateInvestments
 */
LpStateInvestments *
lp_state_investments_new (void)
{
    return g_object_new (LP_TYPE_STATE_INVESTMENTS, NULL);
}
