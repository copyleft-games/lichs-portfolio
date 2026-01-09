/* lp-state-simulating.c - Slumber Simulation Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-simulating.h"
#include "lp-state-wake.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../core/lp-portfolio-history.h"
#include "../investment/lp-portfolio.h"
#include "../simulation/lp-world-simulation.h"
#include <graylib.h>
#include <libregnum.h>

/* Simulation speed (years per second displayed) */
#define SIMULATION_SPEED (2.0)

struct _LpStateSimulating
{
    LrgGameState parent_instance;

    guint    total_years;       /* Total years to simulate */
    guint    current_year;      /* Current year being displayed */
    gdouble  accumulated_time;  /* Time accumulator for year progression */
    gboolean complete;          /* Whether simulation is complete */
    GList   *events;            /* Accumulated events */

    /* UI Labels */
    LrgLabel *label_title;
    LrgLabel *label_hint;
    LrgLabel *label_year;
};

enum
{
    SIGNAL_SIMULATION_COMPLETE,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpStateSimulating, lp_state_simulating, LRG_TYPE_GAME_STATE)

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

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_simulating_dispose (GObject *object)
{
    LpStateSimulating *self = LP_STATE_SIMULATING (object);

    g_clear_object (&self->label_title);
    g_clear_object (&self->label_hint);
    g_clear_object (&self->label_year);

    G_OBJECT_CLASS (lp_state_simulating_parent_class)->dispose (object);
}

/* ==========================================================================
 * Signal Callbacks
 * ========================================================================== */

static void
on_simulation_complete (LpStateSimulating *self,
                        gpointer           user_data)
{
    LpGame *game;
    LpGameData *game_data;
    LrgGameStateManager *manager;
    LpStateWake *wake_state;
    GList *events;
    GPtrArray *slumber_snapshots;
    LpPortfolio *portfolio;
    LpWorldSimulation *world;
    g_autoptr(LrgBigNumber) start_total = NULL;
    LrgBigNumber *start_gold;
    g_autoptr(LrgBigNumber) start_investment = NULL;
    g_autoptr(LrgBigNumber) end_total = NULL;
    LrgBigNumber *end_gold;
    g_autoptr(LrgBigNumber) end_investment = NULL;
    guint64 start_year;
    guint64 end_year;
    LpPortfolioSnapshot *snapshot;

    (void)user_data;

    lp_log_info ("Simulation complete, processing %u years of slumber",
                 self->total_years);

    game = lp_game_get_from_state (LRG_GAME_STATE (self));
    game_data = lp_game_get_game_data (game);
    manager = lrg_game_template_get_state_manager (
        LRG_GAME_TEMPLATE (game));
    portfolio = lp_game_data_get_portfolio (game_data);
    world = lp_game_data_get_world_simulation (game_data);

    /* Capture starting portfolio state */
    start_year = lp_world_simulation_get_current_year (world);
    start_total = lp_portfolio_get_total_value (portfolio);
    start_gold = lp_portfolio_get_gold (portfolio);
    start_investment = lp_portfolio_get_investment_value (portfolio);

    /* Process actual slumber - advances world, calculates returns, etc. */
    events = lp_game_data_slumber (game_data, self->total_years);

    /* Capture ending portfolio state */
    end_year = lp_world_simulation_get_current_year (world);
    end_total = lp_portfolio_get_total_value (portfolio);
    end_gold = lp_portfolio_get_gold (portfolio);
    end_investment = lp_portfolio_get_investment_value (portfolio);

    /* Create snapshots array with start and end values */
    slumber_snapshots = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_portfolio_snapshot_free);

    snapshot = lp_portfolio_snapshot_new (start_year, start_total,
                                           start_gold, start_investment);
    g_ptr_array_add (slumber_snapshots, snapshot);

    snapshot = lp_portfolio_snapshot_new (end_year, end_total,
                                           end_gold, end_investment);
    g_ptr_array_add (slumber_snapshots, snapshot);

    /* Create wake state and pass events + snapshots */
    wake_state = lp_state_wake_new ();
    lp_state_wake_set_events (wake_state, events);
    lp_state_wake_set_slumber_snapshots (wake_state, slumber_snapshots);

    /* Replace simulating with wake state */
    lrg_game_state_manager_replace (manager, LRG_GAME_STATE (wake_state));
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_simulating_enter (LrgGameState *state)
{
    LpStateSimulating *self = LP_STATE_SIMULATING (state);

    lp_log_info ("Entering simulation for %u years", self->total_years);

    self->current_year = 0;
    self->accumulated_time = 0.0;
    self->complete = FALSE;

    /* Connect to complete signal for state transition */
    g_signal_connect (self, "simulation-complete",
                      G_CALLBACK (on_simulation_complete), NULL);
}

static void
lp_state_simulating_exit (LrgGameState *state)
{
    LpStateSimulating *self = LP_STATE_SIMULATING (state);

    lp_log_info ("Exiting simulation");

    /* Events are transferred to wake state, so don't free here */
    self->events = NULL;
}

static void
lp_state_simulating_update (LrgGameState *state,
                            gdouble       delta)
{
    LpStateSimulating *self = LP_STATE_SIMULATING (state);

    if (self->complete)
        return;

    self->accumulated_time += delta;

    /* Advance displayed year based on time */
    while (self->accumulated_time >= (1.0 / SIMULATION_SPEED) &&
           self->current_year < self->total_years)
    {
        self->accumulated_time -= (1.0 / SIMULATION_SPEED);
        self->current_year++;

        lp_log_debug ("Simulating year %u of %u",
                      self->current_year, self->total_years);
    }

    /* Check for completion */
    if (self->current_year >= self->total_years)
    {
        self->complete = TRUE;
        g_signal_emit (self, signals[SIGNAL_SIMULATION_COMPLETE], 0);
    }
}

static void
lp_state_simulating_draw (LrgGameState *state)
{
    LpStateSimulating *self = LP_STATE_SIMULATING (state);
    LpGame *game = lp_game_get_from_state (state);
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) progress_color = NULL;
    g_autoptr(GrlColor) bar_bg_color = NULL;
    g_autoptr(GrlColor) hint_color = NULL;
    gint screen_w, screen_h;
    gint center_x, center_y;
    gchar year_str[64];
    gint bar_width, bar_height, bar_x, bar_y;
    gint progress_width;

    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
    center_x = screen_w / 2;
    center_y = screen_h / 2;

    /* Colors */
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    progress_color = grl_color_new (100, 80, 140, 255);
    bar_bg_color = grl_color_new (40, 40, 50, 255);
    hint_color = grl_color_new (255, 215, 0, 255);

    /* Title */
    draw_label (self->label_title, "SLUMBERING...",
                center_x - 130, center_y - 120, 42, title_color);

    /* Malachar's hint */
    draw_label (self->label_hint, "\"Time flows like sand through an hourglass...\"",
                center_x - 220, center_y - 60, 18, hint_color);

    /* Year counter */
    g_snprintf (year_str, sizeof (year_str), "Year %u of %u",
                self->current_year, self->total_years);
    draw_label (self->label_year, year_str,
                center_x - 80, center_y, 24, text_color);

    /* Progress bar */
    bar_width = 400;
    bar_height = 30;
    bar_x = center_x - bar_width / 2;
    bar_y = center_y + 50;

    /* Bar background */
    grl_draw_rectangle (bar_x, bar_y, bar_width, bar_height, bar_bg_color);

    /* Bar fill */
    if (self->total_years > 0)
    {
        progress_width = (bar_width * self->current_year) / self->total_years;
        grl_draw_rectangle (bar_x, bar_y, progress_width, bar_height, progress_color);
    }
}

static gboolean
lp_state_simulating_handle_input (LrgGameState *state,
                                  gpointer      event)
{
    /*
     * Phase 1 skeleton: Basic input handling.
     * - Space: Speed up simulation
     * - Escape: Cannot cancel mid-simulation
     */
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_simulating_class_init (LpStateSimulatingClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_simulating_dispose;

    state_class->enter = lp_state_simulating_enter;
    state_class->exit = lp_state_simulating_exit;
    state_class->update = lp_state_simulating_update;
    state_class->draw = lp_state_simulating_draw;
    state_class->handle_input = lp_state_simulating_handle_input;

    /**
     * LpStateSimulating::simulation-complete:
     * @self: the #LpStateSimulating
     *
     * Emitted when the simulation visualization is complete.
     */
    signals[SIGNAL_SIMULATION_COMPLETE] =
        g_signal_new ("simulation-complete",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_state_simulating_init (LpStateSimulating *self)
{
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Simulating");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->total_years = 10;
    self->current_year = 0;
    self->accumulated_time = 0.0;
    self->complete = FALSE;
    self->events = NULL;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);
    self->label_hint = lrg_label_new (NULL);
    self->label_year = lrg_label_new (NULL);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_simulating_new:
 *
 * Creates a new slumber simulation state.
 *
 * Returns: (transfer full): A new #LpStateSimulating
 */
LpStateSimulating *
lp_state_simulating_new (void)
{
    return g_object_new (LP_TYPE_STATE_SIMULATING, NULL);
}

/**
 * lp_state_simulating_set_years:
 * @self: an #LpStateSimulating
 * @years: number of years to simulate
 *
 * Sets the number of years to simulate.
 */
void
lp_state_simulating_set_years (LpStateSimulating *self,
                               guint              years)
{
    g_return_if_fail (LP_IS_STATE_SIMULATING (self));

    self->total_years = years;
}
