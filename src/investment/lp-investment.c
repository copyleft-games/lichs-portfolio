/* lp-investment.c - Base Investment Class
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_INVESTMENT
#include "../lp-log.h"

#include "lp-investment.h"

/* Private data for the base class */
typedef struct
{
    gchar        *id;
    gchar        *name;
    gchar        *description;
    gchar        *region_id;

    LpAssetClass  asset_class;
    LpRiskLevel   risk_level;

    LrgBigNumber *purchase_price;
    LrgBigNumber *current_value;
    guint64       purchase_year;
} LpInvestmentPrivate;

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_DESCRIPTION,
    PROP_ASSET_CLASS,
    PROP_RISK_LEVEL,
    PROP_PURCHASE_PRICE,
    PROP_CURRENT_VALUE,
    PROP_PURCHASE_YEAR,
    PROP_REGION_ID,
    N_PROPS
};

enum
{
    SIGNAL_VALUE_CHANGED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_investment_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpInvestment, lp_investment, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (LpInvestment)
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_investment_saveable_init))

/* ==========================================================================
 * Default Virtual Method Implementations
 * ========================================================================== */

/*
 * Default implementation uses compound interest formula:
 * FV = PV * (1 + r)^n
 * Where:
 *   FV = Future Value (returns)
 *   PV = Present Value (current_value)
 *   r  = base return rate
 *   n  = number of years
 */
static LrgBigNumber *
lp_investment_real_calculate_returns (LpInvestment *self,
                                      guint         years)
{
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);
    g_autoptr(LrgBigNumber) base = NULL;
    g_autoptr(LrgBigNumber) rate = NULL;
    g_autoptr(LrgBigNumber) multiplier = NULL;
    g_autoptr(LrgBigNumber) result = NULL;
    gdouble base_rate;
    gdouble risk_mod;
    gdouble effective_rate;
    guint i;

    if (priv->current_value == NULL)
        return lrg_big_number_new_zero ();

    base = lrg_big_number_copy (priv->current_value);
    base_rate = lp_investment_get_base_return_rate (self);
    risk_mod = lp_investment_get_risk_modifier (self);

    /* Apply risk modifier to return rate */
    effective_rate = base_rate * risk_mod;

    /* Compound annually: (1 + rate)^years */
    multiplier = lrg_big_number_new (1.0);

    for (i = 0; i < years; i++)
    {
        g_autoptr(LrgBigNumber) year_rate = NULL;
        g_autoptr(LrgBigNumber) new_mult = NULL;

        year_rate = lrg_big_number_new (1.0 + effective_rate);
        new_mult = lrg_big_number_multiply (multiplier, year_rate);

        lrg_big_number_free (multiplier);
        multiplier = g_steal_pointer (&new_mult);
    }

    result = lrg_big_number_multiply (base, multiplier);

    lp_log_debug ("Calculated returns for %s: %u years at %.2f%% = %s",
                  priv->name,
                  years,
                  effective_rate * 100.0,
                  lrg_big_number_format_short (result));

    return g_steal_pointer (&result);
}

static void
lp_investment_real_apply_event (LpInvestment *self,
                                LpEvent      *event)
{
    /* Base implementation does nothing - subclasses override */
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    lp_log_debug ("Base apply_event called for %s (no effect)", priv->name);
}

static gboolean
lp_investment_real_can_sell (LpInvestment *self)
{
    /* Default: all investments can be sold */
    return TRUE;
}

static gdouble
lp_investment_real_get_risk_modifier (LpInvestment *self)
{
    /* Default: no modification */
    return 1.0;
}

static gdouble
lp_investment_real_get_base_return_rate (LpInvestment *self)
{
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    /* Base rates vary by risk level */
    switch (priv->risk_level)
    {
    case LP_RISK_LEVEL_LOW:
        return 0.03;    /* 3% */
    case LP_RISK_LEVEL_MEDIUM:
        return 0.06;    /* 6% */
    case LP_RISK_LEVEL_HIGH:
        return 0.10;    /* 10% */
    case LP_RISK_LEVEL_EXTREME:
        return 0.15;    /* 15% */
    default:
        return 0.05;    /* 5% default */
    }
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_investment_get_save_id (LrgSaveable *saveable)
{
    LpInvestment *self = LP_INVESTMENT (saveable);
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    /* Each investment has a unique ID */
    return priv->id;
}

static gboolean
lp_investment_save (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpInvestment *self = LP_INVESTMENT (saveable);
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    /* Save all properties */
    lrg_save_context_write_string (context, "id", priv->id);
    lrg_save_context_write_string (context, "name", priv->name);

    if (priv->description != NULL)
        lrg_save_context_write_string (context, "description", priv->description);

    if (priv->region_id != NULL)
        lrg_save_context_write_string (context, "region-id", priv->region_id);

    lrg_save_context_write_int (context, "asset-class", priv->asset_class);
    lrg_save_context_write_int (context, "risk-level", priv->risk_level);
    lrg_save_context_write_uint (context, "purchase-year", priv->purchase_year);

    /* Save BigNumbers as mantissa/exponent pairs */
    if (priv->purchase_price != NULL)
    {
        lrg_save_context_write_double (context, "purchase-price-mantissa",
                                       lrg_big_number_get_mantissa (priv->purchase_price));
        lrg_save_context_write_int (context, "purchase-price-exponent",
                                    lrg_big_number_get_exponent (priv->purchase_price));
    }

    if (priv->current_value != NULL)
    {
        lrg_save_context_write_double (context, "current-value-mantissa",
                                       lrg_big_number_get_mantissa (priv->current_value));
        lrg_save_context_write_int (context, "current-value-exponent",
                                    lrg_big_number_get_exponent (priv->current_value));
    }

    return TRUE;
}

static gboolean
lp_investment_load (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpInvestment *self = LP_INVESTMENT (saveable);
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);
    gdouble mantissa;
    gint64 exponent;

    /* Load string properties */
    g_clear_pointer (&priv->id, g_free);
    priv->id = g_strdup (lrg_save_context_read_string (context, "id", "unknown"));

    g_clear_pointer (&priv->name, g_free);
    priv->name = g_strdup (lrg_save_context_read_string (context, "name", "Unknown Investment"));

    g_clear_pointer (&priv->description, g_free);
    priv->description = g_strdup (lrg_save_context_read_string (context, "description", NULL));

    g_clear_pointer (&priv->region_id, g_free);
    priv->region_id = g_strdup (lrg_save_context_read_string (context, "region-id", NULL));

    /* Load enum properties */
    priv->asset_class = lrg_save_context_read_int (context, "asset-class", LP_ASSET_CLASS_PROPERTY);
    priv->risk_level = lrg_save_context_read_int (context, "risk-level", LP_RISK_LEVEL_MEDIUM);
    priv->purchase_year = lrg_save_context_read_uint (context, "purchase-year", 847);

    /* Load BigNumbers */
    g_clear_pointer (&priv->purchase_price, lrg_big_number_free);
    mantissa = lrg_save_context_read_double (context, "purchase-price-mantissa", 1.0);
    exponent = lrg_save_context_read_int (context, "purchase-price-exponent", 3);
    priv->purchase_price = lrg_big_number_new_from_parts (mantissa, exponent);

    g_clear_pointer (&priv->current_value, lrg_big_number_free);
    mantissa = lrg_save_context_read_double (context, "current-value-mantissa", 1.0);
    exponent = lrg_save_context_read_int (context, "current-value-exponent", 3);
    priv->current_value = lrg_big_number_new_from_parts (mantissa, exponent);

    lp_log_debug ("Loaded investment: %s (%s)", priv->name, priv->id);

    return TRUE;
}

static void
lp_investment_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_investment_get_save_id;
    iface->save = lp_investment_save;
    iface->load = lp_investment_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_investment_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    LpInvestment *self = LP_INVESTMENT (object);
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ID:
        g_value_set_string (value, priv->id);
        break;

    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_DESCRIPTION:
        g_value_set_string (value, priv->description);
        break;

    case PROP_ASSET_CLASS:
        g_value_set_enum (value, priv->asset_class);
        break;

    case PROP_RISK_LEVEL:
        g_value_set_enum (value, priv->risk_level);
        break;

    case PROP_PURCHASE_PRICE:
        g_value_set_boxed (value, priv->purchase_price);
        break;

    case PROP_CURRENT_VALUE:
        g_value_set_boxed (value, priv->current_value);
        break;

    case PROP_PURCHASE_YEAR:
        g_value_set_uint64 (value, priv->purchase_year);
        break;

    case PROP_REGION_ID:
        g_value_set_string (value, priv->region_id);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    LpInvestment *self = LP_INVESTMENT (object);
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ID:
        g_clear_pointer (&priv->id, g_free);
        priv->id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        lp_investment_set_name (self, g_value_get_string (value));
        break;

    case PROP_DESCRIPTION:
        lp_investment_set_description (self, g_value_get_string (value));
        break;

    case PROP_ASSET_CLASS:
        priv->asset_class = g_value_get_enum (value);
        break;

    case PROP_RISK_LEVEL:
        lp_investment_set_risk_level (self, g_value_get_enum (value));
        break;

    case PROP_PURCHASE_PRICE:
        lp_investment_set_purchase_price (self, g_value_dup_boxed (value));
        break;

    case PROP_CURRENT_VALUE:
        lp_investment_set_current_value (self, g_value_dup_boxed (value));
        break;

    case PROP_PURCHASE_YEAR:
        priv->purchase_year = g_value_get_uint64 (value);
        break;

    case PROP_REGION_ID:
        lp_investment_set_region_id (self, g_value_get_string (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_finalize (GObject *object)
{
    LpInvestment *self = LP_INVESTMENT (object);
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    lp_log_debug ("Finalizing investment: %s", priv->id ? priv->id : "(unknown)");

    g_clear_pointer (&priv->id, g_free);
    g_clear_pointer (&priv->name, g_free);
    g_clear_pointer (&priv->description, g_free);
    g_clear_pointer (&priv->region_id, g_free);
    g_clear_pointer (&priv->purchase_price, lrg_big_number_free);
    g_clear_pointer (&priv->current_value, lrg_big_number_free);

    G_OBJECT_CLASS (lp_investment_parent_class)->finalize (object);
}

static void
lp_investment_class_init (LpInvestmentClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_investment_get_property;
    object_class->set_property = lp_investment_set_property;
    object_class->finalize = lp_investment_finalize;

    /* Default virtual method implementations */
    klass->calculate_returns = lp_investment_real_calculate_returns;
    klass->apply_event = lp_investment_real_apply_event;
    klass->can_sell = lp_investment_real_can_sell;
    klass->get_risk_modifier = lp_investment_real_get_risk_modifier;
    klass->get_base_return_rate = lp_investment_real_get_base_return_rate;

    /**
     * LpInvestment:id:
     *
     * Unique identifier for this investment.
     */
    properties[PROP_ID] =
        g_param_spec_string ("id",
                             "ID",
                             "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:name:
     *
     * Display name.
     */
    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Display name",
                             "Unknown Investment",
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:description:
     *
     * Detailed description.
     */
    properties[PROP_DESCRIPTION] =
        g_param_spec_string ("description",
                             "Description",
                             "Detailed description",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:asset-class:
     *
     * The asset class category.
     */
    properties[PROP_ASSET_CLASS] =
        g_param_spec_enum ("asset-class",
                           "Asset Class",
                           "Asset class category",
                           LP_TYPE_ASSET_CLASS,
                           LP_ASSET_CLASS_PROPERTY,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:risk-level:
     *
     * The risk level.
     */
    properties[PROP_RISK_LEVEL] =
        g_param_spec_enum ("risk-level",
                           "Risk Level",
                           "Risk classification",
                           LP_TYPE_RISK_LEVEL,
                           LP_RISK_LEVEL_MEDIUM,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:purchase-price:
     *
     * Original purchase price.
     */
    properties[PROP_PURCHASE_PRICE] =
        g_param_spec_boxed ("purchase-price",
                            "Purchase Price",
                            "Original purchase price",
                            LRG_TYPE_BIG_NUMBER,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:current-value:
     *
     * Current market value.
     */
    properties[PROP_CURRENT_VALUE] =
        g_param_spec_boxed ("current-value",
                            "Current Value",
                            "Current market value",
                            LRG_TYPE_BIG_NUMBER,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:purchase-year:
     *
     * Year of purchase.
     */
    properties[PROP_PURCHASE_YEAR] =
        g_param_spec_uint64 ("purchase-year",
                             "Purchase Year",
                             "Year of purchase",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestment:region-id:
     *
     * ID of the region where this investment is located.
     */
    properties[PROP_REGION_ID] =
        g_param_spec_string ("region-id",
                             "Region ID",
                             "Region where investment is located",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpInvestment::value-changed:
     * @self: the #LpInvestment
     * @old_value: (transfer none): previous value
     * @new_value: (transfer none): new value
     *
     * Emitted when the investment's current value changes.
     */
    signals[SIGNAL_VALUE_CHANGED] =
        g_signal_new ("value-changed",
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
lp_investment_init (LpInvestment *self)
{
    LpInvestmentPrivate *priv = lp_investment_get_instance_private (self);

    priv->id = NULL;
    priv->name = g_strdup ("Unknown Investment");
    priv->description = NULL;
    priv->region_id = NULL;
    priv->asset_class = LP_ASSET_CLASS_PROPERTY;
    priv->risk_level = LP_RISK_LEVEL_MEDIUM;
    priv->purchase_price = lrg_big_number_new (1000.0);
    priv->current_value = lrg_big_number_new (1000.0);
    priv->purchase_year = 0;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_investment_new:
 * @id: unique identifier for this investment
 * @name: display name
 * @asset_class: the #LpAssetClass
 *
 * Creates a new investment with the given properties.
 *
 * Returns: (transfer full): A new #LpInvestment
 */
LpInvestment *
lp_investment_new (const gchar  *id,
                   const gchar  *name,
                   LpAssetClass  asset_class)
{
    return g_object_new (LP_TYPE_INVESTMENT,
                         "id", id,
                         "name", name,
                         "asset-class", asset_class,
                         NULL);
}

/* ==========================================================================
 * Property Getters/Setters
 * ========================================================================== */

const gchar *
lp_investment_get_id (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    priv = lp_investment_get_instance_private (self);
    return priv->id;
}

const gchar *
lp_investment_get_name (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    priv = lp_investment_get_instance_private (self);
    return priv->name;
}

void
lp_investment_set_name (LpInvestment *self,
                        const gchar  *name)
{
    LpInvestmentPrivate *priv;

    g_return_if_fail (LP_IS_INVESTMENT (self));

    priv = lp_investment_get_instance_private (self);

    if (g_strcmp0 (priv->name, name) == 0)
        return;

    g_clear_pointer (&priv->name, g_free);
    priv->name = g_strdup (name);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}

const gchar *
lp_investment_get_description (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    priv = lp_investment_get_instance_private (self);
    return priv->description;
}

void
lp_investment_set_description (LpInvestment *self,
                               const gchar  *description)
{
    LpInvestmentPrivate *priv;

    g_return_if_fail (LP_IS_INVESTMENT (self));

    priv = lp_investment_get_instance_private (self);

    if (g_strcmp0 (priv->description, description) == 0)
        return;

    g_clear_pointer (&priv->description, g_free);
    priv->description = g_strdup (description);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESCRIPTION]);
}

LpAssetClass
lp_investment_get_asset_class (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), LP_ASSET_CLASS_PROPERTY);

    priv = lp_investment_get_instance_private (self);
    return priv->asset_class;
}

LpRiskLevel
lp_investment_get_risk_level (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), LP_RISK_LEVEL_MEDIUM);

    priv = lp_investment_get_instance_private (self);
    return priv->risk_level;
}

void
lp_investment_set_risk_level (LpInvestment *self,
                              LpRiskLevel   level)
{
    LpInvestmentPrivate *priv;

    g_return_if_fail (LP_IS_INVESTMENT (self));

    priv = lp_investment_get_instance_private (self);

    if (priv->risk_level == level)
        return;

    priv->risk_level = level;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RISK_LEVEL]);
}

LrgBigNumber *
lp_investment_get_purchase_price (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    priv = lp_investment_get_instance_private (self);
    return priv->purchase_price;
}

void
lp_investment_set_purchase_price (LpInvestment *self,
                                  LrgBigNumber *price)
{
    LpInvestmentPrivate *priv;

    g_return_if_fail (LP_IS_INVESTMENT (self));
    g_return_if_fail (price != NULL);

    priv = lp_investment_get_instance_private (self);

    g_clear_pointer (&priv->purchase_price, lrg_big_number_free);
    priv->purchase_price = price;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PURCHASE_PRICE]);
}

LrgBigNumber *
lp_investment_get_current_value (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    priv = lp_investment_get_instance_private (self);
    return priv->current_value;
}

void
lp_investment_set_current_value (LpInvestment *self,
                                 LrgBigNumber *value)
{
    LpInvestmentPrivate *priv;
    g_autoptr(LrgBigNumber) old_value = NULL;

    g_return_if_fail (LP_IS_INVESTMENT (self));
    g_return_if_fail (value != NULL);

    priv = lp_investment_get_instance_private (self);

    old_value = priv->current_value;
    priv->current_value = value;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_VALUE]);
    g_signal_emit (self, signals[SIGNAL_VALUE_CHANGED], 0, old_value, priv->current_value);
}

guint64
lp_investment_get_purchase_year (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), 0);

    priv = lp_investment_get_instance_private (self);
    return priv->purchase_year;
}

void
lp_investment_set_purchase_year (LpInvestment *self,
                                 guint64       year)
{
    LpInvestmentPrivate *priv;

    g_return_if_fail (LP_IS_INVESTMENT (self));

    priv = lp_investment_get_instance_private (self);

    if (priv->purchase_year == year)
        return;

    priv->purchase_year = year;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PURCHASE_YEAR]);
}

const gchar *
lp_investment_get_region_id (LpInvestment *self)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    priv = lp_investment_get_instance_private (self);
    return priv->region_id;
}

void
lp_investment_set_region_id (LpInvestment *self,
                             const gchar  *region_id)
{
    LpInvestmentPrivate *priv;

    g_return_if_fail (LP_IS_INVESTMENT (self));

    priv = lp_investment_get_instance_private (self);

    if (g_strcmp0 (priv->region_id, region_id) == 0)
        return;

    g_clear_pointer (&priv->region_id, g_free);
    priv->region_id = g_strdup (region_id);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REGION_ID]);
}

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

LrgBigNumber *
lp_investment_calculate_returns (LpInvestment *self,
                                 guint         years)
{
    LpInvestmentClass *klass;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    klass = LP_INVESTMENT_GET_CLASS (self);
    g_return_val_if_fail (klass->calculate_returns != NULL, NULL);

    return klass->calculate_returns (self, years);
}

void
lp_investment_apply_event (LpInvestment *self,
                           LpEvent      *event)
{
    LpInvestmentClass *klass;

    g_return_if_fail (LP_IS_INVESTMENT (self));

    klass = LP_INVESTMENT_GET_CLASS (self);
    if (klass->apply_event != NULL)
        klass->apply_event (self, event);
}

gboolean
lp_investment_can_sell (LpInvestment *self)
{
    LpInvestmentClass *klass;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), FALSE);

    klass = LP_INVESTMENT_GET_CLASS (self);
    g_return_val_if_fail (klass->can_sell != NULL, FALSE);

    return klass->can_sell (self);
}

gdouble
lp_investment_get_risk_modifier (LpInvestment *self)
{
    LpInvestmentClass *klass;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), 1.0);

    klass = LP_INVESTMENT_GET_CLASS (self);
    g_return_val_if_fail (klass->get_risk_modifier != NULL, 1.0);

    return klass->get_risk_modifier (self);
}

gdouble
lp_investment_get_base_return_rate (LpInvestment *self)
{
    LpInvestmentClass *klass;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), 0.05);

    klass = LP_INVESTMENT_GET_CLASS (self);
    g_return_val_if_fail (klass->get_base_return_rate != NULL, 0.05);

    return klass->get_base_return_rate (self);
}

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

guint64
lp_investment_get_age (LpInvestment *self,
                       guint64       current_year)
{
    LpInvestmentPrivate *priv;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), 0);

    priv = lp_investment_get_instance_private (self);

    if (current_year <= priv->purchase_year)
        return 0;

    return current_year - priv->purchase_year;
}

gdouble
lp_investment_get_return_percentage (LpInvestment *self)
{
    LpInvestmentPrivate *priv;
    gdouble purchase;
    gdouble current;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), 0.0);

    priv = lp_investment_get_instance_private (self);

    if (priv->purchase_price == NULL || priv->current_value == NULL)
        return 0.0;

    purchase = lrg_big_number_to_double (priv->purchase_price);
    current = lrg_big_number_to_double (priv->current_value);

    if (purchase <= 0.0)
        return 0.0;

    return ((current - purchase) / purchase) * 100.0;
}

guint
lp_investment_get_exposure_contribution (LpInvestment *self)
{
    LpInvestmentPrivate *priv;
    gdouble value;
    guint base_exposure;
    guint risk_multiplier;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), 0);

    priv = lp_investment_get_instance_private (self);

    if (priv->current_value == NULL)
        return 0;

    value = lrg_big_number_to_double (priv->current_value);

    /*
     * Base exposure scales with value:
     * < 1000: 0
     * 1000-10000: 1
     * 10000-100000: 2
     * 100000-1M: 3
     * > 1M: 5
     */
    if (value < 1000.0)
        base_exposure = 0;
    else if (value < 10000.0)
        base_exposure = 1;
    else if (value < 100000.0)
        base_exposure = 2;
    else if (value < 1000000.0)
        base_exposure = 3;
    else
        base_exposure = 5;

    /* Risk level multiplier */
    switch (priv->risk_level)
    {
    case LP_RISK_LEVEL_LOW:
        risk_multiplier = 1;
        break;
    case LP_RISK_LEVEL_MEDIUM:
        risk_multiplier = 2;
        break;
    case LP_RISK_LEVEL_HIGH:
        risk_multiplier = 3;
        break;
    case LP_RISK_LEVEL_EXTREME:
        risk_multiplier = 5;
        break;
    default:
        risk_multiplier = 1;
        break;
    }

    /* Dark investments have extra exposure */
    if (priv->asset_class == LP_ASSET_CLASS_DARK)
        risk_multiplier *= 2;

    return base_exposure * risk_multiplier;
}
