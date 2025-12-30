/* lp-investment-trade.c - Trade Investment
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_INVESTMENT
#include "../lp-log.h"

#include "lp-investment-trade.h"

/* Base return rates by trade type */
#define ROUTE_RETURN     (0.06)  /* 6% - consistent route income */
#define COMMODITY_RETURN (0.08)  /* 8% - commodity speculation */
#define GUILD_RETURN     (0.05)  /* 5% - guild dividends */
#define SHIPPING_RETURN  (0.07)  /* 7% - maritime trade */
#define CARAVAN_RETURN   (0.065) /* 6.5% - land routes */

/* Disruption modifiers */
#define DISRUPTED_MODIFIER (0.5)  /* 50% returns when disrupted */
#define CLOSED_MODIFIER    (0.0)  /* No returns when closed */

struct _LpInvestmentTrade
{
    LpInvestment parent_instance;

    LpTradeType   trade_type;
    LpRouteStatus route_status;
    gdouble       market_modifier;

    gchar        *source_region;
    gchar        *destination_region;
    gchar        *commodity_type;
};

enum
{
    PROP_0,
    PROP_TRADE_TYPE,
    PROP_ROUTE_STATUS,
    PROP_MARKET_MODIFIER,
    PROP_SOURCE_REGION,
    PROP_DESTINATION_REGION,
    PROP_COMMODITY_TYPE,
    N_PROPS
};

enum
{
    SIGNAL_ROUTE_STATUS_CHANGED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

G_DEFINE_TYPE (LpInvestmentTrade, lp_investment_trade, LP_TYPE_INVESTMENT)

/* GType registration for enums */
GType
lp_trade_type_get_type (void)
{
    static gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_TRADE_TYPE_ROUTE, "LP_TRADE_TYPE_ROUTE", "route" },
            { LP_TRADE_TYPE_COMMODITY, "LP_TRADE_TYPE_COMMODITY", "commodity" },
            { LP_TRADE_TYPE_GUILD, "LP_TRADE_TYPE_GUILD", "guild" },
            { LP_TRADE_TYPE_SHIPPING, "LP_TRADE_TYPE_SHIPPING", "shipping" },
            { LP_TRADE_TYPE_CARAVAN, "LP_TRADE_TYPE_CARAVAN", "caravan" },
            { 0, NULL, NULL }
        };

        GType type_id = g_enum_register_static ("LpTradeType", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

GType
lp_route_status_get_type (void)
{
    static gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_ROUTE_STATUS_OPEN, "LP_ROUTE_STATUS_OPEN", "open" },
            { LP_ROUTE_STATUS_DISRUPTED, "LP_ROUTE_STATUS_DISRUPTED", "disrupted" },
            { LP_ROUTE_STATUS_CLOSED, "LP_ROUTE_STATUS_CLOSED", "closed" },
            { 0, NULL, NULL }
        };

        GType type_id = g_enum_register_static ("LpRouteStatus", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * Virtual Method Overrides
 * ========================================================================== */

/*
 * Trade returns are affected by route status and market conditions.
 * Variable returns make trade riskier but potentially more profitable.
 */
static LrgBigNumber *
lp_investment_trade_calculate_returns (LpInvestment *investment,
                                       guint         years)
{
    LpInvestmentTrade *self = LP_INVESTMENT_TRADE (investment);
    LrgBigNumber *current_value;
    g_autoptr(LrgBigNumber) result = NULL;
    g_autoptr(LrgBigNumber) base = NULL;
    gdouble rate;
    gdouble status_modifier;
    gdouble effective_rate;
    guint i;

    current_value = lp_investment_get_current_value (investment);
    if (current_value == NULL)
        return lrg_big_number_new_zero ();

    base = lrg_big_number_copy (current_value);

    /* Get base rate for trade type */
    rate = lp_investment_get_base_return_rate (investment);

    /* Apply route status modifier */
    switch (self->route_status)
    {
    case LP_ROUTE_STATUS_OPEN:
        status_modifier = 1.0;
        break;
    case LP_ROUTE_STATUS_DISRUPTED:
        status_modifier = DISRUPTED_MODIFIER;
        break;
    case LP_ROUTE_STATUS_CLOSED:
        status_modifier = CLOSED_MODIFIER;
        break;
    default:
        status_modifier = 1.0;
        break;
    }

    /* Apply market modifier */
    effective_rate = rate * status_modifier * self->market_modifier;

    /* Apply compound interest with variable modifier */
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

    lp_log_debug ("Trade %s returns over %u years: %s -> %s "
                  "(%.2f%% rate, status: %d, market: %.2f)",
                  lp_investment_get_name (investment),
                  years,
                  lrg_big_number_format_short (base),
                  lrg_big_number_format_short (result),
                  effective_rate * 100.0,
                  self->route_status,
                  self->market_modifier);

    return g_steal_pointer (&result);
}

/*
 * Trade investments are sensitive to political and economic events.
 */
static void
lp_investment_trade_apply_event (LpInvestment *investment,
                                 LpEvent      *event)
{
    LpInvestmentTrade *self = LP_INVESTMENT_TRADE (investment);

    (void)event;  /* Will be used in future phases */

    lp_log_debug ("Trade %s: event applied (route status: %d, market: %.2f)",
                  lp_investment_get_name (investment),
                  self->route_status,
                  self->market_modifier);
}

/*
 * Trade investments can be sold unless routes are completely closed.
 */
static gboolean
lp_investment_trade_can_sell (LpInvestment *investment)
{
    (void)investment;

    /* Can't sell closed routes at full value - but still can sell */
    return TRUE;
}

/*
 * Trade risk depends on route status and market volatility.
 */
static gdouble
lp_investment_trade_get_risk_modifier (LpInvestment *investment)
{
    LpInvestmentTrade *self = LP_INVESTMENT_TRADE (investment);
    gdouble base_risk = 1.0;

    /* Disrupted routes are riskier */
    if (self->route_status == LP_ROUTE_STATUS_DISRUPTED)
        base_risk = 1.5;
    else if (self->route_status == LP_ROUTE_STATUS_CLOSED)
        base_risk = 2.0;

    /* Volatile markets increase risk */
    if (self->market_modifier > 1.2 || self->market_modifier < 0.8)
        base_risk *= 1.25;

    return base_risk;
}

/*
 * Base return rate varies by trade type.
 */
static gdouble
lp_investment_trade_get_base_return_rate (LpInvestment *investment)
{
    LpInvestmentTrade *self = LP_INVESTMENT_TRADE (investment);

    switch (self->trade_type)
    {
    case LP_TRADE_TYPE_ROUTE:
        return ROUTE_RETURN;
    case LP_TRADE_TYPE_COMMODITY:
        return COMMODITY_RETURN;
    case LP_TRADE_TYPE_GUILD:
        return GUILD_RETURN;
    case LP_TRADE_TYPE_SHIPPING:
        return SHIPPING_RETURN;
    case LP_TRADE_TYPE_CARAVAN:
        return CARAVAN_RETURN;
    default:
        return 0.06;  /* Default 6% */
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_investment_trade_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    LpInvestmentTrade *self = LP_INVESTMENT_TRADE (object);

    switch (prop_id)
    {
    case PROP_TRADE_TYPE:
        g_value_set_enum (value, self->trade_type);
        break;

    case PROP_ROUTE_STATUS:
        g_value_set_enum (value, self->route_status);
        break;

    case PROP_MARKET_MODIFIER:
        g_value_set_double (value, self->market_modifier);
        break;

    case PROP_SOURCE_REGION:
        g_value_set_string (value, self->source_region);
        break;

    case PROP_DESTINATION_REGION:
        g_value_set_string (value, self->destination_region);
        break;

    case PROP_COMMODITY_TYPE:
        g_value_set_string (value, self->commodity_type);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_trade_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    LpInvestmentTrade *self = LP_INVESTMENT_TRADE (object);

    switch (prop_id)
    {
    case PROP_TRADE_TYPE:
        self->trade_type = g_value_get_enum (value);
        break;

    case PROP_ROUTE_STATUS:
        lp_investment_trade_set_route_status (self, g_value_get_enum (value));
        break;

    case PROP_MARKET_MODIFIER:
        lp_investment_trade_set_market_modifier (self, g_value_get_double (value));
        break;

    case PROP_SOURCE_REGION:
        lp_investment_trade_set_source_region (self, g_value_get_string (value));
        break;

    case PROP_DESTINATION_REGION:
        lp_investment_trade_set_destination_region (self, g_value_get_string (value));
        break;

    case PROP_COMMODITY_TYPE:
        lp_investment_trade_set_commodity_type (self, g_value_get_string (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_investment_trade_finalize (GObject *object)
{
    LpInvestmentTrade *self = LP_INVESTMENT_TRADE (object);

    g_clear_pointer (&self->source_region, g_free);
    g_clear_pointer (&self->destination_region, g_free);
    g_clear_pointer (&self->commodity_type, g_free);

    G_OBJECT_CLASS (lp_investment_trade_parent_class)->finalize (object);
}

static void
lp_investment_trade_class_init (LpInvestmentTradeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpInvestmentClass *investment_class = LP_INVESTMENT_CLASS (klass);

    object_class->get_property = lp_investment_trade_get_property;
    object_class->set_property = lp_investment_trade_set_property;
    object_class->finalize = lp_investment_trade_finalize;

    /* Override virtual methods */
    investment_class->calculate_returns = lp_investment_trade_calculate_returns;
    investment_class->apply_event = lp_investment_trade_apply_event;
    investment_class->can_sell = lp_investment_trade_can_sell;
    investment_class->get_risk_modifier = lp_investment_trade_get_risk_modifier;
    investment_class->get_base_return_rate = lp_investment_trade_get_base_return_rate;

    /**
     * LpInvestmentTrade:trade-type:
     *
     * The subtype of trade investment.
     */
    properties[PROP_TRADE_TYPE] =
        g_param_spec_enum ("trade-type",
                           "Trade Type",
                           "Type of trade investment",
                           LP_TYPE_TRADE_TYPE,
                           LP_TRADE_TYPE_ROUTE,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentTrade:route-status:
     *
     * Current status of the trade route.
     */
    properties[PROP_ROUTE_STATUS] =
        g_param_spec_enum ("route-status",
                           "Route Status",
                           "Current route status",
                           LP_TYPE_ROUTE_STATUS,
                           LP_ROUTE_STATUS_OPEN,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentTrade:market-modifier:
     *
     * Market modifier affecting returns.
     */
    properties[PROP_MARKET_MODIFIER] =
        g_param_spec_double ("market-modifier",
                             "Market Modifier",
                             "Market conditions modifier",
                             0.0, 3.0, 1.0,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentTrade:source-region:
     *
     * Source region ID for route types.
     */
    properties[PROP_SOURCE_REGION] =
        g_param_spec_string ("source-region",
                             "Source Region",
                             "Source region for trade route",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentTrade:destination-region:
     *
     * Destination region ID for route types.
     */
    properties[PROP_DESTINATION_REGION] =
        g_param_spec_string ("destination-region",
                             "Destination Region",
                             "Destination region for trade route",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpInvestmentTrade:commodity-type:
     *
     * Type of commodity being traded.
     */
    properties[PROP_COMMODITY_TYPE] =
        g_param_spec_string ("commodity-type",
                             "Commodity Type",
                             "Type of commodity being traded",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpInvestmentTrade::route-status-changed:
     * @self: the #LpInvestmentTrade
     * @old_status: previous #LpRouteStatus
     * @new_status: new #LpRouteStatus
     *
     * Emitted when the route status changes.
     */
    signals[SIGNAL_ROUTE_STATUS_CHANGED] =
        g_signal_new ("route-status-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      LP_TYPE_ROUTE_STATUS,
                      LP_TYPE_ROUTE_STATUS);
}

static void
lp_investment_trade_init (LpInvestmentTrade *self)
{
    /* Set base class properties */
    lp_investment_set_risk_level (LP_INVESTMENT (self), LP_RISK_LEVEL_MEDIUM);

    /* Initialize trade-specific fields */
    self->trade_type = LP_TRADE_TYPE_ROUTE;
    self->route_status = LP_ROUTE_STATUS_OPEN;
    self->market_modifier = 1.0;
    self->source_region = NULL;
    self->destination_region = NULL;
    self->commodity_type = NULL;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpInvestmentTrade *
lp_investment_trade_new (const gchar *id,
                         const gchar *name,
                         LpTradeType  trade_type)
{
    return g_object_new (LP_TYPE_INVESTMENT_TRADE,
                         "id", id,
                         "name", name,
                         "asset-class", LP_ASSET_CLASS_TRADE,
                         "trade-type", trade_type,
                         NULL);
}

LpInvestmentTrade *
lp_investment_trade_new_with_value (const gchar  *id,
                                    const gchar  *name,
                                    LpTradeType   trade_type,
                                    LrgBigNumber *value)
{
    LpInvestmentTrade *self;

    self = lp_investment_trade_new (id, name, trade_type);

    if (value != NULL)
    {
        lp_investment_set_purchase_price (LP_INVESTMENT (self),
                                          lrg_big_number_copy (value));
        lp_investment_set_current_value (LP_INVESTMENT (self), value);
    }

    return self;
}

/* ==========================================================================
 * Trade-Specific Methods
 * ========================================================================== */

LpTradeType
lp_investment_trade_get_trade_type (LpInvestmentTrade *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_TRADE (self), LP_TRADE_TYPE_ROUTE);

    return self->trade_type;
}

LpRouteStatus
lp_investment_trade_get_route_status (LpInvestmentTrade *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_TRADE (self), LP_ROUTE_STATUS_OPEN);

    return self->route_status;
}

void
lp_investment_trade_set_route_status (LpInvestmentTrade *self,
                                      LpRouteStatus      status)
{
    LpRouteStatus old_status;

    g_return_if_fail (LP_IS_INVESTMENT_TRADE (self));

    if (self->route_status == status)
        return;

    old_status = self->route_status;
    self->route_status = status;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ROUTE_STATUS]);
    g_signal_emit (self, signals[SIGNAL_ROUTE_STATUS_CHANGED], 0, old_status, status);

    lp_log_debug ("Trade %s: route status changed from %d to %d",
                  lp_investment_get_name (LP_INVESTMENT (self)),
                  old_status,
                  status);
}

gdouble
lp_investment_trade_get_market_modifier (LpInvestmentTrade *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_TRADE (self), 1.0);

    return self->market_modifier;
}

void
lp_investment_trade_set_market_modifier (LpInvestmentTrade *self,
                                         gdouble            modifier)
{
    g_return_if_fail (LP_IS_INVESTMENT_TRADE (self));
    g_return_if_fail (modifier >= 0.0 && modifier <= 3.0);

    if (self->market_modifier == modifier)
        return;

    self->market_modifier = modifier;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MARKET_MODIFIER]);
}

const gchar *
lp_investment_trade_get_source_region (LpInvestmentTrade *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_TRADE (self), NULL);

    return self->source_region;
}

void
lp_investment_trade_set_source_region (LpInvestmentTrade *self,
                                       const gchar       *region_id)
{
    g_return_if_fail (LP_IS_INVESTMENT_TRADE (self));

    if (g_strcmp0 (self->source_region, region_id) == 0)
        return;

    g_clear_pointer (&self->source_region, g_free);
    self->source_region = g_strdup (region_id);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SOURCE_REGION]);
}

const gchar *
lp_investment_trade_get_destination_region (LpInvestmentTrade *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_TRADE (self), NULL);

    return self->destination_region;
}

void
lp_investment_trade_set_destination_region (LpInvestmentTrade *self,
                                            const gchar       *region_id)
{
    g_return_if_fail (LP_IS_INVESTMENT_TRADE (self));

    if (g_strcmp0 (self->destination_region, region_id) == 0)
        return;

    g_clear_pointer (&self->destination_region, g_free);
    self->destination_region = g_strdup (region_id);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESTINATION_REGION]);
}

const gchar *
lp_investment_trade_get_commodity_type (LpInvestmentTrade *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_TRADE (self), NULL);

    return self->commodity_type;
}

void
lp_investment_trade_set_commodity_type (LpInvestmentTrade *self,
                                        const gchar       *commodity)
{
    g_return_if_fail (LP_IS_INVESTMENT_TRADE (self));

    if (g_strcmp0 (self->commodity_type, commodity) == 0)
        return;

    g_clear_pointer (&self->commodity_type, g_free);
    self->commodity_type = g_strdup (commodity);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMMODITY_TYPE]);
}

gboolean
lp_investment_trade_is_disrupted (LpInvestmentTrade *self)
{
    g_return_val_if_fail (LP_IS_INVESTMENT_TRADE (self), FALSE);

    return self->route_status != LP_ROUTE_STATUS_OPEN;
}
