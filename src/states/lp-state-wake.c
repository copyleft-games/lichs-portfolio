/* lp-state-wake.c - Wake Report Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-wake.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../core/lp-portfolio-history.h"
#include "../investment/lp-portfolio.h"
#include "../agent/lp-agent-manager.h"
/* #include "../narrative/lp-malachar-voice.h" */
/* #include "../tutorial/lp-tutorial-sequences.h" */
#include "lp-state-analyze.h"
#include "../ui/lp-theme.h"
#include "../lp-input-helpers.h"
#include <graylib.h>
#include <libregnum.h>

struct _LpStateWake
{
    LrgGameState parent_instance;

    GList *events;          /* Events that occurred during slumber */
    guint  current_event;   /* Index of currently displayed event */

    /* Slumber growth data */
    GPtrArray      *slumber_snapshots;  /* Array of LpPortfolioSnapshot */
    LrgLineChart2D *slumber_chart;      /* Year-by-year portfolio growth chart */

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpStateWake, lp_state_wake, LRG_TYPE_GAME_STATE)

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
get_pool_label (LpStateWake *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateWake *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_wake_dispose (GObject *object)
{
    LpStateWake *self = LP_STATE_WAKE (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);
    g_clear_pointer (&self->slumber_snapshots, g_ptr_array_unref);
    g_clear_object (&self->slumber_chart);

    G_OBJECT_CLASS (lp_state_wake_parent_class)->dispose (object);
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_wake_enter (LrgGameState *state)
{
    LpStateWake *self = LP_STATE_WAKE (state);

    lp_log_info ("Entering wake state");

    self->current_event = 0;

    /*
     * TODO: When application has transition manager, trigger fade-in here.
     * The player is "waking up" so we'd fade from black.
     */

    /*
     * TODO: Re-enable tutorial when LrgTutorialManager is initialized
     * in lp_game_real_pre_startup(). The tutorial system requires calling
     * lp_tutorial_sequences_init_tutorials() before use.
     *
     * lp_tutorial_sequences_maybe_start_intro (
     *     lp_tutorial_sequences_get_default ());
     */
}

static void
lp_state_wake_exit (LrgGameState *state)
{
    LpStateWake *self = LP_STATE_WAKE (state);

    lp_log_info ("Exiting wake state");

    g_list_free_full (self->events, g_object_unref);
    self->events = NULL;
}

static void
lp_state_wake_update (LrgGameState *state,
                      gdouble       delta)
{
    (void)delta;

    /* Check for input to continue (Enter/Space/A button) */
    if (LP_INPUT_CONFIRM_PRESSED ())
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Continuing to analyze state");

        /* Replace wake with analyze state */
        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_replace (manager,
            LRG_GAME_STATE (lp_state_analyze_new ()));
    }

    /* ESC/B button to quit */
    if (LP_INPUT_CANCEL_PRESSED ())
    {
        LpGame *game = lp_game_get_from_state (state);
        lrg_game_template_quit (LRG_GAME_TEMPLATE (game));
    }
}

static void
lp_state_wake_draw (LrgGameState *state)
{
    LpStateWake *self = LP_STATE_WAKE (state);
    LpGame *game = lp_game_get_from_state (state);
    LpGameData *game_data = lp_game_get_game_data (game);
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) gold_color = NULL;
    guint64 year;
    gchar year_str[64];
    gint screen_w, screen_h;
    gint center_x;
    gint title_y, year_y, greeting_y, portfolio_y, instructions_y;

    /* Reset label pool for this frame */
    reset_label_pool (self);

    /* Get virtual resolution (render target size) for UI positioning */
    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
    center_x = screen_w / 2;

    /* Calculate vertical positions */
    title_y = screen_h / 7;
    year_y = title_y + 80;
    greeting_y = screen_h * 2 / 5;
    portfolio_y = screen_h * 3 / 5;
    instructions_y = screen_h - 100;

    /* Colors */
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    gold_color = grl_color_new (255, 215, 0, 255);

    /* Draw title */
    draw_label (self->label_title, "THE LICH AWAKENS",
                center_x - 180, title_y, 48, title_color);

    /* Get game data info */
    if (game_data != NULL)
    {
        year = lp_game_data_get_current_year (game_data);
        g_snprintf (year_str, sizeof (year_str), "Year %lu of the Third Age", (gulong)year);
    }
    else
    {
        g_snprintf (year_str, sizeof (year_str), "Year 847 of the Third Age");
    }

    draw_label (get_pool_label (self), year_str,
                center_x - 140, year_y, 24, text_color);

    /* Malachar's greeting */
    draw_label (get_pool_label (self), "\"Ah, you have awakened, my eternal apprentice...\"",
                center_x - 280, greeting_y, 20, gold_color);

    draw_label (get_pool_label (self), "\"The mortal world continues its endless dance of",
                center_x - 280, greeting_y + 40, 18, text_color);
    draw_label (get_pool_label (self), "gold and folly. Let us see what opportunities await.\"",
                center_x - 280, greeting_y + 65, 18, text_color);

    /* Portfolio summary - display dynamic values */
    draw_label (get_pool_label (self), "Portfolio Summary:",
                center_x - 280, portfolio_y, 22, title_color);

    {
        LpPortfolio *portfolio = lp_game_data_get_portfolio (game_data);
        LpAgentManager *agent_mgr = lp_game_data_get_agent_manager (game_data);
        gchar str_buf[64];

        /* Gold display */
        if (portfolio != NULL)
        {
            LrgBigNumber *gold = lp_portfolio_get_gold (portfolio);
            guint inv_count = lp_portfolio_get_investment_count (portfolio);
            gdouble gold_val = lrg_big_number_to_double (gold);

            g_snprintf (str_buf, sizeof (str_buf), "Gold: %.0f gp", gold_val);
            draw_label (get_pool_label (self), str_buf,
                        center_x - 260, portfolio_y + 40, 18, gold_color);

            if (inv_count == 0)
            {
                draw_label (get_pool_label (self), "Investments: None",
                            center_x - 260, portfolio_y + 65, 18, text_color);
            }
            else
            {
                g_snprintf (str_buf, sizeof (str_buf), "Investments: %u", inv_count);
                draw_label (get_pool_label (self), str_buf,
                            center_x - 260, portfolio_y + 65, 18, text_color);
            }
        }
        else
        {
            draw_label (get_pool_label (self), "Gold: 0 gp",
                        center_x - 260, portfolio_y + 40, 18, gold_color);
            draw_label (get_pool_label (self), "Investments: None",
                        center_x - 260, portfolio_y + 65, 18, text_color);
        }

        /* Agent display */
        if (agent_mgr != NULL)
        {
            guint agent_count = lp_agent_manager_get_agent_count (agent_mgr);

            if (agent_count == 0)
            {
                draw_label (get_pool_label (self), "Agents: None",
                            center_x - 260, portfolio_y + 90, 18, text_color);
            }
            else
            {
                g_snprintf (str_buf, sizeof (str_buf), "Agents: %u", agent_count);
                draw_label (get_pool_label (self), str_buf,
                            center_x - 260, portfolio_y + 90, 18, text_color);
            }
        }
        else
        {
            draw_label (get_pool_label (self), "Agents: None",
                        center_x - 260, portfolio_y + 90, 18, text_color);
        }
    }

    /* Draw slumber growth chart if we have data */
    if (self->slumber_snapshots != NULL && self->slumber_snapshots->len > 0)
    {
        gint chart_x;
        gint chart_y;
        gint chart_w;
        gint chart_h;
        guint series_count;

        /* Position chart on right side of screen */
        chart_w = 450;
        chart_h = 280;
        chart_x = center_x + 40;
        chart_y = portfolio_y - 40;

        series_count = lrg_chart_get_series_count (LRG_CHART (self->slumber_chart));
        if (series_count > 0)
        {
            lrg_widget_set_position (LRG_WIDGET (self->slumber_chart),
                                      (gfloat)chart_x, (gfloat)chart_y);
            lrg_widget_set_size (LRG_WIDGET (self->slumber_chart),
                                  (gfloat)chart_w, (gfloat)chart_h);
            lrg_widget_draw (LRG_WIDGET (self->slumber_chart));
        }
    }

    /* Instructions */
    draw_label (get_pool_label (self), "Press ENTER or SPACE to continue...",
                center_x - 180, instructions_y, 16, dim_color);
}

static gboolean
lp_state_wake_handle_input (LrgGameState *state,
                            gpointer      event)
{
    /* Input handled in update via polling */
    (void)state;
    (void)event;
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_wake_finalize (GObject *object)
{
    LpStateWake *self = LP_STATE_WAKE (object);

    g_list_free_full (self->events, g_object_unref);

    G_OBJECT_CLASS (lp_state_wake_parent_class)->finalize (object);
}

static void
lp_state_wake_class_init (LpStateWakeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_wake_dispose;
    object_class->finalize = lp_state_wake_finalize;

    state_class->enter = lp_state_wake_enter;
    state_class->exit = lp_state_wake_exit;
    state_class->update = lp_state_wake_update;
    state_class->draw = lp_state_wake_draw;
    state_class->handle_input = lp_state_wake_handle_input;
}

static void
lp_state_wake_init (LpStateWake *self)
{
    guint i;
    g_autoptr(LrgChartAxisConfig) x_axis = NULL;
    g_autoptr(LrgChartAxisConfig) y_axis = NULL;

    lrg_game_state_set_name (LRG_GAME_STATE (self), "Wake");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->events = NULL;
    self->current_event = 0;
    self->slumber_snapshots = NULL;

    /* Create slumber growth chart */
    self->slumber_chart = lrg_line_chart2d_new ();
    lrg_chart_set_title (LRG_CHART (self->slumber_chart), "Slumber Growth");
    lrg_chart2d_set_show_legend (LRG_CHART2D (self->slumber_chart), FALSE);
    lrg_line_chart2d_set_show_markers (self->slumber_chart, TRUE);
    lrg_line_chart2d_set_smooth (self->slumber_chart, TRUE);
    lrg_line_chart2d_set_fill_area (self->slumber_chart, TRUE);
    lrg_line_chart2d_set_fill_opacity (self->slumber_chart, 0.3f);

    /* Configure axes */
    x_axis = lrg_chart_axis_config_new_with_title ("Year");
    lrg_chart_axis_config_set_show_grid (x_axis, TRUE);
    lrg_chart2d_set_x_axis (LRG_CHART2D (self->slumber_chart), x_axis);

    y_axis = lrg_chart_axis_config_new_with_title ("Value");
    lrg_chart_axis_config_set_show_grid (y_axis, TRUE);
    lrg_chart2d_set_y_axis (LRG_CHART2D (self->slumber_chart), y_axis);

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 12; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_wake_new:
 *
 * Creates a new wake report state.
 *
 * Returns: (transfer full): A new #LpStateWake
 */
LpStateWake *
lp_state_wake_new (void)
{
    return g_object_new (LP_TYPE_STATE_WAKE, NULL);
}

/**
 * lp_state_wake_set_events:
 * @self: an #LpStateWake
 * @events: (transfer full) (element-type LpEvent): List of events
 *
 * Sets the events to display.
 */
void
lp_state_wake_set_events (LpStateWake *self,
                          GList       *events)
{
    g_return_if_fail (LP_IS_STATE_WAKE (self));

    g_list_free_full (self->events, g_object_unref);
    self->events = events;
    self->current_event = 0;
}

/**
 * lp_state_wake_set_slumber_snapshots:
 * @self: an #LpStateWake
 * @snapshots: (transfer full) (element-type LpPortfolioSnapshot): Portfolio snapshots
 *
 * Sets the portfolio snapshots from the slumber period for charting.
 */
void
lp_state_wake_set_slumber_snapshots (LpStateWake *self,
                                      GPtrArray   *snapshots)
{
    LrgChartDataSeries *series;
    guint i;

    g_return_if_fail (LP_IS_STATE_WAKE (self));

    /* Store snapshots */
    if (self->slumber_snapshots != NULL)
        g_ptr_array_unref (self->slumber_snapshots);
    self->slumber_snapshots = snapshots;

    /* Clear and rebuild chart data */
    lrg_chart_clear_series (LRG_CHART (self->slumber_chart));

    if (snapshots == NULL || snapshots->len == 0)
        return;

    /* Create series from snapshots */
    series = lrg_chart_data_series_new ("Portfolio");
    lrg_chart_data_series_set_color (series, lp_theme_get_gold_color ());

    for (i = 0; i < snapshots->len; i++)
    {
        LpPortfolioSnapshot *snapshot;
        LrgBigNumber *value;
        guint64 year;
        gdouble value_double;
        LrgChartDataPoint *pt;

        snapshot = g_ptr_array_index (snapshots, i);
        year = lp_portfolio_snapshot_get_year (snapshot);
        value = lp_portfolio_snapshot_get_total_value (snapshot);
        value_double = lrg_big_number_to_double (value);

        pt = lrg_chart_data_point_new ((gdouble)year, value_double);
        lrg_chart_data_series_add_point_full (series, pt);
    }

    lrg_chart_add_series (LRG_CHART (self->slumber_chart), series);
    lrg_chart_animate_to_data (LRG_CHART (self->slumber_chart), LRG_CHART_ANIM_GROW, 0.5f);
}
