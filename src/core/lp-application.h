/* lp-application.h - Main Application Singleton
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The application singleton owns all major game subsystems and
 * coordinates the main game loop. Access via lp_application_get_default().
 */

#ifndef LP_APPLICATION_H
#define LP_APPLICATION_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_APPLICATION (lp_application_get_type ())

G_DECLARE_FINAL_TYPE (LpApplication, lp_application, LP, APPLICATION, GObject)

/**
 * lp_application_get_default:
 *
 * Gets the default application instance. Creates it if it doesn't exist.
 *
 * Returns: (transfer none): The default #LpApplication instance
 */
LpApplication *
lp_application_get_default (void);

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
                    gchar         **argv);

/**
 * lp_application_quit:
 * @self: an #LpApplication
 *
 * Signals the application to quit. The main loop will exit
 * after the current frame completes.
 */
void
lp_application_quit (LpApplication *self);

/**
 * lp_application_get_engine:
 * @self: an #LpApplication
 *
 * Gets the libregnum engine instance.
 *
 * Returns: (transfer none): The #LrgEngine
 */
LrgEngine *
lp_application_get_engine (LpApplication *self);

/**
 * lp_application_get_state_manager:
 * @self: an #LpApplication
 *
 * Gets the game state manager.
 *
 * Returns: (transfer none): The #LrgGameStateManager
 */
LrgGameStateManager *
lp_application_get_state_manager (LpApplication *self);

/**
 * lp_application_get_game_data:
 * @self: an #LpApplication
 *
 * Gets the current game data. May be %NULL if no game is loaded.
 *
 * Returns: (transfer none) (nullable): The #LpGameData, or %NULL
 */
LpGameData *
lp_application_get_game_data (LpApplication *self);

/**
 * lp_application_get_achievement_manager:
 * @self: an #LpApplication
 *
 * Gets the achievement manager.
 *
 * Returns: (transfer none): The #LpAchievementManager
 */
LpAchievementManager *
lp_application_get_achievement_manager (LpApplication *self);

/**
 * lp_application_new_game:
 * @self: an #LpApplication
 *
 * Starts a new game, creating fresh game data and transitioning
 * to the first awakening state.
 */
void
lp_application_new_game (LpApplication *self);

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
                          GError        **error);

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
                          GError        **error);

G_END_DECLS

#endif /* LP_APPLICATION_H */
