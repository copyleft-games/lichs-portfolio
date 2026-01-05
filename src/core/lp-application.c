/* lp-application.c - Main Application Singleton
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_APP
#include "../lp-log.h"

#include "lp-application.h"
#include "lp-game-data.h"
#include "../achievement/lp-achievement-manager.h"
#include "../states/lp-state-main-menu.h"

#include <libregnum.h>
#include <raylib.h>

/* Window configuration */
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Lich's Portfolio"

/* Target frame time for 60 FPS */
#define TARGET_FRAME_TIME (1.0 / 60.0)

struct _LpApplication
{
    GObject parent_instance;

    LrgEngine           *engine;
    LrgGrlWindow        *window;
    LrgGameStateManager *state_manager;
    LpGameData          *game_data;
    LpAchievementManager *achievement_manager;

    gboolean running;
    gboolean initialized;
};

G_DEFINE_TYPE (LpApplication, lp_application, G_TYPE_OBJECT)

/* Singleton instance */
static LpApplication *default_application = NULL;

/* ==========================================================================
 * Private Functions
 * ========================================================================== */

/*
 * lp_application_startup:
 *
 * Initializes all subsystems. Called once before the main loop.
 */
static gboolean
lp_application_startup (LpApplication  *self,
                        GError        **error)
{
    g_autoptr(GError) local_error = NULL;

    lp_log_info ("Starting Lich's Portfolio...");

    /* Initialize libregnum engine */
    self->engine = lrg_engine_get_default ();

    if (!lrg_engine_startup (self->engine, &local_error))
    {
        g_propagate_error (error, g_steal_pointer (&local_error));
        return FALSE;
    }

    /* Configure fonts with larger base sizes for crisp rendering */
    {
        LrgFontManager *font_mgr;
        LrgTheme *theme;
        GrlFont *font;

        font_mgr = lrg_font_manager_get_default ();

        /* Load fonts at larger base sizes to avoid upscaling blur */
        lrg_font_manager_initialize_with_sizes (font_mgr, 24, 32, 48, NULL);

        /* Apply bilinear filter for smooth edges */
        font = lrg_font_manager_get_default_font (font_mgr);
        if (font != NULL)
        {
            grl_font_set_filter (font, GRL_TEXTURE_FILTER_BILINEAR);
        }

        /* Set theme render sizes (now <= base sizes, so no upscaling) */
        theme = lrg_theme_get_default ();
        lrg_theme_set_font_size_small (theme, 16.0f);
        lrg_theme_set_font_size_normal (theme, 20.0f);
        lrg_theme_set_font_size_large (theme, 32.0f);
    }

    /* Create the game window */
    self->window = lrg_grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    if (self->window == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to create game window");
        return FALSE;
    }

    /* Set the window on the engine */
    lrg_engine_set_window (self->engine, LRG_WINDOW (self->window));

    /*
     * Disable raylib's default ESC-to-close behavior.
     * We handle ESC ourselves in game states.
     */
    SetExitKey (KEY_NULL);

    /* Create game state manager */
    self->state_manager = lrg_game_state_manager_new ();

    /* Create achievement manager (singleton pattern) */
    self->achievement_manager = lp_achievement_manager_get_default ();

    /* Game data is created when starting/loading a game, not at startup */
    self->game_data = NULL;

    /* Push initial state (main menu) */
    lrg_game_state_manager_push (self->state_manager,
                                  LRG_GAME_STATE (lp_state_main_menu_new ()));

    self->initialized = TRUE;

    lp_log_info ("Startup complete");

    return TRUE;
}

/*
 * lp_application_shutdown:
 *
 * Cleans up all subsystems. Called after the main loop exits.
 */
static void
lp_application_shutdown (LpApplication *self)
{
    lp_log_info ("Shutting down...");

    /* Clear game states */
    if (self->state_manager != NULL)
    {
        lrg_game_state_manager_clear (self->state_manager);
    }

    /* Clear game data */
    g_clear_object (&self->game_data);

    /* Clear state manager */
    g_clear_object (&self->state_manager);

    /*
     * Achievement manager is a singleton; don't clear it here.
     * It will be cleaned up when its singleton is released.
     */
    self->achievement_manager = NULL;

    /* Shutdown engine */
    if (self->engine != NULL)
    {
        lrg_engine_shutdown (self->engine);
        self->engine = NULL;
    }

    /* Clean up window */
    g_clear_object (&self->window);

    self->initialized = FALSE;

    lp_log_info ("Shutdown complete");
}

/*
 * lp_application_frame:
 *
 * Processes a single frame: update and draw.
 */
static void
lp_application_frame (LpApplication *self,
                      gdouble        delta)
{
    /* Update engine */
    lrg_engine_update (self->engine, (gfloat)delta);

    /* Update game states */
    lrg_game_state_manager_update (self->state_manager, delta);

    /* Draw game states */
    lrg_game_state_manager_draw (self->state_manager);

    /* Check if state stack is empty (quit) */
    if (lrg_game_state_manager_is_empty (self->state_manager))
    {
        self->running = FALSE;
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_application_finalize (GObject *object)
{
    LpApplication *self = LP_APPLICATION (object);

    /* Ensure we're shut down */
    if (self->initialized)
    {
        lp_application_shutdown (self);
    }

    /* Clear singleton reference */
    if (default_application == self)
    {
        default_application = NULL;
    }

    G_OBJECT_CLASS (lp_application_parent_class)->finalize (object);
}

static void
lp_application_class_init (LpApplicationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_application_finalize;
}

static void
lp_application_init (LpApplication *self)
{
    self->engine = NULL;
    self->window = NULL;
    self->state_manager = NULL;
    self->game_data = NULL;
    self->achievement_manager = NULL;
    self->running = FALSE;
    self->initialized = FALSE;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_application_get_default:
 *
 * Gets the default application instance. Creates it if it doesn't exist.
 *
 * Returns: (transfer none): The default #LpApplication instance
 */
LpApplication *
lp_application_get_default (void)
{
    if (default_application == NULL)
    {
        default_application = g_object_new (LP_TYPE_APPLICATION, NULL);
    }

    return default_application;
}

/**
 * lp_application_run:
 * @self: an #LpApplication
 * @argc: command line argument count
 * @argv: (array length=argc): command line arguments
 *
 * Runs the main game loop. This function blocks until the game exits.
 *
 * Returns: Exit status code (0 for success)
 */
gint
lp_application_run (LpApplication  *self,
                    gint            argc,
                    gchar         **argv)
{
    g_autoptr(GError) error = NULL;
    g_autoptr(GrlColor) clear_color = NULL;
    LrgWindow *window;
    gfloat delta;

    g_return_val_if_fail (LP_IS_APPLICATION (self), 1);

    (void)argc;
    (void)argv;

    /* Initialize subsystems */
    if (!lp_application_startup (self, &error))
    {
        lp_log_error ("Failed to start application: %s",
                      error->message);
        return 1;
    }

    window = LRG_WINDOW (self->window);
    self->running = TRUE;

    /* Dark background color for the lich theme */
    clear_color = grl_color_new (10, 10, 15, 255);

    lp_log_info ("Entering main loop");

    /*
     * Main game loop.
     *
     * The loop runs until the window is closed or quit is requested.
     * Frame timing is handled by the window (vsync or target FPS).
     */
    while (self->running && !lrg_window_should_close (window))
    {
        /* Poll for input events */
        lrg_window_poll_input (window);

        /* Get delta time from the window */
        delta = lrg_window_get_frame_time (window);

        /* Clamp delta to prevent spiral of death */
        if (delta > 0.25f)
        {
            delta = 0.25f;
        }

        /* Begin frame */
        lrg_window_begin_frame (window);
        lrg_window_clear (window, clear_color);

        /* Process frame (update + draw) */
        lp_application_frame (self, (gdouble)delta);

        /* End frame */
        lrg_window_end_frame (window);
    }

    lp_log_info ("Exiting main loop");

    /* Shutdown subsystems */
    lp_application_shutdown (self);

    return 0;
}

/**
 * lp_application_quit:
 * @self: an #LpApplication
 *
 * Signals the application to quit. The main loop will exit
 * after the current frame completes.
 */
void
lp_application_quit (LpApplication *self)
{
    g_return_if_fail (LP_IS_APPLICATION (self));

    lp_log_info ("Quit requested");
    self->running = FALSE;
}

/**
 * lp_application_get_engine:
 * @self: an #LpApplication
 *
 * Gets the libregnum engine instance.
 *
 * Returns: (transfer none): The #LrgEngine
 */
LrgEngine *
lp_application_get_engine (LpApplication *self)
{
    g_return_val_if_fail (LP_IS_APPLICATION (self), NULL);

    return self->engine;
}

/**
 * lp_application_get_state_manager:
 * @self: an #LpApplication
 *
 * Gets the game state manager.
 *
 * Returns: (transfer none): The #LrgGameStateManager
 */
LrgGameStateManager *
lp_application_get_state_manager (LpApplication *self)
{
    g_return_val_if_fail (LP_IS_APPLICATION (self), NULL);

    return self->state_manager;
}

/**
 * lp_application_get_game_data:
 * @self: an #LpApplication
 *
 * Gets the current game data. May be %NULL if no game is loaded.
 *
 * Returns: (transfer none) (nullable): The #LpGameData, or %NULL
 */
LpGameData *
lp_application_get_game_data (LpApplication *self)
{
    g_return_val_if_fail (LP_IS_APPLICATION (self), NULL);

    return self->game_data;
}

/**
 * lp_application_get_achievement_manager:
 * @self: an #LpApplication
 *
 * Gets the achievement manager.
 *
 * Returns: (transfer none): The #LpAchievementManager
 */
LpAchievementManager *
lp_application_get_achievement_manager (LpApplication *self)
{
    g_return_val_if_fail (LP_IS_APPLICATION (self), NULL);

    return self->achievement_manager;
}

/**
 * lp_application_new_game:
 * @self: an #LpApplication
 *
 * Starts a new game, creating fresh game data and transitioning
 * to the wake state (or first awakening for new players).
 */
void
lp_application_new_game (LpApplication *self)
{
    g_return_if_fail (LP_IS_APPLICATION (self));

    lp_log_info ("Starting new game");

    /* Clear any existing game data */
    g_clear_object (&self->game_data);

    /* Create fresh game data */
    self->game_data = lp_game_data_new ();

    /*
     * TODO: Transition to first awakening state for new players,
     * or directly to wake state for returning players.
     * For Phase 1, we just create the data.
     */
    lp_log_info ("New game created, starting year: %lu",
                 (gulong)lp_game_data_get_current_year (self->game_data));
}

/**
 * lp_application_load_game:
 * @self: an #LpApplication
 * @slot: the save slot to load from
 * @error: (nullable): return location for error
 *
 * Loads a saved game from the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_application_load_game (LpApplication  *self,
                          guint           slot,
                          GError        **error)
{
    g_return_val_if_fail (LP_IS_APPLICATION (self), FALSE);

    lp_log_info ("Loading game from slot %u", slot);

    /*
     * TODO: Implement save/load using LrgSaveManager.
     * For Phase 1, this is a skeleton.
     */
    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_SUPPORTED,
                 "Save/load not yet implemented (Phase 1 skeleton)");

    return FALSE;
}

/**
 * lp_application_save_game:
 * @self: an #LpApplication
 * @slot: the save slot to save to
 * @error: (nullable): return location for error
 *
 * Saves the current game to the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_application_save_game (LpApplication  *self,
                          guint           slot,
                          GError        **error)
{
    g_return_val_if_fail (LP_IS_APPLICATION (self), FALSE);

    if (self->game_data == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "No game data to save");
        return FALSE;
    }

    lp_log_info ("Saving game to slot %u", slot);

    /*
     * TODO: Implement save/load using LrgSaveManager.
     * For Phase 1, this is a skeleton.
     */
    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_SUPPORTED,
                 "Save/load not yet implemented (Phase 1 skeleton)");

    return FALSE;
}
