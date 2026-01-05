/* lp-state-analyze.c - World Analysis Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-analyze.h"
#include "lp-state-pause.h"
#include "lp-state-slumber.h"
#include "lp-state-investments.h"
#include "lp-state-agents.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include <graylib.h>
#include <libregnum.h>

struct _LpStateAnalyze
{
    LrgGameState parent_instance;

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpStateAnalyze, lp_state_analyze, LRG_TYPE_GAME_STATE)

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
get_pool_label (LpStateAnalyze *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateAnalyze *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_analyze_dispose (GObject *object)
{
    LpStateAnalyze *self = LP_STATE_ANALYZE (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_analyze_parent_class)->dispose (object);
}

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
    (void)delta;

    /* I for Investments */
    if (grl_input_is_key_pressed (GRL_KEY_I))
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Opening investments screen");

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_push (manager,
            LRG_GAME_STATE (lp_state_investments_new ()));
    }

    /* A for Agents */
    if (grl_input_is_key_pressed (GRL_KEY_A))
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Opening agents screen");

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_push (manager,
            LRG_GAME_STATE (lp_state_agents_new ()));
    }

    /* S to enter slumber configuration */
    if (grl_input_is_key_pressed (GRL_KEY_S))
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Opening slumber configuration");

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_push (manager,
            LRG_GAME_STATE (lp_state_slumber_new ()));
    }

    /* ESC to open pause menu */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Opening pause menu");

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_push (manager,
            LRG_GAME_STATE (lp_state_pause_new ()));
    }
}

static void
lp_state_analyze_draw (LrgGameState *state)
{
    LpStateAnalyze *self = LP_STATE_ANALYZE (state);
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

    /* Reset label pool for this frame */
    reset_label_pool (self);

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
    draw_label (self->label_title, "WORLD ANALYSIS",
                center_x - 140, 30, 32, title_color);

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
    draw_label (get_pool_label (self), year_str,
                center_x - 120, 70, 18, text_color);

    /* Portfolio panel (left side) */
    grl_draw_rectangle (margin, main_area_top, left_panel_w, main_area_h, panel_color);
    draw_label (get_pool_label (self), "Portfolio",
                margin + 15, main_area_top + 10, 24, title_color);
    draw_label (get_pool_label (self), "Gold: 10,000 gp",
                margin + 15, main_area_top + 50, 18, gold_color);
    draw_label (get_pool_label (self), "Investments: 0",
                margin + 15, main_area_top + 80, 16, text_color);
    draw_label (get_pool_label (self), "Total Value: 10,000 gp",
                margin + 15, main_area_top + 105, 16, text_color);

    /* World map placeholder (center) */
    grl_draw_rectangle (center_panel_x, main_area_top, center_panel_w, main_area_h, panel_color);
    draw_label (get_pool_label (self), "World Map",
                center_panel_x + center_panel_w / 2 - 60, main_area_top + 10, 24, title_color);
    draw_label (get_pool_label (self), "(Kingdoms and regions will be displayed here)",
                center_panel_x + center_panel_w / 2 - 190, main_area_top + main_area_h / 2, 16, dim_color);

    /* Agent panel (right side) */
    grl_draw_rectangle (screen_w - right_panel_w - margin, main_area_top, right_panel_w, main_area_h, panel_color);
    draw_label (get_pool_label (self), "Agents",
                screen_w - right_panel_w - margin + 15, main_area_top + 10, 24, title_color);
    draw_label (get_pool_label (self), "No agents recruited",
                screen_w - right_panel_w - margin + 15, main_area_top + 50, 16, dim_color);

    /* Actions bar (bottom) */
    grl_draw_rectangle (margin, bottom_panel_y, screen_w - (margin * 2), bottom_panel_h, panel_color);
    draw_label (get_pool_label (self), "Actions",
                margin + 15, bottom_panel_y + 10, 20, title_color);
    draw_label (get_pool_label (self), "[I] Investments    [A] Agents    [S] Enter Slumber    [ESC] Pause",
                margin + 15, bottom_panel_y + 50, 16, text_color);

    /* Malachar hint */
    draw_label (get_pool_label (self), "\"The mortal kingdoms await your careful analysis, my lord...\"",
                margin + 15, bottom_panel_y + bottom_panel_h - 30, 14, gold_color);
}

static gboolean
lp_state_analyze_handle_input (LrgGameState *state,
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
lp_state_analyze_class_init (LpStateAnalyzeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_analyze_dispose;

    state_class->enter = lp_state_analyze_enter;
    state_class->exit = lp_state_analyze_exit;
    state_class->update = lp_state_analyze_update;
    state_class->draw = lp_state_analyze_draw;
    state_class->handle_input = lp_state_analyze_handle_input;
}

static void
lp_state_analyze_init (LpStateAnalyze *self)
{
    guint i;

    lrg_game_state_set_name (LRG_GAME_STATE (self), "Analyze");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 15; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
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
