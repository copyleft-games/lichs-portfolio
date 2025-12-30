/* lp-state-analyze.c - World Analysis Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-analyze.h"

struct _LpStateAnalyze
{
    LrgGameState parent_instance;

    /* UI state would go here in later phases */
};

G_DEFINE_TYPE (LpStateAnalyze, lp_state_analyze, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_analyze_enter (LrgGameState *state)
{
    lp_log_info ("Entering analyze state");
}

static void
lp_state_analyze_exit (LrgGameState *state)
{
    lp_log_info ("Exiting analyze state");
}

static void
lp_state_analyze_update (LrgGameState *state,
                         gdouble       delta)
{
    /* Update UI animations, tooltips, etc. */
}

static void
lp_state_analyze_draw (LrgGameState *state)
{
    /*
     * Phase 1 skeleton: Placeholder drawing.
     * Full UI will show:
     * - World map with regions
     * - Portfolio summary sidebar
     * - Agent status panel
     * - Available investments
     * - Exposure meter
     */
    lp_log_debug ("Drawing analyze state (skeleton)");
}

static gboolean
lp_state_analyze_handle_input (LrgGameState *state,
                               gpointer      event)
{
    /*
     * Phase 1 skeleton: Basic input handling.
     * - Tab: Cycle through panels
     * - Click: Select investment/agent
     * - Escape: Open pause menu
     * - S: Enter slumber configuration
     */
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_analyze_class_init (LpStateAnalyzeClass *klass)
{
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    state_class->enter = lp_state_analyze_enter;
    state_class->exit = lp_state_analyze_exit;
    state_class->update = lp_state_analyze_update;
    state_class->draw = lp_state_analyze_draw;
    state_class->handle_input = lp_state_analyze_handle_input;
}

static void
lp_state_analyze_init (LpStateAnalyze *self)
{
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Analyze");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_analyze_new:
 *
 * Creates a new world analysis state.
 *
 * Returns: (transfer full): A new #LpStateAnalyze
 */
LpStateAnalyze *
lp_state_analyze_new (void)
{
    return g_object_new (LP_TYPE_STATE_ANALYZE, NULL);
}
