/* lp-state-wake.c - Wake Report Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-wake.h"
#include "../narrative/lp-malachar-voice.h"
#include "../tutorial/lp-tutorial-sequences.h"

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

    /* Maybe start intro tutorial for new players */
    lp_tutorial_sequences_maybe_start_intro (
        lp_tutorial_sequences_get_default ());
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
    /* Animation updates for wake report display */
}

static void
lp_state_wake_draw (LrgGameState *state)
{
    /*
     * Phase 1 skeleton: Placeholder drawing.
     * Full UI will show:
     * - "The Lich Awakens" title
     * - Years slumbered
     * - Portfolio summary
     * - Event list with navigation
     */
    lp_log_debug ("Drawing wake state (skeleton)");
}

static gboolean
lp_state_wake_handle_input (LrgGameState *state,
                            gpointer      event)
{
    /*
     * Phase 1 skeleton: Basic input handling.
     * - Space/Enter: Acknowledge and continue
     * - Left/Right: Navigate events
     */
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
