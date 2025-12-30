/* lp-event-political.c - Political Event Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-event-political.h"
#include "lp-kingdom.h"
#include "../investment/lp-investment.h"
#include "lp-world-simulation.h"
#include <libregnum.h>

struct _LpEventPolitical
{
    LpEvent  parent_instance;

    gint     stability_impact;
    gboolean causes_war;
};

static void lp_event_political_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (LpEventPolitical, lp_event_political, LP_TYPE_EVENT,
                               G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                      lp_event_political_saveable_init))

enum
{
    PROP_0,
    PROP_STABILITY_IMPACT,
    PROP_CAUSES_WAR,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Virtual method implementations
 */

static void
lp_event_political_apply_effects (LpEvent           *event,
                                  LpWorldSimulation *sim)
{
    LpEventPolitical *self = LP_EVENT_POLITICAL (event);
    const gchar *kingdom_id;

    g_return_if_fail (LP_IS_WORLD_SIMULATION (sim));

    kingdom_id = lp_event_get_affects_kingdom_id (event);

    /*
     * Political events primarily affect kingdom stability.
     * The actual kingdom modification would be done through
     * the world simulation's kingdom lookup.
     */
    if (kingdom_id != NULL)
    {
        g_debug ("Political event '%s' affecting kingdom '%s' with stability impact %d",
                 lp_event_get_name (event), kingdom_id, self->stability_impact);

        /* Actual kingdom modification would happen here via simulation */
    }

    if (self->causes_war)
    {
        g_debug ("Political event '%s' triggers war declaration", lp_event_get_name (event));
    }
}

static gdouble
lp_event_political_get_investment_modifier (LpEvent      *event,
                                            LpInvestment *investment)
{
    LpEventPolitical *self = LP_EVENT_POLITICAL (event);
    LpAssetClass asset_class;
    gdouble modifier = 1.0;

    g_return_val_if_fail (LP_IS_INVESTMENT (investment), 1.0);

    asset_class = lp_investment_get_asset_class (investment);

    /*
     * Political instability affects trade and property investments.
     * Wars severely impact all investments in affected regions.
     */
    if (self->causes_war)
    {
        /* War is devastating for investments */
        modifier = 0.5;
    }
    else if (self->stability_impact < -20)
    {
        /* Major instability */
        if (asset_class == LP_ASSET_CLASS_TRADE ||
            asset_class == LP_ASSET_CLASS_PROPERTY)
        {
            modifier = 0.7;
        }
        else if (asset_class == LP_ASSET_CLASS_POLITICAL)
        {
            /* Political investments thrive in instability */
            modifier = 1.3;
        }
    }
    else if (self->stability_impact > 20)
    {
        /* Increased stability benefits trade */
        if (asset_class == LP_ASSET_CLASS_TRADE)
            modifier = 1.2;
    }

    return modifier;
}

static gchar *
lp_event_political_get_narrative_text (LpEvent *event)
{
    LpEventPolitical *self = LP_EVENT_POLITICAL (event);
    const gchar *name;
    const gchar *description;
    const gchar *consequence;

    name = lp_event_get_name (event);
    description = lp_event_get_description (event);

    if (self->causes_war)
        consequence = "The drums of war thunder across the land";
    else if (self->stability_impact < -30)
        consequence = "The foundations of power crumble";
    else if (self->stability_impact < -10)
        consequence = "Unrest spreads through the populace";
    else if (self->stability_impact > 30)
        consequence = "A new era of peace dawns";
    else if (self->stability_impact > 10)
        consequence = "Order is restored to the realm";
    else
        consequence = "The political landscape shifts subtly";

    return g_strdup_printf ("%s\n\n%s\n\n%s",
                            name ? name : "Political Event",
                            description ? description : "",
                            consequence);
}

/*
 * LrgSaveable interface
 */

static const gchar *
lp_event_political_get_save_id (LrgSaveable *saveable)
{
    return lp_event_get_id (LP_EVENT (saveable));
}

static gboolean
lp_event_political_save (LrgSaveable    *saveable,
                         LrgSaveContext *ctx,
                         GError        **error)
{
    LpEventPolitical *self = LP_EVENT_POLITICAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Save parent class data first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->save != NULL)
    {
        if (!parent_iface->save (saveable, ctx, error))
            return FALSE;
    }

    /* Save political-specific data */
    lrg_save_context_write_int (ctx, "stability-impact", self->stability_impact);
    lrg_save_context_write_boolean (ctx, "causes-war", self->causes_war);

    return TRUE;
}

static gboolean
lp_event_political_load (LrgSaveable    *saveable,
                         LrgSaveContext *ctx,
                         GError        **error)
{
    LpEventPolitical *self = LP_EVENT_POLITICAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Load parent class data first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->load != NULL)
    {
        if (!parent_iface->load (saveable, ctx, error))
            return FALSE;
    }

    /* Load political-specific data */
    self->stability_impact = lrg_save_context_read_int (ctx, "stability-impact", 0);
    self->causes_war = lrg_save_context_read_boolean (ctx, "causes-war", FALSE);

    return TRUE;
}

static void
lp_event_political_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_event_political_get_save_id;
    iface->save = lp_event_political_save;
    iface->load = lp_event_political_load;
}

/*
 * GObject implementation
 */

static void
lp_event_political_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    LpEventPolitical *self = LP_EVENT_POLITICAL (object);

    switch (prop_id)
    {
    case PROP_STABILITY_IMPACT:
        g_value_set_int (value, self->stability_impact);
        break;

    case PROP_CAUSES_WAR:
        g_value_set_boolean (value, self->causes_war);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_political_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    LpEventPolitical *self = LP_EVENT_POLITICAL (object);

    switch (prop_id)
    {
    case PROP_STABILITY_IMPACT:
        lp_event_political_set_stability_impact (self, g_value_get_int (value));
        break;

    case PROP_CAUSES_WAR:
        lp_event_political_set_causes_war (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_political_class_init (LpEventPoliticalClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpEventClass *event_class = LP_EVENT_CLASS (klass);

    object_class->get_property = lp_event_political_get_property;
    object_class->set_property = lp_event_political_set_property;

    /* Override virtual methods */
    event_class->apply_effects = lp_event_political_apply_effects;
    event_class->get_investment_modifier = lp_event_political_get_investment_modifier;
    event_class->get_narrative_text = lp_event_political_get_narrative_text;

    /**
     * LpEventPolitical:stability-impact:
     *
     * The stability impact on affected kingdoms.
     * Negative values destabilize, positive values stabilize.
     */
    properties[PROP_STABILITY_IMPACT] =
        g_param_spec_int ("stability-impact",
                          "Stability Impact",
                          "Stability change for affected kingdoms",
                          -100, 100, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpEventPolitical:causes-war:
     *
     * Whether this event causes a war.
     */
    properties[PROP_CAUSES_WAR] =
        g_param_spec_boolean ("causes-war",
                              "Causes War",
                              "Whether this event triggers war",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_event_political_init (LpEventPolitical *self)
{
    self->stability_impact = 0;
    self->causes_war = FALSE;
}

/*
 * Public API
 */

/**
 * lp_event_political_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new political event.
 *
 * Returns: (transfer full): A new #LpEventPolitical
 */
LpEventPolitical *
lp_event_political_new (const gchar *id,
                        const gchar *name)
{
    return g_object_new (LP_TYPE_EVENT_POLITICAL,
                         "id", id,
                         "name", name,
                         "event-type", LP_EVENT_TYPE_POLITICAL,
                         NULL);
}

/**
 * lp_event_political_get_stability_impact:
 * @self: an #LpEventPolitical
 *
 * Gets the stability impact on affected kingdoms.
 *
 * Returns: Stability change (can be negative)
 */
gint
lp_event_political_get_stability_impact (LpEventPolitical *self)
{
    g_return_val_if_fail (LP_IS_EVENT_POLITICAL (self), 0);

    return self->stability_impact;
}

/**
 * lp_event_political_set_stability_impact:
 * @self: an #LpEventPolitical
 * @impact: stability change
 *
 * Sets the stability impact.
 */
void
lp_event_political_set_stability_impact (LpEventPolitical *self,
                                         gint              impact)
{
    g_return_if_fail (LP_IS_EVENT_POLITICAL (self));

    impact = CLAMP (impact, -100, 100);

    if (self->stability_impact != impact)
    {
        self->stability_impact = impact;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STABILITY_IMPACT]);
    }
}

/**
 * lp_event_political_get_causes_war:
 * @self: an #LpEventPolitical
 *
 * Gets whether this event causes a war.
 *
 * Returns: %TRUE if causes war
 */
gboolean
lp_event_political_get_causes_war (LpEventPolitical *self)
{
    g_return_val_if_fail (LP_IS_EVENT_POLITICAL (self), FALSE);

    return self->causes_war;
}

/**
 * lp_event_political_set_causes_war:
 * @self: an #LpEventPolitical
 * @causes_war: whether causes war
 *
 * Sets whether this event causes a war.
 */
void
lp_event_political_set_causes_war (LpEventPolitical *self,
                                   gboolean          causes_war)
{
    g_return_if_fail (LP_IS_EVENT_POLITICAL (self));

    causes_war = !!causes_war;

    if (self->causes_war != causes_war)
    {
        self->causes_war = causes_war;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CAUSES_WAR]);
    }
}
