/* lp-game-data.h - Central Game State Container
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpGameData is the central container for all game state.
 * It owns the portfolio, agent manager, phylactery, ledger,
 * and world simulation.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_GAME_DATA_H
#define LP_GAME_DATA_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_GAME_DATA (lp_game_data_get_type ())

G_DECLARE_FINAL_TYPE (LpGameData, lp_game_data, LP, GAME_DATA, GObject)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_game_data_new:
 *
 * Creates a new game data container with default values.
 *
 * Returns: (transfer full): A new #LpGameData
 */
LpGameData *
lp_game_data_new (void);

/* ==========================================================================
 * Time/Year Management
 * ========================================================================== */

/**
 * lp_game_data_get_current_year:
 * @self: an #LpGameData
 *
 * Gets the current in-game year.
 *
 * Returns: The current year
 */
guint64
lp_game_data_get_current_year (LpGameData *self);

/**
 * lp_game_data_get_total_years_played:
 * @self: an #LpGameData
 *
 * Gets the total years played across all runs.
 *
 * Returns: Total years played
 */
guint64
lp_game_data_get_total_years_played (LpGameData *self);

/* ==========================================================================
 * Subsystem Access
 * ========================================================================== */

/**
 * lp_game_data_get_portfolio:
 * @self: an #LpGameData
 *
 * Gets the player's portfolio.
 *
 * Returns: (transfer none): The #LpPortfolio
 */
LpPortfolio *
lp_game_data_get_portfolio (LpGameData *self);

/**
 * lp_game_data_get_agent_manager:
 * @self: an #LpGameData
 *
 * Gets the agent manager.
 *
 * Returns: (transfer none): The #LpAgentManager
 */
LpAgentManager *
lp_game_data_get_agent_manager (LpGameData *self);

/**
 * lp_game_data_get_phylactery:
 * @self: an #LpGameData
 *
 * Gets the phylactery (upgrade tree).
 *
 * Returns: (transfer none): The #LpPhylactery
 */
LpPhylactery *
lp_game_data_get_phylactery (LpGameData *self);

/**
 * lp_game_data_get_ledger:
 * @self: an #LpGameData
 *
 * Gets the ledger (discovery tracking).
 *
 * Returns: (transfer none): The #LpLedger
 */
LpLedger *
lp_game_data_get_ledger (LpGameData *self);

/**
 * lp_game_data_get_world_simulation:
 * @self: an #LpGameData
 *
 * Gets the world simulation.
 *
 * Returns: (transfer none): The #LpWorldSimulation
 */
LpWorldSimulation *
lp_game_data_get_world_simulation (LpGameData *self);

/**
 * lp_game_data_get_portfolio_history:
 * @self: an #LpGameData
 *
 * Gets the portfolio history tracker.
 *
 * Returns: (transfer none): The #LpPortfolioHistory
 */
LpPortfolioHistory *
lp_game_data_get_portfolio_history (LpGameData *self);

/* ==========================================================================
 * Game Actions
 * ========================================================================== */

/**
 * lp_game_data_start_new_game:
 * @self: an #LpGameData
 *
 * Starts a new game from scratch.
 * Resets all data including ledger and phylactery.
 */
void
lp_game_data_start_new_game (LpGameData *self);

/**
 * lp_game_data_prestige:
 * @self: an #LpGameData
 *
 * Performs a prestige reset.
 * Resets portfolio, agents, world, but keeps ledger and phylactery.
 * Awards phylactery points based on performance.
 *
 * Returns: Number of phylactery points earned
 */
guint64
lp_game_data_prestige (LpGameData *self);

/**
 * lp_game_data_slumber:
 * @self: an #LpGameData
 * @years: number of years to slumber
 *
 * Enters slumber for the specified number of years.
 * Advances the world simulation and processes all events.
 *
 * Returns: (transfer full) (element-type LpEvent): List of events that occurred
 */
GList *
lp_game_data_slumber (LpGameData *self,
                      guint       years);

G_END_DECLS

#endif /* LP_GAME_DATA_H */
