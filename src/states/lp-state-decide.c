/* lp-state-decide.c - Decision Making Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-decide.h"

struct _LpStateDecide
{
    LrgGameState parent_instance;

    /* Decision state would go here in later phases */
};

G_DEFINE_TYPE (LpStateDecide, lp_state_decide, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_decide_enter (LrgGameState *state)
{
    lp_log_info ("Entering decide state");
}

static void
lp_state_decide_exit (LrgGameState *state)
{
    lp_log_info ("Exiting decide state");
}

static void
lp_state_decide_update (LrgGameState *state,
                        gdouble       delta)
{
    /* Update decision UI state */
}

static void
lp_state_decide_draw (LrgGameState *state)
{
    /*
     * Phase 1 skeleton: Placeholder drawing.
     * Full UI will show:
     * - Investment purchase/sell dialogs
     * - Agent recruitment/assignment
     * - Phylactery upgrades
     */
    lp_log_debug ("Drawing decide state (skeleton)");
}

static gboolean
lp_state_decide_handle_input (LrgGameState *state,
                              gpointer      event)
{
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_decide_class_init (LpStateDecideClass *klass)
{
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    state_class->enter = lp_state_decide_enter;
    state_class->exit = lp_state_decide_exit;
    state_class->update = lp_state_decide_update;
    state_class->draw = lp_state_decide_draw;
    state_class->handle_input = lp_state_decide_handle_input;
}

static void
lp_state_decide_init (LpStateDecide *self)
{
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Decide");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_decide_new:
 *
 * Creates a new decision making state.
 *
 * Returns: (transfer full): A new #LpStateDecide
 */
LpStateDecide *
lp_state_decide_new (void)
{
    return g_object_new (LP_TYPE_STATE_DECIDE, NULL);
}
