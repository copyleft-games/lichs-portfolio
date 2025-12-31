/* lp-state-wake.c - Wake Report Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-wake.h"
#include "../core/lp-application.h"
#include "../core/lp-game-data.h"
/* #include "../narrative/lp-malachar-voice.h" */
/* #include "../tutorial/lp-tutorial-sequences.h" */
#include "lp-state-analyze.h"
#include <graylib.h>
#include <libregnum.h>

struct _LpStateWake
{
    LrgGameState parent_instance;

    GList *events;          /* Events that occurred during slumber */
    guint  current_event;   /* Index of currently displayed event */
};

G_DEFINE_TYPE (LpStateWake, lp_state_wake, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_wake_enter (LrgGameState *state)
{
    LpStateWake *self = LP_STATE_WAKE (state);

    lp_log_info ("Entering wake state");

    self->current_event = 0;

    /*
     * TODO: When application has transition manager, trigger fade-in here.
     * The player is "waking up" so we'd fade from black.
     */

    /*
     * TODO: Re-enable tutorial when LrgTutorialManager is initialized
     * in lp_application_startup(). The tutorial system requires calling
     * lp_tutorial_sequences_init_tutorials() before use.
     *
     * lp_tutorial_sequences_maybe_start_intro (
     *     lp_tutorial_sequences_get_default ());
     */
}

static void
lp_state_wake_exit (LrgGameState *state)
{
    LpStateWake *self = LP_STATE_WAKE (state);

    lp_log_info ("Exiting wake state");

    g_list_free_full (self->events, g_object_unref);
    self->events = NULL;
}

static void
lp_state_wake_update (LrgGameState *state,
                      gdouble       delta)
{
    (void)delta;

    /* Check for input to continue */
    if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        LpApplication *app = lp_application_get_default ();
        LrgGameStateManager *manager;

        lp_log_info ("Continuing to analyze state");

        /* Replace wake with analyze state */
        manager = lp_application_get_state_manager (app);
        lrg_game_state_manager_replace (manager,
            LRG_GAME_STATE (lp_state_analyze_new ()));
    }

    /* ESC to return to main menu */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
    {
        lp_application_quit (lp_application_get_default ());
    }
}

static GrlWindow *
get_grl_window (void)
{
    LpApplication *app = lp_application_get_default ();
    LrgEngine *engine = lp_application_get_engine (app);
    LrgWindow *lrg_window = lrg_engine_get_window (engine);

    return lrg_grl_window_get_grl_window (LRG_GRL_WINDOW (lrg_window));
}

static void
lp_state_wake_draw (LrgGameState *state)
{
    LpStateWake *self = LP_STATE_WAKE (state);
    LpApplication *app = lp_application_get_default ();
    LpGameData *game_data = lp_application_get_game_data (app);
    GrlWindow *window;
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) gold_color = NULL;
    guint64 year;
    gchar year_str[64];
    gint screen_w, screen_h;
    gint center_x;
    gint title_y, year_y, greeting_y, portfolio_y, instructions_y;

    (void)self;

    /* Get current window dimensions */
    window = get_grl_window ();
    screen_w = grl_window_get_width (window);
    screen_h = grl_window_get_height (window);
    center_x = screen_w / 2;

    /* Calculate vertical positions */
    title_y = screen_h / 7;
    year_y = title_y + 80;
    greeting_y = screen_h * 2 / 5;
    portfolio_y = screen_h * 3 / 5;
    instructions_y = screen_h - 100;

    /* Colors */
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    gold_color = grl_color_new (255, 215, 0, 255);

    /* Draw title */
    grl_draw_text ("THE LICH AWAKENS", center_x - 180, title_y, 48, title_color);

    /* Get game data info */
    if (game_data != NULL)
    {
        year = lp_game_data_get_current_year (game_data);
        g_snprintf (year_str, sizeof (year_str), "Year %lu of the Third Age", (gulong)year);
    }
    else
    {
        g_snprintf (year_str, sizeof (year_str), "Year 847 of the Third Age");
    }

    grl_draw_text (year_str, center_x - 140, year_y, 24, text_color);

    /* Malachar's greeting */
    grl_draw_text ("\"Ah, you have awakened, my eternal apprentice...\"",
                   center_x - 280, greeting_y, 20, gold_color);

    grl_draw_text ("\"The mortal world continues its endless dance of",
                   center_x - 280, greeting_y + 40, 18, text_color);
    grl_draw_text ("gold and folly. Let us see what opportunities await.\"",
                   center_x - 280, greeting_y + 65, 18, text_color);

    /* Portfolio summary placeholder */
    grl_draw_text ("Portfolio Summary:", center_x - 280, portfolio_y, 22, title_color);
    grl_draw_text ("Starting Gold: 10,000 gp", center_x - 260, portfolio_y + 40, 18, text_color);
    grl_draw_text ("Investments: None", center_x - 260, portfolio_y + 65, 18, text_color);
    grl_draw_text ("Agents: None", center_x - 260, portfolio_y + 90, 18, text_color);

    /* Instructions */
    grl_draw_text ("Press ENTER or SPACE to continue...",
                   center_x - 180, instructions_y, 16, dim_color);
}

static gboolean
lp_state_wake_handle_input (LrgGameState *state,
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
lp_state_wake_finalize (GObject *object)
{
    LpStateWake *self = LP_STATE_WAKE (object);

    g_list_free_full (self->events, g_object_unref);

    G_OBJECT_CLASS (lp_state_wake_parent_class)->finalize (object);
}

static void
lp_state_wake_class_init (LpStateWakeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->finalize = lp_state_wake_finalize;

    state_class->enter = lp_state_wake_enter;
    state_class->exit = lp_state_wake_exit;
    state_class->update = lp_state_wake_update;
    state_class->draw = lp_state_wake_draw;
    state_class->handle_input = lp_state_wake_handle_input;
}

static void
lp_state_wake_init (LpStateWake *self)
{
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Wake");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->events = NULL;
    self->current_event = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_wake_new:
 *
 * Creates a new wake report state.
 *
 * Returns: (transfer full): A new #LpStateWake
 */
LpStateWake *
lp_state_wake_new (void)
{
    return g_object_new (LP_TYPE_STATE_WAKE, NULL);
}

/**
 * lp_state_wake_set_events:
 * @self: an #LpStateWake
 * @events: (transfer full) (element-type LpEvent): List of events
 *
 * Sets the events to display.
 */
void
lp_state_wake_set_events (LpStateWake *self,
                          GList       *events)
{
    g_return_if_fail (LP_IS_STATE_WAKE (self));

    g_list_free_full (self->events, g_object_unref);
    self->events = events;
    self->current_event = 0;
}
