/* lp-prestige.c - Custom Prestige Layer Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-prestige.h"
#include "lp-phylactery.h"
#include "../investment/lp-portfolio.h"

#include <math.h>

/*
 * Prestige configuration:
 * - Minimum threshold: 10,000 gold (to earn at least 1 point)
 * - Formula: points = log10(value) - 3
 * - Bonus multiplier: 1.0 + (points * 0.01) per prestige point
 */
#define PRESTIGE_MINIMUM_THRESHOLD 10000.0
#define PRESTIGE_LOG_OFFSET        3.0
#define PRESTIGE_BONUS_PER_POINT   0.01

struct _LpPrestige
{
    LrgPrestige   parent_instance;

    LpPhylactery *phylactery;
    LpPortfolio  *portfolio;
};

G_DEFINE_TYPE (LpPrestige, lp_prestige, LRG_TYPE_PRESTIGE)

/* ==========================================================================
 * LrgPrestige Virtual Method Overrides
 * ========================================================================== */

/*
 * calculate_reward:
 *
 * Calculates prestige points using the formula:
 * points = floor(log10(portfolio_value) - 3)
 *
 * So 1,000 = 0 points, 10,000 = 1 point, 100,000 = 2 points, etc.
 */
static LrgBigNumber *
lp_prestige_real_calculate_reward (LrgPrestige        *prestige,
                                   const LrgBigNumber *current_value)
{
    gdouble value;
    gdouble points;

    g_return_val_if_fail (current_value != NULL, lrg_big_number_new (0.0));

    value = lrg_big_number_to_double (current_value);

    if (value < PRESTIGE_MINIMUM_THRESHOLD)
    {
        return lrg_big_number_new (0.0);
    }

    /*
     * Formula: points = log10(value) - 3
     * This gives us:
     *   10,000 = 1 point
     *   100,000 = 2 points
     *   1,000,000 = 3 points
     */
    points = floor (log10 (value) - PRESTIGE_LOG_OFFSET);

    if (points < 0.0)
    {
        points = 0.0;
    }

    return lrg_big_number_new (points);
}

/*
 * can_prestige:
 *
 * Requires at least 10,000 gold (1 point minimum).
 */
static gboolean
lp_prestige_real_can_prestige (LrgPrestige        *prestige,
                               const LrgBigNumber *current_value)
{
    gdouble value;

    g_return_val_if_fail (current_value != NULL, FALSE);

    value = lrg_big_number_to_double (current_value);

    return (value >= PRESTIGE_MINIMUM_THRESHOLD);
}

/*
 * on_prestige:
 *
 * Called when prestige is performed. Adds points to phylactery
 * and resets the portfolio.
 */
static void
lp_prestige_real_on_prestige (LrgPrestige        *prestige,
                              const LrgBigNumber *reward)
{
    LpPrestige *self = LP_PRESTIGE (prestige);
    guint64     points;

    points = (guint64)lrg_big_number_to_double (reward);

    lp_log_info ("Prestige performed! Earned %lu phylactery points", (gulong)points);

    /* Add points to phylactery */
    if (self->phylactery != NULL)
    {
        lp_phylactery_add_points (self->phylactery, points);
    }

    /* Reset portfolio to starting gold */
    if (self->portfolio != NULL)
    {
        lp_portfolio_reset (self->portfolio, NULL);
    }
}

/*
 * get_bonus_multiplier:
 *
 * Returns the income multiplier from prestige points.
 * Formula: 1.0 + (points * 0.01)
 */
static gdouble
lp_prestige_real_get_bonus_multiplier (LrgPrestige        *prestige,
                                       const LrgBigNumber *prestige_points)
{
    gdouble points;
    gdouble multiplier;

    points = lrg_big_number_to_double (prestige_points);
    multiplier = 1.0 + (points * PRESTIGE_BONUS_PER_POINT);

    return multiplier;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_prestige_dispose (GObject *object)
{
    LpPrestige *self = LP_PRESTIGE (object);

    /*
     * We don't own these objects, so just clear the references.
     * Don't unref them.
     */
    self->phylactery = NULL;
    self->portfolio = NULL;

    G_OBJECT_CLASS (lp_prestige_parent_class)->dispose (object);
}

static void
lp_prestige_class_init (LpPrestigeClass *klass)
{
    GObjectClass     *object_class = G_OBJECT_CLASS (klass);
    LrgPrestigeClass *prestige_class = LRG_PRESTIGE_CLASS (klass);

    object_class->dispose = lp_prestige_dispose;

    /* Override virtual methods */
    prestige_class->calculate_reward = lp_prestige_real_calculate_reward;
    prestige_class->can_prestige = lp_prestige_real_can_prestige;
    prestige_class->on_prestige = lp_prestige_real_on_prestige;
    prestige_class->get_bonus_multiplier = lp_prestige_real_get_bonus_multiplier;
}

static void
lp_prestige_init (LpPrestige *self)
{
    self->phylactery = NULL;
    self->portfolio = NULL;

    /* Configure base prestige settings */
    lrg_prestige_set_id (LRG_PRESTIGE (self), "phylactery-prestige");
    lrg_prestige_set_name (LRG_PRESTIGE (self), "Phylactery Empowerment");
    lrg_prestige_set_threshold_simple (LRG_PRESTIGE (self), PRESTIGE_MINIMUM_THRESHOLD);
    lrg_prestige_set_scaling_exponent (LRG_PRESTIGE (self), 1.0);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_prestige_new:
 *
 * Creates a new prestige layer with Lich's Portfolio settings.
 *
 * Returns: (transfer full): A new #LpPrestige
 */
LpPrestige *
lp_prestige_new (void)
{
    return g_object_new (LP_TYPE_PRESTIGE, NULL);
}

/**
 * lp_prestige_set_phylactery:
 * @self: an #LpPrestige
 * @phylactery: (nullable): the phylactery to integrate with
 *
 * Sets the phylactery that receives prestige points.
 */
void
lp_prestige_set_phylactery (LpPrestige   *self,
                            LpPhylactery *phylactery)
{
    g_return_if_fail (LP_IS_PRESTIGE (self));

    self->phylactery = phylactery;
}

/**
 * lp_prestige_get_phylactery:
 * @self: an #LpPrestige
 *
 * Gets the associated phylactery.
 *
 * Returns: (transfer none) (nullable): The #LpPhylactery
 */
LpPhylactery *
lp_prestige_get_phylactery (LpPrestige *self)
{
    g_return_val_if_fail (LP_IS_PRESTIGE (self), NULL);

    return self->phylactery;
}

/**
 * lp_prestige_set_portfolio:
 * @self: an #LpPrestige
 * @portfolio: (nullable): the portfolio to reset on prestige
 *
 * Sets the portfolio that gets reset when prestige is performed.
 */
void
lp_prestige_set_portfolio (LpPrestige  *self,
                           LpPortfolio *portfolio)
{
    g_return_if_fail (LP_IS_PRESTIGE (self));

    self->portfolio = portfolio;
}

/**
 * lp_prestige_get_portfolio:
 * @self: an #LpPrestige
 *
 * Gets the associated portfolio.
 *
 * Returns: (transfer none) (nullable): The #LpPortfolio
 */
LpPortfolio *
lp_prestige_get_portfolio (LpPrestige *self)
{
    g_return_val_if_fail (LP_IS_PRESTIGE (self), NULL);

    return self->portfolio;
}

/**
 * lp_prestige_get_pending_points:
 * @self: an #LpPrestige
 * @portfolio_value: current portfolio total value
 *
 * Calculates how many points would be earned if prestige is performed now.
 *
 * Returns: Pending prestige points (may be 0)
 */
guint64
lp_prestige_get_pending_points (LpPrestige         *self,
                                const LrgBigNumber *portfolio_value)
{
    g_autoptr(LrgBigNumber) reward = NULL;

    g_return_val_if_fail (LP_IS_PRESTIGE (self), 0);
    g_return_val_if_fail (portfolio_value != NULL, 0);

    reward = lrg_prestige_calculate_reward (LRG_PRESTIGE (self), portfolio_value);

    return (guint64)lrg_big_number_to_double (reward);
}

/**
 * lp_prestige_can_perform:
 * @self: an #LpPrestige
 * @portfolio_value: current portfolio total value
 *
 * Checks if prestige requirements are met.
 *
 * Returns: %TRUE if prestige is available
 */
gboolean
lp_prestige_can_perform (LpPrestige         *self,
                         const LrgBigNumber *portfolio_value)
{
    g_return_val_if_fail (LP_IS_PRESTIGE (self), FALSE);
    g_return_val_if_fail (portfolio_value != NULL, FALSE);

    return lrg_prestige_can_prestige (LRG_PRESTIGE (self), portfolio_value);
}
