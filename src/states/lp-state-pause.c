/* lp-state-pause.c - Pause Menu Overlay State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-pause.h"

/* Pause menu options */
typedef enum
{
    PAUSE_OPTION_RESUME,
    PAUSE_OPTION_SAVE,
    PAUSE_OPTION_LOAD,
    PAUSE_OPTION_SETTINGS,
    PAUSE_OPTION_MAIN_MENU,
    PAUSE_OPTION_QUIT,
    PAUSE_OPTION_COUNT
} PauseOption;

struct _LpStatePause
{
    LrgGameState parent_instance;

    gint selected_option;
};

G_DEFINE_TYPE (LpStatePause, lp_state_pause, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_pause_enter (LrgGameState *state)
{
    LpStatePause *self = LP_STATE_PAUSE (state);

    lp_log_info ("Game paused");

    self->selected_option = PAUSE_OPTION_RESUME;
}

static void
lp_state_pause_exit (LrgGameState *state)
{
    lp_log_info ("Game unpaused");
}

static void
lp_state_pause_update (LrgGameState *state,
                       gdouble       delta)
{
    /* Pause menu doesn't need much update logic */
}

static void
lp_state_pause_draw (LrgGameState *state)
{
    /*
     * Phase 1 skeleton: Placeholder drawing.
     * Full UI will show:
     * - Semi-transparent overlay
     * - "PAUSED" title
     * - Menu options
     */
    lp_log_debug ("Drawing pause state (skeleton)");
}

static gboolean
lp_state_pause_handle_input (LrgGameState *state,
                             gpointer      event)
{
    /*
     * Phase 1 skeleton: Basic input handling.
     * - Up/Down: Navigate menu
     * - Enter: Select option
     * - Escape: Resume game
     */
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_pause_class_init (LpStatePauseClass *klass)
{
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    state_class->enter = lp_state_pause_enter;
    state_class->exit = lp_state_pause_exit;
    state_class->update = lp_state_pause_update;
    state_class->draw = lp_state_pause_draw;
    state_class->handle_input = lp_state_pause_handle_input;
}

static void
lp_state_pause_init (LpStatePause *self)
{
    /* Pause menu is transparent (shows game beneath) */
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Pause");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), TRUE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->selected_option = PAUSE_OPTION_RESUME;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_pause_new:
 *
 * Creates a new pause menu overlay state.
 *
 * Returns: (transfer full): A new #LpStatePause
 */
LpStatePause *
lp_state_pause_new (void)
{
    return g_object_new (LP_TYPE_STATE_PAUSE, NULL);
}
