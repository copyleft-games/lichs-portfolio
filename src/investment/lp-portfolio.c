/* lp-portfolio.c - Investment Portfolio Container
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_INVESTMENT
#include "../lp-log.h"

#include "lp-portfolio.h"

/* Default starting gold (1000) */
#define DEFAULT_STARTING_GOLD (1000.0)

struct _LpPortfolio
{
    GObject parent_instance;

    LrgBigNumber *gold;         /* Current gold on hand */
    GPtrArray    *investments;  /* Array of LpInvestment (empty in Phase 1) */
};

enum
{
    PROP_0,
    PROP_GOLD,
    N_PROPS
};

enum
{
    SIGNAL_GOLD_CHANGED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_portfolio_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpPortfolio, lp_portfolio, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_portfolio_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_portfolio_get_save_id (LrgSaveable *saveable)
{
    return "portfolio";
}

static gboolean
lp_portfolio_save (LrgSaveable    *saveable,
                   LrgSaveContext *context,
                   GError        **error)
{
    LpPortfolio *self = LP_PORTFOLIO (saveable);

    /* Save gold as mantissa/exponent pair */
    lrg_save_context_write_double (context, "gold-mantissa",
                                   lrg_big_number_get_mantissa (self->gold));
    lrg_save_context_write_int (context, "gold-exponent",
                                lrg_big_number_get_exponent (self->gold));
    lrg_save_context_write_boolean (context, "gold-is-zero",
                                    lrg_big_number_is_zero (self->gold));

    /* Phase 2+: Save investments here */
    lrg_save_context_write_uint (context, "investment-count", 0);

    return TRUE;
}

static gboolean
lp_portfolio_load (LrgSaveable    *saveable,
                   LrgSaveContext *context,
                   GError        **error)
{
    LpPortfolio *self = LP_PORTFOLIO (saveable);
    gdouble mantissa;
    gint64 exponent;
    gboolean is_zero;

    /* Load gold */
    mantissa = lrg_save_context_read_double (context, "gold-mantissa", 1.0);
    exponent = lrg_save_context_read_int (context, "gold-exponent", 3);
    is_zero = lrg_save_context_read_boolean (context, "gold-is-zero", FALSE);

    g_clear_pointer (&self->gold, lrg_big_number_free);

    if (is_zero)
        self->gold = lrg_big_number_new_zero ();
    else
        self->gold = lrg_big_number_new_from_parts (mantissa, exponent);

    lp_log_debug ("Loaded portfolio with gold: %s",
                  lrg_big_number_format_short (self->gold));

    /* Phase 2+: Load investments here */

    return TRUE;
}

static void
lp_portfolio_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_portfolio_get_save_id;
    iface->save = lp_portfolio_save;
    iface->load = lp_portfolio_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_portfolio_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    LpPortfolio *self = LP_PORTFOLIO (object);

    switch (prop_id)
    {
    case PROP_GOLD:
        g_value_set_boxed (value, self->gold);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_portfolio_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    LpPortfolio *self = LP_PORTFOLIO (object);

    switch (prop_id)
    {
    case PROP_GOLD:
        lp_portfolio_set_gold (self, g_value_dup_boxed (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_portfolio_finalize (GObject *object)
{
    LpPortfolio *self = LP_PORTFOLIO (object);

    lp_log_debug ("Finalizing portfolio");

    g_clear_pointer (&self->gold, lrg_big_number_free);
    g_clear_pointer (&self->investments, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_portfolio_parent_class)->finalize (object);
}

static void
lp_portfolio_class_init (LpPortfolioClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_portfolio_get_property;
    object_class->set_property = lp_portfolio_set_property;
    object_class->finalize = lp_portfolio_finalize;

    /**
     * LpPortfolio:gold:
     *
     * The current gold amount.
     */
    properties[PROP_GOLD] =
        g_param_spec_boxed ("gold",
                            "Gold",
                            "Current gold amount",
                            LRG_TYPE_BIG_NUMBER,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpPortfolio::gold-changed:
     * @self: the #LpPortfolio
     * @old_gold: (transfer none): Previous gold amount
     * @new_gold: (transfer none): New gold amount
     *
     * Emitted when the gold amount changes.
     */
    signals[SIGNAL_GOLD_CHANGED] =
        g_signal_new ("gold-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      LRG_TYPE_BIG_NUMBER,
                      LRG_TYPE_BIG_NUMBER);
}

static void
lp_portfolio_init (LpPortfolio *self)
{
    self->gold = lrg_big_number_new (DEFAULT_STARTING_GOLD);
    self->investments = g_ptr_array_new_with_free_func (g_object_unref);
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_portfolio_new:
 *
 * Creates a new portfolio with default starting gold.
 *
 * Returns: (transfer full): A new #LpPortfolio
 */
LpPortfolio *
lp_portfolio_new (void)
{
    return g_object_new (LP_TYPE_PORTFOLIO, NULL);
}

/**
 * lp_portfolio_new_with_gold:
 * @initial_gold: (transfer full): Initial gold amount
 *
 * Creates a new portfolio with specified starting gold.
 *
 * Returns: (transfer full): A new #LpPortfolio
 */
LpPortfolio *
lp_portfolio_new_with_gold (LrgBigNumber *initial_gold)
{
    LpPortfolio *self = g_object_new (LP_TYPE_PORTFOLIO, NULL);

    if (initial_gold != NULL)
    {
        g_clear_pointer (&self->gold, lrg_big_number_free);
        self->gold = initial_gold;
    }

    return self;
}

/* ==========================================================================
 * Gold Management
 * ========================================================================== */

/**
 * lp_portfolio_get_gold:
 * @self: an #LpPortfolio
 *
 * Gets the current gold amount.
 *
 * Returns: (transfer none): The current gold
 */
LrgBigNumber *
lp_portfolio_get_gold (LpPortfolio *self)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    return self->gold;
}

/**
 * lp_portfolio_set_gold:
 * @self: an #LpPortfolio
 * @gold: (transfer full): The new gold amount
 *
 * Sets the gold amount directly.
 */
void
lp_portfolio_set_gold (LpPortfolio  *self,
                       LrgBigNumber *gold)
{
    g_autoptr(LrgBigNumber) old_gold = NULL;

    g_return_if_fail (LP_IS_PORTFOLIO (self));
    g_return_if_fail (gold != NULL);

    old_gold = self->gold;
    self->gold = gold;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_GOLD]);
    g_signal_emit (self, signals[SIGNAL_GOLD_CHANGED], 0, old_gold, self->gold);
}

/**
 * lp_portfolio_add_gold:
 * @self: an #LpPortfolio
 * @amount: (transfer none): Amount to add
 *
 * Adds gold to the portfolio.
 */
void
lp_portfolio_add_gold (LpPortfolio        *self,
                       const LrgBigNumber *amount)
{
    g_autoptr(LrgBigNumber) new_gold = NULL;
    g_autoptr(LrgBigNumber) old_gold = NULL;

    g_return_if_fail (LP_IS_PORTFOLIO (self));
    g_return_if_fail (amount != NULL);

    old_gold = lrg_big_number_copy (self->gold);
    new_gold = lrg_big_number_add (self->gold, amount);

    g_clear_pointer (&self->gold, lrg_big_number_free);
    self->gold = g_steal_pointer (&new_gold);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_GOLD]);
    g_signal_emit (self, signals[SIGNAL_GOLD_CHANGED], 0, old_gold, self->gold);
}

/**
 * lp_portfolio_subtract_gold:
 * @self: an #LpPortfolio
 * @amount: (transfer none): Amount to subtract
 *
 * Subtracts gold from the portfolio.
 *
 * Returns: %TRUE if enough gold was available
 */
gboolean
lp_portfolio_subtract_gold (LpPortfolio        *self,
                            const LrgBigNumber *amount)
{
    g_autoptr(LrgBigNumber) new_gold = NULL;
    g_autoptr(LrgBigNumber) old_gold = NULL;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), FALSE);
    g_return_val_if_fail (amount != NULL, FALSE);

    if (!lp_portfolio_can_afford (self, amount))
        return FALSE;

    old_gold = lrg_big_number_copy (self->gold);
    new_gold = lrg_big_number_subtract (self->gold, amount);

    /* Ensure we don't go negative */
    if (lrg_big_number_is_negative (new_gold))
    {
        lrg_big_number_free (new_gold);
        new_gold = lrg_big_number_new_zero ();
    }

    g_clear_pointer (&self->gold, lrg_big_number_free);
    self->gold = g_steal_pointer (&new_gold);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_GOLD]);
    g_signal_emit (self, signals[SIGNAL_GOLD_CHANGED], 0, old_gold, self->gold);

    return TRUE;
}

/**
 * lp_portfolio_can_afford:
 * @self: an #LpPortfolio
 * @cost: (transfer none): Cost to check
 *
 * Checks if the portfolio has enough gold.
 *
 * Returns: %TRUE if gold >= cost
 */
gboolean
lp_portfolio_can_afford (LpPortfolio        *self,
                         const LrgBigNumber *cost)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO (self), FALSE);
    g_return_val_if_fail (cost != NULL, FALSE);

    return lrg_big_number_compare (self->gold, cost) >= 0;
}

/* ==========================================================================
 * Investment Management (Skeleton)
 * ========================================================================== */

/**
 * lp_portfolio_get_investments:
 * @self: an #LpPortfolio
 *
 * Gets the list of investments.
 *
 * Returns: (transfer none) (element-type LpInvestment): Array of investments
 */
GPtrArray *
lp_portfolio_get_investments (LpPortfolio *self)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    return self->investments;
}

/**
 * lp_portfolio_get_investment_count:
 * @self: an #LpPortfolio
 *
 * Gets the number of investments.
 *
 * Returns: Number of investments
 */
guint
lp_portfolio_get_investment_count (LpPortfolio *self)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO (self), 0);

    return self->investments->len;
}

/**
 * lp_portfolio_get_total_value:
 * @self: an #LpPortfolio
 *
 * Gets the total value of all investments plus gold.
 *
 * Returns: (transfer full): Total portfolio value
 */
LrgBigNumber *
lp_portfolio_get_total_value (LpPortfolio *self)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    /*
     * Phase 1 skeleton: Just return gold amount.
     * Phase 2+: Sum gold + all investment values.
     */
    return lrg_big_number_copy (self->gold);
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_portfolio_reset:
 * @self: an #LpPortfolio
 * @starting_gold: (transfer full) (nullable): New starting gold
 *
 * Resets the portfolio to initial state.
 */
void
lp_portfolio_reset (LpPortfolio  *self,
                    LrgBigNumber *starting_gold)
{
    g_return_if_fail (LP_IS_PORTFOLIO (self));

    lp_log_debug ("Resetting portfolio");

    /* Clear investments */
    g_ptr_array_set_size (self->investments, 0);

    /* Reset gold */
    g_clear_pointer (&self->gold, lrg_big_number_free);

    if (starting_gold != NULL)
        self->gold = starting_gold;
    else
        self->gold = lrg_big_number_new (DEFAULT_STARTING_GOLD);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_GOLD]);
}
