/* lp-prestige-manager.h - Prestige System Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpPrestigeManager manages the prestige/reset mechanics for the game.
 * Prestige rewards players with "Echoes" (memory fragments) that can be
 * spent on permanent bonuses in the four Echo specialization trees.
 *
 * This is a derivable type to allow for testing and future extensibility.
 *
 * Implements LrgSaveable for persistence (Echoes persist across prestige).
 */

#ifndef LP_PRESTIGE_MANAGER_H
#define LP_PRESTIGE_MANAGER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_PRESTIGE_MANAGER (lp_prestige_manager_get_type ())

G_DECLARE_DERIVABLE_TYPE (LpPrestigeManager, lp_prestige_manager, LP, PRESTIGE_MANAGER, GObject)

/**
 * LpPrestigeManagerClass:
 * @parent_class: Parent class
 * @calculate_echo_reward: Calculate Echoes to be gained from prestige
 * @can_prestige: Check if prestige is currently available
 * @on_prestige: Called when prestige is performed
 * @get_bonus_multiplier: Calculate cumulative prestige bonus multiplier
 *
 * Virtual table for #LpPrestigeManager.
 */
struct _LpPrestigeManagerClass
{
    GObjectClass parent_class;

    /*< public >*/

    /**
     * LpPrestigeManagerClass::calculate_echo_reward:
     * @self: an #LpPrestigeManager
     * @total_gold: Total gold accumulated this run
     * @years_played: Years played this run
     *
     * Calculates how many Echoes would be gained from prestige.
     * Default formula: log10(total_gold) * sqrt(years_played) / 10
     *
     * Returns: (transfer full): Echoes to gain
     */
    LrgBigNumber * (*calculate_echo_reward)  (LpPrestigeManager  *self,
                                              const LrgBigNumber *total_gold,
                                              guint64             years_played);

    /**
     * LpPrestigeManagerClass::can_prestige:
     * @self: an #LpPrestigeManager
     * @total_gold: Total gold accumulated this run
     * @years_played: Years played this run
     *
     * Checks if prestige requirements are met.
     * Default: years_played >= 100 and total_gold >= 1,000,000
     *
     * Returns: %TRUE if prestige is available
     */
    gboolean       (*can_prestige)           (LpPrestigeManager  *self,
                                              const LrgBigNumber *total_gold,
                                              guint64             years_played);

    /**
     * LpPrestigeManagerClass::on_prestige:
     * @self: an #LpPrestigeManager
     * @echoes_gained: Echoes being awarded
     *
     * Called when prestige is performed. Subclasses can override
     * to perform additional cleanup or state changes.
     */
    void           (*on_prestige)            (LpPrestigeManager  *self,
                                              const LrgBigNumber *echoes_gained);

    /**
     * LpPrestigeManagerClass::get_bonus_multiplier:
     * @self: an #LpPrestigeManager
     *
     * Calculates the bonus multiplier from prestige count and upgrades.
     * Default: 1.0 + (0.1 * times_prestiged) + tree_bonuses
     *
     * Returns: Multiplier value (1.0 = no bonus)
     */
    gdouble        (*get_bonus_multiplier)   (LpPrestigeManager *self);

    /*< private >*/
    gpointer _reserved[8];
};

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_prestige_manager_new:
 *
 * Creates a new prestige manager.
 *
 * Returns: (transfer full): A new #LpPrestigeManager
 */
LpPrestigeManager *
lp_prestige_manager_new (void);

/* ==========================================================================
 * Echo Management
 * ========================================================================== */

/**
 * lp_prestige_manager_get_echoes:
 * @self: an #LpPrestigeManager
 *
 * Gets the current Echo (prestige point) count.
 *
 * Returns: (transfer none): Current Echoes
 */
const LrgBigNumber *
lp_prestige_manager_get_echoes (LpPrestigeManager *self);

/**
 * lp_prestige_manager_get_total_echoes_earned:
 * @self: an #LpPrestigeManager
 *
 * Gets the total Echoes ever earned.
 *
 * Returns: (transfer none): Total Echoes earned
 */
const LrgBigNumber *
lp_prestige_manager_get_total_echoes_earned (LpPrestigeManager *self);

/**
 * lp_prestige_manager_spend_echoes:
 * @self: an #LpPrestigeManager
 * @amount: Echoes to spend
 *
 * Spends Echoes. Used when purchasing Echo tree upgrades.
 *
 * Returns: %TRUE if successfully spent
 */
gboolean
lp_prestige_manager_spend_echoes (LpPrestigeManager  *self,
                                  const LrgBigNumber *amount);

/**
 * lp_prestige_manager_get_times_prestiged:
 * @self: an #LpPrestigeManager
 *
 * Gets how many times prestige has been performed.
 *
 * Returns: Prestige count
 */
guint64
lp_prestige_manager_get_times_prestiged (LpPrestigeManager *self);

/* ==========================================================================
 * Echo Specialization Trees
 * ========================================================================== */

/**
 * lp_prestige_manager_get_echo_tree:
 * @self: an #LpPrestigeManager
 * @tree: Which specialization tree to get
 *
 * Gets the unlock tree for a specific Echo specialization.
 *
 * Returns: (transfer none): The unlock tree
 */
LrgUnlockTree *
lp_prestige_manager_get_echo_tree (LpPrestigeManager *self,
                                   LpEchoTree         tree);

/**
 * lp_prestige_manager_unlock_upgrade:
 * @self: an #LpPrestigeManager
 * @tree: Which specialization tree
 * @upgrade_id: ID of the upgrade to unlock
 *
 * Attempts to unlock an upgrade in an Echo tree.
 * Deducts cost from Echoes if successful.
 *
 * Returns: %TRUE if successfully unlocked
 */
gboolean
lp_prestige_manager_unlock_upgrade (LpPrestigeManager *self,
                                    LpEchoTree         tree,
                                    const gchar       *upgrade_id);

/**
 * lp_prestige_manager_has_upgrade:
 * @self: an #LpPrestigeManager
 * @tree: Which specialization tree
 * @upgrade_id: ID of the upgrade to check
 *
 * Checks if an upgrade is unlocked.
 *
 * Returns: %TRUE if unlocked
 */
gboolean
lp_prestige_manager_has_upgrade (LpPrestigeManager *self,
                                 LpEchoTree         tree,
                                 const gchar       *upgrade_id);

/* ==========================================================================
 * Prestige Operations
 * ========================================================================== */

/**
 * lp_prestige_manager_calculate_echo_reward:
 * @self: an #LpPrestigeManager
 * @total_gold: Total gold accumulated this run
 * @years_played: Years played this run
 *
 * Calculates how many Echoes would be gained from prestige.
 *
 * Returns: (transfer full): Pending Echoes
 */
LrgBigNumber *
lp_prestige_manager_calculate_echo_reward (LpPrestigeManager  *self,
                                           const LrgBigNumber *total_gold,
                                           guint64             years_played);

/**
 * lp_prestige_manager_can_prestige:
 * @self: an #LpPrestigeManager
 * @total_gold: Total gold accumulated this run
 * @years_played: Years played this run
 *
 * Checks if prestige is available.
 *
 * Returns: %TRUE if prestige requirements are met
 */
gboolean
lp_prestige_manager_can_prestige (LpPrestigeManager  *self,
                                  const LrgBigNumber *total_gold,
                                  guint64             years_played);

/**
 * lp_prestige_manager_perform_prestige:
 * @self: an #LpPrestigeManager
 * @total_gold: Total gold accumulated this run
 * @years_played: Years played this run
 *
 * Performs prestige, adding reward to Echoes.
 * Emits the ::prestige-performed signal.
 *
 * Returns: (transfer full): Echoes awarded (for display)
 */
LrgBigNumber *
lp_prestige_manager_perform_prestige (LpPrestigeManager  *self,
                                      const LrgBigNumber *total_gold,
                                      guint64             years_played);

/**
 * lp_prestige_manager_get_bonus_multiplier:
 * @self: an #LpPrestigeManager
 *
 * Gets the current bonus multiplier from prestige.
 *
 * Returns: Bonus multiplier (1.0 = no bonus)
 */
gdouble
lp_prestige_manager_get_bonus_multiplier (LpPrestigeManager *self);

/* ==========================================================================
 * Bonus Queries (from Echo Trees)
 * ========================================================================== */

/**
 * lp_prestige_manager_get_starting_gold_multiplier:
 * @self: an #LpPrestigeManager
 *
 * Gets starting gold multiplier from Economist tree.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_prestige_manager_get_starting_gold_multiplier (LpPrestigeManager *self);

/**
 * lp_prestige_manager_get_compound_interest_bonus:
 * @self: an #LpPrestigeManager
 *
 * Gets bonus to compound interest from Economist tree.
 *
 * Returns: Additive bonus (e.g., 0.02 = +2%)
 */
gdouble
lp_prestige_manager_get_compound_interest_bonus (LpPrestigeManager *self);

/**
 * lp_prestige_manager_get_ledger_retention:
 * @self: an #LpPrestigeManager
 *
 * Gets fraction of Ledger entries to keep on prestige from Scholar tree.
 *
 * Returns: Retention (0.0 = keep none, 1.0 = keep all)
 */
gdouble
lp_prestige_manager_get_ledger_retention (LpPrestigeManager *self);

/**
 * lp_prestige_manager_get_gold_retention:
 * @self: an #LpPrestigeManager
 *
 * Gets fraction of gold to keep on prestige from Architect tree.
 *
 * Returns: Retention (0.0 = keep none, 1.0 = keep all)
 */
gdouble
lp_prestige_manager_get_gold_retention (LpPrestigeManager *self);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_prestige_manager_reset:
 * @self: an #LpPrestigeManager
 *
 * Resets all prestige progress (Echoes, counts, tree unlocks).
 * Used for full game reset (NOT normal prestige).
 */
void
lp_prestige_manager_reset (LpPrestigeManager *self);

G_END_DECLS

#endif /* LP_PRESTIGE_MANAGER_H */
