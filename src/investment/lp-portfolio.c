/* lp-portfolio.c - Investment Portfolio Container
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_INVESTMENT
#include "../lp-log.h"

#include "lp-portfolio.h"
#include "lp-investment.h"
#include "lp-investment-property.h"
#include "lp-investment-trade.h"
#include "lp-investment-financial.h"

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
    SIGNAL_INVESTMENT_ADDED,
    SIGNAL_INVESTMENT_REMOVED,
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
    guint i;

    /* Save gold as mantissa/exponent pair */
    lrg_save_context_write_double (context, "gold-mantissa",
                                   lrg_big_number_get_mantissa (self->gold));
    lrg_save_context_write_int (context, "gold-exponent",
                                lrg_big_number_get_exponent (self->gold));
    lrg_save_context_write_boolean (context, "gold-is-zero",
                                    lrg_big_number_is_zero (self->gold));

    /* Save investments */
    lrg_save_context_write_uint (context, "investment-count", self->investments->len);

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);
        g_autofree gchar *section_name = NULL;

        section_name = g_strdup_printf ("investment-%u", i);
        lrg_save_context_begin_section (context, section_name);

        if (!lrg_saveable_save (LRG_SAVEABLE (inv), context, error))
        {
            lrg_save_context_end_section (context);
            return FALSE;
        }

        lrg_save_context_end_section (context);
    }

    lp_log_debug ("Saved portfolio: %s gold, %u investments",
                  lrg_big_number_format_short (self->gold),
                  self->investments->len);

    return TRUE;
}

/*
 * Helper to create an investment from saved asset class.
 * We need to create the right subclass type before loading.
 */
static LpInvestment *
create_investment_for_class (LpAssetClass asset_class)
{
    switch (asset_class)
    {
    case LP_ASSET_CLASS_PROPERTY:
        return LP_INVESTMENT (lp_investment_property_new ("temp", "temp",
                                                          LP_PROPERTY_TYPE_AGRICULTURAL));

    case LP_ASSET_CLASS_TRADE:
        return LP_INVESTMENT (lp_investment_trade_new ("temp", "temp",
                                                       LP_TRADE_TYPE_ROUTE));

    case LP_ASSET_CLASS_FINANCIAL:
        return LP_INVESTMENT (lp_investment_financial_new ("temp", "temp",
                                                           LP_FINANCIAL_TYPE_CROWN_BOND));

    /* Future phases */
    case LP_ASSET_CLASS_MAGICAL:
    case LP_ASSET_CLASS_POLITICAL:
    case LP_ASSET_CLASS_DARK:
    default:
        lp_log_warning ("Unknown asset class %d, creating generic property", asset_class);
        return LP_INVESTMENT (lp_investment_property_new ("temp", "temp",
                                                          LP_PROPERTY_TYPE_AGRICULTURAL));
    }
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
    guint64 inv_count;
    guint i;

    /* Load gold */
    mantissa = lrg_save_context_read_double (context, "gold-mantissa", 1.0);
    exponent = lrg_save_context_read_int (context, "gold-exponent", 3);
    is_zero = lrg_save_context_read_boolean (context, "gold-is-zero", FALSE);

    g_clear_pointer (&self->gold, lrg_big_number_free);

    if (is_zero)
        self->gold = lrg_big_number_new_zero ();
    else
        self->gold = lrg_big_number_new_from_parts (mantissa, exponent);

    /* Clear existing investments before loading */
    g_ptr_array_set_size (self->investments, 0);

    /* Load investments */
    inv_count = lrg_save_context_read_uint (context, "investment-count", 0);

    for (i = 0; i < inv_count; i++)
    {
        g_autofree gchar *section_name = NULL;
        LpInvestment *inv;
        LpAssetClass asset_class;

        section_name = g_strdup_printf ("investment-%u", i);

        if (!lrg_save_context_enter_section (context, section_name))
        {
            lp_log_warning ("Missing investment section: %s", section_name);
            continue;
        }

        /* Read asset class first to create the right type */
        asset_class = (LpAssetClass) lrg_save_context_read_int (context, "asset-class",
                                                                 LP_ASSET_CLASS_PROPERTY);

        inv = create_investment_for_class (asset_class);

        if (!lrg_saveable_load (LRG_SAVEABLE (inv), context, error))
        {
            g_object_unref (inv);
            lrg_save_context_leave_section (context);
            return FALSE;
        }

        g_ptr_array_add (self->investments, inv);
        lrg_save_context_leave_section (context);
    }

    lp_log_debug ("Loaded portfolio: %s gold, %u investments",
                  lrg_big_number_format_short (self->gold),
                  self->investments->len);

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

    /**
     * LpPortfolio::investment-added:
     * @self: the #LpPortfolio
     * @investment: (transfer none): The investment that was added
     *
     * Emitted when an investment is added to the portfolio.
     */
    signals[SIGNAL_INVESTMENT_ADDED] =
        g_signal_new ("investment-added",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      LP_TYPE_INVESTMENT);

    /**
     * LpPortfolio::investment-removed:
     * @self: the #LpPortfolio
     * @investment: (transfer none): The investment that was removed
     *
     * Emitted when an investment is removed from the portfolio.
     */
    signals[SIGNAL_INVESTMENT_REMOVED] =
        g_signal_new ("investment-removed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      LP_TYPE_INVESTMENT);
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
 * Investment Management
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
 * lp_portfolio_add_investment:
 * @self: an #LpPortfolio
 * @investment: (transfer full): Investment to add
 *
 * Adds an investment to the portfolio. The portfolio takes ownership.
 */
void
lp_portfolio_add_investment (LpPortfolio  *self,
                             LpInvestment *investment)
{
    g_return_if_fail (LP_IS_PORTFOLIO (self));
    g_return_if_fail (LP_IS_INVESTMENT (investment));

    g_ptr_array_add (self->investments, investment);

    lp_log_debug ("Added investment: %s (%s)",
                  lp_investment_get_name (investment),
                  lp_investment_get_id (investment));

    g_signal_emit (self, signals[SIGNAL_INVESTMENT_ADDED], 0, investment);
}

/**
 * lp_portfolio_remove_investment:
 * @self: an #LpPortfolio
 * @investment: Investment to remove
 *
 * Removes an investment from the portfolio.
 *
 * Returns: %TRUE if the investment was found and removed
 */
gboolean
lp_portfolio_remove_investment (LpPortfolio  *self,
                                LpInvestment *investment)
{
    guint i;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), FALSE);
    g_return_val_if_fail (LP_IS_INVESTMENT (investment), FALSE);

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);

        if (inv == investment)
        {
            /* Emit signal before removing (so handlers can access it) */
            g_signal_emit (self, signals[SIGNAL_INVESTMENT_REMOVED], 0, investment);

            lp_log_debug ("Removed investment: %s (%s)",
                          lp_investment_get_name (investment),
                          lp_investment_get_id (investment));

            /* Remove with index to avoid O(n) search in g_ptr_array_remove */
            g_ptr_array_remove_index (self->investments, i);
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * lp_portfolio_remove_investment_by_id:
 * @self: an #LpPortfolio
 * @investment_id: ID of investment to remove
 *
 * Removes an investment by its ID.
 *
 * Returns: %TRUE if the investment was found and removed
 */
gboolean
lp_portfolio_remove_investment_by_id (LpPortfolio *self,
                                      const gchar *investment_id)
{
    LpInvestment *inv;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), FALSE);
    g_return_val_if_fail (investment_id != NULL, FALSE);

    inv = lp_portfolio_get_investment_by_id (self, investment_id);
    if (inv == NULL)
        return FALSE;

    return lp_portfolio_remove_investment (self, inv);
}

/**
 * lp_portfolio_get_investment_by_id:
 * @self: an #LpPortfolio
 * @investment_id: ID to search for
 *
 * Finds an investment by its ID.
 *
 * Returns: (transfer none) (nullable): The investment, or %NULL if not found
 */
LpInvestment *
lp_portfolio_get_investment_by_id (LpPortfolio *self,
                                   const gchar *investment_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);
    g_return_val_if_fail (investment_id != NULL, NULL);

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);
        const gchar *inv_id = lp_investment_get_id (inv);

        if (g_strcmp0 (inv_id, investment_id) == 0)
            return inv;
    }

    return NULL;
}

/**
 * lp_portfolio_get_investments_by_class:
 * @self: an #LpPortfolio
 * @asset_class: The #LpAssetClass to filter by
 *
 * Gets all investments of a specific asset class.
 *
 * Returns: (transfer container) (element-type LpInvestment): Array of matching investments
 */
GPtrArray *
lp_portfolio_get_investments_by_class (LpPortfolio  *self,
                                       LpAssetClass  asset_class)
{
    GPtrArray *result;
    guint i;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    result = g_ptr_array_new ();

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);

        if (lp_investment_get_asset_class (inv) == asset_class)
            g_ptr_array_add (result, inv);
    }

    return result;
}

/**
 * lp_portfolio_get_investments_by_risk:
 * @self: an #LpPortfolio
 * @risk_level: The #LpRiskLevel to filter by
 *
 * Gets all investments of a specific risk level.
 *
 * Returns: (transfer container) (element-type LpInvestment): Array of matching investments
 */
GPtrArray *
lp_portfolio_get_investments_by_risk (LpPortfolio *self,
                                      LpRiskLevel  risk_level)
{
    GPtrArray *result;
    guint i;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    result = g_ptr_array_new ();

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);

        if (lp_investment_get_risk_level (inv) == risk_level)
            g_ptr_array_add (result, inv);
    }

    return result;
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
    g_autoptr(LrgBigNumber) inv_value = NULL;
    g_autoptr(LrgBigNumber) total = NULL;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    inv_value = lp_portfolio_get_investment_value (self);
    total = lrg_big_number_add (self->gold, inv_value);

    return g_steal_pointer (&total);
}

/**
 * lp_portfolio_get_investment_value:
 * @self: an #LpPortfolio
 *
 * Gets the total value of investments only (excluding gold).
 *
 * Returns: (transfer full): Total investment value
 */
LrgBigNumber *
lp_portfolio_get_investment_value (LpPortfolio *self)
{
    g_autoptr(LrgBigNumber) total = NULL;
    guint i;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    total = lrg_big_number_new_zero ();

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);
        LrgBigNumber *value = lp_investment_get_current_value (inv);

        if (value != NULL)
        {
            g_autoptr(LrgBigNumber) new_total = lrg_big_number_add (total, value);
            lrg_big_number_free (total);
            total = g_steal_pointer (&new_total);
        }
    }

    return g_steal_pointer (&total);
}

/**
 * lp_portfolio_calculate_income:
 * @self: an #LpPortfolio
 * @years: Number of years to calculate
 *
 * Calculates the expected income from all investments over the
 * specified number of years. Does not modify the portfolio.
 *
 * Returns: (transfer full): Expected income as #LrgBigNumber
 */
LrgBigNumber *
lp_portfolio_calculate_income (LpPortfolio *self,
                               guint        years)
{
    g_autoptr(LrgBigNumber) total_income = NULL;
    guint i;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    if (years == 0)
        return lrg_big_number_new_zero ();

    total_income = lrg_big_number_new_zero ();

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);
        g_autoptr(LrgBigNumber) returns = NULL;
        LrgBigNumber *current_value;
        g_autoptr(LrgBigNumber) income = NULL;
        g_autoptr(LrgBigNumber) new_total = NULL;

        current_value = lp_investment_get_current_value (inv);
        if (current_value == NULL)
            continue;

        /* Calculate returns (new value after years) */
        returns = lp_investment_calculate_returns (inv, years);
        if (returns == NULL)
            continue;

        /* Income = returns - current value */
        income = lrg_big_number_subtract (returns, current_value);

        /* Only count positive income */
        if (!lrg_big_number_is_negative (income))
        {
            new_total = lrg_big_number_add (total_income, income);
            lrg_big_number_free (total_income);
            total_income = g_steal_pointer (&new_total);
        }
    }

    lp_log_debug ("Calculated income for %u years: %s",
                  years, lrg_big_number_format_short (total_income));

    return g_steal_pointer (&total_income);
}

/**
 * lp_portfolio_apply_slumber:
 * @self: an #LpPortfolio
 * @years: Number of years slumbered
 *
 * Applies the effects of slumber to all investments.
 * Updates investment values and adds income to gold.
 *
 * Returns: (transfer full): Total income earned during slumber
 */
LrgBigNumber *
lp_portfolio_apply_slumber (LpPortfolio *self,
                            guint        years)
{
    g_autoptr(LrgBigNumber) total_income = NULL;
    guint i;

    g_return_val_if_fail (LP_IS_PORTFOLIO (self), NULL);

    if (years == 0)
        return lrg_big_number_new_zero ();

    total_income = lrg_big_number_new_zero ();

    lp_log_debug ("Applying slumber for %u years to %u investments",
                  years, self->investments->len);

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);
        g_autoptr(LrgBigNumber) returns = NULL;
        LrgBigNumber *current_value;
        g_autoptr(LrgBigNumber) income = NULL;
        g_autoptr(LrgBigNumber) new_total = NULL;

        current_value = lp_investment_get_current_value (inv);
        if (current_value == NULL)
            continue;

        /* Calculate and apply returns */
        returns = lp_investment_calculate_returns (inv, years);
        if (returns == NULL)
            continue;

        /* Income = returns - current value */
        income = lrg_big_number_subtract (returns, current_value);

        /* Update investment to new value */
        lp_investment_set_current_value (inv, lrg_big_number_copy (returns));

        /* Accumulate positive income */
        if (!lrg_big_number_is_negative (income))
        {
            new_total = lrg_big_number_add (total_income, income);
            lrg_big_number_free (total_income);
            total_income = g_steal_pointer (&new_total);
        }
    }

    /* Add income to gold */
    lp_portfolio_add_gold (self, total_income);

    lp_log_debug ("Slumber complete: earned %s gold",
                  lrg_big_number_format_short (total_income));

    return g_steal_pointer (&total_income);
}

/**
 * lp_portfolio_apply_event:
 * @self: an #LpPortfolio
 * @event: The #LpEvent to apply
 *
 * Applies an event to all investments in the portfolio.
 */
void
lp_portfolio_apply_event (LpPortfolio *self,
                          LpEvent     *event)
{
    guint i;

    g_return_if_fail (LP_IS_PORTFOLIO (self));
    g_return_if_fail (event != NULL);

    lp_log_debug ("Applying event to %u investments", self->investments->len);

    for (i = 0; i < self->investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (self->investments, i);
        lp_investment_apply_event (inv, event);
    }
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
