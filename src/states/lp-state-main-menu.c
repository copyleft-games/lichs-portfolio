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

    /* UI Labels */
    LrgLabel  *label_title;
    LrgLabel  *label_subtitle;
    LrgLabel  *label_instructions;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpStateMainMenu, lp_state_main_menu, LRG_TYPE_GAME_STATE)

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
get_pool_label (LpStateMainMenu *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateMainMenu *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_main_menu_dispose (GObject *object)
{
    LpStateMainMenu *self = LP_STATE_MAIN_MENU (object);

    g_clear_object (&self->label_title);
    g_clear_object (&self->label_subtitle);
    g_clear_object (&self->label_instructions);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_main_menu_parent_class)->dispose (object);
}

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

    /* Reset label pool for this frame */
    reset_label_pool (self);

    /* Get virtual resolution (render target size) for UI positioning */
    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
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
    draw_label (self->label_title, "LICH'S PORTFOLIO",
                center_x - 200, title_y, 48, title_color);

    /* Draw subtitle */
    draw_label (self->label_subtitle, "An Immortal Investment Strategy",
                center_x - 180, subtitle_y, 20, normal_color);

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
            draw_label (get_pool_label (self), ">",
                        center_x - 100 + x_offset, menu_y + (i * y_spacing), 24, color);
        }

        draw_label (get_pool_label (self), menu_items[i],
                    center_x - 70 + x_offset, menu_y + (i * y_spacing), 24, color);
    }

    /* Draw instructions at bottom */
    draw_label (self->label_instructions, "Use UP/DOWN to navigate, ENTER to select",
                center_x - 200, instructions_y, 16, disabled_color);
}

static void
lp_state_main_menu_update (LrgGameState *state,
                           gdouble       delta)
{
    LpStateMainMenu *self = LP_STATE_MAIN_MENU (state);

    (void)delta;

    /* Handle input in update since we poll input each frame */

    /* Navigate up (including vim keys) */
    if (grl_input_is_key_pressed (GRL_KEY_UP) ||
        grl_input_is_key_pressed (GRL_KEY_K))
    {
        self->selected_option--;
        if (self->selected_option < 0)
        {
            self->selected_option = MENU_OPTION_COUNT - 1;
        }
    }

    /* Navigate down (including vim keys) */
    if (grl_input_is_key_pressed (GRL_KEY_DOWN) ||
        grl_input_is_key_pressed (GRL_KEY_J))
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
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_main_menu_dispose;

    state_class->enter = lp_state_main_menu_enter;
    state_class->exit = lp_state_main_menu_exit;
    state_class->update = lp_state_main_menu_update;
    state_class->draw = lp_state_main_menu_draw;
    state_class->handle_input = lp_state_main_menu_handle_input;
}

static void
lp_state_main_menu_init (LpStateMainMenu *self)
{
    guint i;

    lrg_game_state_set_name (LRG_GAME_STATE (self), "MainMenu");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->selected_option = MENU_OPTION_NEW_GAME;
    self->has_save_file = FALSE;

    /* Create dedicated labels for static text */
    self->label_title = lrg_label_new (NULL);
    self->label_subtitle = lrg_label_new (NULL);
    self->label_instructions = lrg_label_new (NULL);

    /* Create label pool for dynamic text (menu items + selection indicators) */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 10; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
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
