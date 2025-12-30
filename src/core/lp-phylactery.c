/* lp-phylactery.c - Upgrade Tree System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-phylactery.h"

struct _LpPhylactery
{
    GObject parent_instance;

    guint64     points;              /* Available phylactery points */
    guint64     total_points_earned; /* All-time points earned */
    GHashTable *upgrades;            /* upgrade_id -> level (skeleton: empty) */
};

enum
{
    PROP_0,
    PROP_POINTS,
    PROP_TOTAL_POINTS_EARNED,
    N_PROPS
};

enum
{
    SIGNAL_POINTS_CHANGED,
    SIGNAL_UPGRADE_PURCHASED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_phylactery_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpPhylactery, lp_phylactery, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_phylactery_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_phylactery_get_save_id (LrgSaveable *saveable)
{
    return "phylactery";
}

static gboolean
lp_phylactery_save (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpPhylactery *self = LP_PHYLACTERY (saveable);

    lrg_save_context_write_uint (context, "points", self->points);
    lrg_save_context_write_uint (context, "total-points-earned",
                                 self->total_points_earned);

    /* Phase 7+: Save upgrade states */
    lrg_save_context_write_uint (context, "upgrade-count", 0);

    return TRUE;
}

static gboolean
lp_phylactery_load (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpPhylactery *self = LP_PHYLACTERY (saveable);

    self->points = lrg_save_context_read_uint (context, "points", 0);
    self->total_points_earned = lrg_save_context_read_uint (
        context, "total-points-earned", 0);

    /* Phase 7+: Load upgrade states */
    g_hash_table_remove_all (self->upgrades);

    lp_log_debug ("Loaded phylactery: %lu points available, %lu total earned",
                  self->points, self->total_points_earned);

    return TRUE;
}

static void
lp_phylactery_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_phylactery_get_save_id;
    iface->save = lp_phylactery_save;
    iface->load = lp_phylactery_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_phylactery_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    LpPhylactery *self = LP_PHYLACTERY (object);

    switch (prop_id)
    {
    case PROP_POINTS:
        g_value_set_uint64 (value, self->points);
        break;

    case PROP_TOTAL_POINTS_EARNED:
        g_value_set_uint64 (value, self->total_points_earned);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_phylactery_finalize (GObject *object)
{
    LpPhylactery *self = LP_PHYLACTERY (object);

    lp_log_debug ("Finalizing phylactery");

    g_clear_pointer (&self->upgrades, g_hash_table_unref);

    G_OBJECT_CLASS (lp_phylactery_parent_class)->finalize (object);
}

static void
lp_phylactery_class_init (LpPhylacteryClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_phylactery_get_property;
    object_class->finalize = lp_phylactery_finalize;

    /**
     * LpPhylactery:points:
     *
     * Available phylactery points.
     */
    properties[PROP_POINTS] =
        g_param_spec_uint64 ("points",
                             "Points",
                             "Available phylactery points",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpPhylactery:total-points-earned:
     *
     * Total phylactery points earned all-time.
     */
    properties[PROP_TOTAL_POINTS_EARNED] =
        g_param_spec_uint64 ("total-points-earned",
                             "Total Points Earned",
                             "Total phylactery points earned all-time",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpPhylactery::points-changed:
     * @self: the #LpPhylactery
     * @old_points: previous point count
     * @new_points: new point count
     *
     * Emitted when available points change.
     */
    signals[SIGNAL_POINTS_CHANGED] =
        g_signal_new ("points-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_UINT64,
                      G_TYPE_UINT64);

    /**
     * LpPhylactery::upgrade-purchased:
     * @self: the #LpPhylactery
     * @upgrade_id: the purchased upgrade ID
     *
     * Emitted when an upgrade is purchased.
     */
    signals[SIGNAL_UPGRADE_PURCHASED] =
        g_signal_new ("upgrade-purchased",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_STRING);
}

static void
lp_phylactery_init (LpPhylactery *self)
{
    self->points = 0;
    self->total_points_earned = 0;
    self->upgrades = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            g_free, NULL);
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_phylactery_new:
 *
 * Creates a new phylactery.
 *
 * Returns: (transfer full): A new #LpPhylactery
 */
LpPhylactery *
lp_phylactery_new (void)
{
    return g_object_new (LP_TYPE_PHYLACTERY, NULL);
}

/* ==========================================================================
 * Points Management
 * ========================================================================== */

/**
 * lp_phylactery_get_points:
 * @self: an #LpPhylactery
 *
 * Gets the number of available phylactery points.
 *
 * Returns: Available points
 */
guint64
lp_phylactery_get_points (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    return self->points;
}

/**
 * lp_phylactery_get_total_points_earned:
 * @self: an #LpPhylactery
 *
 * Gets the total points ever earned.
 *
 * Returns: Total points earned
 */
guint64
lp_phylactery_get_total_points_earned (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    return self->total_points_earned;
}

/**
 * lp_phylactery_add_points:
 * @self: an #LpPhylactery
 * @points: number of points to add
 *
 * Adds phylactery points.
 */
void
lp_phylactery_add_points (LpPhylactery *self,
                          guint64       points)
{
    guint64 old_points;

    g_return_if_fail (LP_IS_PHYLACTERY (self));

    if (points == 0)
        return;

    old_points = self->points;
    self->points += points;
    self->total_points_earned += points;

    lp_log_info ("Added %lu phylactery points (now: %lu)",
                 points, self->points);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POINTS]);
    g_signal_emit (self, signals[SIGNAL_POINTS_CHANGED], 0,
                   old_points, self->points);
}

/* ==========================================================================
 * Upgrade Management (Skeleton)
 * ========================================================================== */

/**
 * lp_phylactery_get_upgrade_count:
 * @self: an #LpPhylactery
 *
 * Gets the number of purchased upgrades.
 *
 * Returns: Number of purchased upgrades
 */
guint
lp_phylactery_get_upgrade_count (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    return g_hash_table_size (self->upgrades);
}

/**
 * lp_phylactery_has_upgrade:
 * @self: an #LpPhylactery
 * @upgrade_id: the upgrade ID to check
 *
 * Checks if an upgrade has been purchased.
 *
 * Returns: %TRUE if the upgrade is owned
 */
gboolean
lp_phylactery_has_upgrade (LpPhylactery *self,
                           const gchar  *upgrade_id)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    return g_hash_table_contains (self->upgrades, upgrade_id);
}

/**
 * lp_phylactery_purchase_upgrade:
 * @self: an #LpPhylactery
 * @upgrade_id: the upgrade ID to purchase
 *
 * Purchases an upgrade if requirements are met.
 *
 * Returns: %TRUE if successfully purchased
 */
gboolean
lp_phylactery_purchase_upgrade (LpPhylactery *self,
                                const gchar  *upgrade_id)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    /*
     * Phase 1 skeleton: No upgrade definitions loaded.
     * Phase 7+: Look up upgrade, check requirements/cost, purchase.
     */
    lp_log_debug ("Upgrade purchase attempted: %s (skeleton - not available)",
                  upgrade_id);

    return FALSE;
}

/* ==========================================================================
 * Bonus Calculation (Skeleton)
 * ========================================================================== */

/**
 * lp_phylactery_get_starting_gold_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to starting gold from upgrades.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_phylactery_get_starting_gold_bonus (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 1.0);

    /* Phase 1 skeleton: No bonuses */
    return 1.0;
}

/**
 * lp_phylactery_get_income_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to all income from upgrades.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_phylactery_get_income_bonus (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 1.0);

    /* Phase 1 skeleton: No bonuses */
    return 1.0;
}

/**
 * lp_phylactery_get_exposure_decay_bonus:
 * @self: an #LpPhylactery
 *
 * Gets the bonus to exposure decay from upgrades.
 *
 * Returns: Flat bonus to decay rate
 */
guint
lp_phylactery_get_exposure_decay_bonus (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    /* Phase 1 skeleton: No bonuses */
    return 0;
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_phylactery_reset_upgrades:
 * @self: an #LpPhylactery
 *
 * Resets all upgrades and refunds points.
 */
void
lp_phylactery_reset_upgrades (LpPhylactery *self)
{
    g_return_if_fail (LP_IS_PHYLACTERY (self));

    lp_log_debug ("Resetting phylactery upgrades");

    /* Refund all spent points */
    self->points = self->total_points_earned;
    g_hash_table_remove_all (self->upgrades);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POINTS]);
}
