/* lp-synergy-manager.h - Synergy Detection Singleton
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Detects and tracks synergies between investments.
 * Synergies provide bonus returns when certain investment combinations
 * are held together.
 */

#ifndef LP_SYNERGY_MANAGER_H
#define LP_SYNERGY_MANAGER_H

#include <glib-object.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_SYNERGY_MANAGER (lp_synergy_manager_get_type ())

G_DECLARE_FINAL_TYPE (LpSynergyManager, lp_synergy_manager,
                      LP, SYNERGY_MANAGER, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_synergy_manager_get_default:
 *
 * Gets the default synergy manager instance.
 * Creates it if it doesn't exist.
 *
 * Returns: (transfer none): The default #LpSynergyManager instance
 */
LpSynergyManager *
lp_synergy_manager_get_default (void);

/* ==========================================================================
 * Synergy Detection (Skeleton - Phase 2+)
 * ========================================================================== */

/**
 * lp_synergy_manager_get_active_synergies:
 * @self: an #LpSynergyManager
 *
 * Gets the list of currently active synergies.
 *
 * Note: This is a skeleton implementation for Phase 1.
 * Full synergy detection will be implemented in Phase 2.
 *
 * Returns: (transfer none) (element-type LpSynergy): Array of active synergies
 */
GPtrArray *
lp_synergy_manager_get_active_synergies (LpSynergyManager *self);

/**
 * lp_synergy_manager_get_synergy_count:
 * @self: an #LpSynergyManager
 *
 * Gets the number of currently active synergies.
 *
 * Returns: Number of active synergies
 */
guint
lp_synergy_manager_get_synergy_count (LpSynergyManager *self);

/**
 * lp_synergy_manager_recalculate:
 * @self: an #LpSynergyManager
 * @portfolio: (nullable): the portfolio to analyze
 *
 * Recalculates active synergies based on the current portfolio.
 * Should be called when investments change.
 *
 * Note: This is a skeleton implementation for Phase 1.
 */
void
lp_synergy_manager_recalculate (LpSynergyManager *self,
                                LpPortfolio      *portfolio);

/**
 * lp_synergy_manager_get_total_bonus:
 * @self: an #LpSynergyManager
 *
 * Gets the total bonus multiplier from all active synergies.
 *
 * Returns: Total bonus as a multiplier (1.0 = no bonus)
 */
gdouble
lp_synergy_manager_get_total_bonus (LpSynergyManager *self);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_synergy_manager_reset:
 * @self: an #LpSynergyManager
 *
 * Resets the synergy manager to initial state.
 * Called when starting a new game or after prestige.
 */
void
lp_synergy_manager_reset (LpSynergyManager *self);

G_END_DECLS

#endif /* LP_SYNERGY_MANAGER_H */
