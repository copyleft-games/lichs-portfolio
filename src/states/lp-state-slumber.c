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
/* #include "../tutorial/lp-tutorial-sequences.h" */
#include "../lp-input-helpers.h"
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

    /* UI Labels */
    LrgLabel  *label_title;
    LrgLabel  *label_question;
    LrgLabel  *label_duration;
    LrgLabel  *label_years;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpStateSlumber, lp_state_slumber, LRG_TYPE_GAME_STATE)

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
get_pool_label (LpStateSlumber *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateSlumber *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_slumber_dispose (GObject *object)
{
    LpStateSlumber *self = LP_STATE_SLUMBER (object);

    g_clear_object (&self->label_title);
    g_clear_object (&self->label_question);
    g_clear_object (&self->label_duration);
    g_clear_object (&self->label_years);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_slumber_parent_class)->dispose (object);
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_slumber_enter (LrgGameState *state)
{
    LpStateSlumber *self = LP_STATE_SLUMBER (state);

    lp_log_info ("Entering slumber configuration");

    self->slumber_years = DEFAULT_SLUMBER_YEARS;

    /*
     * TODO: Re-enable tutorial when LrgTutorialManager is initialized
     * in lp_game_real_pre_startup(). The tutorial system requires calling
     * lp_tutorial_sequences_init_tutorials() before use.
     *
     * lp_tutorial_sequences_maybe_start_slumber (
     *     lp_tutorial_sequences_get_default ());
     */
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

    /* Up to increase years (including vim keys and gamepad D-pad) */
    if (LP_INPUT_NAV_UP_PRESSED ())
    {
        if (self->slumber_years < MAX_SLUMBER_YEARS)
            self->slumber_years += 10;
    }

    /* Down to decrease years (including vim keys and gamepad D-pad) */
    if (LP_INPUT_NAV_DOWN_PRESSED ())
    {
        if (self->slumber_years > MIN_SLUMBER_YEARS)
        {
            if (self->slumber_years >= 10)
                self->slumber_years -= 10;
            else
                self->slumber_years = MIN_SLUMBER_YEARS;
        }
    }

    /* Enter/A button to confirm and begin slumber */
    if (LP_INPUT_CONFIRM_PRESSED ())
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

    /* Escape/B button to cancel */
    if (LP_INPUT_CANCEL_PRESSED ())
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

    /* Reset label pool for this frame */
    reset_label_pool (self);

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
    draw_label (self->label_title, "PREPARE FOR SLUMBER",
                center_x - 180, panel_y + 30, 36, title_color);

    /* Malachar's question */
    draw_label (self->label_question, "\"How long shall you rest, my lord?\"",
                center_x - 180, panel_y + 90, 18, text_color);

    /* Year selector */
    draw_label (self->label_duration, "Duration:",
                center_x - 60, panel_y + 150, 20, text_color);

    g_snprintf (years_str, sizeof (years_str), "%u years", self->slumber_years);
    draw_label (self->label_years, years_str,
                center_x - 50, panel_y + 190, 32, value_color);

    /* Instructions */
    draw_label (get_pool_label (self), "UP/DOWN to adjust duration",
                center_x - 130, panel_y + 260, 16, dim_color);
    draw_label (get_pool_label (self), "ENTER to confirm, ESC to cancel",
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
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_slumber_dispose;

    state_class->enter = lp_state_slumber_enter;
    state_class->exit = lp_state_slumber_exit;
    state_class->update = lp_state_slumber_update;
    state_class->draw = lp_state_slumber_draw;
    state_class->handle_input = lp_state_slumber_handle_input;
}

static void
lp_state_slumber_init (LpStateSlumber *self)
{
    guint i;

    lrg_game_state_set_name (LRG_GAME_STATE (self), "Slumber");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), FALSE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->slumber_years = DEFAULT_SLUMBER_YEARS;

    /* Create dedicated labels for static text */
    self->label_title = lrg_label_new (NULL);
    self->label_question = lrg_label_new (NULL);
    self->label_duration = lrg_label_new (NULL);
    self->label_years = lrg_label_new (NULL);

    /* Create label pool for instruction text */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 4; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
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
