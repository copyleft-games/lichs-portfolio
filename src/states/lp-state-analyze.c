/* lp-state-analyze.c - World Analysis Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-analyze.h"
#include "lp-state-main-menu.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include <graylib.h>
#include <libregnum.h>

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
    (void)state;
    (void)delta;

    /* ESC to return to main menu */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Returning to main menu");

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_replace (manager,
            LRG_GAME_STATE (lp_state_main_menu_new ()));
    }
}

static void
lp_state_analyze_draw (LrgGameState *state)
{
    LpGame *game = lp_game_get_from_state (state);
    LpGameData *game_data = lp_game_get_game_data (game);
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) gold_color = NULL;
    g_autoptr(GrlColor) panel_color = NULL;
    guint64 year;
    gchar year_str[64];
    gint screen_w, screen_h;
    gint center_x;
    gint margin, header_h, main_area_top, main_area_h;
    gint left_panel_w, right_panel_w, center_panel_x, center_panel_w;
    gint bottom_panel_y, bottom_panel_h;

    /* Get virtual resolution (render target size) for UI positioning */
    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
    center_x = screen_w / 2;

    /* Calculate layout proportions */
    margin = 20;
    header_h = 100;
    main_area_top = header_h + margin;
    bottom_panel_h = screen_h / 5;
    bottom_panel_y = screen_h - bottom_panel_h - margin;
    main_area_h = bottom_panel_y - main_area_top - margin;

    /* Panel widths: left and right are 23% each, center fills the rest */
    left_panel_w = (screen_w * 23) / 100;
    right_panel_w = (screen_w * 23) / 100;
    center_panel_x = margin + left_panel_w + margin;
    center_panel_w = screen_w - left_panel_w - right_panel_w - (margin * 4);

    /* Colors */
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    gold_color = grl_color_new (255, 215, 0, 255);
    panel_color = grl_color_new (30, 30, 40, 255);

    /* Draw header */
    grl_draw_text ("WORLD ANALYSIS", center_x - 140, 30, 32, title_color);

    /* Get and display current year */
    if (game_data != NULL)
    {
        year = lp_game_data_get_current_year (game_data);
        g_snprintf (year_str, sizeof (year_str), "Year %lu of the Third Age", (gulong)year);
    }
    else
    {
        g_snprintf (year_str, sizeof (year_str), "Year 847 of the Third Age");
    }
    grl_draw_text (year_str, center_x - 120, 70, 18, text_color);

    /* Portfolio panel (left side) */
    grl_draw_rectangle (margin, main_area_top, left_panel_w, main_area_h, panel_color);
    grl_draw_text ("Portfolio", margin + 15, main_area_top + 10, 24, title_color);
    grl_draw_text ("Gold: 10,000 gp", margin + 15, main_area_top + 50, 18, gold_color);
    grl_draw_text ("Investments: 0", margin + 15, main_area_top + 80, 16, text_color);
    grl_draw_text ("Total Value: 10,000 gp", margin + 15, main_area_top + 105, 16, text_color);

    /* World map placeholder (center) */
    grl_draw_rectangle (center_panel_x, main_area_top, center_panel_w, main_area_h, panel_color);
    grl_draw_text ("World Map", center_panel_x + center_panel_w / 2 - 60, main_area_top + 10, 24, title_color);
    grl_draw_text ("(Kingdoms and regions will be displayed here)",
                   center_panel_x + center_panel_w / 2 - 190, main_area_top + main_area_h / 2, 16, dim_color);

    /* Agent panel (right side) */
    grl_draw_rectangle (screen_w - right_panel_w - margin, main_area_top, right_panel_w, main_area_h, panel_color);
    grl_draw_text ("Agents", screen_w - right_panel_w - margin + 15, main_area_top + 10, 24, title_color);
    grl_draw_text ("No agents recruited", screen_w - right_panel_w - margin + 15, main_area_top + 50, 16, dim_color);

    /* Actions bar (bottom) */
    grl_draw_rectangle (margin, bottom_panel_y, screen_w - (margin * 2), bottom_panel_h, panel_color);
    grl_draw_text ("Actions", margin + 15, bottom_panel_y + 10, 20, title_color);
    grl_draw_text ("[I] Investments    [A] Agents    [S] Enter Slumber    [ESC] Main Menu",
                   margin + 15, bottom_panel_y + 50, 16, text_color);

    /* Malachar hint */
    grl_draw_text ("\"The mortal kingdoms await your careful analysis, my lord...\"",
                   margin + 15, bottom_panel_y + bottom_panel_h - 30, 14, gold_color);
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
