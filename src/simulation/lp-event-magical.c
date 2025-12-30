/* lp-event-magical.c - Magical Event Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-event-magical.h"
#include "../investment/lp-investment.h"
#include "lp-world-simulation.h"
#include <libregnum.h>

struct _LpEventMagical
{
    LpEvent  parent_instance;

    gint     exposure_impact;
    gboolean affects_dark_investments;
};

static void lp_event_magical_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (LpEventMagical, lp_event_magical, LP_TYPE_EVENT,
                               G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                      lp_event_magical_saveable_init))

enum
{
    PROP_0,
    PROP_EXPOSURE_IMPACT,
    PROP_AFFECTS_DARK_INVESTMENTS,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Virtual method implementations
 */

static void
lp_event_magical_apply_effects (LpEvent           *event,
                                LpWorldSimulation *sim)
{
    LpEventMagical *self = LP_EVENT_MAGICAL (event);

    g_return_if_fail (LP_IS_WORLD_SIMULATION (sim));

    /*
     * Magical events affect the player's exposure level.
     * Exposure is a core mechanic representing how much
     * the mortal world suspects the lich's existence.
     */
    g_debug ("Magical event '%s' with exposure impact %d",
             lp_event_get_name (event), self->exposure_impact);

    if (self->affects_dark_investments)
    {
        g_debug ("Event affects dark investments specifically");
    }

    /* Actual exposure modification would happen via ExposureManager */
}

static gdouble
lp_event_magical_get_investment_modifier (LpEvent      *event,
                                          LpInvestment *investment)
{
    LpEventMagical *self = LP_EVENT_MAGICAL (event);
    LpAssetClass asset_class;
    gdouble modifier = 1.0;

    g_return_val_if_fail (LP_IS_INVESTMENT (investment), 1.0);

    asset_class = lp_investment_get_asset_class (investment);

    /*
     * Magical events primarily affect magical and dark investments.
     * Divine intervention can harm dark investments.
     * Artifact discoveries can boost magical investments.
     */
    if (self->affects_dark_investments && asset_class == LP_ASSET_CLASS_DARK)
    {
        /* Dark investments affected - could be good or bad */
        if (self->exposure_impact > 0)
        {
            /* Increased exposure is bad for dark investments */
            modifier = 0.6;
        }
        else
        {
            /* Decreased exposure benefits dark investments */
            modifier = 1.4;
        }
    }
    else if (asset_class == LP_ASSET_CLASS_MAGICAL)
    {
        /* Magical investments always respond to magical events */
        if (self->exposure_impact > 20)
        {
            /* Major magical disturbance */
            modifier = 0.8;
        }
        else if (self->exposure_impact < -20)
        {
            /* Magical concealment benefits */
            modifier = 1.3;
        }
    }

    return modifier;
}

static gchar *
lp_event_magical_get_narrative_text (LpEvent *event)
{
    LpEventMagical *self = LP_EVENT_MAGICAL (event);
    const gchar *name;
    const gchar *description;
    const gchar *arcane_impact;

    name = lp_event_get_name (event);
    description = lp_event_get_description (event);

    if (self->exposure_impact > 30)
        arcane_impact = "The veil between worlds grows thin - mortals sense dark powers";
    else if (self->exposure_impact > 10)
        arcane_impact = "Whispers of sorcery spread through the land";
    else if (self->exposure_impact < -30)
        arcane_impact = "A shroud of forgetfulness descends upon the realm";
    else if (self->exposure_impact < -10)
        arcane_impact = "The mundane world remains blissfully ignorant";
    else
        arcane_impact = "The currents of magic shift imperceptibly";

    if (self->affects_dark_investments)
    {
        return g_strdup_printf ("%s\n\n%s\n\n%s\n\nYour dark investments tremble...",
                                name ? name : "Magical Event",
                                description ? description : "",
                                arcane_impact);
    }

    return g_strdup_printf ("%s\n\n%s\n\n%s",
                            name ? name : "Magical Event",
                            description ? description : "",
                            arcane_impact);
}

/*
 * LrgSaveable interface
 */

static const gchar *
lp_event_magical_get_save_id (LrgSaveable *saveable)
{
    return lp_event_get_id (LP_EVENT (saveable));
}

static gboolean
lp_event_magical_save (LrgSaveable    *saveable,
                       LrgSaveContext *ctx,
                       GError        **error)
{
    LpEventMagical *self = LP_EVENT_MAGICAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Save parent class data first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->save != NULL)
    {
        if (!parent_iface->save (saveable, ctx, error))
            return FALSE;
    }

    /* Save magical-specific data */
    lrg_save_context_write_int (ctx, "exposure-impact", self->exposure_impact);
    lrg_save_context_write_boolean (ctx, "affects-dark-investments",
                                    self->affects_dark_investments);

    return TRUE;
}

static gboolean
lp_event_magical_load (LrgSaveable    *saveable,
                       LrgSaveContext *ctx,
                       GError        **error)
{
    LpEventMagical *self = LP_EVENT_MAGICAL (saveable);
    LrgSaveableInterface *parent_iface;

    /* Load parent class data first */
    parent_iface = g_type_interface_peek_parent (
        g_type_interface_peek (G_OBJECT_GET_CLASS (self), LRG_TYPE_SAVEABLE));
    if (parent_iface != NULL && parent_iface->load != NULL)
    {
        if (!parent_iface->load (saveable, ctx, error))
            return FALSE;
    }

    /* Load magical-specific data */
    self->exposure_impact = lrg_save_context_read_int (ctx, "exposure-impact", 0);
    self->affects_dark_investments = lrg_save_context_read_boolean (
        ctx, "affects-dark-investments", FALSE);

    return TRUE;
}

static void
lp_event_magical_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_event_magical_get_save_id;
    iface->save = lp_event_magical_save;
    iface->load = lp_event_magical_load;
}

/*
 * GObject implementation
 */

static void
lp_event_magical_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    LpEventMagical *self = LP_EVENT_MAGICAL (object);

    switch (prop_id)
    {
    case PROP_EXPOSURE_IMPACT:
        g_value_set_int (value, self->exposure_impact);
        break;

    case PROP_AFFECTS_DARK_INVESTMENTS:
        g_value_set_boolean (value, self->affects_dark_investments);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_magical_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    LpEventMagical *self = LP_EVENT_MAGICAL (object);

    switch (prop_id)
    {
    case PROP_EXPOSURE_IMPACT:
        lp_event_magical_set_exposure_impact (self, g_value_get_int (value));
        break;

    case PROP_AFFECTS_DARK_INVESTMENTS:
        lp_event_magical_set_affects_dark_investments (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_event_magical_class_init (LpEventMagicalClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LpEventClass *event_class = LP_EVENT_CLASS (klass);

    object_class->get_property = lp_event_magical_get_property;
    object_class->set_property = lp_event_magical_set_property;

    /* Override virtual methods */
    event_class->apply_effects = lp_event_magical_apply_effects;
    event_class->get_investment_modifier = lp_event_magical_get_investment_modifier;
    event_class->get_narrative_text = lp_event_magical_get_narrative_text;

    /**
     * LpEventMagical:exposure-impact:
     *
     * The exposure impact from this event.
     * Positive values increase exposure, negative decrease it.
     */
    properties[PROP_EXPOSURE_IMPACT] =
        g_param_spec_int ("exposure-impact",
                          "Exposure Impact",
                          "Change to exposure level",
                          -100, 100, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpEventMagical:affects-dark-investments:
     *
     * Whether this event specifically affects dark investments.
     */
    properties[PROP_AFFECTS_DARK_INVESTMENTS] =
        g_param_spec_boolean ("affects-dark-investments",
                              "Affects Dark Investments",
                              "Whether dark investments are specifically affected",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_event_magical_init (LpEventMagical *self)
{
    self->exposure_impact = 0;
    self->affects_dark_investments = FALSE;
}

/*
 * Public API
 */

/**
 * lp_event_magical_new:
 * @id: unique identifier
 * @name: display name
 *
 * Creates a new magical event.
 *
 * Returns: (transfer full): A new #LpEventMagical
 */
LpEventMagical *
lp_event_magical_new (const gchar *id,
                      const gchar *name)
{
    return g_object_new (LP_TYPE_EVENT_MAGICAL,
                         "id", id,
                         "name", name,
                         "event-type", LP_EVENT_TYPE_MAGICAL,
                         NULL);
}

/**
 * lp_event_magical_get_exposure_impact:
 * @self: an #LpEventMagical
 *
 * Gets the exposure impact from this event.
 *
 * Returns: Exposure change (can be negative)
 */
gint
lp_event_magical_get_exposure_impact (LpEventMagical *self)
{
    g_return_val_if_fail (LP_IS_EVENT_MAGICAL (self), 0);

    return self->exposure_impact;
}

/**
 * lp_event_magical_set_exposure_impact:
 * @self: an #LpEventMagical
 * @impact: exposure change
 *
 * Sets the exposure impact.
 */
void
lp_event_magical_set_exposure_impact (LpEventMagical *self,
                                      gint            impact)
{
    g_return_if_fail (LP_IS_EVENT_MAGICAL (self));

    impact = CLAMP (impact, -100, 100);

    if (self->exposure_impact != impact)
    {
        self->exposure_impact = impact;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EXPOSURE_IMPACT]);
    }
}

/**
 * lp_event_magical_get_affects_dark_investments:
 * @self: an #LpEventMagical
 *
 * Gets whether this event affects dark investments specifically.
 *
 * Returns: %TRUE if affects dark investments
 */
gboolean
lp_event_magical_get_affects_dark_investments (LpEventMagical *self)
{
    g_return_val_if_fail (LP_IS_EVENT_MAGICAL (self), FALSE);

    return self->affects_dark_investments;
}

/**
 * lp_event_magical_set_affects_dark_investments:
 * @self: an #LpEventMagical
 * @affects: whether affects dark investments
 *
 * Sets whether this event affects dark investments.
 */
void
lp_event_magical_set_affects_dark_investments (LpEventMagical *self,
                                               gboolean        affects)
{
    g_return_if_fail (LP_IS_EVENT_MAGICAL (self));

    affects = !!affects;

    if (self->affects_dark_investments != affects)
    {
        self->affects_dark_investments = affects;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AFFECTS_DARK_INVESTMENTS]);
    }
}
