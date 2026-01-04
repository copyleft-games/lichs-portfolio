/* lp-state-welcome-back.c - Offline Progress Notification State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-welcome-back.h"
#include "../core/lp-game.h"
#include <graylib.h>
#include <math.h>

struct _LpStateWelcomeBack
{
    LrgGameState parent_instance;

    gdouble       seconds_offline;
    LrgBigNumber *gold_earned;

    /* Animation */
    gdouble       anim_timer;
    gboolean      show_prompt;
};

G_DEFINE_TYPE (LpStateWelcomeBack, lp_state_welcome_back, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * Helper Functions
 * ========================================================================== */

/*
 * format_time_offline:
 *
 * Formats seconds into a human-readable time string.
 * e.g., "12 hours", "3 days", "2 weeks"
 */
static void
format_time_offline (gdouble  seconds,
                     gchar   *buffer,
                     gsize    buffer_size)
{
    gdouble minutes;
    gdouble hours;
    gdouble days;
    gdouble weeks;

    minutes = seconds / 60.0;
    hours = minutes / 60.0;
    days = hours / 24.0;
    weeks = days / 7.0;

    if (weeks >= 1.0)
    {
        if (weeks >= 2.0)
            g_snprintf (buffer, buffer_size, "%.0f weeks", floor (weeks));
        else
            g_snprintf (buffer, buffer_size, "1 week");
    }
    else if (days >= 1.0)
    {
        if (days >= 2.0)
            g_snprintf (buffer, buffer_size, "%.0f days", floor (days));
        else
            g_snprintf (buffer, buffer_size, "1 day");
    }
    else if (hours >= 1.0)
    {
        if (hours >= 2.0)
            g_snprintf (buffer, buffer_size, "%.0f hours", floor (hours));
        else
            g_snprintf (buffer, buffer_size, "1 hour");
    }
    else if (minutes >= 1.0)
    {
        if (minutes >= 2.0)
            g_snprintf (buffer, buffer_size, "%.0f minutes", floor (minutes));
        else
            g_snprintf (buffer, buffer_size, "1 minute");
    }
    else
    {
        g_snprintf (buffer, buffer_size, "moments");
    }
}

/*
 * format_gold_amount:
 *
 * Formats gold with suffix for large numbers.
 */
static void
format_gold_amount (const LrgBigNumber *gold,
                    gchar              *buffer,
                    gsize               buffer_size)
{
    gdouble value;

    if (gold == NULL)
    {
        g_snprintf (buffer, buffer_size, "0");
        return;
    }

    value = lrg_big_number_to_double (gold);

    if (value >= 1000000000.0)
    {
        g_snprintf (buffer, buffer_size, "%.2fB", value / 1000000000.0);
    }
    else if (value >= 1000000.0)
    {
        g_snprintf (buffer, buffer_size, "%.2fM", value / 1000000.0);
    }
    else if (value >= 1000.0)
    {
        g_snprintf (buffer, buffer_size, "%.2fK", value / 1000.0);
    }
    else
    {
        g_snprintf (buffer, buffer_size, "%.0f", value);
    }
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_welcome_back_enter (LrgGameState *state)
{
    LpStateWelcomeBack *self = LP_STATE_WELCOME_BACK (state);

    lp_log_info ("Entering welcome back state");

    self->anim_timer = 0.0;
    self->show_prompt = TRUE;
}

static void
lp_state_welcome_back_exit (LrgGameState *state)
{
    (void)state;

    lp_log_info ("Exiting welcome back state");
}

static void
lp_state_welcome_back_update (LrgGameState *state,
                               gdouble       delta)
{
    LpStateWelcomeBack *self = LP_STATE_WELCOME_BACK (state);

    /* Animate the "Press to continue" prompt */
    self->anim_timer += delta;
    if (self->anim_timer >= 0.5)
    {
        self->anim_timer = 0.0;
        self->show_prompt = !self->show_prompt;
    }

    /* Check for input to continue */
    if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE) ||
        grl_input_is_key_pressed (GRL_KEY_ESCAPE))
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Continuing from welcome back");

        /* Pop this state to return to the game */
        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_pop (manager);
    }
}

static void
lp_state_welcome_back_draw (LrgGameState *state)
{
    LpStateWelcomeBack *self = LP_STATE_WELCOME_BACK (state);
    LpGame *game = lp_game_get_from_state (state);
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) panel_color = NULL;
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) gold_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    gint screen_w, screen_h;
    gint center_x, center_y;
    gint panel_x, panel_y, panel_w, panel_h;
    gchar time_str[64];
    gchar gold_str[64];

    /* Get virtual resolution (render target size) for UI positioning */
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
    bg_color = grl_color_new (10, 10, 15, 230);
    panel_color = grl_color_new (25, 25, 35, 255);
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    gold_color = grl_color_new (255, 215, 0, 255);
    dim_color = grl_color_new (100, 100, 100, 255);

    /* Draw semi-transparent background overlay */
    grl_draw_rectangle (0, 0, screen_w, screen_h, bg_color);

    /* Draw panel background */
    grl_draw_rectangle (panel_x, panel_y, panel_w, panel_h, panel_color);

    /* Draw title */
    grl_draw_text ("WELCOME BACK", center_x - 130, panel_y + 30, 36, title_color);

    /* Malachar's greeting */
    grl_draw_text ("\"Ah, you have returned, my eternal apprentice...\"",
                   center_x - 210, panel_y + 90, 18, text_color);

    /* Format time away */
    format_time_offline (self->seconds_offline, time_str, sizeof (time_str));

    grl_draw_text ("Time in slumber:", center_x - 100, panel_y + 140, 20, text_color);
    grl_draw_text (time_str, center_x - 60, panel_y + 170, 24, title_color);

    /* Format gold earned */
    format_gold_amount (self->gold_earned, gold_str, sizeof (gold_str));

    grl_draw_text ("Gold earned:", center_x - 80, panel_y + 210, 20, text_color);

    {
        gchar gold_display[128];
        g_snprintf (gold_display, sizeof (gold_display), "+%s gp", gold_str);
        grl_draw_text (gold_display, center_x - 60, panel_y + 240, 28, gold_color);
    }

    /* Draw prompt (blinking) */
    if (self->show_prompt)
    {
        grl_draw_text ("Press ENTER or SPACE to continue...",
                       center_x - 170, panel_y + panel_h - 50, 16, dim_color);
    }
}

static gboolean
lp_state_welcome_back_handle_input (LrgGameState *state,
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
lp_state_welcome_back_finalize (GObject *object)
{
    LpStateWelcomeBack *self = LP_STATE_WELCOME_BACK (object);

    g_clear_pointer (&self->gold_earned, lrg_big_number_free);

    G_OBJECT_CLASS (lp_state_welcome_back_parent_class)->finalize (object);
}

static void
lp_state_welcome_back_class_init (LpStateWelcomeBackClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->finalize = lp_state_welcome_back_finalize;

    state_class->enter = lp_state_welcome_back_enter;
    state_class->exit = lp_state_welcome_back_exit;
    state_class->update = lp_state_welcome_back_update;
    state_class->draw = lp_state_welcome_back_draw;
    state_class->handle_input = lp_state_welcome_back_handle_input;
}

static void
lp_state_welcome_back_init (LpStateWelcomeBack *self)
{
    lrg_game_state_set_name (LRG_GAME_STATE (self), "WelcomeBack");

    /*
     * Transparent so the underlying game state is visible behind the overlay.
     * Blocking so the underlying state doesn't update while we're showing.
     */
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), TRUE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->seconds_offline = 0.0;
    self->gold_earned = NULL;
    self->anim_timer = 0.0;
    self->show_prompt = TRUE;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_welcome_back_new:
 *
 * Creates a new welcome back state.
 *
 * Returns: (transfer full): A new #LpStateWelcomeBack
 */
LpStateWelcomeBack *
lp_state_welcome_back_new (void)
{
    return g_object_new (LP_TYPE_STATE_WELCOME_BACK, NULL);
}

/**
 * lp_state_welcome_back_set_offline_data:
 * @self: an #LpStateWelcomeBack
 * @seconds_offline: how long the player was away
 * @gold_earned: (transfer none): gold earned while offline
 *
 * Sets the offline progress data to display.
 */
void
lp_state_welcome_back_set_offline_data (LpStateWelcomeBack *self,
                                         gdouble             seconds_offline,
                                         const LrgBigNumber *gold_earned)
{
    g_return_if_fail (LP_IS_STATE_WELCOME_BACK (self));

    self->seconds_offline = seconds_offline;

    g_clear_pointer (&self->gold_earned, lrg_big_number_free);
    if (gold_earned != NULL)
    {
        self->gold_earned = lrg_big_number_copy (gold_earned);
    }
}
