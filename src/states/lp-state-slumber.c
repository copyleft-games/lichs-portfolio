/* lp-state-slumber.c - Slumber Configuration Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-slumber.h"
#include "../tutorial/lp-tutorial-sequences.h"

/* Default and range for slumber duration */
#define DEFAULT_SLUMBER_YEARS (10)
#define MIN_SLUMBER_YEARS     (1)
#define MAX_SLUMBER_YEARS     (100)

struct _LpStateSlumber
{
    LrgGameState parent_instance;

    guint slumber_years;    /* Configured slumber duration */
};

G_DEFINE_TYPE (LpStateSlumber, lp_state_slumber, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_slumber_enter (LrgGameState *state)
{
    LpStateSlumber *self = LP_STATE_SLUMBER (state);

    lp_log_info ("Entering slumber configuration");

    self->slumber_years = DEFAULT_SLUMBER_YEARS;

    /* Maybe start slumber tutorial for first-time users */
    lp_tutorial_sequences_maybe_start_slumber (
        lp_tutorial_sequences_get_default ());
}

static void
lp_state_slumber_exit (LrgGameState *state)
{
    lp_log_info ("Exiting slumber configuration");

    /*
     * TODO: When application has transition manager, trigger fade-out here.
     * The player is "entering slumber" so we'd fade to black.
     */
}

static void
lp_state_slumber_update (LrgGameState *state,
                         gdouble       delta)
{
    /* Update UI animations */
}

static void
lp_state_slumber_draw (LrgGameState *state)
{
    /*
     * Phase 1 skeleton: Placeholder drawing.
     * Full UI will show:
     * - Year slider/selector
     * - Standing orders configuration
     * - Projected outcomes
     * - Confirm/Cancel buttons
     */
    lp_log_debug ("Drawing slumber state (skeleton)");
}

static gboolean
lp_state_slumber_handle_input (LrgGameState *state,
                               gpointer      event)
{
    /*
     * Phase 1 skeleton: Basic input handling.
     * - Up/Down: Adjust years
     * - Enter: Confirm and begin slumber
     * - Escape: Cancel and return
     */
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_slumber_class_init (LpStateSlumberClass *klass)
{
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    state_class->enter = lp_state_slumber_enter;
    state_class->exit = lp_state_slumber_exit;
    state_class->update = lp_state_slumber_update;
    state_class->draw = lp_state_slumber_draw;
    state_class->handle_input = lp_state_slumber_handle_input;
}

static void
lp_state_slumber_init (LpStateSlumber *self)
{
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Slumber");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->slumber_years = DEFAULT_SLUMBER_YEARS;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_slumber_new:
 *
 * Creates a new slumber configuration state.
 *
 * Returns: (transfer full): A new #LpStateSlumber
 */
LpStateSlumber *
lp_state_slumber_new (void)
{
    return g_object_new (LP_TYPE_STATE_SLUMBER, NULL);
}

/**
 * lp_state_slumber_get_years:
 * @self: an #LpStateSlumber
 *
 * Gets the configured slumber duration.
 *
 * Returns: Number of years
 */
guint
lp_state_slumber_get_years (LpStateSlumber *self)
{
    g_return_val_if_fail (LP_IS_STATE_SLUMBER (self), DEFAULT_SLUMBER_YEARS);

    return self->slumber_years;
}
