/* lp-synergy-manager.c - Synergy Detection Singleton
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-synergy-manager.h"

struct _LpSynergyManager
{
    GObject parent_instance;

    GPtrArray *active_synergies;    /* Array of LpSynergy (skeleton: empty) */
    gdouble    total_bonus;         /* Cached total bonus multiplier */
};

enum
{
    SIGNAL_SYNERGIES_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

static LpSynergyManager *default_manager = NULL;

G_DEFINE_TYPE (LpSynergyManager, lp_synergy_manager, G_TYPE_OBJECT)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_synergy_manager_get_default:
 *
 * Gets the default synergy manager instance.
 *
 * Returns: (transfer none): The default #LpSynergyManager
 */
LpSynergyManager *
lp_synergy_manager_get_default (void)
{
    if (default_manager == NULL)
        default_manager = g_object_new (LP_TYPE_SYNERGY_MANAGER, NULL);

    return default_manager;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_synergy_manager_finalize (GObject *object)
{
    LpSynergyManager *self = LP_SYNERGY_MANAGER (object);

    lp_log_debug ("Finalizing synergy manager");

    g_clear_pointer (&self->active_synergies, g_ptr_array_unref);

    if (default_manager == self)
        default_manager = NULL;

    G_OBJECT_CLASS (lp_synergy_manager_parent_class)->finalize (object);
}

static void
lp_synergy_manager_class_init (LpSynergyManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_synergy_manager_finalize;

    /**
     * LpSynergyManager::synergies-changed:
     * @self: the #LpSynergyManager
     *
     * Emitted when the set of active synergies changes.
     */
    signals[SIGNAL_SYNERGIES_CHANGED] =
        g_signal_new ("synergies-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_synergy_manager_init (LpSynergyManager *self)
{
    /* Create empty array - will hold LpSynergy objects in Phase 2+ */
    self->active_synergies = g_ptr_array_new_with_free_func (g_object_unref);
    self->total_bonus = 1.0;
}

/* ==========================================================================
 * Synergy Detection (Skeleton)
 * ========================================================================== */

/**
 * lp_synergy_manager_get_active_synergies:
 * @self: an #LpSynergyManager
 *
 * Gets the list of currently active synergies.
 *
 * Returns: (transfer none) (element-type LpSynergy): Array of active synergies
 */
GPtrArray *
lp_synergy_manager_get_active_synergies (LpSynergyManager *self)
{
    g_return_val_if_fail (LP_IS_SYNERGY_MANAGER (self), NULL);

    return self->active_synergies;
}

/**
 * lp_synergy_manager_get_synergy_count:
 * @self: an #LpSynergyManager
 *
 * Gets the number of currently active synergies.
 *
 * Returns: Number of active synergies
 */
guint
lp_synergy_manager_get_synergy_count (LpSynergyManager *self)
{
    g_return_val_if_fail (LP_IS_SYNERGY_MANAGER (self), 0);

    return self->active_synergies->len;
}

/**
 * lp_synergy_manager_recalculate:
 * @self: an #LpSynergyManager
 * @portfolio: (nullable): the portfolio to analyze
 *
 * Recalculates active synergies based on the current portfolio.
 *
 * Note: Skeleton implementation - always results in no synergies.
 */
void
lp_synergy_manager_recalculate (LpSynergyManager *self,
                                LpPortfolio      *portfolio)
{
    g_return_if_fail (LP_IS_SYNERGY_MANAGER (self));

    /*
     * Phase 1 skeleton: No synergy detection implemented yet.
     * In Phase 2+, this will:
     * 1. Clear current active synergies
     * 2. Iterate through all defined synergy rules
     * 3. Check if portfolio meets each synergy's requirements
     * 4. Add matching synergies to active_synergies
     * 5. Recalculate total_bonus
     * 6. Emit synergies-changed signal
     */

    lp_log_debug ("Synergy recalculation requested (skeleton - no-op)");

    /* Skeleton: no synergies, no bonus */
    g_ptr_array_set_size (self->active_synergies, 0);
    self->total_bonus = 1.0;
}

/**
 * lp_synergy_manager_get_total_bonus:
 * @self: an #LpSynergyManager
 *
 * Gets the total bonus multiplier from all active synergies.
 *
 * Returns: Total bonus as a multiplier (1.0 = no bonus)
 */
gdouble
lp_synergy_manager_get_total_bonus (LpSynergyManager *self)
{
    g_return_val_if_fail (LP_IS_SYNERGY_MANAGER (self), 1.0);

    return self->total_bonus;
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_synergy_manager_reset:
 * @self: an #LpSynergyManager
 *
 * Resets the synergy manager to initial state.
 */
void
lp_synergy_manager_reset (LpSynergyManager *self)
{
    g_return_if_fail (LP_IS_SYNERGY_MANAGER (self));

    lp_log_debug ("Resetting synergy manager");

    g_ptr_array_set_size (self->active_synergies, 0);
    self->total_bonus = 1.0;

    g_signal_emit (self, signals[SIGNAL_SYNERGIES_CHANGED], 0);
}
