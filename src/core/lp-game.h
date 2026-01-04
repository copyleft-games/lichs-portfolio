/* lp-game.h - Main Game (LrgIdle2DTemplate Subclass)
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpGame is the main game class for Lich's Portfolio. It extends
 * LrgIdle2DTemplate to gain automatic offline progress calculation,
 * prestige mechanics, 2D viewport scaling, and template-managed game loop.
 *
 * ## Two Time Systems
 *
 * The game has two independent time-passage mechanics:
 *
 * 1. **Real-time offline progress**: When the app is closed and reopened,
 *    the template calculates gold earned based on investment income.
 *    This is a small bonus (10% efficiency, 1 week cap).
 *
 * 2. **Explicit slumber**: The core game mechanic where Malachar sleeps
 *    for decades/centuries, advancing the world simulation and triggering
 *    events. This uses lp_game_data_slumber().
 *
 * ## Usage
 *
 * ```c
 * int main (int argc, char **argv)
 * {
 *     g_autoptr(LpGame) game = lp_game_new ();
 *     return lrg_game_template_run (LRG_GAME_TEMPLATE (game), argc, argv);
 * }
 * ```
 */

#ifndef LP_GAME_H
#define LP_GAME_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_GAME (lp_game_get_type ())

G_DECLARE_FINAL_TYPE (LpGame, lp_game, LP, GAME, LrgIdle2DTemplate)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_game_new:
 *
 * Creates a new game instance.
 *
 * Returns: (transfer full): A new #LpGame
 */
LpGame *
lp_game_new (void);

/* ==========================================================================
 * State Access Helper
 * ========================================================================== */

/**
 * lp_game_get_from_state:
 * @state: an #LrgGameState
 *
 * Gets the game instance from a game state. States can use this to access
 * the game without needing a singleton.
 *
 * The game is retrieved from the state manager's template reference.
 *
 * Returns: (transfer none) (nullable): The #LpGame, or %NULL if not found
 */
LpGame *
lp_game_get_from_state (LrgGameState *state);

/* ==========================================================================
 * Subsystem Access
 * ========================================================================== */

/**
 * lp_game_get_game_data:
 * @self: an #LpGame
 *
 * Gets the current game data. May be %NULL if no game is loaded.
 *
 * Returns: (transfer none) (nullable): The #LpGameData, or %NULL
 */
LpGameData *
lp_game_get_game_data (LpGame *self);

/**
 * lp_game_get_phylactery:
 * @self: an #LpGame
 *
 * Gets the phylactery (upgrade tree) from the current game data.
 *
 * Returns: (transfer none) (nullable): The #LpPhylactery, or %NULL
 */
LpPhylactery *
lp_game_get_phylactery (LpGame *self);

/**
 * lp_game_get_prestige_layer:
 * @self: an #LpGame
 *
 * Gets the prestige layer (LpPrestige) from the template.
 *
 * Returns: (transfer none) (nullable): The #LpPrestige, or %NULL
 */
LpPrestige *
lp_game_get_prestige_layer (LpGame *self);

/**
 * lp_game_get_achievement_manager:
 * @self: an #LpGame
 *
 * Gets the achievement manager.
 *
 * Returns: (transfer none): The #LpAchievementManager
 */
LpAchievementManager *
lp_game_get_achievement_manager (LpGame *self);

/* ==========================================================================
 * Game Management
 * ========================================================================== */

/**
 * lp_game_new_game:
 * @self: an #LpGame
 *
 * Starts a new game, creating fresh game data.
 */
void
lp_game_new_game (LpGame *self);

/**
 * lp_game_load_game:
 * @self: an #LpGame
 * @slot: the save slot to load from
 * @error: (nullable): return location for error
 *
 * Loads a saved game from the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_game_load_game (LpGame   *self,
                   guint     slot,
                   GError  **error);

/**
 * lp_game_save_game:
 * @self: an #LpGame
 * @slot: the save slot to save to
 * @error: (nullable): return location for error
 *
 * Saves the current game to the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_game_save_game (LpGame   *self,
                   guint     slot,
                   GError  **error);

/* ==========================================================================
 * Idle System Integration
 * ========================================================================== */

/**
 * lp_game_sync_generators:
 * @self: an #LpGame
 *
 * Synchronizes investments to idle calculator generators.
 * Call this after investments are added, removed, or modified.
 */
void
lp_game_sync_generators (LpGame *self);

G_END_DECLS

#endif /* LP_GAME_H */
