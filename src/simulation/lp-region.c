/* lp-region.c - Geographic Region
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_SIMULATION
#include "../lp-log.h"

#include "lp-region.h"

/* Default values */
#define DEFAULT_POPULATION       (10000)
#define DEFAULT_RESOURCE_MODIFIER (1.0)

/* Geography bonus values */
#define COASTAL_TRADE_BONUS      (1.25)
#define FOREST_CONCEALMENT_BONUS (1.20)
#define SWAMP_CONCEALMENT_BONUS  (1.35)
#define MOUNTAIN_RESOURCE_BONUS  (1.15)
#define INLAND_RESOURCE_BONUS    (1.10)
#define DESERT_MAGIC_BONUS       (1.20)

/* Devastation threshold for signal emission */
#define DEVASTATION_THRESHOLD    (0.5)

struct _LpRegion
{
    GObject parent_instance;

    gchar          *id;
    gchar          *name;
    LpGeographyType geography_type;
    gchar          *owning_kingdom_id;
    guint           population;
    gdouble         resource_modifier;
    gboolean        trade_connected;
    GPtrArray      *trade_route_ids;  /* Array of gchar* */
};

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_GEOGRAPHY_TYPE,
    PROP_OWNING_KINGDOM_ID,
    PROP_POPULATION,
    PROP_RESOURCE_MODIFIER,
    PROP_TRADE_CONNECTED,
    N_PROPS
};

enum
{
    SIGNAL_OWNERSHIP_CHANGED,
    SIGNAL_DEVASTATED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations */
static void lp_region_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpRegion, lp_region, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_region_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_region_get_save_id (LrgSaveable *saveable)
{
    LpRegion *self = LP_REGION (saveable);

    return self->id;
}

static gboolean
lp_region_save (LrgSaveable    *saveable,
                LrgSaveContext *context,
                GError        **error)
{
    LpRegion *self = LP_REGION (saveable);
    guint i;

    (void)error;

    lrg_save_context_write_string (context, "id", self->id);
    lrg_save_context_write_string (context, "name", self->name);
    lrg_save_context_write_int (context, "geography-type", self->geography_type);

    if (self->owning_kingdom_id != NULL)
        lrg_save_context_write_string (context, "owning-kingdom-id", self->owning_kingdom_id);

    lrg_save_context_write_uint (context, "population", self->population);
    lrg_save_context_write_double (context, "resource-modifier", self->resource_modifier);
    lrg_save_context_write_boolean (context, "trade-connected", self->trade_connected);

    /* Save trade routes */
    lrg_save_context_write_uint (context, "trade-route-count", self->trade_route_ids->len);

    for (i = 0; i < self->trade_route_ids->len; i++)
    {
        g_autofree gchar *key = g_strdup_printf ("trade-route-%u", i);
        lrg_save_context_write_string (context, key,
                                       g_ptr_array_index (self->trade_route_ids, i));
    }

    return TRUE;
}

static gboolean
lp_region_load (LrgSaveable    *saveable,
                LrgSaveContext *context,
                GError        **error)
{
    LpRegion *self = LP_REGION (saveable);
    g_autofree gchar *kingdom_id = NULL;
    guint route_count;
    guint i;

    (void)error;

    g_clear_pointer (&self->id, g_free);
    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->owning_kingdom_id, g_free);

    self->id = lrg_save_context_read_string (context, "id", "unknown");
    self->name = lrg_save_context_read_string (context, "name", "Unknown Region");
    self->geography_type = lrg_save_context_read_int (context, "geography-type",
                                                      LP_GEOGRAPHY_TYPE_INLAND);

    kingdom_id = lrg_save_context_read_string (context, "owning-kingdom-id", NULL);
    self->owning_kingdom_id = g_steal_pointer (&kingdom_id);

    self->population = lrg_save_context_read_uint (context, "population", DEFAULT_POPULATION);
    self->resource_modifier = lrg_save_context_read_double (context, "resource-modifier",
                                                            DEFAULT_RESOURCE_MODIFIER);
    self->trade_connected = lrg_save_context_read_boolean (context, "trade-connected", FALSE);

    /* Load trade routes */
    g_ptr_array_set_size (self->trade_route_ids, 0);
    route_count = lrg_save_context_read_uint (context, "trade-route-count", 0);

    for (i = 0; i < route_count; i++)
    {
        g_autofree gchar *key = g_strdup_printf ("trade-route-%u", i);
        gchar *route_id = lrg_save_context_read_string (context, key, NULL);

        if (route_id != NULL)
            g_ptr_array_add (self->trade_route_ids, route_id);
    }

    return TRUE;
}

static void
lp_region_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_region_get_save_id;
    iface->save = lp_region_save;
    iface->load = lp_region_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_region_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    LpRegion *self = LP_REGION (object);

    switch (prop_id)
    {
    case PROP_ID:
        g_value_set_string (value, self->id);
        break;

    case PROP_NAME:
        g_value_set_string (value, self->name);
        break;

    case PROP_GEOGRAPHY_TYPE:
        g_value_set_enum (value, self->geography_type);
        break;

    case PROP_OWNING_KINGDOM_ID:
        g_value_set_string (value, self->owning_kingdom_id);
        break;

    case PROP_POPULATION:
        g_value_set_uint (value, self->population);
        break;

    case PROP_RESOURCE_MODIFIER:
        g_value_set_double (value, self->resource_modifier);
        break;

    case PROP_TRADE_CONNECTED:
        g_value_set_boolean (value, self->trade_connected);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_region_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    LpRegion *self = LP_REGION (object);

    switch (prop_id)
    {
    case PROP_ID:
        g_free (self->id);
        self->id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        lp_region_set_name (self, g_value_get_string (value));
        break;

    case PROP_GEOGRAPHY_TYPE:
        self->geography_type = g_value_get_enum (value);
        break;

    case PROP_OWNING_KINGDOM_ID:
        lp_region_set_owning_kingdom_id (self, g_value_get_string (value));
        break;

    case PROP_POPULATION:
        lp_region_set_population (self, g_value_get_uint (value));
        break;

    case PROP_RESOURCE_MODIFIER:
        lp_region_set_resource_modifier (self, g_value_get_double (value));
        break;

    case PROP_TRADE_CONNECTED:
        lp_region_set_trade_connected (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_region_finalize (GObject *object)
{
    LpRegion *self = LP_REGION (object);

    g_clear_pointer (&self->id, g_free);
    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->owning_kingdom_id, g_free);
    g_clear_pointer (&self->trade_route_ids, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_region_parent_class)->finalize (object);
}

static void
lp_region_class_init (LpRegionClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_region_get_property;
    object_class->set_property = lp_region_set_property;
    object_class->finalize = lp_region_finalize;

    /**
     * LpRegion:id:
     *
     * Unique identifier for this region.
     */
    properties[PROP_ID] =
        g_param_spec_string ("id",
                             "ID",
                             "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpRegion:name:
     *
     * Display name for this region.
     */
    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Display name",
                             "Unknown Region",
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpRegion:geography-type:
     *
     * The geography type of this region.
     */
    properties[PROP_GEOGRAPHY_TYPE] =
        g_param_spec_enum ("geography-type",
                           "Geography Type",
                           "Type of regional geography",
                           LP_TYPE_GEOGRAPHY_TYPE,
                           LP_GEOGRAPHY_TYPE_INLAND,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpRegion:owning-kingdom-id:
     *
     * ID of the kingdom that owns this region.
     */
    properties[PROP_OWNING_KINGDOM_ID] =
        g_param_spec_string ("owning-kingdom-id",
                             "Owning Kingdom ID",
                             "ID of the owning kingdom",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpRegion:population:
     *
     * Population count for this region.
     */
    properties[PROP_POPULATION] =
        g_param_spec_uint ("population",
                           "Population",
                           "Population count",
                           0, G_MAXUINT, DEFAULT_POPULATION,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpRegion:resource-modifier:
     *
     * Resource productivity modifier (1.0 = baseline).
     */
    properties[PROP_RESOURCE_MODIFIER] =
        g_param_spec_double ("resource-modifier",
                             "Resource Modifier",
                             "Resource productivity modifier",
                             0.0, 10.0, DEFAULT_RESOURCE_MODIFIER,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpRegion:trade-connected:
     *
     * Whether this region has trade connections.
     */
    properties[PROP_TRADE_CONNECTED] =
        g_param_spec_boolean ("trade-connected",
                              "Trade Connected",
                              "Whether region has trade routes",
                              FALSE,
                              G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpRegion::ownership-changed:
     * @self: the #LpRegion
     * @old_kingdom_id: (nullable): the previous owner's ID
     * @new_kingdom_id: (nullable): the new owner's ID
     *
     * Emitted when the region changes ownership.
     */
    signals[SIGNAL_OWNERSHIP_CHANGED] =
        g_signal_new ("ownership-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_STRING, G_TYPE_STRING);

    /**
     * LpRegion::devastated:
     * @self: the #LpRegion
     *
     * Emitted when the region is severely devastated.
     */
    signals[SIGNAL_DEVASTATED] =
        g_signal_new ("devastated",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_region_init (LpRegion *self)
{
    self->id = NULL;
    self->name = g_strdup ("Unknown Region");
    self->geography_type = LP_GEOGRAPHY_TYPE_INLAND;
    self->owning_kingdom_id = NULL;
    self->population = DEFAULT_POPULATION;
    self->resource_modifier = DEFAULT_RESOURCE_MODIFIER;
    self->trade_connected = FALSE;
    self->trade_route_ids = g_ptr_array_new_with_free_func (g_free);
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpRegion *
lp_region_new (const gchar    *id,
               const gchar    *name,
               LpGeographyType geography_type)
{
    return g_object_new (LP_TYPE_REGION,
                         "id", id,
                         "name", name,
                         "geography-type", geography_type,
                         NULL);
}

LpRegion *
lp_region_new_full (const gchar    *id,
                    const gchar    *name,
                    LpGeographyType geography_type,
                    guint           population,
                    gdouble         resource_modifier)
{
    return g_object_new (LP_TYPE_REGION,
                         "id", id,
                         "name", name,
                         "geography-type", geography_type,
                         "population", population,
                         "resource-modifier", resource_modifier,
                         NULL);
}

/* ==========================================================================
 * Property Accessors
 * ========================================================================== */

const gchar *
lp_region_get_id (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), NULL);

    return self->id;
}

const gchar *
lp_region_get_name (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), NULL);

    return self->name;
}

void
lp_region_set_name (LpRegion    *self,
                    const gchar *name)
{
    g_return_if_fail (LP_IS_REGION (self));

    if (g_strcmp0 (self->name, name) == 0)
        return;

    g_free (self->name);
    self->name = g_strdup (name);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}

LpGeographyType
lp_region_get_geography_type (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), LP_GEOGRAPHY_TYPE_INLAND);

    return self->geography_type;
}

const gchar *
lp_region_get_owning_kingdom_id (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), NULL);

    return self->owning_kingdom_id;
}

void
lp_region_set_owning_kingdom_id (LpRegion    *self,
                                 const gchar *kingdom_id)
{
    g_autofree gchar *old_id = NULL;

    g_return_if_fail (LP_IS_REGION (self));

    if (g_strcmp0 (self->owning_kingdom_id, kingdom_id) == 0)
        return;

    old_id = self->owning_kingdom_id;
    self->owning_kingdom_id = g_strdup (kingdom_id);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OWNING_KINGDOM_ID]);

    g_signal_emit (self, signals[SIGNAL_OWNERSHIP_CHANGED], 0, old_id, kingdom_id);

    lp_log_debug ("Region %s ownership changed: %s -> %s",
                  self->name,
                  old_id != NULL ? old_id : "(none)",
                  kingdom_id != NULL ? kingdom_id : "(none)");
}

guint
lp_region_get_population (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), 0);

    return self->population;
}

void
lp_region_set_population (LpRegion *self,
                          guint     population)
{
    g_return_if_fail (LP_IS_REGION (self));

    if (self->population == population)
        return;

    self->population = population;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POPULATION]);
}

gdouble
lp_region_get_resource_modifier (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), DEFAULT_RESOURCE_MODIFIER);

    return self->resource_modifier;
}

void
lp_region_set_resource_modifier (LpRegion *self,
                                 gdouble   modifier)
{
    g_return_if_fail (LP_IS_REGION (self));

    if (self->resource_modifier == modifier)
        return;

    self->resource_modifier = modifier;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RESOURCE_MODIFIER]);
}

gboolean
lp_region_get_trade_connected (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), FALSE);

    return self->trade_connected;
}

void
lp_region_set_trade_connected (LpRegion *self,
                               gboolean  connected)
{
    g_return_if_fail (LP_IS_REGION (self));

    connected = !!connected;

    if (self->trade_connected == connected)
        return;

    self->trade_connected = connected;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRADE_CONNECTED]);
}

/* ==========================================================================
 * Trade Routes
 * ========================================================================== */

GPtrArray *
lp_region_get_trade_route_ids (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), NULL);

    return self->trade_route_ids;
}

void
lp_region_add_trade_route (LpRegion    *self,
                           const gchar *region_id)
{
    g_return_if_fail (LP_IS_REGION (self));
    g_return_if_fail (region_id != NULL);

    /* Check if already exists */
    if (lp_region_has_trade_route_to (self, region_id))
        return;

    g_ptr_array_add (self->trade_route_ids, g_strdup (region_id));

    /* Update trade connected status */
    if (self->trade_route_ids->len > 0 && !self->trade_connected)
        lp_region_set_trade_connected (self, TRUE);

    lp_log_debug ("Region %s: added trade route to %s", self->name, region_id);
}

gboolean
lp_region_remove_trade_route (LpRegion    *self,
                              const gchar *region_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_REGION (self), FALSE);
    g_return_val_if_fail (region_id != NULL, FALSE);

    for (i = 0; i < self->trade_route_ids->len; i++)
    {
        const gchar *id = g_ptr_array_index (self->trade_route_ids, i);

        if (g_strcmp0 (id, region_id) == 0)
        {
            g_ptr_array_remove_index (self->trade_route_ids, i);

            /* Update trade connected status */
            if (self->trade_route_ids->len == 0 && self->trade_connected)
                lp_region_set_trade_connected (self, FALSE);

            lp_log_debug ("Region %s: removed trade route to %s", self->name, region_id);
            return TRUE;
        }
    }

    return FALSE;
}

gboolean
lp_region_has_trade_route_to (LpRegion    *self,
                              const gchar *region_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_REGION (self), FALSE);
    g_return_val_if_fail (region_id != NULL, FALSE);

    for (i = 0; i < self->trade_route_ids->len; i++)
    {
        const gchar *id = g_ptr_array_index (self->trade_route_ids, i);

        if (g_strcmp0 (id, region_id) == 0)
            return TRUE;
    }

    return FALSE;
}

/* ==========================================================================
 * Geography Bonuses
 * ========================================================================== */

gdouble
lp_region_get_geography_trade_bonus (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), 1.0);

    switch (self->geography_type)
    {
    case LP_GEOGRAPHY_TYPE_COASTAL:
        return COASTAL_TRADE_BONUS;

    default:
        return 1.0;
    }
}

gdouble
lp_region_get_geography_resource_bonus (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), 1.0);

    switch (self->geography_type)
    {
    case LP_GEOGRAPHY_TYPE_MOUNTAIN:
        return MOUNTAIN_RESOURCE_BONUS;

    case LP_GEOGRAPHY_TYPE_INLAND:
        return INLAND_RESOURCE_BONUS;

    case LP_GEOGRAPHY_TYPE_DESERT:
        return DESERT_MAGIC_BONUS;  /* Magical resources */

    default:
        return 1.0;
    }
}

gdouble
lp_region_get_geography_concealment_bonus (LpRegion *self)
{
    g_return_val_if_fail (LP_IS_REGION (self), 1.0);

    switch (self->geography_type)
    {
    case LP_GEOGRAPHY_TYPE_SWAMP:
        return SWAMP_CONCEALMENT_BONUS;

    case LP_GEOGRAPHY_TYPE_FOREST:
        return FOREST_CONCEALMENT_BONUS;

    default:
        return 1.0;
    }
}

/* ==========================================================================
 * Events
 * ========================================================================== */

void
lp_region_devastate (LpRegion *self,
                     gdouble   severity)
{
    guint population_loss;
    gdouble resource_loss;

    g_return_if_fail (LP_IS_REGION (self));

    severity = CLAMP (severity, 0.0, 1.0);

    /* Calculate losses */
    population_loss = (guint)(self->population * severity * 0.5);
    resource_loss = severity * 0.3;

    /* Apply losses */
    if (population_loss > 0)
    {
        guint new_pop = self->population > population_loss ?
                        self->population - population_loss : 0;
        lp_region_set_population (self, new_pop);
    }

    if (resource_loss > 0)
    {
        gdouble new_mod = self->resource_modifier - resource_loss;
        new_mod = MAX (new_mod, 0.1);  /* Don't go below 10% */
        lp_region_set_resource_modifier (self, new_mod);
    }

    lp_log_warning ("Region %s devastated (severity %.0f%%): "
                    "lost %u population, resource modifier now %.2f",
                    self->name, severity * 100.0,
                    population_loss, self->resource_modifier);

    /* Emit signal if severe */
    if (severity >= DEVASTATION_THRESHOLD)
    {
        g_signal_emit (self, signals[SIGNAL_DEVASTATED], 0);
    }
}
