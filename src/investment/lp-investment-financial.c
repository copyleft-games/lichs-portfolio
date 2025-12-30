/* lp-investment-financial.c - Financial Investment
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_INVESTMENT
#include "../lp-log.h"

#include "lp-investment-financial.h"

/* Base return rates by financial type */
#define CROWN_BOND_RETURN    (0.04)   /* 4% - safest */
#define NOBLE_DEBT_RETURN    (0.06)   /* 6% - moderate risk */
#define MERCHANT_NOTE_RETURN (0.07)   /* 7% - commercial risk */
#define INSURANCE_RETURN     (0.05)   /* 5% - premium income */
#define USURY_RETURN         (0.12)   /* 12% - high risk high reward */

/* Default recovery rates */
#define CROWN_BOND_RECOVERY    (0.50)  /* 50% - kingdoms usually pay eventually */
#define NOBLE_DEBT_RECOVERY    (0.30)  /* 30% - nobles may lose lands */
#define MERCHANT_NOTE_RECOVERY (0.20)  /* 20% - merchants may go bankrupt */
#define INSURANCE_RECOVERY     (0.00)  /* 0% - insurance pools just fail */
#define USURY_RECOVERY         (0.10)  /* 10% - high risk means low recovery */

struct _LpInvestmentFinancial
{
    LpInvestment parent_instance;

    LpFinancialType financial_type;
    LpDebtStatus    debt_status;
    gdouble         interest_rate;
    guint64         maturity_year;

    LrgBigNumber   *face_value;
    gchar          *issuer_id;
};

enum
{
    PROP_0,
    PROP_FINANCIAL_TYPE,
    PROP_DEBT_STATUS,
    PROP_INTEREST_RATE,
    PROP_FACE_VALUE,
    PROP_MATURITY_YEAR,
    PROP_ISSUER_ID,
    N_PROPS
};

enum
{
    SIGNAL_DEBT_STATUS_CHANGED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpInvestmentFinancial, lp_investment_financial, LP_TYPE_INVESTMENT)

/* GType registration for enums */
GType
lp_financial_type_get_type (void)
{
    static gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_FINANCIAL_TYPE_CROWN_BOND, "LP_FINANCIAL_TYPE_CROWN_BOND", "crown-bond" },
            { LP_FINANCIAL_TYPE_NOBLE_DEBT, "LP_FINANCIAL_TYPE_NOBLE_DEBT", "noble-debt" },
            { LP_FINANCIAL_TYPE_MERCHANT_NOTE, "LP_FINANCIAL_TYPE_MERCHANT_NOTE", "merchant-note" },
            { LP_FINANCIAL_TYPE_INSURANCE, "LP_FINANCIAL_TYPE_INSURANCE", "insurance" },
            { LP_FINANCIAL_TYPE_USURY, "LP_FINANCIAL_TYPE_USURY", "usury" },
            { 0, NULL, NULL }
        };

        GType type_id = g_enum_register_static ("LpFinancialType", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

GType
lp_debt_status_get_type (void)
{
    static gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_DEBT_STATUS_PERFORMING, "LP_DEBT_STATUS_PERFORMING", "performing" },
            { LP_DEBT_STATUS_DELINQUENT, "LP_DEBT_STATUS_DELINQUENT", "delinquent" },
            { LP_DEBT_STATUS_DEFAULT, "LP_DEBT_STATUS_DEFAULT", "default" },
            { 0, NULL, NULL }
        };

        GType type_id = g_enum_register_static ("LpDebtStatus", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * Virtual Method Overrides
 * ========================================================================== */

/*
 * Financial returns are fixed interest unless defaulted.
 * Defaulted instruments return nothing and lose value.
 */
static LrgBigNumber *
lp_investment_financial_calculate_returns (LpInvestment *investment,
                                           guint         years)
{
    LpInvestmentFinancial *self = LP_INVESTMENT_FINANCIAL (investment);
    g_autoptr(LrgBigNumber) result = NULL;
    g_autoptr(LrgBigNumber) annual_interest = NULL;
    gdouble recovery;
    guint i;

    /* If defaulted, apply recovery rate */
    if (self->debt_status == LP_DEBT_STATUS_DEFAULT)
    {
        recovery = lp_investment_financial_get_default_recovery_rate (self);

        if (self->face_value != NULL)
        {
            g_autoptr(LrgBigNumber) recovery_mult = lrg_big_number_new (recovery);
            result = lrg_big_number_multiply (self->face_value, recovery_mult);
        }
        else
        {
            result = lrg_big_number_new_zero ();
        }

        lp_log_debug ("Financial %s: defaulted, recovery rate %.0f%%",
                      lp_investment_get_name (investment),
                      recovery * 100.0);

        return g_steal_pointer (&result);
    }

    /* Normal case: face value + accumulated interest */
    if (self->face_value == NULL)
        return lrg_big_number_new_zero ();

    result = lrg_big_number_copy (self->face_value);
    annual_interest = lp_investment_financial_calculate_interest_payment (self);

    /* Add interest for each year (simple interest for bonds) */
    for (i = 0; i < years; i++)
    {
        g_autoptr(LrgBigNumber) new_result = NULL;

        /* Delinquent pays half interest */
        if (self->debt_status == LP_DEBT_STATUS_DELINQUENT)
        {
            g_autoptr(LrgBigNumber) half_interest = NULL;
            g_autoptr(LrgBigNumber) half = lrg_big_number_new (0.5);

            half_interest = lrg_big_number_multiply (annual_interest, half);
            new_result = lrg_big_number_add (result, half_interest);
        }
        else
        {
            new_result = lrg_big_number_add (result, annual_interest);
        }

        lrg_big_number_free (result);
        result = g_steal_pointer (&new_result);
    }

    lp_log_debug ("Financial %s returns over %u years: %s (%.2f%% rate, status: %d)",
                  lp_investment_get_name (investment),
                  years,
                  lrg_big_number_format_short (result),
                  self->interest_rate * 100.0,
                  self->debt_status);

    return g_steal_pointer (&result);
}

/*
 * Financial instruments are vulnerable to economic and political events
 * that affect the issuer's ability to pay.
 */
static void
lp_investment_financial_apply_event (LpInvestment *investment,
                                     LpEvent      *event)
{
    LpInvestmentFinancial *self = LP_INVESTMENT_FINANCIAL (investment);

    (void)event;  /* Will be used in future phases */

    lp_log_debug ("Financial %s: event applied (status: %d, issuer: %s)",
                  lp_investment_get_name (investment),
                  self->debt_status,
                  self->issuer_id ? self->issuer_id : "(none)");
}

/*
 * Defaulted instruments can still be sold (at recovery value).
 */
static gboolean
lp_investment_financial_can_sell (LpInvestment *investment)
{
    (void)investment;
    return TRUE;
}

/*
 * Risk varies by financial type and current status.
 */
static gdouble
lp_investment_financial_get_risk_modifier (LpInvestment *investment)
{
    LpInvestmentFinancial *self = LP_INVESTMENT_FINANCIAL (investment);
    gdouble base_risk = 1.0;

    /* Type-based risk */
    switch (self->financial_type)
    {
    case LP_FINANCIAL_TYPE_CROWN_BOND:
        base_risk = 0.8;  /* Lower risk */
        break;
    case LP_FINANCIAL_TYPE_NOBLE_DEBT:
        base_risk = 1.0;
        break;
    case LP_FINANCIAL_TYPE_MERCHANT_NOTE:
        base_risk = 1.2;
        break;
    case LP_FINANCIAL_TYPE_INSURANCE:
        base_risk = 1.0;
        break;
    case LP_FINANCIAL_TYPE_USURY:
        base_risk = 1.5;  /* Higher risk */
        break;
    default:
        base_risk = 1.0;
        break;
    }

    /* Status-based modifier */
    if (self->debt_status == LP_DEBT_STATUS_DELINQUENT)
        base_risk *= 1.5;
    else if (self->debt_status == LP_DEBT_STATUS_DEFAULT)
        base_risk *= 2.0;

    return base_risk;
}

/*
 * Financial instruments use their fixed interest rate.
 */
static gdouble
lp_investment_financial_get_base_return_rate (LpInvestment *investment)
{
    LpInvestmentFinancial *self = LP_INVESTMENT_FINANCIAL (investment);

    /* Use the fixed interest rate if set */
    if (self->interest_rate > 0.0)
        return self->interest_rate;

    /* Otherwise use type defaults */
    switch (self->financial_type)
    {
    case LP_FINANCIAL_TYPE_CROWN_BOND:
        return CROWN_BOND_RETURN;
    case LP_FINANCIAL_TYPE_NOBLE_DEBT:
        return NOBLE_DEBT_RETURN;
    case LP_FINANCIAL_TYPE_MERCHANT_NOTE:
        return MERCHANT_NOTE_RETURN;
    case LP_FINANCIAL_TYPE_INSURANCE:
        return INSURANCE_RETURN;
    case LP_FINANCIAL_TYPE_USURY:
        return USURY_RETURN;
    default:
        return 0.05;  /* Default 5% */
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_investment_financial_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
    LpInvestmentFinancial *self = LP_INVESTMENT_FINANCIAL (object);

    switch (prop_id)
    {
    case PROP_FINANCIAL_TYPE:
        g_value_set_enum (value, self->financial_type);
        break;

    case PROP_DEBT_STATUS:
        g_value_set_enum (value, self->debt_status);
        break;

    case PROP_INTEREST_RATE:
        g_value_set_double (value, self->interest_rate);
        break;

    case PROP_FACE_VALUE:
        g_value_set_boxed (value, self->face_value);
        break;

    case PROP_MATURITY_YEAR:
        g_value_set_uint64 (value, self->maturity_year);
        break;

    case PROP_ISSUER_ID:
        g_value_set_string (value, self->issuer_id);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_financial_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
    LpInvestmentFinancial *self = LP_INVESTMENT_FINANCIAL (object);

    switch (prop_id)
    {
    case PROP_FINANCIAL_TYPE:
        self->financial_type = g_value_get_enum (value);
        break;

    case PROP_DEBT_STATUS:
        lp_investment_financial_set_debt_status (self, g_value_get_enum (value));
        break;

    case PROP_INTEREST_RATE:
        lp_investment_financial_set_interest_rate (self, g_value_get_double (value));
        break;

    case PROP_FACE_VALUE:
        lp_investment_financial_set_face_value (self, g_value_dup_boxed (value));
        break;

    case PROP_MATURITY_YEAR:
        self->maturity_year = g_value_get_uint64 (value);
        break;

    case PROP_ISSUER_ID:
        lp_investment_financial_set_issuer_id (self, g_value_get_string (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_financial_finalize (GObject *object)
{
    LpInvestmentFinancial *self = LP_INVESTMENT_FINANCIAL (object);

    g_clear_pointer (&self->face_value, lrg_big_number_free);
    g_clear_pointer (&self->issuer_id, g_free);

    G_OBJECT_CLASS (lp_investment_financial_parent_class)->finalize (object);
}

static void
lp_investment_financial_class_init (LpInvestmentFinancialClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpInvestmentClass *investment_class = LP_INVESTMENT_CLASS (klass);

    object_class->get_property = lp_investment_financial_get_property;
    object_class->set_property = lp_investment_financial_set_property;
    object_class->finalize = lp_investment_financial_finalize;

    /* Override virtual methods */
    investment_class->calculate_returns = lp_investment_financial_calculate_returns;
    investment_class->apply_event = lp_investment_financial_apply_event;
    investment_class->can_sell = lp_investment_financial_can_sell;
    investment_class->get_risk_modifier = lp_investment_financial_get_risk_modifier;
    investment_class->get_base_return_rate = lp_investment_financial_get_base_return_rate;

    /**
     * LpInvestmentFinancial:financial-type:
     *
     * The subtype of financial investment.
     */
    properties[PROP_FINANCIAL_TYPE] =
        g_param_spec_enum ("financial-type",
                           "Financial Type",
                           "Type of financial instrument",
                           LP_TYPE_FINANCIAL_TYPE,
                           LP_FINANCIAL_TYPE_CROWN_BOND,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentFinancial:debt-status:
     *
     * Current status of the debt.
     */
    properties[PROP_DEBT_STATUS] =
        g_param_spec_enum ("debt-status",
                           "Debt Status",
                           "Current debt status",
                           LP_TYPE_DEBT_STATUS,
                           LP_DEBT_STATUS_PERFORMING,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentFinancial:interest-rate:
     *
     * Fixed interest rate.
     */
    properties[PROP_INTEREST_RATE] =
        g_param_spec_double ("interest-rate",
                             "Interest Rate",
                             "Annual interest rate",
                             0.0, 1.0, 0.05,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentFinancial:face-value:
     *
     * Face (principal) value of the instrument.
     */
    properties[PROP_FACE_VALUE] =
        g_param_spec_boxed ("face-value",
                            "Face Value",
                            "Principal value",
                            LRG_TYPE_BIG_NUMBER,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentFinancial:maturity-year:
     *
     * Year when instrument matures.
     */
    properties[PROP_MATURITY_YEAR] =
        g_param_spec_uint64 ("maturity-year",
                             "Maturity Year",
                             "Year when instrument matures",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentFinancial:issuer-id:
     *
     * ID of the issuing entity.
     */
    properties[PROP_ISSUER_ID] =
        g_param_spec_string ("issuer-id",
                             "Issuer ID",
                             "Entity that issued this instrument",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpInvestmentFinancial::debt-status-changed:
     * @self: the #LpInvestmentFinancial
     * @old_status: previous #LpDebtStatus
     * @new_status: new #LpDebtStatus
     *
     * Emitted when the debt status changes.
     */
    signals[SIGNAL_DEBT_STATUS_CHANGED] =
        g_signal_new ("debt-status-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      LP_TYPE_DEBT_STATUS,
                      LP_TYPE_DEBT_STATUS);
}

static void
lp_investment_financial_init (LpInvestmentFinancial *self)
{
    /* Set base class properties - risk varies by type */
    lp_investment_set_risk_level (LP_INVESTMENT (self), LP_RISK_LEVEL_MEDIUM);

    /* Initialize financial-specific fields */
    self->financial_type = LP_FINANCIAL_TYPE_CROWN_BOND;
    self->debt_status = LP_DEBT_STATUS_PERFORMING;
    self->interest_rate = 0.05;  /* Default 5% */
    self->face_value = lrg_big_number_new (1000.0);
    self->maturity_year = 0;
    self->issuer_id = NULL;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpInvestmentFinancial *
lp_investment_financial_new (const gchar     *id,
                             const gchar     *name,
                             LpFinancialType  financial_type)
{
    return g_object_new (LP_TYPE_INVESTMENT_FINANCIAL,
                         "id", id,
                         "name", name,
                         "asset-class", LP_ASSET_CLASS_FINANCIAL,
                         "financial-type", financial_type,
                         NULL);
}

LpInvestmentFinancial *
lp_investment_financial_new_with_value (const gchar     *id,
                                        const gchar     *name,
                                        LpFinancialType  financial_type,
                                        LrgBigNumber    *face_value,
                                        gdouble          interest_rate)
{
    LpInvestmentFinancial *self;

    self = g_object_new (LP_TYPE_INVESTMENT_FINANCIAL,
                         "id", id,
                         "name", name,
                         "asset-class", LP_ASSET_CLASS_FINANCIAL,
                         "financial-type", financial_type,
                         "interest-rate", interest_rate,
                         NULL);

    if (face_value != NULL)
    {
        lp_investment_financial_set_face_value (self, lrg_big_number_copy (face_value));
        lp_investment_set_purchase_price (LP_INVESTMENT (self),
                                          lrg_big_number_copy (face_value));
        lp_investment_set_current_value (LP_INVESTMENT (self), face_value);
    }

    return self;
}

/* ==========================================================================
 * Financial-Specific Methods
 * ========================================================================== */

LpFinancialType
lp_investment_financial_get_financial_type (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), LP_FINANCIAL_TYPE_CROWN_BOND);

    return self->financial_type;
}

LpDebtStatus
lp_investment_financial_get_debt_status (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), LP_DEBT_STATUS_PERFORMING);

    return self->debt_status;
}

void
lp_investment_financial_set_debt_status (LpInvestmentFinancial *self,
                                         LpDebtStatus           status)
{
    LpDebtStatus old_status;

    g_return_if_fail (LP_IS_INVESTMENT_FINANCIAL (self));

    if (self->debt_status == status)
        return;

    old_status = self->debt_status;
    self->debt_status = status;

    /* Update current value on default */
    if (status == LP_DEBT_STATUS_DEFAULT)
    {
        gdouble recovery = lp_investment_financial_get_default_recovery_rate (self);
        g_autoptr(LrgBigNumber) recovery_mult = lrg_big_number_new (recovery);
        g_autoptr(LrgBigNumber) new_value = NULL;

        if (self->face_value != NULL)
            new_value = lrg_big_number_multiply (self->face_value, recovery_mult);
        else
            new_value = lrg_big_number_new_zero ();

        lp_investment_set_current_value (LP_INVESTMENT (self), g_steal_pointer (&new_value));
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DEBT_STATUS]);
    g_signal_emit (self, signals[SIGNAL_DEBT_STATUS_CHANGED], 0, old_status, status);

    lp_log_debug ("Financial %s: debt status changed from %d to %d",
                  lp_investment_get_name (LP_INVESTMENT (self)),
                  old_status,
                  status);
}

gdouble
lp_investment_financial_get_interest_rate (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), 0.0);

    return self->interest_rate;
}

void
lp_investment_financial_set_interest_rate (LpInvestmentFinancial *self,
                                           gdouble                rate)
{
    g_return_if_fail (LP_IS_INVESTMENT_FINANCIAL (self));
    g_return_if_fail (rate >= 0.0 && rate <= 1.0);

    if (self->interest_rate == rate)
        return;

    self->interest_rate = rate;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INTEREST_RATE]);
}

LrgBigNumber *
lp_investment_financial_get_face_value (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), NULL);

    return self->face_value;
}

void
lp_investment_financial_set_face_value (LpInvestmentFinancial *self,
                                        LrgBigNumber          *value)
{
    g_return_if_fail (LP_IS_INVESTMENT_FINANCIAL (self));
    g_return_if_fail (value != NULL);

    g_clear_pointer (&self->face_value, lrg_big_number_free);
    self->face_value = value;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_FACE_VALUE]);
}

guint64
lp_investment_financial_get_maturity_year (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), 0);

    return self->maturity_year;
}

void
lp_investment_financial_set_maturity_year (LpInvestmentFinancial *self,
                                           guint64                year)
{
    g_return_if_fail (LP_IS_INVESTMENT_FINANCIAL (self));

    if (self->maturity_year == year)
        return;

    self->maturity_year = year;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MATURITY_YEAR]);
}

const gchar *
lp_investment_financial_get_issuer_id (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), NULL);

    return self->issuer_id;
}

void
lp_investment_financial_set_issuer_id (LpInvestmentFinancial *self,
                                       const gchar           *issuer_id)
{
    g_return_if_fail (LP_IS_INVESTMENT_FINANCIAL (self));

    if (g_strcmp0 (self->issuer_id, issuer_id) == 0)
        return;

    g_clear_pointer (&self->issuer_id, g_free);
    self->issuer_id = g_strdup (issuer_id);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ISSUER_ID]);
}

LrgBigNumber *
lp_investment_financial_calculate_interest_payment (LpInvestmentFinancial *self)
{
    g_autoptr(LrgBigNumber) rate = NULL;
    g_autoptr(LrgBigNumber) payment = NULL;

    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), NULL);

    if (self->face_value == NULL)
        return lrg_big_number_new_zero ();

    rate = lrg_big_number_new (self->interest_rate);
    payment = lrg_big_number_multiply (self->face_value, rate);

    return g_steal_pointer (&payment);
}

gboolean
lp_investment_financial_is_defaulted (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), FALSE);

    return self->debt_status == LP_DEBT_STATUS_DEFAULT;
}

gboolean
lp_investment_financial_is_matured (LpInvestmentFinancial *self,
                                    guint64                current_year)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), FALSE);

    /* 0 means no maturity */
    if (self->maturity_year == 0)
        return FALSE;

    return current_year >= self->maturity_year;
}

gdouble
lp_investment_financial_get_default_recovery_rate (LpInvestmentFinancial *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_FINANCIAL (self), 0.0);

    switch (self->financial_type)
    {
    case LP_FINANCIAL_TYPE_CROWN_BOND:
        return CROWN_BOND_RECOVERY;
    case LP_FINANCIAL_TYPE_NOBLE_DEBT:
        return NOBLE_DEBT_RECOVERY;
    case LP_FINANCIAL_TYPE_MERCHANT_NOTE:
        return MERCHANT_NOTE_RECOVERY;
    case LP_FINANCIAL_TYPE_INSURANCE:
        return INSURANCE_RECOVERY;
    case LP_FINANCIAL_TYPE_USURY:
        return USURY_RECOVERY;
    default:
        return 0.20;  /* Default 20% */
    }
}
