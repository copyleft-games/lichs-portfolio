/* lp-state-slumber.c - Slumber Configuration Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-slumber.h"
#include "lp-state-simulating.h"
#include "../core/lp-game.h"
#include "../tutorial/lp-tutorial-sequences.h"
#include <graylib.h>
#include <libregnum.h>

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
    LpStateSlumber *self = LP_STATE_SLUMBER (state);

    (void)delta;

    /* Up to increase years (including vim keys) */
    if (grl_input_is_key_pressed (GRL_KEY_UP) ||
        grl_input_is_key_pressed (GRL_KEY_K))
    {
        if (self->slumber_years < MAX_SLUMBER_YEARS)
            self->slumber_years += 10;
    }

    /* Down to decrease years (including vim keys) */
    if (grl_input_is_key_pressed (GRL_KEY_DOWN) ||
        grl_input_is_key_pressed (GRL_KEY_J))
    {
        if (self->slumber_years > MIN_SLUMBER_YEARS)
        {
            if (self->slumber_years >= 10)
                self->slumber_years -= 10;
            else
                self->slumber_years = MIN_SLUMBER_YEARS;
        }
    }

    /* Enter to confirm and begin slumber */
    if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        LpGame *game;
        LrgGameStateManager *manager;
        LpStateSimulating *simulating;

        lp_log_info ("Beginning slumber for %u years", self->slumber_years);

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));

        simulating = lp_state_simulating_new ();
        lp_state_simulating_set_years (simulating, self->slumber_years);

        /* Replace slumber with simulating */
        lrg_game_state_manager_replace (manager,
            LRG_GAME_STATE (simulating));
    }

    /* Escape to cancel */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Cancelling slumber configuration");

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_pop (manager);
    }
}

static void
lp_state_slumber_draw (LrgGameState *state)
{
    LpStateSlumber *self = LP_STATE_SLUMBER (state);
    LpGame *game = lp_game_get_from_state (state);
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) value_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) panel_color = NULL;
    gint screen_w, screen_h;
    gint center_x, center_y;
    gint panel_x, panel_y, panel_w, panel_h;
    gchar years_str[32];

    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
    center_x = screen_w / 2;
    center_y = screen_h / 2;

    /* Panel dimensions */
    panel_w = 500;
    panel_h = 350;
    panel_x = center_x - panel_w / 2;
    panel_y = center_y - panel_h / 2;

    /* Colors */
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    value_color = grl_color_new (255, 215, 0, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    panel_color = grl_color_new (25, 25, 35, 255);

    /* Draw panel background */
    grl_draw_rectangle (panel_x, panel_y, panel_w, panel_h, panel_color);

    /* Title */
    grl_draw_text ("PREPARE FOR SLUMBER", center_x - 180, panel_y + 30, 36, title_color);

    /* Malachar's question */
    grl_draw_text ("\"How long shall you rest, my lord?\"",
                   center_x - 180, panel_y + 90, 18, text_color);

    /* Year selector */
    grl_draw_text ("Duration:", center_x - 60, panel_y + 150, 20, text_color);

    g_snprintf (years_str, sizeof (years_str), "%u years", self->slumber_years);
    grl_draw_text (years_str, center_x - 50, panel_y + 190, 32, value_color);

    /* Instructions */
    grl_draw_text ("UP/DOWN to adjust duration",
                   center_x - 130, panel_y + 260, 16, dim_color);
    grl_draw_text ("ENTER to confirm, ESC to cancel",
                   center_x - 150, panel_y + 285, 16, dim_color);
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
