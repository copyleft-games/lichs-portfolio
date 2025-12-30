/* lp-state-simulating.c - Slumber Simulation Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-simulating.h"

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
};

enum
{
    SIGNAL_SIMULATION_COMPLETE,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpStateSimulating, lp_state_simulating, LRG_TYPE_GAME_STATE)

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
    /*
     * Phase 1 skeleton: Placeholder drawing.
     * Full UI will show:
     * - Year counter animation
     * - Progress bar
     * - Key events as they occur
     * - Background world map effects
     */
    lp_log_debug ("Drawing simulating state (skeleton)");
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
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

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
