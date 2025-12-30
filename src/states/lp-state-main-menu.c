/* lp-state-main-menu.c - Main Menu Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-main-menu.h"

/* Menu options */
typedef enum
{
    MENU_OPTION_NEW_GAME,
    MENU_OPTION_CONTINUE,
    MENU_OPTION_SETTINGS,
    MENU_OPTION_QUIT,
    MENU_OPTION_COUNT
} MenuOption;

struct _LpStateMainMenu
{
    LrgGameState parent_instance;

    gint      selected_option;
    gboolean  has_save_file;
};

G_DEFINE_TYPE (LpStateMainMenu, lp_state_main_menu, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_main_menu_enter (LrgGameState *state)
{
    LpStateMainMenu *self = LP_STATE_MAIN_MENU (state);

    lp_log_info ("Entering main menu");

    self->selected_option = MENU_OPTION_NEW_GAME;
    self->has_save_file = FALSE;  /* TODO: Check for save file */
}

static void
lp_state_main_menu_exit (LrgGameState *state)
{
    lp_log_info ("Exiting main menu");
}

static void
lp_state_main_menu_update (LrgGameState *state,
                           gdouble       delta)
{
    /*
     * Main menu has minimal update logic.
     * Could animate background effects, title, etc.
     */
}

static void
lp_state_main_menu_draw (LrgGameState *state)
{
    /*
     * Phase 1 skeleton: Minimal drawing.
     * Full UI will be implemented in later phases.
     *
     * TODO: Draw menu options with proper styling:
     * - Title: "Lich's Portfolio"
     * - New Game
     * - Continue (grayed if no save)
     * - Settings
     * - Quit
     */

    /* Placeholder - actual rendering would use graylib */
    lp_log_debug ("Drawing main menu (skeleton)");
}

static gboolean
lp_state_main_menu_handle_input (LrgGameState *state,
                                 gpointer      event)
{
    LpStateMainMenu *self = LP_STATE_MAIN_MENU (state);

    /*
     * Phase 1 skeleton: Basic input handling placeholder.
     * Full input handling with proper UI navigation in later phases.
     *
     * Expected inputs:
     * - Up/Down: Navigate menu
     * - Enter/Space: Select option
     * - Escape: Quit (from menu)
     */

    /* Placeholder for now */
    (void)self;

    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_main_menu_class_init (LpStateMainMenuClass *klass)
{
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    state_class->enter = lp_state_main_menu_enter;
    state_class->exit = lp_state_main_menu_exit;
    state_class->update = lp_state_main_menu_update;
    state_class->draw = lp_state_main_menu_draw;
    state_class->handle_input = lp_state_main_menu_handle_input;
}

static void
lp_state_main_menu_init (LpStateMainMenu *self)
{
    lrg_game_state_set_name (LRG_GAME_STATE (self), "MainMenu");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->selected_option = MENU_OPTION_NEW_GAME;
    self->has_save_file = FALSE;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_main_menu_new:
 *
 * Creates a new main menu state.
 *
 * Returns: (transfer full): A new #LpStateMainMenu
 */
LpStateMainMenu *
lp_state_main_menu_new (void)
{
    return g_object_new (LP_TYPE_STATE_MAIN_MENU, NULL);
}
