/* lp-tutorial-sequences.h - Tutorial Sequence Definitions
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpTutorialSequences provides tutorial registration and condition
 * checking for first-time player guidance.
 *
 * Tutorials:
 * - "intro"      - Introduction to the lich's awakening
 * - "investment" - First portfolio visit guidance
 * - "slumber"    - Time passage and slumber mechanics
 */

#ifndef LP_TUTORIAL_SEQUENCES_H
#define LP_TUTORIAL_SEQUENCES_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_TUTORIAL_SEQUENCES (lp_tutorial_sequences_get_type ())

G_DECLARE_FINAL_TYPE (LpTutorialSequences, lp_tutorial_sequences, LP, TUTORIAL_SEQUENCES, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_tutorial_sequences_get_default:
 *
 * Gets the default tutorial sequences instance.
 * Initializes and registers all tutorials on first call.
 *
 * Returns: (transfer none): The default #LpTutorialSequences
 */
LpTutorialSequences *
lp_tutorial_sequences_get_default (void);

/**
 * lp_tutorial_sequences_set_game:
 * @self: an #LpTutorialSequences
 * @game: the game instance to use for accessing game data
 *
 * Sets the game reference used for condition checking.
 * Must be called before using tutorials that need game data.
 */
void
lp_tutorial_sequences_set_game (LpTutorialSequences *self,
                                 LpGame              *game);

/* ==========================================================================
 * Tutorial Control
 * ========================================================================== */

/**
 * lp_tutorial_sequences_init_tutorials:
 * @self: an #LpTutorialSequences
 * @manager: the #LrgTutorialManager to register with
 * @error: (nullable): return location for error
 *
 * Loads and registers all game tutorials with the manager.
 * Should be called during application initialization.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_tutorial_sequences_init_tutorials (LpTutorialSequences  *self,
                                       LrgTutorialManager   *manager,
                                       GError              **error);

/**
 * lp_tutorial_sequences_maybe_start_intro:
 * @self: an #LpTutorialSequences
 *
 * Starts the intro tutorial if this is a new game.
 * Safe to call multiple times - checks completion status.
 */
void
lp_tutorial_sequences_maybe_start_intro (LpTutorialSequences *self);

/**
 * lp_tutorial_sequences_maybe_start_investment:
 * @self: an #LpTutorialSequences
 *
 * Starts the investment tutorial on first portfolio visit.
 * Safe to call multiple times - checks completion status.
 */
void
lp_tutorial_sequences_maybe_start_investment (LpTutorialSequences *self);

/**
 * lp_tutorial_sequences_maybe_start_slumber:
 * @self: an #LpTutorialSequences
 *
 * Starts the slumber tutorial on first slumber attempt.
 * Safe to call multiple times - checks completion status.
 */
void
lp_tutorial_sequences_maybe_start_slumber (LpTutorialSequences *self);

/**
 * lp_tutorial_sequences_check_condition:
 * @condition_id: the condition ID to check
 * @user_data: user data (unused)
 *
 * Callback for tutorial condition checking.
 * Used by LrgTutorialManager to evaluate step conditions.
 *
 * Returns: %TRUE if the condition is met
 */
gboolean
lp_tutorial_sequences_check_condition (const gchar *condition_id,
                                        gpointer     user_data);

/* ==========================================================================
 * Tutorial IDs
 * ========================================================================== */

#define LP_TUTORIAL_INTRO       "intro"
#define LP_TUTORIAL_INVESTMENT  "investment"
#define LP_TUTORIAL_SLUMBER     "slumber"

/* ==========================================================================
 * Condition IDs
 * ========================================================================== */

#define LP_CONDITION_HAS_GOLD             "has_gold"
#define LP_CONDITION_PORTFOLIO_OPEN       "portfolio_open"
#define LP_CONDITION_HAS_INVESTMENT       "has_investment"
#define LP_CONDITION_SLUMBER_SELECTED     "slumber_selected"

G_END_DECLS

#endif /* LP_TUTORIAL_SEQUENCES_H */
