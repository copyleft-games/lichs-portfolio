/* lp-state-settings.c - Settings Menu Overlay State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-settings.h"

/* Settings tabs */
typedef enum
{
    SETTINGS_TAB_GRAPHICS,
    SETTINGS_TAB_AUDIO,
    SETTINGS_TAB_GAMEPLAY,
    SETTINGS_TAB_CONTROLS,
    SETTINGS_TAB_COUNT
} SettingsTab;

struct _LpStateSettings
{
    LrgGameState parent_instance;

    SettingsTab current_tab;
    gint        selected_option;
};

G_DEFINE_TYPE (LpStateSettings, lp_state_settings, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_settings_enter (LrgGameState *state)
{
    LpStateSettings *self = LP_STATE_SETTINGS (state);

    lp_log_info ("Entering settings");

    self->current_tab = SETTINGS_TAB_GRAPHICS;
    self->selected_option = 0;
}

static void
lp_state_settings_exit (LrgGameState *state)
{
    lp_log_info ("Exiting settings");
}

static void
lp_state_settings_update (LrgGameState *state,
                          gdouble       delta)
{
    /* Settings menu animation updates */
}

static void
lp_state_settings_draw (LrgGameState *state)
{
    /*
     * Phase 1 skeleton: Placeholder drawing.
     * Full UI will show:
     * - Tab bar (Graphics, Audio, Gameplay, Controls)
     * - Settings list for selected tab
     * - Apply/Cancel buttons
     */
    lp_log_debug ("Drawing settings state (skeleton)");
}

static gboolean
lp_state_settings_handle_input (LrgGameState *state,
                                gpointer      event)
{
    /*
     * Phase 1 skeleton: Basic input handling.
     * - Tab/Shift+Tab: Switch tabs
     * - Up/Down: Navigate options
     * - Left/Right: Adjust values
     * - Enter: Apply changes
     * - Escape: Cancel and return
     */
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_settings_class_init (LpStateSettingsClass *klass)
{
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    state_class->enter = lp_state_settings_enter;
    state_class->exit = lp_state_settings_exit;
    state_class->update = lp_state_settings_update;
    state_class->draw = lp_state_settings_draw;
    state_class->handle_input = lp_state_settings_handle_input;
}

static void
lp_state_settings_init (LpStateSettings *self)
{
    /* Settings menu is transparent (can show from main menu or pause) */
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Settings");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), TRUE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->current_tab = SETTINGS_TAB_GRAPHICS;
    self->selected_option = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_settings_new:
 *
 * Creates a new settings menu overlay state.
 *
 * Returns: (transfer full): A new #LpStateSettings
 */
LpStateSettings *
lp_state_settings_new (void)
{
    return g_object_new (LP_TYPE_STATE_SETTINGS, NULL);
}
