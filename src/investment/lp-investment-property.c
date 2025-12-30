/* lp-investment-property.c - Property Investment
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_INVESTMENT
#include "../lp-log.h"

#include "lp-investment-property.h"

/* Maximum number of improvements per property */
#define MAX_IMPROVEMENTS (5)

/* Base return rates by property type */
#define AGRICULTURAL_RETURN (0.03)  /* 3% - reliable food production */
#define URBAN_RETURN        (0.04)  /* 4% - rent and commerce */
#define MINING_RETURN       (0.05)  /* 5% - resource extraction */
#define TIMBER_RETURN       (0.035) /* 3.5% - renewable resource */
#define COASTAL_RETURN      (0.045) /* 4.5% - trade bonus */

/* Upkeep as percentage of value */
#define BASE_UPKEEP_RATE (0.005)  /* 0.5% annual upkeep */

struct _LpInvestmentProperty
{
    LpInvestment parent_instance;

    LpPropertyType property_type;
    gdouble        stability_bonus;
    guint          improvements;
};

enum
{
    PROP_0,
    PROP_PROPERTY_TYPE,
    PROP_STABILITY_BONUS,
    PROP_IMPROVEMENTS,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (LpInvestmentProperty, lp_investment_property, LP_TYPE_INVESTMENT)

/* GType registration for LpPropertyType enum */
GType
lp_property_type_get_type (void)
{
    static gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_PROPERTY_TYPE_AGRICULTURAL, "LP_PROPERTY_TYPE_AGRICULTURAL", "agricultural" },
            { LP_PROPERTY_TYPE_URBAN, "LP_PROPERTY_TYPE_URBAN", "urban" },
            { LP_PROPERTY_TYPE_MINING, "LP_PROPERTY_TYPE_MINING", "mining" },
            { LP_PROPERTY_TYPE_TIMBER, "LP_PROPERTY_TYPE_TIMBER", "timber" },
            { LP_PROPERTY_TYPE_COASTAL, "LP_PROPERTY_TYPE_COASTAL", "coastal" },
            { 0, NULL, NULL }
        };

        GType type_id = g_enum_register_static ("LpPropertyType", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * Virtual Method Overrides
 * ========================================================================== */

/*
 * Property returns are calculated with compound interest plus
 * bonuses for improvements and stability.
 */
static LrgBigNumber *
lp_investment_property_calculate_returns (LpInvestment *investment,
                                          guint         years)
{
    LpInvestmentProperty *self = LP_INVESTMENT_PROPERTY (investment);
    LrgBigNumber *current_value;
    g_autoptr(LrgBigNumber) result = NULL;
    g_autoptr(LrgBigNumber) base = NULL;
    gdouble rate;
    gdouble improvement_bonus;
    gdouble effective_rate;
    guint i;

    current_value = lp_investment_get_current_value (investment);
    if (current_value == NULL)
        return lrg_big_number_new_zero ();

    base = lrg_big_number_copy (current_value);

    /* Get base rate for property type */
    rate = lp_investment_get_base_return_rate (investment);

    /* Improvements add 0.5% per improvement */
    improvement_bonus = self->improvements * 0.005;
    effective_rate = rate + improvement_bonus;

    /* Apply compound interest */
    result = lrg_big_number_copy (base);

    for (i = 0; i < years; i++)
    {
        g_autoptr(LrgBigNumber) growth = NULL;
        g_autoptr(LrgBigNumber) new_result = NULL;

        growth = lrg_big_number_new (1.0 + effective_rate);
        new_result = lrg_big_number_multiply (result, growth);

        lrg_big_number_free (result);
        result = g_steal_pointer (&new_result);
    }

    lp_log_debug ("Property %s returns over %u years: %s -> %s (%.2f%% rate)",
                  lp_investment_get_name (investment),
                  years,
                  lrg_big_number_format_short (base),
                  lrg_big_number_format_short (result),
                  effective_rate * 100.0);

    return g_steal_pointer (&result);
}

/*
 * Properties are very resilient to events due to their stability bonus.
 * Economic events have reduced impact, political events have minimal impact.
 */
static void
lp_investment_property_apply_event (LpInvestment *investment,
                                    LpEvent      *event)
{
    LpInvestmentProperty *self = LP_INVESTMENT_PROPERTY (investment);

    (void)event;  /* Will be used in future phases */

    /*
     * Property investments survive upheaval well.
     * For now, just log the event application.
     * In future phases, apply value modifications based on event type.
     */
    lp_log_debug ("Property %s: event applied (stability bonus: %.2f)",
                  lp_investment_get_name (investment),
                  self->stability_bonus);
}

/*
 * Properties can always be sold, unless they have specific encumbrances
 * (to be added in future phases).
 */
static gboolean
lp_investment_property_can_sell (LpInvestment *investment)
{
    (void)investment;
    return TRUE;
}

/*
 * Properties have a stability bonus that reduces effective risk.
 */
static gdouble
lp_investment_property_get_risk_modifier (LpInvestment *investment)
{
    LpInvestmentProperty *self = LP_INVESTMENT_PROPERTY (investment);

    /* Higher stability = lower risk (inverse relationship) */
    return 1.0 / self->stability_bonus;
}

/*
 * Base return rate varies by property type.
 */
static gdouble
lp_investment_property_get_base_return_rate (LpInvestment *investment)
{
    LpInvestmentProperty *self = LP_INVESTMENT_PROPERTY (investment);

    switch (self->property_type)
    {
    case LP_PROPERTY_TYPE_AGRICULTURAL:
        return AGRICULTURAL_RETURN;
    case LP_PROPERTY_TYPE_URBAN:
        return URBAN_RETURN;
    case LP_PROPERTY_TYPE_MINING:
        return MINING_RETURN;
    case LP_PROPERTY_TYPE_TIMBER:
        return TIMBER_RETURN;
    case LP_PROPERTY_TYPE_COASTAL:
        return COASTAL_RETURN;
    default:
        return 0.035;  /* Default 3.5% */
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_investment_property_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
    LpInvestmentProperty *self = LP_INVESTMENT_PROPERTY (object);

    switch (prop_id)
    {
    case PROP_PROPERTY_TYPE:
        g_value_set_enum (value, self->property_type);
        break;

    case PROP_STABILITY_BONUS:
        g_value_set_double (value, self->stability_bonus);
        break;

    case PROP_IMPROVEMENTS:
        g_value_set_uint (value, self->improvements);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_property_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
    LpInvestmentProperty *self = LP_INVESTMENT_PROPERTY (object);

    switch (prop_id)
    {
    case PROP_PROPERTY_TYPE:
        self->property_type = g_value_get_enum (value);
        break;

    case PROP_STABILITY_BONUS:
        self->stability_bonus = g_value_get_double (value);
        break;

    case PROP_IMPROVEMENTS:
        self->improvements = g_value_get_uint (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_property_class_init (LpInvestmentPropertyClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpInvestmentClass *investment_class = LP_INVESTMENT_CLASS (klass);

    object_class->get_property = lp_investment_property_get_property;
    object_class->set_property = lp_investment_property_set_property;

    /* Override virtual methods */
    investment_class->calculate_returns = lp_investment_property_calculate_returns;
    investment_class->apply_event = lp_investment_property_apply_event;
    investment_class->can_sell = lp_investment_property_can_sell;
    investment_class->get_risk_modifier = lp_investment_property_get_risk_modifier;
    investment_class->get_base_return_rate = lp_investment_property_get_base_return_rate;

    /**
     * LpInvestmentProperty:property-type:
     *
     * The subtype of property.
     */
    properties[PROP_PROPERTY_TYPE] =
        g_param_spec_enum ("property-type",
                           "Property Type",
                           "Type of property investment",
                           LP_TYPE_PROPERTY_TYPE,
                           LP_PROPERTY_TYPE_AGRICULTURAL,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentProperty:stability-bonus:
     *
     * Stability bonus (1.0 = normal, higher = more stable).
     */
    properties[PROP_STABILITY_BONUS] =
        g_param_spec_double ("stability-bonus",
                             "Stability Bonus",
                             "Resistance to value loss during crises",
                             0.5, 3.0, 1.2,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentProperty:improvements:
     *
     * Number of improvements made to this property.
     */
    properties[PROP_IMPROVEMENTS] =
        g_param_spec_uint ("improvements",
                           "Improvements",
                           "Number of improvements made",
                           0, MAX_IMPROVEMENTS, 0,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_investment_property_init (LpInvestmentProperty *self)
{
    /* Set base class properties */
    lp_investment_set_risk_level (LP_INVESTMENT (self), LP_RISK_LEVEL_LOW);

    /* Initialize property-specific fields */
    self->property_type = LP_PROPERTY_TYPE_AGRICULTURAL;
    self->stability_bonus = 1.2;  /* Properties are 20% more stable than average */
    self->improvements = 0;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_investment_property_new:
 * @id: unique identifier
 * @name: display name
 * @property_type: the #LpPropertyType
 *
 * Creates a new property investment.
 *
 * Returns: (transfer full): A new #LpInvestmentProperty
 */
LpInvestmentProperty *
lp_investment_property_new (const gchar    *id,
                            const gchar    *name,
                            LpPropertyType  property_type)
{
    return g_object_new (LP_TYPE_INVESTMENT_PROPERTY,
                         "id", id,
                         "name", name,
                         "asset-class", LP_ASSET_CLASS_PROPERTY,
                         "property-type", property_type,
                         NULL);
}

/**
 * lp_investment_property_new_with_value:
 * @id: unique identifier
 * @name: display name
 * @property_type: the #LpPropertyType
 * @value: (transfer full): initial value
 *
 * Creates a new property investment with specified value.
 *
 * Returns: (transfer full): A new #LpInvestmentProperty
 */
LpInvestmentProperty *
lp_investment_property_new_with_value (const gchar    *id,
                                       const gchar    *name,
                                       LpPropertyType  property_type,
                                       LrgBigNumber   *value)
{
    LpInvestmentProperty *self;

    self = lp_investment_property_new (id, name, property_type);

    if (value != NULL)
    {
        lp_investment_set_purchase_price (LP_INVESTMENT (self),
                                          lrg_big_number_copy (value));
        lp_investment_set_current_value (LP_INVESTMENT (self), value);
    }

    return self;
}

/* ==========================================================================
 * Property-Specific Methods
 * ========================================================================== */

LpPropertyType
lp_investment_property_get_property_type (LpInvestmentProperty *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_PROPERTY (self), LP_PROPERTY_TYPE_AGRICULTURAL);

    return self->property_type;
}

gdouble
lp_investment_property_get_stability_bonus (LpInvestmentProperty *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_PROPERTY (self), 1.0);

    return self->stability_bonus;
}

void
lp_investment_property_set_stability_bonus (LpInvestmentProperty *self,
                                            gdouble               bonus)
{
    g_return_if_fail (LP_IS_INVESTMENT_PROPERTY (self));
    g_return_if_fail (bonus >= 0.5 && bonus <= 3.0);

    if (self->stability_bonus == bonus)
        return;

    self->stability_bonus = bonus;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STABILITY_BONUS]);
}

guint
lp_investment_property_get_improvements (LpInvestmentProperty *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_PROPERTY (self), 0);

    return self->improvements;
}

gboolean
lp_investment_property_add_improvement (LpInvestmentProperty *self,
                                        LrgBigNumber         *cost)
{
    LrgBigNumber *current_value;
    g_autoptr(LrgBigNumber) new_value = NULL;

    g_return_val_if_fail (LP_IS_INVESTMENT_PROPERTY (self), FALSE);
    g_return_val_if_fail (cost != NULL, FALSE);

    if (self->improvements >= MAX_IMPROVEMENTS)
    {
        lp_log_debug ("Property %s: cannot add improvement (max %u reached)",
                      lp_investment_get_name (LP_INVESTMENT (self)),
                      MAX_IMPROVEMENTS);
        lrg_big_number_free (cost);
        return FALSE;
    }

    /* Improvement increases value by the cost */
    current_value = lp_investment_get_current_value (LP_INVESTMENT (self));
    new_value = lrg_big_number_add (current_value, cost);

    lp_investment_set_current_value (LP_INVESTMENT (self), g_steal_pointer (&new_value));

    self->improvements++;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IMPROVEMENTS]);

    lp_log_debug ("Property %s: improvement %u added (cost: %s)",
                  lp_investment_get_name (LP_INVESTMENT (self)),
                  self->improvements,
                  lrg_big_number_format_short (cost));

    lrg_big_number_free (cost);

    return TRUE;
}

LrgBigNumber *
lp_investment_property_get_upkeep_cost (LpInvestmentProperty *self)
{
    LrgBigNumber *current_value;
    g_autoptr(LrgBigNumber) rate = NULL;
    g_autoptr(LrgBigNumber) upkeep = NULL;

    g_return_val_if_fail (LP_IS_INVESTMENT_PROPERTY (self), NULL);

    current_value = lp_investment_get_current_value (LP_INVESTMENT (self));
    if (current_value == NULL)
        return lrg_big_number_new_zero ();

    /* Base upkeep + 0.1% per improvement */
    rate = lrg_big_number_new (BASE_UPKEEP_RATE + (self->improvements * 0.001));
    upkeep = lrg_big_number_multiply (current_value, rate);

    return g_steal_pointer (&upkeep);
}

gboolean
lp_investment_property_is_developed (LpInvestmentProperty *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_PROPERTY (self), FALSE);

    return self->improvements >= MAX_IMPROVEMENTS;
}
