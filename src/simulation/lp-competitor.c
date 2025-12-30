/* lp-competitor.c - Immortal Competitor Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-competitor.h"
#include "lp-world-simulation.h"
#include "lp-event.h"
#include <libregnum.h>

struct _LpCompetitor
{
    GObject parent_instance;

    gchar            *id;
    gchar            *name;
    LpCompetitorType  competitor_type;
    LpCompetitorStance stance;

    /* AI personality traits (0-100) */
    gint power_level;
    gint aggression;
    gint greed;
    gint cunning;

    /* State */
    gboolean is_active;
    gboolean is_known;
    guint    player_threat_level;

    /* Territory control */
    GPtrArray *territory_region_ids;

    /* AI components */
    LrgBehaviorTree *behavior_tree;
    LrgBlackboard   *blackboard;
};

static void lp_competitor_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (LpCompetitor, lp_competitor, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                      lp_competitor_saveable_init))

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_COMPETITOR_TYPE,
    PROP_STANCE,
    PROP_POWER_LEVEL,
    PROP_AGGRESSION,
    PROP_GREED,
    PROP_CUNNING,
    PROP_IS_ACTIVE,
    PROP_IS_KNOWN,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_DISCOVERED,
    SIGNAL_STANCE_CHANGED,
    SIGNAL_TERRITORY_EXPANDED,
    SIGNAL_TERRITORY_LOST,
    SIGNAL_DESTROYED,
    SIGNAL_ALLIANCE_PROPOSED,
    SIGNAL_CONFLICT_DECLARED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * AI decision helper functions
 */

static void
update_blackboard_state (LpCompetitor *self)
{
    /*
     * Update the blackboard with current state for behavior tree decisions.
     */
    lrg_blackboard_set_int (self->blackboard, "power-level", self->power_level);
    lrg_blackboard_set_int (self->blackboard, "aggression", self->aggression);
    lrg_blackboard_set_int (self->blackboard, "greed", self->greed);
    lrg_blackboard_set_int (self->blackboard, "cunning", self->cunning);
    lrg_blackboard_set_int (self->blackboard, "stance", (gint)self->stance);
    lrg_blackboard_set_int (self->blackboard, "territory-count",
                            (gint)self->territory_region_ids->len);
    lrg_blackboard_set_int (self->blackboard, "player-threat",
                            (gint)self->player_threat_level);
    lrg_blackboard_set_bool (self->blackboard, "is-known", self->is_known);
}

static gboolean
should_expand (LpCompetitor *self)
{
    /*
     * Determine if the competitor should try to expand.
     * Based on greed and current power level.
     */
    gint expansion_desire;

    expansion_desire = (self->greed + self->power_level) / 2;

    /* More territory = less desire to expand (diminishing returns) */
    expansion_desire -= (gint)(self->territory_region_ids->len * 5);

    return g_random_int_range (0, 100) < expansion_desire;
}

static gboolean
should_consider_player_threat (LpCompetitor *self)
{
    /*
     * High cunning competitors monitor the player more closely.
     */
    return self->cunning > 50 && self->player_threat_level > 30;
}

static void
evaluate_stance_change (LpCompetitor *self)
{
    LpCompetitorStance old_stance;
    gint hostility_score;

    old_stance = self->stance;

    /*
     * Calculate hostility based on:
     * - Aggression trait
     * - Player threat level
     * - Current stance momentum
     */
    hostility_score = self->aggression;
    hostility_score += (gint)(self->player_threat_level / 2);

    /* Cunning competitors are more measured */
    if (self->cunning > 60)
        hostility_score -= 20;

    /* Greed can push toward hostile (competition) or friendly (alliance) */
    if (self->greed > 70)
    {
        if (self->player_threat_level > 50)
            hostility_score += 10;  /* Threat to wealth */
        else
            hostility_score -= 10;  /* Potential partner */
    }

    /* Determine new stance based on score */
    if (hostility_score > 80)
    {
        self->stance = LP_COMPETITOR_STANCE_HOSTILE;
    }
    else if (hostility_score > 60)
    {
        self->stance = LP_COMPETITOR_STANCE_WARY;
    }
    else if (hostility_score > 40)
    {
        self->stance = LP_COMPETITOR_STANCE_NEUTRAL;
    }
    else if (hostility_score > 20)
    {
        self->stance = LP_COMPETITOR_STANCE_FRIENDLY;
    }
    /* Allied stance requires explicit action, not drift */

    if (self->stance != old_stance)
    {
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STANCE]);
        g_signal_emit (self, signals[SIGNAL_STANCE_CHANGED], 0, old_stance, self->stance);
    }
}

/*
 * LrgSaveable interface
 */

static const gchar *
lp_competitor_get_save_id (LrgSaveable *saveable)
{
    return LP_COMPETITOR (saveable)->id;
}

static gboolean
lp_competitor_save (LrgSaveable    *saveable,
                    LrgSaveContext *ctx,
                    GError        **error)
{
    LpCompetitor *self = LP_COMPETITOR (saveable);

    lrg_save_context_write_string (ctx, "id", self->id);
    lrg_save_context_write_string (ctx, "name", self->name);
    lrg_save_context_write_int (ctx, "competitor-type", (gint)self->competitor_type);
    lrg_save_context_write_int (ctx, "stance", (gint)self->stance);

    lrg_save_context_write_int (ctx, "power-level", self->power_level);
    lrg_save_context_write_int (ctx, "aggression", self->aggression);
    lrg_save_context_write_int (ctx, "greed", self->greed);
    lrg_save_context_write_int (ctx, "cunning", self->cunning);

    lrg_save_context_write_boolean (ctx, "is-active", self->is_active);
    lrg_save_context_write_boolean (ctx, "is-known", self->is_known);
    lrg_save_context_write_uint (ctx, "player-threat-level", self->player_threat_level);

    /* Save territory using section-based approach */
    {
        guint i;
        g_autofree gchar *key = NULL;

        lrg_save_context_begin_section (ctx, "territory");
        lrg_save_context_write_uint (ctx, "count", self->territory_region_ids->len);
        for (i = 0; i < self->territory_region_ids->len; i++)
        {
            const gchar *region_id = g_ptr_array_index (self->territory_region_ids, i);
            key = g_strdup_printf ("%u", i);
            lrg_save_context_write_string (ctx, key, region_id);
            g_free (key);
            key = NULL;
        }
        lrg_save_context_end_section (ctx);
    }

    return TRUE;
}

static gboolean
lp_competitor_load (LrgSaveable    *saveable,
                    LrgSaveContext *ctx,
                    GError        **error)
{
    LpCompetitor *self = LP_COMPETITOR (saveable);

    g_clear_pointer (&self->id, g_free);
    g_clear_pointer (&self->name, g_free);

    self->id = lrg_save_context_read_string (ctx, "id", NULL);
    self->name = lrg_save_context_read_string (ctx, "name", NULL);
    self->competitor_type = (LpCompetitorType)lrg_save_context_read_int (
        ctx, "competitor-type", LP_COMPETITOR_TYPE_DRAGON);
    self->stance = (LpCompetitorStance)lrg_save_context_read_int (
        ctx, "stance", LP_COMPETITOR_STANCE_UNKNOWN);

    self->power_level = lrg_save_context_read_int (ctx, "power-level", 50);
    self->aggression = lrg_save_context_read_int (ctx, "aggression", 50);
    self->greed = lrg_save_context_read_int (ctx, "greed", 50);
    self->cunning = lrg_save_context_read_int (ctx, "cunning", 50);

    self->is_active = lrg_save_context_read_boolean (ctx, "is-active", TRUE);
    self->is_known = lrg_save_context_read_boolean (ctx, "is-known", FALSE);
    self->player_threat_level = lrg_save_context_read_uint (ctx, "player-threat-level", 0);

    /* Load territory using section-based approach */
    g_ptr_array_set_size (self->territory_region_ids, 0);
    if (lrg_save_context_enter_section (ctx, "territory"))
    {
        guint count;
        guint i;
        g_autofree gchar *key = NULL;

        count = lrg_save_context_read_uint (ctx, "count", 0);
        for (i = 0; i < count; i++)
        {
            gchar *region_id;
            key = g_strdup_printf ("%u", i);
            region_id = lrg_save_context_read_string (ctx, key, NULL);
            if (region_id != NULL)
            {
                g_ptr_array_add (self->territory_region_ids, region_id);
            }
            g_free (key);
            key = NULL;
        }
        lrg_save_context_leave_section (ctx);
    }

    return TRUE;
}

static void
lp_competitor_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_competitor_get_save_id;
    iface->save = lp_competitor_save;
    iface->load = lp_competitor_load;
}

/*
 * GObject implementation
 */

static void
lp_competitor_finalize (GObject *object)
{
    LpCompetitor *self = LP_COMPETITOR (object);

    g_clear_pointer (&self->id, g_free);
    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->territory_region_ids, g_ptr_array_unref);
    g_clear_object (&self->behavior_tree);
    g_clear_object (&self->blackboard);

    G_OBJECT_CLASS (lp_competitor_parent_class)->finalize (object);
}

static void
lp_competitor_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    LpCompetitor *self = LP_COMPETITOR (object);

    switch (prop_id)
    {
    case PROP_ID:
        g_value_set_string (value, self->id);
        break;

    case PROP_NAME:
        g_value_set_string (value, self->name);
        break;

    case PROP_COMPETITOR_TYPE:
        g_value_set_enum (value, self->competitor_type);
        break;

    case PROP_STANCE:
        g_value_set_enum (value, self->stance);
        break;

    case PROP_POWER_LEVEL:
        g_value_set_int (value, self->power_level);
        break;

    case PROP_AGGRESSION:
        g_value_set_int (value, self->aggression);
        break;

    case PROP_GREED:
        g_value_set_int (value, self->greed);
        break;

    case PROP_CUNNING:
        g_value_set_int (value, self->cunning);
        break;

    case PROP_IS_ACTIVE:
        g_value_set_boolean (value, self->is_active);
        break;

    case PROP_IS_KNOWN:
        g_value_set_boolean (value, self->is_known);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_competitor_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    LpCompetitor *self = LP_COMPETITOR (object);

    switch (prop_id)
    {
    case PROP_ID:
        g_free (self->id);
        self->id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        lp_competitor_set_name (self, g_value_get_string (value));
        break;

    case PROP_COMPETITOR_TYPE:
        self->competitor_type = g_value_get_enum (value);
        break;

    case PROP_STANCE:
        lp_competitor_set_stance (self, g_value_get_enum (value));
        break;

    case PROP_POWER_LEVEL:
        lp_competitor_set_power_level (self, g_value_get_int (value));
        break;

    case PROP_AGGRESSION:
        lp_competitor_set_aggression (self, g_value_get_int (value));
        break;

    case PROP_GREED:
        lp_competitor_set_greed (self, g_value_get_int (value));
        break;

    case PROP_CUNNING:
        lp_competitor_set_cunning (self, g_value_get_int (value));
        break;

    case PROP_IS_ACTIVE:
        lp_competitor_set_is_active (self, g_value_get_boolean (value));
        break;

    case PROP_IS_KNOWN:
        lp_competitor_set_is_known (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_competitor_class_init (LpCompetitorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_competitor_finalize;
    object_class->get_property = lp_competitor_get_property;
    object_class->set_property = lp_competitor_set_property;

    /**
     * LpCompetitor:id:
     *
     * Unique identifier for this competitor.
     */
    properties[PROP_ID] =
        g_param_spec_string ("id",
                             "ID",
                             "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:name:
     *
     * Display name of this competitor.
     */
    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Display name",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:competitor-type:
     *
     * Type of immortal.
     */
    properties[PROP_COMPETITOR_TYPE] =
        g_param_spec_enum ("competitor-type",
                           "Competitor Type",
                           "Type of immortal",
                           LP_TYPE_COMPETITOR_TYPE,
                           LP_COMPETITOR_TYPE_DRAGON,
                           G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:stance:
     *
     * Attitude toward the player.
     */
    properties[PROP_STANCE] =
        g_param_spec_enum ("stance",
                           "Stance",
                           "Attitude toward player",
                           LP_TYPE_COMPETITOR_STANCE,
                           LP_COMPETITOR_STANCE_UNKNOWN,
                           G_PARAM_READWRITE |
                           G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:power-level:
     *
     * Overall power level (0-100).
     */
    properties[PROP_POWER_LEVEL] =
        g_param_spec_int ("power-level",
                          "Power Level",
                          "Overall power (0-100)",
                          0, 100, 50,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:aggression:
     *
     * Aggression trait (0-100).
     */
    properties[PROP_AGGRESSION] =
        g_param_spec_int ("aggression",
                          "Aggression",
                          "Aggression trait (0-100)",
                          0, 100, 50,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:greed:
     *
     * Greed trait (0-100).
     */
    properties[PROP_GREED] =
        g_param_spec_int ("greed",
                          "Greed",
                          "Greed trait (0-100)",
                          0, 100, 50,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:cunning:
     *
     * Cunning trait (0-100).
     */
    properties[PROP_CUNNING] =
        g_param_spec_int ("cunning",
                          "Cunning",
                          "Cunning trait (0-100)",
                          0, 100, 50,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:is-active:
     *
     * Whether this competitor is actively participating.
     */
    properties[PROP_IS_ACTIVE] =
        g_param_spec_boolean ("is-active",
                              "Is Active",
                              "Whether actively participating",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    /**
     * LpCompetitor:is-known:
     *
     * Whether the player has discovered this competitor.
     */
    properties[PROP_IS_KNOWN] =
        g_param_spec_boolean ("is-known",
                              "Is Known",
                              "Whether discovered by player",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpCompetitor::discovered:
     * @self: the competitor
     *
     * Emitted when the player discovers this competitor.
     */
    signals[SIGNAL_DISCOVERED] =
        g_signal_new ("discovered",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpCompetitor::stance-changed:
     * @self: the competitor
     * @old_stance: the previous stance
     * @new_stance: the new stance
     *
     * Emitted when the competitor's stance toward the player changes.
     */
    signals[SIGNAL_STANCE_CHANGED] =
        g_signal_new ("stance-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 2,
                      LP_TYPE_COMPETITOR_STANCE,
                      LP_TYPE_COMPETITOR_STANCE);

    /**
     * LpCompetitor::territory-expanded:
     * @self: the competitor
     * @region_id: the newly acquired region
     *
     * Emitted when the competitor expands into new territory.
     */
    signals[SIGNAL_TERRITORY_EXPANDED] =
        g_signal_new ("territory-expanded",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_STRING);

    /**
     * LpCompetitor::territory-lost:
     * @self: the competitor
     * @region_id: the lost region
     *
     * Emitted when the competitor loses territory.
     */
    signals[SIGNAL_TERRITORY_LOST] =
        g_signal_new ("territory-lost",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_STRING);

    /**
     * LpCompetitor::destroyed:
     * @self: the competitor
     *
     * Emitted when the competitor is destroyed.
     */
    signals[SIGNAL_DESTROYED] =
        g_signal_new ("destroyed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpCompetitor::alliance-proposed:
     * @self: the competitor
     *
     * Emitted when the competitor proposes an alliance.
     */
    signals[SIGNAL_ALLIANCE_PROPOSED] =
        g_signal_new ("alliance-proposed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpCompetitor::conflict-declared:
     * @self: the competitor
     *
     * Emitted when the competitor declares conflict.
     */
    signals[SIGNAL_CONFLICT_DECLARED] =
        g_signal_new ("conflict-declared",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_competitor_init (LpCompetitor *self)
{
    self->id = NULL;
    self->name = NULL;
    self->competitor_type = LP_COMPETITOR_TYPE_DRAGON;
    self->stance = LP_COMPETITOR_STANCE_UNKNOWN;

    self->power_level = 50;
    self->aggression = 50;
    self->greed = 50;
    self->cunning = 50;

    self->is_active = TRUE;
    self->is_known = FALSE;
    self->player_threat_level = 0;

    self->territory_region_ids = g_ptr_array_new_with_free_func (g_free);

    /* Initialize AI components */
    self->blackboard = lrg_blackboard_new ();
    self->behavior_tree = lrg_behavior_tree_new ();
}

/*
 * Public API
 */

LpCompetitor *
lp_competitor_new (const gchar      *id,
                   const gchar      *name,
                   LpCompetitorType  competitor_type)
{
    return g_object_new (LP_TYPE_COMPETITOR,
                         "id", id,
                         "name", name,
                         "competitor-type", competitor_type,
                         NULL);
}

const gchar *
lp_competitor_get_id (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), NULL);

    return self->id;
}

const gchar *
lp_competitor_get_name (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), NULL);

    return self->name;
}

void
lp_competitor_set_name (LpCompetitor *self,
                        const gchar  *name)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    if (g_strcmp0 (self->name, name) != 0)
    {
        g_free (self->name);
        self->name = g_strdup (name);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
    }
}

LpCompetitorType
lp_competitor_get_competitor_type (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), LP_COMPETITOR_TYPE_DRAGON);

    return self->competitor_type;
}

LpCompetitorStance
lp_competitor_get_stance (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), LP_COMPETITOR_STANCE_UNKNOWN);

    return self->stance;
}

void
lp_competitor_set_stance (LpCompetitor       *self,
                          LpCompetitorStance  stance)
{
    LpCompetitorStance old_stance;

    g_return_if_fail (LP_IS_COMPETITOR (self));

    if (self->stance != stance)
    {
        old_stance = self->stance;
        self->stance = stance;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STANCE]);
        g_signal_emit (self, signals[SIGNAL_STANCE_CHANGED], 0, old_stance, stance);
    }
}

gint
lp_competitor_get_power_level (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), 50);

    return self->power_level;
}

void
lp_competitor_set_power_level (LpCompetitor *self,
                               gint          level)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    level = CLAMP (level, 0, 100);

    if (self->power_level != level)
    {
        self->power_level = level;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POWER_LEVEL]);
    }
}

gint
lp_competitor_get_aggression (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), 50);

    return self->aggression;
}

void
lp_competitor_set_aggression (LpCompetitor *self,
                              gint          aggression)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    aggression = CLAMP (aggression, 0, 100);

    if (self->aggression != aggression)
    {
        self->aggression = aggression;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AGGRESSION]);
    }
}

gint
lp_competitor_get_greed (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), 50);

    return self->greed;
}

void
lp_competitor_set_greed (LpCompetitor *self,
                         gint          greed)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    greed = CLAMP (greed, 0, 100);

    if (self->greed != greed)
    {
        self->greed = greed;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_GREED]);
    }
}

gint
lp_competitor_get_cunning (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), 50);

    return self->cunning;
}

void
lp_competitor_set_cunning (LpCompetitor *self,
                           gint          cunning)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    cunning = CLAMP (cunning, 0, 100);

    if (self->cunning != cunning)
    {
        self->cunning = cunning;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CUNNING]);
    }
}

gboolean
lp_competitor_get_is_active (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), FALSE);

    return self->is_active;
}

void
lp_competitor_set_is_active (LpCompetitor *self,
                             gboolean      active)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    active = !!active;

    if (self->is_active != active)
    {
        self->is_active = active;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_ACTIVE]);
    }
}

gboolean
lp_competitor_get_is_known (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), FALSE);

    return self->is_known;
}

void
lp_competitor_set_is_known (LpCompetitor *self,
                            gboolean      known)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    known = !!known;

    if (self->is_known != known)
    {
        self->is_known = known;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_KNOWN]);
    }
}

GPtrArray *
lp_competitor_get_territory_region_ids (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), NULL);

    return self->territory_region_ids;
}

void
lp_competitor_add_territory (LpCompetitor *self,
                             const gchar  *region_id)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));
    g_return_if_fail (region_id != NULL);

    if (!lp_competitor_has_territory (self, region_id))
    {
        g_ptr_array_add (self->territory_region_ids, g_strdup (region_id));
        g_signal_emit (self, signals[SIGNAL_TERRITORY_EXPANDED], 0, region_id);
    }
}

gboolean
lp_competitor_remove_territory (LpCompetitor *self,
                                const gchar  *region_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_COMPETITOR (self), FALSE);
    g_return_val_if_fail (region_id != NULL, FALSE);

    for (i = 0; i < self->territory_region_ids->len; i++)
    {
        if (g_strcmp0 (g_ptr_array_index (self->territory_region_ids, i), region_id) == 0)
        {
            g_ptr_array_remove_index (self->territory_region_ids, i);
            g_signal_emit (self, signals[SIGNAL_TERRITORY_LOST], 0, region_id);
            return TRUE;
        }
    }

    return FALSE;
}

gboolean
lp_competitor_has_territory (LpCompetitor *self,
                             const gchar  *region_id)
{
    guint i;

    g_return_val_if_fail (LP_IS_COMPETITOR (self), FALSE);
    g_return_val_if_fail (region_id != NULL, FALSE);

    for (i = 0; i < self->territory_region_ids->len; i++)
    {
        if (g_strcmp0 (g_ptr_array_index (self->territory_region_ids, i), region_id) == 0)
            return TRUE;
    }

    return FALSE;
}

void
lp_competitor_tick_year (LpCompetitor      *self,
                         LpWorldSimulation *sim)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    if (!self->is_active)
        return;

    /* Update AI state */
    update_blackboard_state (self);

    /* Tick the behavior tree for AI decisions */
    lrg_behavior_tree_tick (self->behavior_tree, 1.0f);

    /* Check if we should expand */
    if (should_expand (self))
    {
        lp_competitor_expand_territory (self, sim);
    }

    /* Evaluate stance changes based on current state */
    if (should_consider_player_threat (self))
    {
        evaluate_stance_change (self);
    }

    /* Power level naturally fluctuates slightly */
    self->power_level += g_random_int_range (-2, 3);
    self->power_level = CLAMP (self->power_level, 0, 100);
}

void
lp_competitor_react_to_event (LpCompetitor *self,
                              LpEvent      *event)
{
    LpEventType event_type;
    LpEventSeverity severity;

    g_return_if_fail (LP_IS_COMPETITOR (self));
    g_return_if_fail (LP_IS_EVENT (event));

    if (!self->is_active)
        return;

    event_type = lp_event_get_event_type (event);
    severity = lp_event_get_severity (event);

    /*
     * Different competitor types react differently to events.
     */
    switch (self->competitor_type)
    {
    case LP_COMPETITOR_TYPE_DRAGON:
        /* Dragons are territorial - major political events concern them */
        if (event_type == LP_EVENT_TYPE_POLITICAL &&
            severity >= LP_EVENT_SEVERITY_MAJOR)
        {
            self->aggression += 10;
            self->aggression = MIN (self->aggression, 100);
        }
        break;

    case LP_COMPETITOR_TYPE_VAMPIRE:
        /* Vampires thrive in chaos */
        if (event_type == LP_EVENT_TYPE_POLITICAL &&
            severity >= LP_EVENT_SEVERITY_MODERATE)
        {
            self->power_level += 5;
            self->power_level = MIN (self->power_level, 100);
        }
        break;

    case LP_COMPETITOR_TYPE_LICH:
        /* Liches are concerned about magical events */
        if (event_type == LP_EVENT_TYPE_MAGICAL)
        {
            self->cunning += 5;
            self->cunning = MIN (self->cunning, 100);
        }
        break;

    case LP_COMPETITOR_TYPE_FAE:
        /* Fae react to magical disturbances */
        if (event_type == LP_EVENT_TYPE_MAGICAL &&
            severity >= LP_EVENT_SEVERITY_MAJOR)
        {
            self->greed += 10;
            self->greed = MIN (self->greed, 100);
        }
        break;

    case LP_COMPETITOR_TYPE_DEMON:
        /* Demons are opportunistic */
        if (severity >= LP_EVENT_SEVERITY_CATASTROPHIC)
        {
            self->aggression += 15;
            self->aggression = MIN (self->aggression, 100);
        }
        break;

    default:
        break;
    }
}

gboolean
lp_competitor_expand_territory (LpCompetitor      *self,
                                LpWorldSimulation *sim)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), FALSE);

    /*
     * Territory expansion logic would query the world simulation
     * for available regions and add one based on competitor traits.
     * For now, this is a placeholder that will be connected to
     * the world simulation once it's updated.
     */
    g_debug ("Competitor '%s' attempting territory expansion", self->name);

    return FALSE;
}

guint
lp_competitor_get_player_threat_level (LpCompetitor *self)
{
    g_return_val_if_fail (LP_IS_COMPETITOR (self), 0);

    return self->player_threat_level;
}

void
lp_competitor_discover (LpCompetitor *self)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    if (!self->is_known)
    {
        self->is_known = TRUE;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_KNOWN]);
        g_signal_emit (self, signals[SIGNAL_DISCOVERED], 0);
    }
}

void
lp_competitor_destroy (LpCompetitor *self)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    if (self->is_active)
    {
        self->is_active = FALSE;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_ACTIVE]);
        g_signal_emit (self, signals[SIGNAL_DESTROYED], 0);
    }
}

void
lp_competitor_propose_alliance (LpCompetitor *self)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    g_signal_emit (self, signals[SIGNAL_ALLIANCE_PROPOSED], 0);
}

void
lp_competitor_declare_conflict (LpCompetitor *self)
{
    g_return_if_fail (LP_IS_COMPETITOR (self));

    self->stance = LP_COMPETITOR_STANCE_HOSTILE;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STANCE]);
    g_signal_emit (self, signals[SIGNAL_CONFLICT_DECLARED], 0);
}
