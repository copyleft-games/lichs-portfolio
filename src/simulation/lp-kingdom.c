/* lp-kingdom.c - Political Kingdom
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_SIMULATION
#include "../lp-log.h"

#include "lp-kingdom.h"

/* Default attribute values */
#define DEFAULT_ATTRIBUTE_VALUE (50)
#define MIN_ATTRIBUTE           (0)
#define MAX_ATTRIBUTE           (100)

/* Collapse thresholds */
#define COLLAPSE_THRESHOLD      (10)   /* Stability below this triggers roll */
#define COLLAPSE_BASE_CHANCE    (0.05) /* 5% base collapse chance */

/* War thresholds */
#define WAR_MILITARY_THRESHOLD  (60)   /* Military above this considers war */
#define WAR_BASE_CHANCE         (0.02) /* 2% base war chance */

/* Crusade thresholds */
#define CRUSADE_TOLERANCE_THRESHOLD (30) /* Tolerance below this triggers roll */
#define CRUSADE_BASE_CHANCE     (0.01)   /* 1% base crusade chance */

/* Yearly drift */
#define YEARLY_ATTRIBUTE_DRIFT  (2)    /* Max +/- per year */

struct _LpKingdom
{
    GObject parent_instance;

    gchar *id;
    gchar *name;

    /* Core attributes (0-100) */
    gint stability;
    gint prosperity;
    gint military;
    gint culture;
    gint tolerance;

    /* State */
    gchar   *ruler_name;
    guint    dynasty_years;
    gboolean is_collapsed;
    gchar   *at_war_with_id;

    /* Collections */
    GPtrArray  *region_ids;  /* Array of gchar* */
    GHashTable *relations;   /* kingdom_id -> GINT_TO_POINTER(LpKingdomRelation) */
};

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_STABILITY,
    PROP_PROSPERITY,
    PROP_MILITARY,
    PROP_CULTURE,
    PROP_TOLERANCE,
    PROP_RULER_NAME,
    PROP_DYNASTY_YEARS,
    PROP_IS_COLLAPSED,
    PROP_AT_WAR_WITH_ID,
    N_PROPS
};

enum
{
    SIGNAL_ATTRIBUTE_CHANGED,
    SIGNAL_COLLAPSED,
    SIGNAL_WAR_DECLARED,
    SIGNAL_WAR_ENDED,
    SIGNAL_CRUSADE_LAUNCHED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations */
static void lp_kingdom_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpKingdom, lp_kingdom, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_kingdom_saveable_init))

/* ==========================================================================
 * Helper Functions
 * ========================================================================== */

static gint
clamp_attribute (gint value)
{
    return CLAMP (value, MIN_ATTRIBUTE, MAX_ATTRIBUTE);
}

static void
emit_attribute_changed (LpKingdom   *self,
                        const gchar *attr_name,
                        gint         old_value,
                        gint         new_value)
{
    if (old_value != new_value)
    {
        g_signal_emit (self, signals[SIGNAL_ATTRIBUTE_CHANGED], 0,
                       attr_name, old_value, new_value);
    }
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_kingdom_get_save_id (LrgSaveable *saveable)
{
    LpKingdom *self = LP_KINGDOM (saveable);

    return self->id;
}

static gboolean
lp_kingdom_save (LrgSaveable    *saveable,
                 LrgSaveContext *context,
                 GError        **error)
{
    LpKingdom *self = LP_KINGDOM (saveable);
    GHashTableIter iter;
    gpointer key, value;
    guint i;
    guint relation_idx;

    (void)error;

    /* Core properties */
    lrg_save_context_write_string (context, "id", self->id);
    lrg_save_context_write_string (context, "name", self->name);

    /* Attributes */
    lrg_save_context_write_int (context, "stability", self->stability);
    lrg_save_context_write_int (context, "prosperity", self->prosperity);
    lrg_save_context_write_int (context, "military", self->military);
    lrg_save_context_write_int (context, "culture", self->culture);
    lrg_save_context_write_int (context, "tolerance", self->tolerance);

    /* State */
    if (self->ruler_name != NULL)
        lrg_save_context_write_string (context, "ruler-name", self->ruler_name);
    lrg_save_context_write_uint (context, "dynasty-years", self->dynasty_years);
    lrg_save_context_write_boolean (context, "is-collapsed", self->is_collapsed);
    if (self->at_war_with_id != NULL)
        lrg_save_context_write_string (context, "at-war-with-id", self->at_war_with_id);

    /* Region IDs */
    lrg_save_context_write_uint (context, "region-count", self->region_ids->len);
    for (i = 0; i < self->region_ids->len; i++)
    {
        g_autofree gchar *key_str = g_strdup_printf ("region-%u", i);
        lrg_save_context_write_string (context, key_str,
                                       g_ptr_array_index (self->region_ids, i));
    }

    /* Relations */
    lrg_save_context_write_uint (context, "relation-count",
                                 g_hash_table_size (self->relations));
    relation_idx = 0;
    g_hash_table_iter_init (&iter, self->relations);
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        g_autofree gchar *key_str = g_strdup_printf ("relation-%u-kingdom", relation_idx);
        g_autofree gchar *val_str = g_strdup_printf ("relation-%u-type", relation_idx);

        lrg_save_context_write_string (context, key_str, (const gchar *)key);
        lrg_save_context_write_int (context, val_str, GPOINTER_TO_INT (value));
        relation_idx++;
    }

    return TRUE;
}

static gboolean
lp_kingdom_load (LrgSaveable    *saveable,
                 LrgSaveContext *context,
                 GError        **error)
{
    LpKingdom *self = LP_KINGDOM (saveable);
    guint region_count;
    guint relation_count;
    guint i;

    (void)error;

    /* Clear existing data */
    g_clear_pointer (&self->id, g_free);
    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->ruler_name, g_free);
    g_clear_pointer (&self->at_war_with_id, g_free);
    g_ptr_array_set_size (self->region_ids, 0);
    g_hash_table_remove_all (self->relations);

    /* Core properties */
    self->id = lrg_save_context_read_string (context, "id", "unknown");
    self->name = lrg_save_context_read_string (context, "name", "Unknown Kingdom");

    /* Attributes */
    self->stability = lrg_save_context_read_int (context, "stability", DEFAULT_ATTRIBUTE_VALUE);
    self->prosperity = lrg_save_context_read_int (context, "prosperity", DEFAULT_ATTRIBUTE_VALUE);
    self->military = lrg_save_context_read_int (context, "military", DEFAULT_ATTRIBUTE_VALUE);
    self->culture = lrg_save_context_read_int (context, "culture", DEFAULT_ATTRIBUTE_VALUE);
    self->tolerance = lrg_save_context_read_int (context, "tolerance", DEFAULT_ATTRIBUTE_VALUE);

    /* State */
    self->ruler_name = lrg_save_context_read_string (context, "ruler-name", NULL);
    self->dynasty_years = lrg_save_context_read_uint (context, "dynasty-years", 0);
    self->is_collapsed = lrg_save_context_read_boolean (context, "is-collapsed", FALSE);
    self->at_war_with_id = lrg_save_context_read_string (context, "at-war-with-id", NULL);

    /* Region IDs */
    region_count = lrg_save_context_read_uint (context, "region-count", 0);
    for (i = 0; i < region_count; i++)
    {
        g_autofree gchar *key_str = g_strdup_printf ("region-%u", i);
        gchar *region_id = lrg_save_context_read_string (context, key_str, NULL);

        if (region_id != NULL)
            g_ptr_array_add (self->region_ids, region_id);
    }

    /* Relations */
    relation_count = lrg_save_context_read_uint (context, "relation-count", 0);
    for (i = 0; i < relation_count; i++)
    {
        g_autofree gchar *key_str = g_strdup_printf ("relation-%u-kingdom", i);
        g_autofree gchar *val_str = g_strdup_printf ("relation-%u-type", i);
        gchar *kingdom_id = lrg_save_context_read_string (context, key_str, NULL);
        gint relation_type = lrg_save_context_read_int (context, val_str,
                                                        LP_KINGDOM_RELATION_NEUTRAL);

        if (kingdom_id != NULL)
        {
            g_hash_table_insert (self->relations, kingdom_id,
                                 GINT_TO_POINTER (relation_type));
        }
    }

    return TRUE;
}

static void
lp_kingdom_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_kingdom_get_save_id;
    iface->save = lp_kingdom_save;
    iface->load = lp_kingdom_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_kingdom_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    LpKingdom *self = LP_KINGDOM (object);

    switch (prop_id)
    {
    case PROP_ID:
        g_value_set_string (value, self->id);
        break;

    case PROP_NAME:
        g_value_set_string (value, self->name);
        break;

    case PROP_STABILITY:
        g_value_set_int (value, self->stability);
        break;

    case PROP_PROSPERITY:
        g_value_set_int (value, self->prosperity);
        break;

    case PROP_MILITARY:
        g_value_set_int (value, self->military);
        break;

    case PROP_CULTURE:
        g_value_set_int (value, self->culture);
        break;

    case PROP_TOLERANCE:
        g_value_set_int (value, self->tolerance);
        break;

    case PROP_RULER_NAME:
        g_value_set_string (value, self->ruler_name);
        break;

    case PROP_DYNASTY_YEARS:
        g_value_set_uint (value, self->dynasty_years);
        break;

    case PROP_IS_COLLAPSED:
        g_value_set_boolean (value, self->is_collapsed);
        break;

    case PROP_AT_WAR_WITH_ID:
        g_value_set_string (value, self->at_war_with_id);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_kingdom_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    LpKingdom *self = LP_KINGDOM (object);

    switch (prop_id)
    {
    case PROP_ID:
        g_free (self->id);
        self->id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        lp_kingdom_set_name (self, g_value_get_string (value));
        break;

    case PROP_STABILITY:
        lp_kingdom_set_stability (self, g_value_get_int (value));
        break;

    case PROP_PROSPERITY:
        lp_kingdom_set_prosperity (self, g_value_get_int (value));
        break;

    case PROP_MILITARY:
        lp_kingdom_set_military (self, g_value_get_int (value));
        break;

    case PROP_CULTURE:
        lp_kingdom_set_culture (self, g_value_get_int (value));
        break;

    case PROP_TOLERANCE:
        lp_kingdom_set_tolerance (self, g_value_get_int (value));
        break;

    case PROP_RULER_NAME:
        lp_kingdom_set_ruler_name (self, g_value_get_string (value));
        break;

    case PROP_DYNASTY_YEARS:
        lp_kingdom_set_dynasty_years (self, g_value_get_uint (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_kingdom_finalize (GObject *object)
{
    LpKingdom *self = LP_KINGDOM (object);

    g_clear_pointer (&self->id, g_free);
    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->ruler_name, g_free);
    g_clear_pointer (&self->at_war_with_id, g_free);
    g_clear_pointer (&self->region_ids, g_ptr_array_unref);
    g_clear_pointer (&self->relations, g_hash_table_unref);

    G_OBJECT_CLASS (lp_kingdom_parent_class)->finalize (object);
}

static void
lp_kingdom_class_init (LpKingdomClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_kingdom_get_property;
    object_class->set_property = lp_kingdom_set_property;
    object_class->finalize = lp_kingdom_finalize;

    /* Properties */
    properties[PROP_ID] =
        g_param_spec_string ("id", "ID", "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_NAME] =
        g_param_spec_string ("name", "Name", "Display name",
                             "Unknown Kingdom",
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_STABILITY] =
        g_param_spec_int ("stability", "Stability", "Government stability (0-100)",
                          MIN_ATTRIBUTE, MAX_ATTRIBUTE, DEFAULT_ATTRIBUTE_VALUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    properties[PROP_PROSPERITY] =
        g_param_spec_int ("prosperity", "Prosperity", "Economic health (0-100)",
                          MIN_ATTRIBUTE, MAX_ATTRIBUTE, DEFAULT_ATTRIBUTE_VALUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    properties[PROP_MILITARY] =
        g_param_spec_int ("military", "Military", "Military strength (0-100)",
                          MIN_ATTRIBUTE, MAX_ATTRIBUTE, DEFAULT_ATTRIBUTE_VALUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    properties[PROP_CULTURE] =
        g_param_spec_int ("culture", "Culture", "Cultural strength (0-100)",
                          MIN_ATTRIBUTE, MAX_ATTRIBUTE, DEFAULT_ATTRIBUTE_VALUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    properties[PROP_TOLERANCE] =
        g_param_spec_int ("tolerance", "Tolerance", "Magic/undead tolerance (0-100)",
                          MIN_ATTRIBUTE, MAX_ATTRIBUTE, DEFAULT_ATTRIBUTE_VALUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    properties[PROP_RULER_NAME] =
        g_param_spec_string ("ruler-name", "Ruler Name", "Current ruler's name",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_DYNASTY_YEARS] =
        g_param_spec_uint ("dynasty-years", "Dynasty Years",
                           "Years the current dynasty has ruled",
                           0, G_MAXUINT, 0,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    properties[PROP_IS_COLLAPSED] =
        g_param_spec_boolean ("is-collapsed", "Is Collapsed",
                              "Whether the kingdom has collapsed",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_AT_WAR_WITH_ID] =
        g_param_spec_string ("at-war-with-id", "At War With",
                             "ID of enemy kingdom",
                             NULL,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /* Signals */

    /**
     * LpKingdom::attribute-changed:
     * @self: the #LpKingdom
     * @attribute_name: name of the changed attribute
     * @old_value: previous value
     * @new_value: new value
     *
     * Emitted when a core attribute changes.
     */
    signals[SIGNAL_ATTRIBUTE_CHANGED] =
        g_signal_new ("attribute-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 3,
                      G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);

    /**
     * LpKingdom::collapsed:
     * @self: the #LpKingdom
     *
     * Emitted when the kingdom collapses.
     */
    signals[SIGNAL_COLLAPSED] =
        g_signal_new ("collapsed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpKingdom::war-declared:
     * @self: the #LpKingdom
     * @enemy_kingdom_id: ID of the enemy kingdom
     *
     * Emitted when the kingdom declares war.
     */
    signals[SIGNAL_WAR_DECLARED] =
        g_signal_new ("war-declared",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_STRING);

    /**
     * LpKingdom::war-ended:
     * @self: the #LpKingdom
     * @enemy_kingdom_id: ID of the enemy kingdom
     * @victory: whether this kingdom won
     *
     * Emitted when a war ends.
     */
    signals[SIGNAL_WAR_ENDED] =
        g_signal_new ("war-ended",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);

    /**
     * LpKingdom::crusade-launched:
     * @self: the #LpKingdom
     *
     * Emitted when the kingdom launches a crusade against the undead.
     */
    signals[SIGNAL_CRUSADE_LAUNCHED] =
        g_signal_new ("crusade-launched",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_kingdom_init (LpKingdom *self)
{
    self->id = NULL;
    self->name = g_strdup ("Unknown Kingdom");

    self->stability = DEFAULT_ATTRIBUTE_VALUE;
    self->prosperity = DEFAULT_ATTRIBUTE_VALUE;
    self->military = DEFAULT_ATTRIBUTE_VALUE;
    self->culture = DEFAULT_ATTRIBUTE_VALUE;
    self->tolerance = DEFAULT_ATTRIBUTE_VALUE;

    self->ruler_name = NULL;
    self->dynasty_years = 0;
    self->is_collapsed = FALSE;
    self->at_war_with_id = NULL;

    self->region_ids = g_ptr_array_new_with_free_func (g_free);
    self->relations = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpKingdom *
lp_kingdom_new (const gchar *id,
                const gchar *name)
{
    return g_object_new (LP_TYPE_KINGDOM,
                         "id", id,
                         "name", name,
                         NULL);
}

LpKingdom *
lp_kingdom_new_full (const gchar *id,
                     const gchar *name,
                     gint         stability,
                     gint         prosperity,
                     gint         military,
                     gint         culture,
                     gint         tolerance)
{
    return g_object_new (LP_TYPE_KINGDOM,
                         "id", id,
                         "name", name,
                         "stability", stability,
                         "prosperity", prosperity,
                         "military", military,
                         "culture", culture,
                         "tolerance", tolerance,
                         NULL);
}

/* ==========================================================================
 * Property Accessors
 * ========================================================================== */

const gchar *
lp_kingdom_get_id (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), NULL);
    return self->id;
}

const gchar *
lp_kingdom_get_name (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), NULL);
    return self->name;
}

void
lp_kingdom_set_name (LpKingdom   *self,
                     const gchar *name)
{
    g_return_if_fail (LP_IS_KINGDOM (self));

    if (g_strcmp0 (self->name, name) == 0)
        return;

    g_free (self->name);
    self->name = g_strdup (name);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}

/* Attribute accessors with change signals */

gint
lp_kingdom_get_stability (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), DEFAULT_ATTRIBUTE_VALUE);
    return self->stability;
}

void
lp_kingdom_set_stability (LpKingdom *self,
                          gint       value)
{
    gint old_value;

    g_return_if_fail (LP_IS_KINGDOM (self));

    value = clamp_attribute (value);
    if (self->stability == value)
        return;

    old_value = self->stability;
    self->stability = value;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STABILITY]);
    emit_attribute_changed (self, "stability", old_value, value);
}

gint
lp_kingdom_get_prosperity (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), DEFAULT_ATTRIBUTE_VALUE);
    return self->prosperity;
}

void
lp_kingdom_set_prosperity (LpKingdom *self,
                           gint       value)
{
    gint old_value;

    g_return_if_fail (LP_IS_KINGDOM (self));

    value = clamp_attribute (value);
    if (self->prosperity == value)
        return;

    old_value = self->prosperity;
    self->prosperity = value;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROSPERITY]);
    emit_attribute_changed (self, "prosperity", old_value, value);
}

gint
lp_kingdom_get_military (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), DEFAULT_ATTRIBUTE_VALUE);
    return self->military;
}

void
lp_kingdom_set_military (LpKingdom *self,
                         gint       value)
{
    gint old_value;

    g_return_if_fail (LP_IS_KINGDOM (self));

    value = clamp_attribute (value);
    if (self->military == value)
        return;

    old_value = self->military;
    self->military = value;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MILITARY]);
    emit_attribute_changed (self, "military", old_value, value);
}

gint
lp_kingdom_get_culture (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), DEFAULT_ATTRIBUTE_VALUE);
    return self->culture;
}

void
lp_kingdom_set_culture (LpKingdom *self,
                        gint       value)
{
    gint old_value;

    g_return_if_fail (LP_IS_KINGDOM (self));

    value = clamp_attribute (value);
    if (self->culture == value)
        return;

    old_value = self->culture;
    self->culture = value;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CULTURE]);
    emit_attribute_changed (self, "culture", old_value, value);
}

gint
lp_kingdom_get_tolerance (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), DEFAULT_ATTRIBUTE_VALUE);
    return self->tolerance;
}

void
lp_kingdom_set_tolerance (LpKingdom *self,
                          gint       value)
{
    gint old_value;

    g_return_if_fail (LP_IS_KINGDOM (self));

    value = clamp_attribute (value);
    if (self->tolerance == value)
        return;

    old_value = self->tolerance;
    self->tolerance = value;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TOLERANCE]);
    emit_attribute_changed (self, "tolerance", old_value, value);
}

/* State accessors */

const gchar *
lp_kingdom_get_ruler_name (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), NULL);
    return self->ruler_name;
}

void
lp_kingdom_set_ruler_name (LpKingdom   *self,
                           const gchar *name)
{
    g_return_if_fail (LP_IS_KINGDOM (self));

    if (g_strcmp0 (self->ruler_name, name) == 0)
        return;

    g_free (self->ruler_name);
    self->ruler_name = g_strdup (name);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RULER_NAME]);
}

guint
lp_kingdom_get_dynasty_years (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), 0);
    return self->dynasty_years;
}

void
lp_kingdom_set_dynasty_years (LpKingdom *self,
                              guint      years)
{
    g_return_if_fail (LP_IS_KINGDOM (self));

    if (self->dynasty_years == years)
        return;

    self->dynasty_years = years;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DYNASTY_YEARS]);
}

gboolean
lp_kingdom_get_is_collapsed (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), FALSE);
    return self->is_collapsed;
}

const gchar *
lp_kingdom_get_at_war_with_id (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), NULL);
    return self->at_war_with_id;
}

/* ==========================================================================
 * Region Management
 * ========================================================================== */

GPtrArray *
lp_kingdom_get_region_ids (LpKingdom *self)
{
    g_return_val_if_fail (LP_IS_KINGDOM (self), NULL);
    return self->region_ids;
}

void
lp_kingdom_add_region (LpKingdom   *self,
                       const gchar *region_id)
{
    g_return_if_fail (LP_IS_KINGDOM (self));
    g_return_if_fail (region_id != NULL);

    if (lp_kingdom_owns_region (self, region_id))
        return;

    g_ptr_array_add (self->region_ids, g_strdup (region_id));
    lp_log_debug ("Kingdom %s: added region %s", self->name, region_id);
}

gboolean
lp_kingdom_remove_region (LpKingdom   *self,
                          const gchar *region_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_KINGDOM (self), FALSE);
    g_return_val_if_fail (region_id != NULL, FALSE);

    for (i = 0; i < self->region_ids->len; i++)
    {
        const gchar *id = g_ptr_array_index (self->region_ids, i);

        if (g_strcmp0 (id, region_id) == 0)
        {
            g_ptr_array_remove_index (self->region_ids, i);
            lp_log_debug ("Kingdom %s: removed region %s", self->name, region_id);
            return TRUE;
        }
    }

    return FALSE;
}

gboolean
lp_kingdom_owns_region (LpKingdom   *self,
                        const gchar *region_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_KINGDOM (self), FALSE);
    g_return_val_if_fail (region_id != NULL, FALSE);

    for (i = 0; i < self->region_ids->len; i++)
    {
        const gchar *id = g_ptr_array_index (self->region_ids, i);

        if (g_strcmp0 (id, region_id) == 0)
            return TRUE;
    }

    return FALSE;
}

/* ==========================================================================
 * Diplomatic Relations
 * ========================================================================== */

LpKingdomRelation
lp_kingdom_get_relation (LpKingdom   *self,
                         const gchar *other_kingdom_id)
{
    gpointer value;

    g_return_val_if_fail (LP_IS_KINGDOM (self), LP_KINGDOM_RELATION_NEUTRAL);
    g_return_val_if_fail (other_kingdom_id != NULL, LP_KINGDOM_RELATION_NEUTRAL);

    if (g_hash_table_lookup_extended (self->relations, other_kingdom_id, NULL, &value))
        return GPOINTER_TO_INT (value);

    return LP_KINGDOM_RELATION_NEUTRAL;
}

void
lp_kingdom_set_relation (LpKingdom        *self,
                         const gchar      *other_kingdom_id,
                         LpKingdomRelation relation)
{
    g_return_if_fail (LP_IS_KINGDOM (self));
    g_return_if_fail (other_kingdom_id != NULL);

    g_hash_table_insert (self->relations, g_strdup (other_kingdom_id),
                         GINT_TO_POINTER (relation));

    lp_log_debug ("Kingdom %s: relation with %s set to %d",
                  self->name, other_kingdom_id, relation);
}

/* ==========================================================================
 * Yearly Tick and Rolls
 * ========================================================================== */

void
lp_kingdom_tick_year (LpKingdom *self)
{
    gint drift;

    g_return_if_fail (LP_IS_KINGDOM (self));

    if (self->is_collapsed)
        return;

    /* Increment dynasty years */
    self->dynasty_years++;

    /*
     * Apply attribute drift.
     * Each attribute drifts randomly toward 50 (mean reversion)
     * with some randomness.
     */

    /* Stability drifts based on prosperity and at war status */
    drift = g_random_int_range (-YEARLY_ATTRIBUTE_DRIFT, YEARLY_ATTRIBUTE_DRIFT + 1);
    if (self->prosperity > 60)
        drift += 1;
    if (self->prosperity < 40)
        drift -= 1;
    if (self->at_war_with_id != NULL)
        drift -= 2;
    lp_kingdom_set_stability (self, self->stability + drift);

    /* Prosperity drifts based on stability and trade */
    drift = g_random_int_range (-YEARLY_ATTRIBUTE_DRIFT, YEARLY_ATTRIBUTE_DRIFT + 1);
    if (self->stability > 60)
        drift += 1;
    if (self->stability < 40)
        drift -= 1;
    if (self->at_war_with_id != NULL)
        drift -= 1;
    lp_kingdom_set_prosperity (self, self->prosperity + drift);

    /* Military drifts based on prosperity and at war status */
    drift = g_random_int_range (-YEARLY_ATTRIBUTE_DRIFT / 2, YEARLY_ATTRIBUTE_DRIFT / 2 + 1);
    if (self->at_war_with_id != NULL)
        drift += 2;  /* War builds military */
    lp_kingdom_set_military (self, self->military + drift);

    /* Culture is mostly stable */
    drift = g_random_int_range (-1, 2);
    lp_kingdom_set_culture (self, self->culture + drift);

    /* Tolerance drifts slowly */
    drift = g_random_int_range (-1, 2);
    lp_kingdom_set_tolerance (self, self->tolerance + drift);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DYNASTY_YEARS]);
}

gboolean
lp_kingdom_roll_collapse (LpKingdom *self)
{
    gdouble collapse_chance;
    gdouble roll;

    g_return_val_if_fail (LP_IS_KINGDOM (self), FALSE);

    if (self->is_collapsed)
        return FALSE;

    if (self->stability > COLLAPSE_THRESHOLD)
        return FALSE;

    /*
     * Collapse chance increases as stability decreases:
     * At stability 10: 5% + (10-10)/10 * 15% = 5%
     * At stability 5:  5% + (10-5)/10 * 15% = 12.5%
     * At stability 0:  5% + (10-0)/10 * 15% = 20%
     */
    collapse_chance = COLLAPSE_BASE_CHANCE +
                      ((COLLAPSE_THRESHOLD - self->stability) / (gdouble)COLLAPSE_THRESHOLD) * 0.15;

    roll = g_random_double ();

    if (roll < collapse_chance)
    {
        lp_kingdom_collapse (self);
        return TRUE;
    }

    return FALSE;
}

gboolean
lp_kingdom_roll_war (LpKingdom   *self,
                     const gchar *target_kingdom_id)
{
    LpKingdomRelation relation;
    gdouble war_chance;
    gdouble roll;

    g_return_val_if_fail (LP_IS_KINGDOM (self), FALSE);
    g_return_val_if_fail (target_kingdom_id != NULL, FALSE);

    if (self->is_collapsed)
        return FALSE;

    if (self->at_war_with_id != NULL)
        return FALSE;  /* Already at war */

    if (self->military < WAR_MILITARY_THRESHOLD)
        return FALSE;

    relation = lp_kingdom_get_relation (self, target_kingdom_id);

    /* Can't declare war on allies */
    if (relation == LP_KINGDOM_RELATION_ALLIANCE)
        return FALSE;

    /* Higher chance against rivals */
    war_chance = WAR_BASE_CHANCE;
    if (relation == LP_KINGDOM_RELATION_RIVALRY)
        war_chance *= 3.0;

    /* Military strength increases chance */
    war_chance += (self->military - WAR_MILITARY_THRESHOLD) / 100.0 * 0.05;

    roll = g_random_double ();

    if (roll < war_chance)
    {
        g_free (self->at_war_with_id);
        self->at_war_with_id = g_strdup (target_kingdom_id);

        lp_kingdom_set_relation (self, target_kingdom_id, LP_KINGDOM_RELATION_WAR);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AT_WAR_WITH_ID]);
        g_signal_emit (self, signals[SIGNAL_WAR_DECLARED], 0, target_kingdom_id);

        lp_log_info ("Kingdom %s declared war on %s", self->name, target_kingdom_id);
        return TRUE;
    }

    return FALSE;
}

gboolean
lp_kingdom_roll_crusade (LpKingdom *self,
                         gboolean   exposure_detected)
{
    gdouble crusade_chance;
    gdouble roll;

    g_return_val_if_fail (LP_IS_KINGDOM (self), FALSE);

    if (self->is_collapsed)
        return FALSE;

    if (!exposure_detected)
        return FALSE;

    if (self->tolerance > CRUSADE_TOLERANCE_THRESHOLD)
        return FALSE;

    /*
     * Crusade chance based on tolerance:
     * Lower tolerance = higher chance
     */
    crusade_chance = CRUSADE_BASE_CHANCE +
                     ((CRUSADE_TOLERANCE_THRESHOLD - self->tolerance) /
                      (gdouble)CRUSADE_TOLERANCE_THRESHOLD) * 0.10;

    /* High culture increases crusade chance */
    if (self->culture > 70)
        crusade_chance *= 1.5;

    roll = g_random_double ();

    if (roll < crusade_chance)
    {
        g_signal_emit (self, signals[SIGNAL_CRUSADE_LAUNCHED], 0);
        lp_log_warning ("Kingdom %s launched a crusade against the undead!",
                        self->name);
        return TRUE;
    }

    return FALSE;
}

void
lp_kingdom_end_war (LpKingdom *self,
                    gboolean   victory)
{
    g_autofree gchar *enemy_id = NULL;

    g_return_if_fail (LP_IS_KINGDOM (self));

    if (self->at_war_with_id == NULL)
        return;

    enemy_id = g_steal_pointer (&self->at_war_with_id);

    /* Apply war consequences */
    if (victory)
    {
        lp_kingdom_set_stability (self, self->stability + 10);
        lp_kingdom_set_prosperity (self, self->prosperity + 5);
        lp_log_info ("Kingdom %s won the war against %s", self->name, enemy_id);
    }
    else
    {
        lp_kingdom_set_stability (self, self->stability - 15);
        lp_kingdom_set_prosperity (self, self->prosperity - 10);
        lp_kingdom_set_military (self, self->military - 10);
        lp_log_info ("Kingdom %s lost the war against %s", self->name, enemy_id);
    }

    /* Reset relation to rivalry after war */
    lp_kingdom_set_relation (self, enemy_id, LP_KINGDOM_RELATION_RIVALRY);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AT_WAR_WITH_ID]);
    g_signal_emit (self, signals[SIGNAL_WAR_ENDED], 0, enemy_id, victory);
}

void
lp_kingdom_collapse (LpKingdom *self)
{
    g_return_if_fail (LP_IS_KINGDOM (self));

    if (self->is_collapsed)
        return;

    self->is_collapsed = TRUE;
    self->stability = 0;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_COLLAPSED]);
    g_signal_emit (self, signals[SIGNAL_COLLAPSED], 0);

    lp_log_warning ("Kingdom %s has collapsed!", self->name);
}
