/* lp-state-pause.c - Pause Menu Overlay State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-pause.h"
#include "lp-state-main-menu.h"
#include "lp-state-settings.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../save/lp-save-manager.h"
#include "../ui/lp-theme.h"

/* Pause menu options */
typedef enum
{
    PAUSE_OPTION_RESUME,
    PAUSE_OPTION_SAVE,
    PAUSE_OPTION_SETTINGS,
    PAUSE_OPTION_MAIN_MENU,
    PAUSE_OPTION_COUNT
} PauseOption;

/* Menu option labels */
static const gchar *option_labels[PAUSE_OPTION_COUNT] =
{
    "Resume",
    "Save Game",
    "Settings",
    "Quit to Menu"
};

/* Label pool size for dynamic text */
#define LABEL_POOL_SIZE (10)

struct _LpStatePause
{
    LrgGameState parent_instance;

    gint selected_option;

    /* UI Labels */
    GPtrArray *label_pool;
    guint      label_pool_index;

    /* Save feedback */
    gboolean   show_save_feedback;
    gfloat     save_feedback_timer;
    gboolean   save_success;
};

G_DEFINE_TYPE (LpStatePause, lp_state_pause, LRG_TYPE_GAME_STATE)

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
get_pool_label (LpStatePause *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStatePause *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Action Handlers
 * ========================================================================== */

static void
pause_do_save (LpStatePause *self)
{
    LpGame *game;
    LpGameData *game_data;
    LpSaveManager *save_mgr;
    g_autoptr(GError) error = NULL;

    game = lp_game_get_from_state (LRG_GAME_STATE (self));
    if (game == NULL)
    {
        lp_log_warning ("Cannot save: no game instance");
        self->save_success = FALSE;
        self->show_save_feedback = TRUE;
        self->save_feedback_timer = 2.0f;
        return;
    }

    game_data = lp_game_get_game_data (game);
    if (game_data == NULL)
    {
        lp_log_warning ("Cannot save: no game data");
        self->save_success = FALSE;
        self->show_save_feedback = TRUE;
        self->save_feedback_timer = 2.0f;
        return;
    }

    save_mgr = lp_save_manager_get_default ();

    if (lp_save_manager_autosave (save_mgr, game_data, &error))
    {
        lp_log_info ("Game saved successfully");
        self->save_success = TRUE;
    }
    else
    {
        lp_log_warning ("Failed to save game: %s",
                        error ? error->message : "unknown error");
        self->save_success = FALSE;
    }

    self->show_save_feedback = TRUE;
    self->save_feedback_timer = 2.0f;
}

static void
pause_do_quit_to_menu (LpStatePause *self)
{
    LpGame *game;
    LpGameData *game_data;
    LpSaveManager *save_mgr;
    LrgGameStateManager *manager;
    g_autoptr(GError) error = NULL;

    game = lp_game_get_from_state (LRG_GAME_STATE (self));
    if (game == NULL)
    {
        lp_log_warning ("Cannot quit to menu: no game instance");
        return;
    }

    /* Autosave before quitting */
    game_data = lp_game_get_game_data (game);
    if (game_data != NULL)
    {
        save_mgr = lp_save_manager_get_default ();
        if (!lp_save_manager_autosave (save_mgr, game_data, &error))
        {
            lp_log_warning ("Autosave failed on quit: %s",
                            error ? error->message : "unknown error");
        }
        else
        {
            lp_log_info ("Autosaved before quitting to menu");
        }
    }

    manager = lrg_game_template_get_state_manager (LRG_GAME_TEMPLATE (game));

    /* Clear all states and push main menu */
    lrg_game_state_manager_clear (manager);
    lrg_game_state_manager_push (manager,
                                  LRG_GAME_STATE (lp_state_main_menu_new ()));

    lp_log_info ("Returned to main menu");
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_pause_enter (LrgGameState *state)
{
    LpStatePause *self = LP_STATE_PAUSE (state);

    lp_log_info ("Game paused");

    self->selected_option = PAUSE_OPTION_RESUME;
    self->show_save_feedback = FALSE;
    self->save_feedback_timer = 0.0f;
}

static void
lp_state_pause_exit (LrgGameState *state)
{
    (void)state;
    lp_log_info ("Game unpaused");
}

static void
lp_state_pause_update (LrgGameState *state,
                       gdouble       delta)
{
    LpStatePause *self = LP_STATE_PAUSE (state);
    LpGame *game;
    LrgGameStateManager *manager;

    game = lp_game_get_from_state (state);
    if (game == NULL)
        return;

    manager = lrg_game_template_get_state_manager (LRG_GAME_TEMPLATE (game));

    /* Handle input via polling */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
    {
        /* Resume game */
        lrg_game_state_manager_pop (manager);
        return;
    }

    /* Navigation */
    if (grl_input_is_key_pressed (GRL_KEY_UP) ||
        grl_input_is_key_pressed (GRL_KEY_W))
    {
        self->selected_option--;
        if (self->selected_option < 0)
            self->selected_option = PAUSE_OPTION_COUNT - 1;
    }

    if (grl_input_is_key_pressed (GRL_KEY_DOWN) ||
        grl_input_is_key_pressed (GRL_KEY_S))
    {
        self->selected_option++;
        if (self->selected_option >= PAUSE_OPTION_COUNT)
            self->selected_option = 0;
    }

    /* Selection */
    if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        switch (self->selected_option)
        {
            case PAUSE_OPTION_RESUME:
                lrg_game_state_manager_pop (manager);
                break;

            case PAUSE_OPTION_SAVE:
                pause_do_save (self);
                break;

            case PAUSE_OPTION_SETTINGS:
                /* Push the game's settings state */
                lrg_game_state_manager_push (manager,
                    LRG_GAME_STATE (lp_state_settings_new ()));
                break;

            case PAUSE_OPTION_MAIN_MENU:
                pause_do_quit_to_menu (self);
                break;

            default:
                break;
        }
    }

    /* Update save feedback timer */
    if (self->show_save_feedback)
    {
        self->save_feedback_timer -= (gfloat)delta;
        if (self->save_feedback_timer <= 0.0f)
        {
            self->show_save_feedback = FALSE;
        }
    }
}

static void
lp_state_pause_draw (LrgGameState *state)
{
    LpStatePause *self = LP_STATE_PAUSE (state);
    LpGame *game;
    LrgTheme *theme;
    gfloat screen_width, screen_height;
    gfloat panel_width, panel_height;
    gfloat panel_x, panel_y;
    gfloat padding;
    gfloat font_size_large, font_size;
    gfloat y_offset;
    gint i;
    const GrlColor *text_color;
    const GrlColor *secondary_color;
    const GrlColor *accent_color;
    const GrlColor *surface_color;
    const GrlColor *border_color;

    /* Reset label pool */
    reset_label_pool (self);

    game = lp_game_get_from_state (state);
    if (game == NULL)
        return;

    theme = lrg_theme_get_default ();

    /* Get virtual resolution for UI positioning */
    screen_width = (gfloat)lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_height = (gfloat)lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));

    padding = lrg_theme_get_padding_normal (theme);
    font_size_large = lrg_theme_get_font_size_large (theme);
    font_size = lrg_theme_get_font_size_normal (theme);

    text_color = lrg_theme_get_text_color (theme);
    secondary_color = lrg_theme_get_text_secondary_color (theme);
    accent_color = lrg_theme_get_accent_color (theme);
    surface_color = lrg_theme_get_surface_color (theme);
    border_color = lrg_theme_get_border_color (theme);

    /* Draw semi-transparent overlay */
    {
        g_autoptr(GrlColor) overlay = grl_color_new (0, 0, 0, 180);
        grl_draw_rectangle (0, 0, screen_width, screen_height, overlay);
    }

    /* Calculate panel dimensions */
    panel_width = 300.0f;
    panel_height = font_size_large + padding * 3 +
                   (font_size + padding) * PAUSE_OPTION_COUNT + padding * 2;
    panel_x = (screen_width - panel_width) / 2.0f;
    panel_y = (screen_height - panel_height) / 2.0f;

    /* Draw panel background */
    grl_draw_rectangle (panel_x, panel_y, panel_width, panel_height, surface_color);

    /* Draw panel border */
    grl_draw_rectangle_lines (panel_x, panel_y, panel_width, panel_height, border_color);

    /* Draw title */
    y_offset = panel_y + padding;
    {
        gfloat title_width = grl_measure_text ("PAUSED", font_size_large);
        gfloat title_x = panel_x + (panel_width - title_width) / 2.0f;

        draw_label (get_pool_label (self), "PAUSED",
                    title_x, y_offset,
                    font_size_large, text_color);
    }
    y_offset += font_size_large + padding * 2;

    /* Draw separator */
    grl_draw_line (panel_x + padding, y_offset,
                   panel_x + panel_width - padding, y_offset,
                   border_color);
    y_offset += padding;

    /* Draw menu options */
    for (i = 0; i < PAUSE_OPTION_COUNT; i++)
    {
        const GrlColor *item_color;
        const gchar *prefix = "  ";

        if (i == self->selected_option)
        {
            item_color = accent_color;
            prefix = "> ";
        }
        else
        {
            item_color = secondary_color;
        }

        /* Draw selection indicator and label */
        {
            g_autofree gchar *label_text = g_strdup_printf ("%s%s",
                                                            prefix,
                                                            option_labels[i]);
            draw_label (get_pool_label (self), label_text,
                        panel_x + padding, y_offset,
                        font_size, item_color);
        }

        y_offset += font_size + padding;
    }

    /* Draw save feedback if showing */
    if (self->show_save_feedback)
    {
        const GrlColor *feedback_color;
        const gchar *feedback_text;
        gfloat feedback_width;
        gfloat feedback_x;

        if (self->save_success)
        {
            feedback_color = lrg_theme_get_success_color (theme);
            feedback_text = "Game Saved!";
        }
        else
        {
            feedback_color = lrg_theme_get_error_color (theme);
            feedback_text = "Save Failed!";
        }

        feedback_width = grl_measure_text (feedback_text, font_size);
        feedback_x = panel_x + (panel_width - feedback_width) / 2.0f;

        draw_label (get_pool_label (self), feedback_text,
                    feedback_x, panel_y + panel_height + padding,
                    font_size, feedback_color);
    }

    /* Draw hint at bottom */
    {
        const gchar *hint = "ESC to resume";
        gfloat hint_width = grl_measure_text (hint, font_size);
        gfloat hint_x = (screen_width - hint_width) / 2.0f;
        gfloat hint_y = screen_height - font_size - padding;

        draw_label (get_pool_label (self), hint,
                    hint_x, hint_y,
                    font_size, secondary_color);
    }
}

static gboolean
lp_state_pause_handle_input (LrgGameState *state,
                             gpointer      event)
{
    /* Input handled in update via polling */
    (void)state;
    (void)event;
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_pause_dispose (GObject *object)
{
    LpStatePause *self = LP_STATE_PAUSE (object);

    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_pause_parent_class)->dispose (object);
}

static void
lp_state_pause_class_init (LpStatePauseClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_pause_dispose;

    state_class->enter = lp_state_pause_enter;
    state_class->exit = lp_state_pause_exit;
    state_class->update = lp_state_pause_update;
    state_class->draw = lp_state_pause_draw;
    state_class->handle_input = lp_state_pause_handle_input;
}

static void
lp_state_pause_init (LpStatePause *self)
{
    guint i;

    /* Pause menu is transparent (shows game beneath) */
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Pause");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), TRUE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->selected_option = PAUSE_OPTION_RESUME;
    self->show_save_feedback = FALSE;
    self->save_feedback_timer = 0.0f;
    self->save_success = FALSE;

    /* Create label pool */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < LABEL_POOL_SIZE; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
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
