/* lp-event-economic.c - Economic Event Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-event-economic.h"
#include "../investment/lp-investment.h"
#include <libregnum.h>

struct _LpEventEconomic
{
    LpEvent  parent_instance;

    gdouble  market_modifier;
    gint     affected_asset_class;
};

static void lp_event_economic_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (LpEventEconomic, lp_event_economic, LP_TYPE_EVENT,
                               G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                      lp_event_economic_saveable_init))

enum
{
    PROP_0,
    PROP_MARKET_MODIFIER,
    PROP_AFFECTED_ASSET_CLASS,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Virtual method implementations
 */

static gdouble
lp_event_economic_get_investment_modifier (LpEvent      *event,
                                           LpInvestment *investment)
{
    LpEventEconomic *self = LP_EVENT_ECONOMIC (event);
    LpAssetClass asset_class;

    g_return_val_if_fail (LP_IS_INVESTMENT (investment), 1.0);

    /*
     * If this event affects all asset classes (-1) or the specific
     * asset class of the investment, apply the modifier.
     */
    asset_class = lp_investment_get_asset_class (investment);

    if (self->affected_asset_class == -1 ||
        self->affected_asset_class == (gint)asset_class)
    {
        return self->market_modifier;
    }

    /* No effect on this investment */
    return 1.0;
}

static gchar *
lp_event_economic_get_narrative_text (LpEvent *event)
{
    LpEventEconomic *self = LP_EVENT_ECONOMIC (event);
    const gchar *name;
    const gchar *description;
    const gchar *impact_desc;

    name = lp_event_get_name (event);
    description = lp_event_get_description (event);

    if (self->market_modifier > 1.2)
        impact_desc = "Markets surge with opportunity";
    else if (self->market_modifier > 1.0)
        impact_desc = "Markets show modest gains";
    else if (self->market_modifier > 0.8)
        impact_desc = "Markets experience minor turbulence";
    else
        impact_desc = "Markets plunge into crisis";

    return g_strdup_printf ("%s\n\n%s\n\n%s (%.0f%% modifier)",
                            name ? name : "Economic Event",
                            description ? description : "",
                            impact_desc,
                            self->market_modifier * 100.0);
}

/*
 * LrgSaveable interface
 */

static const gchar *
lp_event_economic_get_save_id (LrgSaveable *saveable)
{
    return lp_event_get_id (LP_EVENT (saveable));
}

static gboolean
lp_event_economic_save (LrgSaveable    *saveable,
                        LrgSaveContext *ctx,
                        GError        **error)
{
    LpEventEconomic *self = LP_EVENT_ECONOMIC (saveable);
    LrgSaveableInterface *parent_iface;

    /* Save parent class data first by calling base LpEvent's save */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->save != NULL)
    {
        if (!parent_iface->save (saveable, ctx, error))
            return FALSE;
    }

    /* Save economic-specific data */
    lrg_save_context_write_double (ctx, "market-modifier", self->market_modifier);
    lrg_save_context_write_int (ctx, "affected-asset-class", self->affected_asset_class);

    return TRUE;
}

static gboolean
lp_event_economic_load (LrgSaveable    *saveable,
                        LrgSaveContext *ctx,
                        GError        **error)
{
    LpEventEconomic *self = LP_EVENT_ECONOMIC (saveable);
    LrgSaveableInterface *parent_iface;

    /* Load parent class data first by calling base LpEvent's load */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->load != NULL)
    {
        if (!parent_iface->load (saveable, ctx, error))
            return FALSE;
    }

    /* Load economic-specific data */
    self->market_modifier = lrg_save_context_read_double (ctx, "market-modifier", 1.0);
    self->affected_asset_class = lrg_save_context_read_int (ctx, "affected-asset-class", -1);

    return TRUE;
}

static void
lp_event_economic_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_event_economic_get_save_id;
    iface->save = lp_event_economic_save;
    iface->load = lp_event_economic_load;
}

/*
 * GObject implementation
 */

static void
lp_event_economic_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    LpEventEconomic *self = LP_EVENT_ECONOMIC (object);

    switch (prop_id)
    {
    case PROP_MARKET_MODIFIER:
        g_value_set_double (value, self->market_modifier);
        break;

    case PROP_AFFECTED_ASSET_CLASS:
        g_value_set_int (value, self->affected_asset_class);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_economic_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    LpEventEconomic *self = LP_EVENT_ECONOMIC (object);

    switch (prop_id)
    {
    case PROP_MARKET_MODIFIER:
        lp_event_economic_set_market_modifier (self, g_value_get_double (value));
        break;

    case PROP_AFFECTED_ASSET_CLASS:
        lp_event_economic_set_affected_asset_class (self, g_value_get_int (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_economic_class_init (LpEventEconomicClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpEventClass *event_class = LP_EVENT_CLASS (klass);

    object_class->get_property = lp_event_economic_get_property;
    object_class->set_property = lp_event_economic_set_property;

    /* Override virtual methods */
    event_class->get_investment_modifier = lp_event_economic_get_investment_modifier;
    event_class->get_narrative_text = lp_event_economic_get_narrative_text;

    /**
     * LpEventEconomic:market-modifier:
     *
     * The market-wide modifier from this event.
     * A value of 1.0 means no change, >1.0 is a boost, <1.0 is a penalty.
     */
    properties[PROP_MARKET_MODIFIER] =
        g_param_spec_double ("market-modifier",
                             "Market Modifier",
                             "Market-wide return modifier",
                             0.0, 10.0, 1.0,
                             G_PARAM_READWRITE |
                             G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpEventEconomic:affected-asset-class:
     *
     * The asset class primarily affected by this event.
     * A value of -1 means all asset classes are affected.
     */
    properties[PROP_AFFECTED_ASSET_CLASS] =
        g_param_spec_int ("affected-asset-class",
                          "Affected Asset Class",
                          "Asset class affected (-1 for all)",
                          -1, G_MAXINT, -1,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_event_economic_init (LpEventEconomic *self)
{
    self->market_modifier = 1.0;
    self->affected_asset_class = -1;
}

/*
 * Public API
 */

/**
 * lp_event_economic_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new economic event.
 *
 * Returns: (transfer full): A new #LpEventEconomic
 */
LpEventEconomic *
lp_event_economic_new (const gchar *id,
                       const gchar *name)
{
    return g_object_new (LP_TYPE_EVENT_ECONOMIC,
                         "id", id,
                         "name", name,
                         "event-type", LP_EVENT_TYPE_ECONOMIC,
                         NULL);
}

/**
 * lp_event_economic_get_market_modifier:
 * @self: an #LpEventEconomic
 *
 * Gets the market-wide modifier from this event.
 *
 * Returns: Market modifier (1.0 = no change)
 */
gdouble
lp_event_economic_get_market_modifier (LpEventEconomic *self)
{
    g_return_val_if_fail (LP_IS_EVENT_ECONOMIC (self), 1.0);

    return self->market_modifier;
}

/**
 * lp_event_economic_set_market_modifier:
 * @self: an #LpEventEconomic
 * @modifier: the market modifier
 *
 * Sets the market-wide modifier.
 */
void
lp_event_economic_set_market_modifier (LpEventEconomic *self,
                                       gdouble          modifier)
{
    g_return_if_fail (LP_IS_EVENT_ECONOMIC (self));

    if (self->market_modifier != modifier)
    {
        self->market_modifier = modifier;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MARKET_MODIFIER]);
    }
}

/**
 * lp_event_economic_get_affected_asset_class:
 * @self: an #LpEventEconomic
 *
 * Gets the asset class primarily affected by this event.
 *
 * Returns: The #LpAssetClass, or -1 if all classes affected
 */
gint
lp_event_economic_get_affected_asset_class (LpEventEconomic *self)
{
    g_return_val_if_fail (LP_IS_EVENT_ECONOMIC (self), -1);

    return self->affected_asset_class;
}

/**
 * lp_event_economic_set_affected_asset_class:
 * @self: an #LpEventEconomic
 * @asset_class: the #LpAssetClass, or -1 for all
 *
 * Sets which asset class is affected.
 */
void
lp_event_economic_set_affected_asset_class (LpEventEconomic *self,
                                            gint             asset_class)
{
    g_return_if_fail (LP_IS_EVENT_ECONOMIC (self));

    if (self->affected_asset_class != asset_class)
    {
        self->affected_asset_class = asset_class;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AFFECTED_ASSET_CLASS]);
    }
}
