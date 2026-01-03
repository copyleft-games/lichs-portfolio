/* lp-state-main-menu.c - Main Menu Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-main-menu.h"
#include "lp-state-wake.h"
#include "lp-state-settings.h"
#include "../core/lp-game.h"
#include <graylib.h>
#include <libregnum.h>

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
lp_state_main_menu_draw (LrgGameState *state)
{
    LpStateMainMenu *self = LP_STATE_MAIN_MENU (state);
    LpGame *game = lp_game_get_from_state (state);
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) selected_color = NULL;
    g_autoptr(GrlColor) normal_color = NULL;
    g_autoptr(GrlColor) disabled_color = NULL;
    gint screen_w, screen_h;
    gint center_x, center_y;
    gint title_y, subtitle_y, menu_y, instructions_y;
    gint y_spacing;
    gint i;

    const gchar *menu_items[] = {
        "New Game",
        "Continue",
        "Settings",
        "Quit"
    };

    /* Get current window dimensions */
    lrg_game_template_get_window_size (LRG_GAME_TEMPLATE (game),
                                        &screen_w, &screen_h);
    center_x = screen_w / 2;
    center_y = screen_h / 2;

    /* Calculate vertical positions based on screen height */
    title_y = screen_h / 7;
    subtitle_y = title_y + 60;
    menu_y = center_y - 60;
    y_spacing = 50;
    instructions_y = screen_h - 120;

    /* Colors for the dark lich theme */
    title_color = grl_color_new (180, 150, 200, 255);     /* Purple-ish */
    selected_color = grl_color_new (255, 215, 0, 255);    /* Gold */
    normal_color = grl_color_new (200, 200, 200, 255);    /* Light gray */
    disabled_color = grl_color_new (100, 100, 100, 255);  /* Dark gray */

    /* Draw title */
    grl_draw_text ("LICH'S PORTFOLIO", center_x - 200, title_y, 48, title_color);

    /* Draw subtitle */
    grl_draw_text ("An Immortal Investment Strategy", center_x - 180, subtitle_y, 20, normal_color);

    /* Draw menu options */
    for (i = 0; i < MENU_OPTION_COUNT; i++)
    {
        GrlColor *color;
        gint x_offset;

        /* Determine color based on state */
        if (i == self->selected_option)
        {
            color = selected_color;
            x_offset = -10;  /* Indent selected item */
        }
        else if (i == MENU_OPTION_CONTINUE && !self->has_save_file)
        {
            color = disabled_color;
            x_offset = 0;
        }
        else
        {
            color = normal_color;
            x_offset = 0;
        }

        /* Draw selection indicator for selected item */
        if (i == self->selected_option)
        {
            grl_draw_text (">", center_x - 100 + x_offset, menu_y + (i * y_spacing), 24, color);
        }

        grl_draw_text (menu_items[i], center_x - 70 + x_offset, menu_y + (i * y_spacing), 24, color);
    }

    /* Draw instructions at bottom */
    grl_draw_text ("Use UP/DOWN to navigate, ENTER to select",
                   center_x - 200, instructions_y, 16, disabled_color);
}

static void
lp_state_main_menu_update (LrgGameState *state,
                           gdouble       delta)
{
    LpStateMainMenu *self = LP_STATE_MAIN_MENU (state);

    (void)delta;

    /* Handle input in update since we poll input each frame */

    /* Navigate up */
    if (grl_input_is_key_pressed (GRL_KEY_UP) ||
        grl_input_is_key_pressed (GRL_KEY_W))
    {
        self->selected_option--;
        if (self->selected_option < 0)
        {
            self->selected_option = MENU_OPTION_COUNT - 1;
        }
    }

    /* Navigate down */
    if (grl_input_is_key_pressed (GRL_KEY_DOWN) ||
        grl_input_is_key_pressed (GRL_KEY_S))
    {
        self->selected_option++;
        if (self->selected_option >= MENU_OPTION_COUNT)
        {
            self->selected_option = 0;
        }
    }

    /* Select option */
    if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        switch (self->selected_option)
        {
        case MENU_OPTION_NEW_GAME:
            lp_log_info ("New Game selected");
            {
                LpGame *game = lp_game_get_from_state (state);
                LrgGameStateManager *manager;

                /* Start a new game */
                lp_game_new_game (game);

                /* Replace main menu with wake state */
                manager = lrg_game_template_get_state_manager (
                    LRG_GAME_TEMPLATE (game));
                lrg_game_state_manager_replace (manager,
                    LRG_GAME_STATE (lp_state_wake_new ()));
            }
            break;

        case MENU_OPTION_CONTINUE:
            if (self->has_save_file)
            {
                lp_log_info ("Continue selected");
                /* TODO: Load game and transition */
            }
            break;

        case MENU_OPTION_SETTINGS:
            lp_log_info ("Settings selected");
            {
                LpGame *game = lp_game_get_from_state (state);
                LrgGameStateManager *manager;

                manager = lrg_game_template_get_state_manager (
                    LRG_GAME_TEMPLATE (game));
                lrg_game_state_manager_push (manager,
                    LRG_GAME_STATE (lp_state_settings_new ()));
            }
            break;

        case MENU_OPTION_QUIT:
            lp_log_info ("Quit selected");
            {
                LpGame *game = lp_game_get_from_state (state);
                lrg_game_template_quit (LRG_GAME_TEMPLATE (game));
            }
            break;

        default:
            break;
        }
    }

    /*
     * Note: ESC does not quit from main menu.
     * Use the Quit menu option instead.
     * This prevents conflict with ESC-to-return from settings overlay.
     */
}

static gboolean
lp_state_main_menu_handle_input (LrgGameState *state,
                                 gpointer      event)
{
    /* Input is handled in update() via polling */
    (void)state;
    (void)event;

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
