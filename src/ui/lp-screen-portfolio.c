/* lp-screen-portfolio.c - Portfolio Management Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-screen-portfolio.h"
#include "lp-theme.h"
#include "lp-widget-exposure-meter.h"
#include "lp-widget-synergy-indicator.h"
#include "../investment/lp-portfolio.h"
#include "../investment/lp-investment.h"

struct _LpScreenPortfolio
{
    LrgContainer parent_instance;

    /* Data binding */
    LpPortfolio *portfolio;

    /* View state */
    LpPortfolioViewMode view_mode;
    LpInvestment *selected_investment;
    gint          selection_index;

    /* Child widgets */
    LpWidgetExposureMeter    *exposure_meter;
    LpWidgetSynergyIndicator *synergy_indicator;

    /* Cached display data */
    GPtrArray *displayed_investments;
};

enum
{
    PROP_0,
    PROP_PORTFOLIO,
    PROP_VIEW_MODE,
    PROP_SELECTED_INVESTMENT,
    N_PROPS
};

enum
{
    SIGNAL_INVESTMENT_SELECTED,
    SIGNAL_BUY_REQUESTED,
    SIGNAL_SELL_REQUESTED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpScreenPortfolio, lp_screen_portfolio, LRG_TYPE_CONTAINER)

/* ==========================================================================
 * Forward Declarations
 * ========================================================================== */

static void lp_screen_portfolio_draw           (LrgWidget *widget);
static void lp_screen_portfolio_layout_children (LrgContainer *container);
static gboolean lp_screen_portfolio_handle_event (LrgWidget        *widget,
                                                   const LrgUIEvent *event);

/* ==========================================================================
 * Helper Functions
 * ========================================================================== */

/*
 * rebuild_investment_list:
 *
 * Rebuilds the list of investments to display from the portfolio.
 */
static void
rebuild_investment_list (LpScreenPortfolio *self)
{
    GPtrArray *investments;
    guint i;
    guint count;

    /* Clear existing list */
    g_ptr_array_set_size (self->displayed_investments, 0);

    if (self->portfolio == NULL)
        return;

    /* Add all investments from portfolio */
    investments = lp_portfolio_get_investments (self->portfolio);
    count = (investments != NULL) ? investments->len : 0;

    for (i = 0; i < count; i++)
    {
        LpInvestment *inv = g_ptr_array_index (investments, i);
        if (inv != NULL)
            g_ptr_array_add (self->displayed_investments, inv);
    }

    /* Reset selection if out of bounds */
    if (self->selection_index >= (gint)self->displayed_investments->len)
        self->selection_index = MAX (0, (gint)self->displayed_investments->len - 1);

    /* Update selected investment pointer */
    if (self->displayed_investments->len > 0 && self->selection_index >= 0)
    {
        self->selected_investment = g_ptr_array_index (self->displayed_investments,
                                                        self->selection_index);
    }
    else
    {
        self->selected_investment = NULL;
    }
}

/*
 * on_portfolio_changed:
 *
 * Signal handler for portfolio investment changes.
 */
static void
on_portfolio_changed (LpPortfolio       *portfolio,
                      LpScreenPortfolio *self)
{
    rebuild_investment_list (self);
}

/*
 * get_asset_class_color:
 *
 * Gets the theme color for an asset class.
 */
static const GrlColor *
get_asset_class_color (LpAssetClass asset_class)
{
    switch (asset_class)
    {
        case LP_ASSET_CLASS_PROPERTY:
            return lrg_theme_get_accent_color (lrg_theme_get_default ());
        case LP_ASSET_CLASS_TRADE:
            return lp_theme_get_gold_color ();
        case LP_ASSET_CLASS_FINANCIAL:
            return lrg_theme_get_success_color (lrg_theme_get_default ());
        case LP_ASSET_CLASS_MAGICAL:
            return lp_theme_get_synergy_color ();
        case LP_ASSET_CLASS_POLITICAL:
            return lrg_theme_get_secondary_color (lrg_theme_get_default ());
        case LP_ASSET_CLASS_DARK:
            return lp_theme_get_danger_color ();
        default:
            return lrg_theme_get_text_color (lrg_theme_get_default ());
    }
}

/* ==========================================================================
 * LrgWidget Virtual Methods
 * ========================================================================== */

/*
 * lp_screen_portfolio_draw:
 *
 * Draws the portfolio screen content.
 */
static void
lp_screen_portfolio_draw (LrgWidget *widget)
{
    LpScreenPortfolio *self;
    LrgTheme *theme;
    const GrlColor *bg_color;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *border_color;
    const GrlColor *accent_color;
    GrlRectangle rect;
    gfloat x;
    gfloat y;
    gfloat width;
    gfloat height;
    gfloat padding;
    gfloat font_size;
    gfloat font_size_large;
    gfloat font_size_small;
    gfloat header_height;
    gfloat list_y;
    gfloat item_height;
    guint i;

    self = LP_SCREEN_PORTFOLIO (widget);
    theme = lrg_theme_get_default ();

    /* Get bounds using individual accessors */
    x = lrg_widget_get_world_x (widget);
    y = lrg_widget_get_world_y (widget);
    width = lrg_widget_get_width (widget);
    height = lrg_widget_get_height (widget);

    /* Get theme values */
    padding = lrg_theme_get_padding_normal (theme);
    font_size = lrg_theme_get_font_size_normal (theme);
    font_size_large = lrg_theme_get_font_size_large (theme);
    font_size_small = lrg_theme_get_font_size_small (theme);

    /* Get colors */
    bg_color = lrg_theme_get_background_color (theme);
    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    border_color = lrg_theme_get_border_color (theme);
    accent_color = lrg_theme_get_accent_color (theme);

    /* Draw background */
    grl_draw_rectangle ((gint)x, (gint)y, (gint)width, (gint)height, bg_color);

    /* Draw header section */
    header_height = font_size_large + padding * 3;
    grl_draw_rectangle ((gint)x, (gint)y, (gint)width, (gint)header_height,
                         lrg_theme_get_surface_color (theme));
    grl_draw_line ((gint)x, (gint)(y + header_height),
                   (gint)(x + width), (gint)(y + header_height),
                   border_color);

    /* Draw title */
    grl_draw_text ("Portfolio", (gint)(x + padding), (gint)(y + padding),
                   (gint)font_size_large, text_color);

    /* Draw total wealth if portfolio is set */
    if (self->portfolio != NULL)
    {
        g_autoptr(LrgBigNumber) total = lp_portfolio_get_total_value (self->portfolio);
        g_autofree gchar *wealth_str = lrg_big_number_format_short (total);
        g_autofree gchar *wealth_text = g_strdup_printf ("Total: %s gold", wealth_str);
        gint text_width = grl_measure_text (wealth_text, (gint)font_size);

        grl_draw_text (wealth_text, (gint)(x + width - text_width - padding),
                       (gint)(y + padding + (font_size_large - font_size) / 2),
                       (gint)font_size, lp_theme_get_gold_color ());
    }

    /* Investment list area */
    list_y = y + header_height + padding;
    item_height = font_size * 2.5f;

    /* Draw view mode tabs */
    {
        gfloat tab_x = x + padding;
        gfloat tab_y = list_y;
        const gchar *tabs[] = {"List", "Allocation", "Performance"};
        gint j;

        for (j = 0; j < 3; j++)
        {
            gint tab_text_width = grl_measure_text (tabs[j], (gint)font_size_small);
            gfloat tab_width = tab_text_width + padding * 2;
            gboolean selected = (j == (gint)self->view_mode);

            rect.x = tab_x;
            rect.y = tab_y;
            rect.width = tab_width;
            rect.height = font_size_small + padding;

            if (selected)
            {
                grl_draw_rectangle_rec (&rect, accent_color);
                grl_draw_text (tabs[j], (gint)(tab_x + padding), (gint)(tab_y + padding / 2),
                               (gint)font_size_small, bg_color);
            }
            else
            {
                grl_draw_rectangle_lines_ex (&rect, 1.0f, border_color);
                grl_draw_text (tabs[j], (gint)(tab_x + padding), (gint)(tab_y + padding / 2),
                               (gint)font_size_small, secondary_color);
            }

            tab_x += tab_width + padding / 2;
        }

        list_y += font_size_small + padding * 2;
    }

    /* Draw investment list */
    if (self->view_mode == LP_PORTFOLIO_VIEW_LIST)
    {
        if (self->displayed_investments->len == 0)
        {
            grl_draw_text ("No investments. Press B to buy.",
                           (gint)(x + padding), (gint)(list_y + padding),
                           (gint)font_size, secondary_color);
        }
        else
        {
            for (i = 0; i < self->displayed_investments->len; i++)
            {
                LpInvestment *inv = g_ptr_array_index (self->displayed_investments, i);
                gfloat item_y = list_y + i * item_height;
                gboolean is_selected = ((gint)i == self->selection_index);
                const gchar *name;
                LpAssetClass asset_class;
                const GrlColor *class_color;
                g_autoptr(LrgBigNumber) value = NULL;
                g_autofree gchar *value_str = NULL;

                /* Draw selection highlight */
                if (is_selected)
                {
                    rect.x = x + padding / 2;
                    rect.y = item_y;
                    rect.width = width - padding;
                    rect.height = item_height;

                    grl_draw_rectangle_rec (&rect, lrg_theme_get_surface_color (theme));
                    grl_draw_rectangle_lines_ex (&rect, 1.0f, accent_color);
                }

                /* Get investment details */
                name = lp_investment_get_name (inv);
                asset_class = lp_investment_get_asset_class (inv);
                class_color = get_asset_class_color (asset_class);
                value = lp_investment_get_current_value (inv);
                value_str = lrg_big_number_format_short (value);

                /* Draw asset class indicator */
                grl_draw_rectangle ((gint)(x + padding),
                                     (gint)(item_y + item_height / 2 - font_size / 2),
                                     4, (gint)font_size, class_color);

                /* Draw investment name */
                grl_draw_text (name, (gint)(x + padding * 2 + 4.0f), (gint)(item_y + padding / 2),
                               (gint)font_size, is_selected ? text_color : secondary_color);

                /* Draw value */
                {
                    gint val_width = grl_measure_text (value_str, (gint)font_size_small);
                    grl_draw_text (value_str,
                                   (gint)(x + width - val_width - padding),
                                   (gint)(item_y + padding / 2 + font_size - font_size_small),
                                   (gint)font_size_small, lp_theme_get_gold_color ());
                }
            }
        }
    }
    else if (self->view_mode == LP_PORTFOLIO_VIEW_ALLOCATION)
    {
        /* Placeholder for allocation view */
        grl_draw_text ("Asset allocation view - coming soon",
                       (gint)(x + padding), (gint)(list_y + padding),
                       (gint)font_size, secondary_color);
    }
    else if (self->view_mode == LP_PORTFOLIO_VIEW_PERFORMANCE)
    {
        /* Placeholder for performance view */
        grl_draw_text ("Performance history view - coming soon",
                       (gint)(x + padding), (gint)(list_y + padding),
                       (gint)font_size, secondary_color);
    }

    /* Draw footer with controls hint */
    {
        gfloat footer_y = y + height - font_size_small - padding * 2;
        grl_draw_line ((gint)x, (gint)(footer_y - padding),
                       (gint)(x + width), (gint)(footer_y - padding),
                       border_color);
        grl_draw_text ("[B]uy  [S]ell  [Tab]View  [Up/Down]Select",
                       (gint)(x + padding), (gint)footer_y, (gint)font_size_small, secondary_color);
    }

    /* Draw child widgets (exposure meter, synergy indicator) */
    LRG_WIDGET_CLASS (lp_screen_portfolio_parent_class)->draw (widget);
}

/*
 * lp_screen_portfolio_layout_children:
 *
 * Positions child widgets.
 */
static void
lp_screen_portfolio_layout_children (LrgContainer *container)
{
    LpScreenPortfolio *self;
    LrgTheme *theme;
    gfloat x;
    gfloat y;
    gfloat width;
    gfloat padding;
    gfloat header_height;
    gfloat pref_w;
    gfloat pref_h;
    gfloat widget_x;
    gfloat widget_y;
    gfloat widget_width;
    gfloat widget_height;

    self = LP_SCREEN_PORTFOLIO (container);
    theme = lrg_theme_get_default ();

    x = lrg_widget_get_world_x (LRG_WIDGET (container));
    y = lrg_widget_get_world_y (LRG_WIDGET (container));
    width = lrg_widget_get_width (LRG_WIDGET (container));
    padding = lrg_theme_get_padding_normal (theme);
    header_height = lrg_theme_get_font_size_large (theme) + padding * 3;

    /* Position synergy indicator in header, right side */
    if (self->synergy_indicator != NULL)
    {
        lrg_widget_measure (LRG_WIDGET (self->synergy_indicator), &pref_w, &pref_h);

        widget_width = pref_w;
        widget_height = MIN (pref_h, header_height - padding);
        widget_x = x + width - widget_width - padding * 10;
        widget_y = y + (header_height - widget_height) / 2;

        lrg_widget_set_x (LRG_WIDGET (self->synergy_indicator), widget_x - x);
        lrg_widget_set_y (LRG_WIDGET (self->synergy_indicator), widget_y - y);
        lrg_widget_set_width (LRG_WIDGET (self->synergy_indicator), widget_width);
        lrg_widget_set_height (LRG_WIDGET (self->synergy_indicator), widget_height);
    }

    /* Position exposure meter in header, after synergy indicator */
    if (self->exposure_meter != NULL)
    {
        lrg_widget_measure (LRG_WIDGET (self->exposure_meter), &pref_w, &pref_h);

        widget_width = 120.0f;  /* Fixed width for meter */
        widget_height = MIN (pref_h, header_height - padding);
        widget_x = x + width - widget_width - padding * 20 - 150.0f;
        widget_y = y + (header_height - widget_height) / 2;

        lrg_widget_set_x (LRG_WIDGET (self->exposure_meter), widget_x - x);
        lrg_widget_set_y (LRG_WIDGET (self->exposure_meter), widget_y - y);
        lrg_widget_set_width (LRG_WIDGET (self->exposure_meter), widget_width);
        lrg_widget_set_height (LRG_WIDGET (self->exposure_meter), widget_height);
    }
}

/*
 * lp_screen_portfolio_handle_event:
 *
 * Handles keyboard input for navigation.
 */
static gboolean
lp_screen_portfolio_handle_event (LrgWidget        *widget,
                                   const LrgUIEvent *event)
{
    LpScreenPortfolio *self;
    LrgUIEventType event_type;
    GrlKey key;

    self = LP_SCREEN_PORTFOLIO (widget);
    event_type = lrg_ui_event_get_event_type (event);

    if (event_type != LRG_UI_EVENT_KEY_DOWN)
        return FALSE;

    key = lrg_ui_event_get_key (event);

    switch (key)
    {
        case GRL_KEY_UP:
            if (self->selection_index > 0)
            {
                self->selection_index--;
                self->selected_investment =
                    g_ptr_array_index (self->displayed_investments, self->selection_index);
                g_signal_emit (self, signals[SIGNAL_INVESTMENT_SELECTED], 0,
                               self->selected_investment);
            }
            return TRUE;

        case GRL_KEY_DOWN:
            if (self->selection_index < (gint)self->displayed_investments->len - 1)
            {
                self->selection_index++;
                self->selected_investment =
                    g_ptr_array_index (self->displayed_investments, self->selection_index);
                g_signal_emit (self, signals[SIGNAL_INVESTMENT_SELECTED], 0,
                               self->selected_investment);
            }
            return TRUE;

        case GRL_KEY_TAB:
            /* Cycle view mode */
            self->view_mode = (self->view_mode + 1) % 3;
            g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VIEW_MODE]);
            return TRUE;

        case GRL_KEY_B:
            g_signal_emit (self, signals[SIGNAL_BUY_REQUESTED], 0);
            return TRUE;

        case GRL_KEY_S:
            if (self->selected_investment != NULL)
            {
                g_signal_emit (self, signals[SIGNAL_SELL_REQUESTED], 0,
                               self->selected_investment);
            }
            return TRUE;

        default:
            break;
    }

    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_screen_portfolio_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    LpScreenPortfolio *self = LP_SCREEN_PORTFOLIO (object);

    switch (prop_id)
    {
        case PROP_PORTFOLIO:
            g_value_set_object (value, self->portfolio);
            break;

        case PROP_VIEW_MODE:
            g_value_set_uint (value, self->view_mode);
            break;

        case PROP_SELECTED_INVESTMENT:
            g_value_set_object (value, self->selected_investment);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_portfolio_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    LpScreenPortfolio *self = LP_SCREEN_PORTFOLIO (object);

    switch (prop_id)
    {
        case PROP_PORTFOLIO:
            lp_screen_portfolio_set_portfolio (self, g_value_get_object (value));
            break;

        case PROP_VIEW_MODE:
            lp_screen_portfolio_set_view_mode (self, g_value_get_uint (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_screen_portfolio_dispose (GObject *object)
{
    LpScreenPortfolio *self = LP_SCREEN_PORTFOLIO (object);

    /* Disconnect portfolio signals */
    if (self->portfolio != NULL)
    {
        g_signal_handlers_disconnect_by_data (self->portfolio, self);
        g_clear_object (&self->portfolio);
    }

    /* Child widgets are managed by container */
    self->exposure_meter = NULL;
    self->synergy_indicator = NULL;

    G_OBJECT_CLASS (lp_screen_portfolio_parent_class)->dispose (object);
}

static void
lp_screen_portfolio_finalize (GObject *object)
{
    LpScreenPortfolio *self = LP_SCREEN_PORTFOLIO (object);

    g_clear_pointer (&self->displayed_investments, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_screen_portfolio_parent_class)->finalize (object);
}

static void
lp_screen_portfolio_class_init (LpScreenPortfolioClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgWidgetClass *widget_class = LRG_WIDGET_CLASS (klass);
    LrgContainerClass *container_class = LRG_CONTAINER_CLASS (klass);

    object_class->get_property = lp_screen_portfolio_get_property;
    object_class->set_property = lp_screen_portfolio_set_property;
    object_class->dispose = lp_screen_portfolio_dispose;
    object_class->finalize = lp_screen_portfolio_finalize;

    widget_class->draw = lp_screen_portfolio_draw;
    widget_class->handle_event = lp_screen_portfolio_handle_event;

    container_class->layout_children = lp_screen_portfolio_layout_children;

    /**
     * LpScreenPortfolio:portfolio:
     *
     * The portfolio to display.
     */
    properties[PROP_PORTFOLIO] =
        g_param_spec_object ("portfolio",
                             "Portfolio",
                             "The portfolio to display",
                             LP_TYPE_PORTFOLIO,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * LpScreenPortfolio:view-mode:
     *
     * The current view mode (LpPortfolioViewMode).
     */
    properties[PROP_VIEW_MODE] =
        g_param_spec_uint ("view-mode",
                           "View Mode",
                           "The portfolio view mode",
                           0, LP_PORTFOLIO_VIEW_PERFORMANCE,
                           LP_PORTFOLIO_VIEW_LIST,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * LpScreenPortfolio:selected-investment:
     *
     * The currently selected investment.
     */
    properties[PROP_SELECTED_INVESTMENT] =
        g_param_spec_object ("selected-investment",
                             "Selected Investment",
                             "The currently selected investment",
                             LP_TYPE_INVESTMENT,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpScreenPortfolio::investment-selected:
     * @self: the #LpScreenPortfolio
     * @investment: (nullable): the selected investment
     *
     * Emitted when an investment is selected.
     */
    signals[SIGNAL_INVESTMENT_SELECTED] =
        g_signal_new ("investment-selected",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1, LP_TYPE_INVESTMENT);

    /**
     * LpScreenPortfolio::buy-requested:
     * @self: the #LpScreenPortfolio
     *
     * Emitted when the user requests to buy an investment.
     */
    signals[SIGNAL_BUY_REQUESTED] =
        g_signal_new ("buy-requested",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpScreenPortfolio::sell-requested:
     * @self: the #LpScreenPortfolio
     * @investment: the investment to sell
     *
     * Emitted when the user requests to sell an investment.
     */
    signals[SIGNAL_SELL_REQUESTED] =
        g_signal_new ("sell-requested",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1, LP_TYPE_INVESTMENT);
}

static void
lp_screen_portfolio_init (LpScreenPortfolio *self)
{
    self->portfolio = NULL;
    self->view_mode = LP_PORTFOLIO_VIEW_LIST;
    self->selected_investment = NULL;
    self->selection_index = 0;

    /* Create child widgets */
    self->exposure_meter = lp_widget_exposure_meter_new ();
    lrg_container_add_child (LRG_CONTAINER (self), LRG_WIDGET (self->exposure_meter));

    self->synergy_indicator = lp_widget_synergy_indicator_new ();
    lp_widget_synergy_indicator_set_compact (self->synergy_indicator, TRUE);
    lrg_container_add_child (LRG_CONTAINER (self), LRG_WIDGET (self->synergy_indicator));

    /* Initialize display list */
    self->displayed_investments = g_ptr_array_new ();
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_screen_portfolio_new:
 *
 * Creates a new portfolio screen.
 *
 * Returns: (transfer full): A new #LpScreenPortfolio
 */
LpScreenPortfolio *
lp_screen_portfolio_new (void)
{
    return g_object_new (LP_TYPE_SCREEN_PORTFOLIO, NULL);
}

/**
 * lp_screen_portfolio_get_portfolio:
 * @self: an #LpScreenPortfolio
 *
 * Gets the portfolio being displayed.
 *
 * Returns: (transfer none) (nullable): The #LpPortfolio, or %NULL
 */
LpPortfolio *
lp_screen_portfolio_get_portfolio (LpScreenPortfolio *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_PORTFOLIO (self), NULL);

    return self->portfolio;
}

/**
 * lp_screen_portfolio_set_portfolio:
 * @self: an #LpScreenPortfolio
 * @portfolio: (nullable): the portfolio to display
 *
 * Sets the portfolio to display.
 */
void
lp_screen_portfolio_set_portfolio (LpScreenPortfolio *self,
                                    LpPortfolio       *portfolio)
{
    g_return_if_fail (LP_IS_SCREEN_PORTFOLIO (self));
    g_return_if_fail (portfolio == NULL || LP_IS_PORTFOLIO (portfolio));

    if (self->portfolio == portfolio)
        return;

    /* Disconnect old portfolio */
    if (self->portfolio != NULL)
    {
        g_signal_handlers_disconnect_by_data (self->portfolio, self);
        g_object_unref (self->portfolio);
    }

    /* Set new portfolio */
    self->portfolio = portfolio ? g_object_ref (portfolio) : NULL;

    /* Connect to changes */
    if (self->portfolio != NULL)
    {
        g_signal_connect (self->portfolio, "investment-added",
                          G_CALLBACK (on_portfolio_changed), self);
        g_signal_connect (self->portfolio, "investment-removed",
                          G_CALLBACK (on_portfolio_changed), self);
    }

    rebuild_investment_list (self);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PORTFOLIO]);
}

/**
 * lp_screen_portfolio_get_view_mode:
 * @self: an #LpScreenPortfolio
 *
 * Gets the current view mode.
 *
 * Returns: The current #LpPortfolioViewMode
 */
LpPortfolioViewMode
lp_screen_portfolio_get_view_mode (LpScreenPortfolio *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_PORTFOLIO (self), LP_PORTFOLIO_VIEW_LIST);

    return self->view_mode;
}

/**
 * lp_screen_portfolio_set_view_mode:
 * @self: an #LpScreenPortfolio
 * @mode: the view mode
 *
 * Sets the view mode for the portfolio display.
 */
void
lp_screen_portfolio_set_view_mode (LpScreenPortfolio   *self,
                                    LpPortfolioViewMode  mode)
{
    g_return_if_fail (LP_IS_SCREEN_PORTFOLIO (self));

    if (self->view_mode != mode)
    {
        self->view_mode = mode;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VIEW_MODE]);
    }
}

/**
 * lp_screen_portfolio_get_selected_investment:
 * @self: an #LpScreenPortfolio
 *
 * Gets the currently selected investment.
 *
 * Returns: (transfer none) (nullable): The selected #LpInvestment, or %NULL
 */
LpInvestment *
lp_screen_portfolio_get_selected_investment (LpScreenPortfolio *self)
{
    g_return_val_if_fail (LP_IS_SCREEN_PORTFOLIO (self), NULL);

    return self->selected_investment;
}

/**
 * lp_screen_portfolio_select_investment:
 * @self: an #LpScreenPortfolio
 * @investment: (nullable): the investment to select
 *
 * Selects an investment in the list view.
 */
void
lp_screen_portfolio_select_investment (LpScreenPortfolio *self,
                                        LpInvestment      *investment)
{
    guint i;

    g_return_if_fail (LP_IS_SCREEN_PORTFOLIO (self));

    if (investment == NULL)
    {
        self->selection_index = -1;
        self->selected_investment = NULL;
    }
    else
    {
        /* Find investment in list */
        for (i = 0; i < self->displayed_investments->len; i++)
        {
            if (g_ptr_array_index (self->displayed_investments, i) == investment)
            {
                self->selection_index = i;
                self->selected_investment = investment;
                break;
            }
        }
    }

    g_signal_emit (self, signals[SIGNAL_INVESTMENT_SELECTED], 0, self->selected_investment);
}

/**
 * lp_screen_portfolio_show_buy_dialog:
 * @self: an #LpScreenPortfolio
 *
 * Shows the buy investment dialog.
 */
void
lp_screen_portfolio_show_buy_dialog (LpScreenPortfolio *self)
{
    g_return_if_fail (LP_IS_SCREEN_PORTFOLIO (self));

    /* Emit signal - actual dialog handled by game state */
    g_signal_emit (self, signals[SIGNAL_BUY_REQUESTED], 0);
}

/**
 * lp_screen_portfolio_sell_selected:
 * @self: an #LpScreenPortfolio
 *
 * Initiates a sell action for the currently selected investment.
 */
void
lp_screen_portfolio_sell_selected (LpScreenPortfolio *self)
{
    g_return_if_fail (LP_IS_SCREEN_PORTFOLIO (self));

    if (self->selected_investment != NULL)
    {
        g_signal_emit (self, signals[SIGNAL_SELL_REQUESTED], 0,
                       self->selected_investment);
    }
}

/**
 * lp_screen_portfolio_refresh:
 * @self: an #LpScreenPortfolio
 *
 * Refreshes the portfolio display from current data.
 */
void
lp_screen_portfolio_refresh (LpScreenPortfolio *self)
{
    g_return_if_fail (LP_IS_SCREEN_PORTFOLIO (self));

    rebuild_investment_list (self);
}
