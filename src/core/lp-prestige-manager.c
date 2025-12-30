/* lp-prestige-manager.c - Prestige System Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <math.h>

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-prestige-manager.h"

/* Number of Echo specialization trees */
#define NUM_ECHO_TREES 4

typedef struct
{
    LrgBigNumber *echoes;               /* Available Echo points */
    LrgBigNumber *total_echoes_earned;  /* All-time Echoes earned */
    guint64       times_prestiged;      /* Number of prestige resets */

    /* Echo specialization trees (one per LpEchoTree) */
    LrgUnlockTree *echo_trees[NUM_ECHO_TREES];
} LpPrestigeManagerPrivate;

enum
{
    PROP_0,
    PROP_ECHOES,
    PROP_TOTAL_ECHOES_EARNED,
    PROP_TIMES_PRESTIGED,
    N_PROPS
};

enum
{
    SIGNAL_PRESTIGE_PERFORMED,
    SIGNAL_UPGRADE_UNLOCKED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_prestige_manager_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpPrestigeManager, lp_prestige_manager, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (LpPrestigeManager)
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_prestige_manager_saveable_init))

/* ==========================================================================
 * Echo Tree Initialization
 * ========================================================================== */

/*
 * Creates the Economist echo tree with all upgrades.
 * The Economist tree focuses on financial and compound interest bonuses.
 */
static LrgUnlockTree *
create_economist_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Tier 1: Startup Capital - 2x starting gold */
    node = lrg_unlock_node_new ("startup-capital", "Startup Capital");
    lrg_unlock_node_set_description (node, "Begin each run with double your starting gold");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_node_set_tier (node, 1);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    /* Tier 2: Market Sense - +15% divination accuracy */
    node = lrg_unlock_node_new ("market-sense", "Market Sense");
    lrg_unlock_node_set_description (node, "Gain +15% accuracy on market predictions");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_node_set_tier (node, 2);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "market-sense", "startup-capital");
    lrg_unlock_node_free (node);

    /* Tier 3: Compound Master - +2% base interest */
    node = lrg_unlock_node_new ("compound-master", "Compound Master");
    lrg_unlock_node_set_description (node, "All investments gain +2% base interest rate");
    lrg_unlock_node_set_cost_simple (node, 10.0);
    lrg_unlock_node_set_tier (node, 3);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "compound-master", "market-sense");
    lrg_unlock_node_free (node);

    /* Tier 4: Perfect Foresight - See 50 years ahead */
    node = lrg_unlock_node_new ("perfect-foresight", "Perfect Foresight");
    lrg_unlock_node_set_description (node, "Divination reveals events 50 years in advance");
    lrg_unlock_node_set_cost_simple (node, 25.0);
    lrg_unlock_node_set_tier (node, 4);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "perfect-foresight", "compound-master");
    lrg_unlock_node_free (node);

    return tree;
}

/*
 * Creates the Manipulator echo tree with all upgrades.
 * The Manipulator tree focuses on agent and political bonuses.
 */
static LrgUnlockTree *
create_manipulator_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Tier 1: Established Network - Start with agent family */
    node = lrg_unlock_node_new ("established-network", "Established Network");
    lrg_unlock_node_set_description (node, "Begin each run with an established agent family");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_node_set_tier (node, 1);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    /* Tier 2: Whisper Network - Double agent mechanics */
    node = lrg_unlock_node_new ("whisper-network", "Whisper Network");
    lrg_unlock_node_set_description (node, "Agents can serve as double agents");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_node_set_tier (node, 2);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "whisper-network", "established-network");
    lrg_unlock_node_free (node);

    /* Tier 3: Shadow Council - Political influence 2x */
    node = lrg_unlock_node_new ("shadow-council", "Shadow Council");
    lrg_unlock_node_set_description (node, "Double the effectiveness of political investments");
    lrg_unlock_node_set_cost_simple (node, 10.0);
    lrg_unlock_node_set_tier (node, 3);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "shadow-council", "whisper-network");
    lrg_unlock_node_free (node);

    /* Tier 4: Puppetmaster - Competitors start weaker */
    node = lrg_unlock_node_new ("puppetmaster", "Puppetmaster");
    lrg_unlock_node_set_description (node, "Immortal competitors begin with reduced power");
    lrg_unlock_node_set_cost_simple (node, 25.0);
    lrg_unlock_node_set_tier (node, 4);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "puppetmaster", "shadow-council");
    lrg_unlock_node_free (node);

    return tree;
}

/*
 * Creates the Scholar echo tree with all upgrades.
 * The Scholar tree focuses on ledger and discovery bonuses.
 */
static LrgUnlockTree *
create_scholar_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Tier 1: Memory Fragments - Keep 25% of Ledger */
    node = lrg_unlock_node_new ("memory-fragments", "Memory Fragments");
    lrg_unlock_node_set_description (node, "Retain 25% of Ledger discoveries on prestige");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_node_set_tier (node, 1);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    /* Tier 2: Pattern Recognition - +25% discovery speed */
    node = lrg_unlock_node_new ("pattern-recognition", "Pattern Recognition");
    lrg_unlock_node_set_description (node, "Discover Ledger entries 25% faster");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_node_set_tier (node, 2);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "pattern-recognition", "memory-fragments");
    lrg_unlock_node_free (node);

    /* Tier 3: Cosmic Insight - Hidden investments unlocked */
    node = lrg_unlock_node_new ("cosmic-insight", "Cosmic Insight");
    lrg_unlock_node_set_description (node, "Gain access to hidden investment opportunities");
    lrg_unlock_node_set_cost_simple (node, 10.0);
    lrg_unlock_node_set_tier (node, 3);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "cosmic-insight", "pattern-recognition");
    lrg_unlock_node_free (node);

    /* Tier 4: Omniscience - Full Ledger persists */
    node = lrg_unlock_node_new ("omniscience", "Omniscience");
    lrg_unlock_node_set_description (node, "The Ledger persists completely across prestige");
    lrg_unlock_node_set_cost_simple (node, 25.0);
    lrg_unlock_node_set_tier (node, 4);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "omniscience", "cosmic-insight");
    lrg_unlock_node_free (node);

    return tree;
}

/*
 * Creates the Architect echo tree with all upgrades.
 * The Architect tree focuses on preservation and project bonuses.
 */
static LrgUnlockTree *
create_architect_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Tier 1: Phylactery Preservation - Keep 1 upgrade */
    node = lrg_unlock_node_new ("phylactery-preservation", "Phylactery Preservation");
    lrg_unlock_node_set_description (node, "Retain one Phylactery upgrade on prestige");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_node_set_tier (node, 1);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    /* Tier 2: Eternal Projects - Keep 25% megaproject progress */
    node = lrg_unlock_node_new ("eternal-projects", "Eternal Projects");
    lrg_unlock_node_set_description (node, "Megaprojects retain 25% progress on prestige");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_node_set_tier (node, 2);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "eternal-projects", "phylactery-preservation");
    lrg_unlock_node_free (node);

    /* Tier 3: Dimensional Vault - Keep 50% of gold */
    node = lrg_unlock_node_new ("dimensional-vault", "Dimensional Vault");
    lrg_unlock_node_set_description (node, "Retain 50% of gold on prestige");
    lrg_unlock_node_set_cost_simple (node, 10.0);
    lrg_unlock_node_set_tier (node, 3);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "dimensional-vault", "eternal-projects");
    lrg_unlock_node_free (node);

    /* Tier 4: Immortal Holdings - Keep 1 investment */
    node = lrg_unlock_node_new ("immortal-holdings", "Immortal Holdings");
    lrg_unlock_node_set_description (node, "One investment persists across prestige");
    lrg_unlock_node_set_cost_simple (node, 25.0);
    lrg_unlock_node_set_tier (node, 4);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_tree_add_requirement (tree, "immortal-holdings", "dimensional-vault");
    lrg_unlock_node_free (node);

    return tree;
}

/* ==========================================================================
 * Virtual Method Defaults
 * ========================================================================== */

static LrgBigNumber *
lp_prestige_manager_real_calculate_echo_reward (LpPrestigeManager  *self,
                                                const LrgBigNumber *total_gold,
                                                guint64             years_played)
{
    /*
     * Default formula: log10(total_gold) * sqrt(years_played) / 10
     * This rewards both wealth accumulation and patience.
     */
    gdouble gold_value;
    gdouble log_gold;
    gdouble years_factor;
    gdouble echoes;

    gold_value = lrg_big_number_to_double (total_gold);
    if (gold_value <= 1.0)
        return lrg_big_number_new (0.0);

    log_gold = log10 (gold_value);
    years_factor = sqrt ((gdouble)years_played);
    echoes = (log_gold * years_factor) / 10.0;

    /* Floor to whole number */
    echoes = floor (echoes);

    return lrg_big_number_new (echoes);
}

static gboolean
lp_prestige_manager_real_can_prestige (LpPrestigeManager  *self,
                                       const LrgBigNumber *total_gold,
                                       guint64             years_played)
{
    /*
     * Default requirements:
     * - At least 100 years played
     * - At least 1,000,000 gold accumulated
     */
    g_autoptr(LrgBigNumber) threshold = NULL;

    if (years_played < 100)
        return FALSE;

    threshold = lrg_big_number_new (1000000.0);
    return lrg_big_number_compare (total_gold, threshold) >= 0;
}

static void
lp_prestige_manager_real_on_prestige (LpPrestigeManager  *self,
                                      const LrgBigNumber *echoes_gained)
{
    /* Default: just log it */
    g_autofree gchar *echoes_str = lrg_big_number_format_short (echoes_gained);
    lp_log_info ("Prestige performed, gained %s Echoes", echoes_str);
}

static gdouble
lp_prestige_manager_real_get_bonus_multiplier (LpPrestigeManager *self)
{
    LpPrestigeManagerPrivate *priv;
    gdouble multiplier;
    gint i;

    priv = lp_prestige_manager_get_instance_private (self);

    /*
     * Base multiplier: 1.0 + (0.1 * times_prestiged)
     * Each prestige gives +10% permanent bonus.
     */
    multiplier = 1.0 + (0.1 * (gdouble)priv->times_prestiged);

    /* Add bonuses from Echo trees */
    for (i = 0; i < NUM_ECHO_TREES; i++)
    {
        gdouble progress = lrg_unlock_tree_get_progress (priv->echo_trees[i]);
        /* Each tree can add up to 0.5 multiplier at full completion */
        multiplier += progress * 0.5;
    }

    return multiplier;
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_prestige_manager_get_save_id (LrgSaveable *saveable)
{
    return "prestige-manager";
}

static gboolean
lp_prestige_manager_save (LrgSaveable    *saveable,
                          LrgSaveContext *context,
                          GError        **error)
{
    LpPrestigeManager *self = LP_PRESTIGE_MANAGER (saveable);
    LpPrestigeManagerPrivate *priv = lp_prestige_manager_get_instance_private (self);
    gint i;

    /* Save echoes as mantissa/exponent pair */
    lrg_save_context_write_double (context, "echoes-mantissa",
                                   lrg_big_number_get_mantissa (priv->echoes));
    lrg_save_context_write_int (context, "echoes-exponent",
                                lrg_big_number_get_exponent (priv->echoes));

    /* Save total echoes as mantissa/exponent pair */
    lrg_save_context_write_double (context, "total-echoes-mantissa",
                                   lrg_big_number_get_mantissa (priv->total_echoes_earned));
    lrg_save_context_write_int (context, "total-echoes-exponent",
                                lrg_big_number_get_exponent (priv->total_echoes_earned));
    lrg_save_context_write_uint (context, "times-prestiged", priv->times_prestiged);

    /* Save each Echo tree's unlocked nodes */
    for (i = 0; i < NUM_ECHO_TREES; i++)
    {
        g_autofree gchar *tree_name = NULL;
        GPtrArray *unlocked;
        guint j;

        tree_name = g_strdup_printf ("echo-tree-%d", i);
        lrg_save_context_begin_section (context, tree_name);

        unlocked = lrg_unlock_tree_get_unlocked (priv->echo_trees[i]);
        lrg_save_context_write_uint (context, "unlocked-count", unlocked->len);

        for (j = 0; j < unlocked->len; j++)
        {
            LrgUnlockNode *node = g_ptr_array_index (unlocked, j);
            g_autofree gchar *key = g_strdup_printf ("unlock-%u", j);
            lrg_save_context_write_string (context, key, lrg_unlock_node_get_id (node));
        }

        g_ptr_array_unref (unlocked);
        lrg_save_context_end_section (context);
    }

    return TRUE;
}

static gboolean
lp_prestige_manager_load (LrgSaveable    *saveable,
                          LrgSaveContext *context,
                          GError        **error)
{
    LpPrestigeManager *self = LP_PRESTIGE_MANAGER (saveable);
    LpPrestigeManagerPrivate *priv = lp_prestige_manager_get_instance_private (self);
    gdouble mantissa;
    gint64 exponent;
    gint i;

    /* Load echoes from mantissa/exponent */
    mantissa = lrg_save_context_read_double (context, "echoes-mantissa", 0.0);
    exponent = lrg_save_context_read_int (context, "echoes-exponent", 0);
    lrg_big_number_free (priv->echoes);
    if (mantissa == 0.0)
        priv->echoes = lrg_big_number_new_zero ();
    else
        priv->echoes = lrg_big_number_new_from_parts (mantissa, exponent);

    /* Load total echoes from mantissa/exponent */
    mantissa = lrg_save_context_read_double (context, "total-echoes-mantissa", 0.0);
    exponent = lrg_save_context_read_int (context, "total-echoes-exponent", 0);
    lrg_big_number_free (priv->total_echoes_earned);
    if (mantissa == 0.0)
        priv->total_echoes_earned = lrg_big_number_new_zero ();
    else
        priv->total_echoes_earned = lrg_big_number_new_from_parts (mantissa, exponent);

    priv->times_prestiged = lrg_save_context_read_uint (context, "times-prestiged", 0);

    /* Load each Echo tree's unlocked nodes */
    for (i = 0; i < NUM_ECHO_TREES; i++)
    {
        g_autofree gchar *tree_name = NULL;
        guint unlock_count;
        guint j;

        /* Reset tree first */
        lrg_unlock_tree_reset (priv->echo_trees[i]);

        tree_name = g_strdup_printf ("echo-tree-%d", i);
        if (!lrg_save_context_enter_section (context, tree_name))
            continue;

        unlock_count = (guint)lrg_save_context_read_uint (context, "unlocked-count", 0);

        for (j = 0; j < unlock_count; j++)
        {
            g_autofree gchar *key = g_strdup_printf ("unlock-%u", j);
            const gchar *upgrade_id = lrg_save_context_read_string (context, key, NULL);

            if (upgrade_id != NULL)
                lrg_unlock_tree_unlock (priv->echo_trees[i], upgrade_id);
        }

        lrg_save_context_leave_section (context);
    }

    {
        g_autofree gchar *echoes_str = lrg_big_number_format_short (priv->echoes);
        lp_log_debug ("Loaded prestige manager: %s Echoes, %lu times prestiged",
                      echoes_str, priv->times_prestiged);
    }

    return TRUE;
}

static void
lp_prestige_manager_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_prestige_manager_get_save_id;
    iface->save = lp_prestige_manager_save;
    iface->load = lp_prestige_manager_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_prestige_manager_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    LpPrestigeManager *self = LP_PRESTIGE_MANAGER (object);
    LpPrestigeManagerPrivate *priv = lp_prestige_manager_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_ECHOES:
        g_value_set_boxed (value, priv->echoes);
        break;

    case PROP_TOTAL_ECHOES_EARNED:
        g_value_set_boxed (value, priv->total_echoes_earned);
        break;

    case PROP_TIMES_PRESTIGED:
        g_value_set_uint64 (value, priv->times_prestiged);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_prestige_manager_finalize (GObject *object)
{
    LpPrestigeManager *self = LP_PRESTIGE_MANAGER (object);
    LpPrestigeManagerPrivate *priv = lp_prestige_manager_get_instance_private (self);
    gint i;

    lp_log_debug ("Finalizing prestige manager");

    lrg_big_number_free (priv->echoes);
    lrg_big_number_free (priv->total_echoes_earned);

    for (i = 0; i < NUM_ECHO_TREES; i++)
        g_clear_object (&priv->echo_trees[i]);

    G_OBJECT_CLASS (lp_prestige_manager_parent_class)->finalize (object);
}

static void
lp_prestige_manager_class_init (LpPrestigeManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_prestige_manager_get_property;
    object_class->finalize = lp_prestige_manager_finalize;

    /* Virtual method defaults */
    klass->calculate_echo_reward = lp_prestige_manager_real_calculate_echo_reward;
    klass->can_prestige = lp_prestige_manager_real_can_prestige;
    klass->on_prestige = lp_prestige_manager_real_on_prestige;
    klass->get_bonus_multiplier = lp_prestige_manager_real_get_bonus_multiplier;

    /**
     * LpPrestigeManager:echoes:
     *
     * Current Echo point count.
     */
    properties[PROP_ECHOES] =
        g_param_spec_boxed ("echoes",
                            "Echoes",
                            "Current Echo points",
                            LRG_TYPE_BIG_NUMBER,
                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * LpPrestigeManager:total-echoes-earned:
     *
     * Total Echoes ever earned.
     */
    properties[PROP_TOTAL_ECHOES_EARNED] =
        g_param_spec_boxed ("total-echoes-earned",
                            "Total Echoes Earned",
                            "Total Echo points earned all-time",
                            LRG_TYPE_BIG_NUMBER,
                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * LpPrestigeManager:times-prestiged:
     *
     * Number of times prestige has been performed.
     */
    properties[PROP_TIMES_PRESTIGED] =
        g_param_spec_uint64 ("times-prestiged",
                             "Times Prestiged",
                             "Number of prestige resets performed",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpPrestigeManager::prestige-performed:
     * @self: the #LpPrestigeManager
     * @echoes_gained: Echoes earned from this prestige
     *
     * Emitted when prestige is performed.
     */
    signals[SIGNAL_PRESTIGE_PERFORMED] =
        g_signal_new ("prestige-performed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      LRG_TYPE_BIG_NUMBER);

    /**
     * LpPrestigeManager::upgrade-unlocked:
     * @self: the #LpPrestigeManager
     * @tree: which Echo tree
     * @upgrade_id: ID of the unlocked upgrade
     *
     * Emitted when an Echo tree upgrade is unlocked.
     */
    signals[SIGNAL_UPGRADE_UNLOCKED] =
        g_signal_new ("upgrade-unlocked",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      LP_TYPE_ECHO_TREE,
                      G_TYPE_STRING);
}

static void
lp_prestige_manager_init (LpPrestigeManager *self)
{
    LpPrestigeManagerPrivate *priv = lp_prestige_manager_get_instance_private (self);

    priv->echoes = lrg_big_number_new (0.0);
    priv->total_echoes_earned = lrg_big_number_new (0.0);
    priv->times_prestiged = 0;

    /* Initialize Echo trees */
    priv->echo_trees[LP_ECHO_TREE_ECONOMIST] = create_economist_tree ();
    priv->echo_trees[LP_ECHO_TREE_MANIPULATOR] = create_manipulator_tree ();
    priv->echo_trees[LP_ECHO_TREE_SCHOLAR] = create_scholar_tree ();
    priv->echo_trees[LP_ECHO_TREE_ARCHITECT] = create_architect_tree ();
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_prestige_manager_new:
 *
 * Creates a new prestige manager.
 *
 * Returns: (transfer full): A new #LpPrestigeManager
 */
LpPrestigeManager *
lp_prestige_manager_new (void)
{
    return g_object_new (LP_TYPE_PRESTIGE_MANAGER, NULL);
}

/* ==========================================================================
 * Echo Management
 * ========================================================================== */

/**
 * lp_prestige_manager_get_echoes:
 * @self: an #LpPrestigeManager
 *
 * Gets the current Echo count.
 *
 * Returns: (transfer none): Current Echoes
 */
const LrgBigNumber *
lp_prestige_manager_get_echoes (LpPrestigeManager *self)
{
    LpPrestigeManagerPrivate *priv;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), NULL);

    priv = lp_prestige_manager_get_instance_private (self);
    return priv->echoes;
}

/**
 * lp_prestige_manager_get_total_echoes_earned:
 * @self: an #LpPrestigeManager
 *
 * Gets the total Echoes ever earned.
 *
 * Returns: (transfer none): Total Echoes earned
 */
const LrgBigNumber *
lp_prestige_manager_get_total_echoes_earned (LpPrestigeManager *self)
{
    LpPrestigeManagerPrivate *priv;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), NULL);

    priv = lp_prestige_manager_get_instance_private (self);
    return priv->total_echoes_earned;
}

/**
 * lp_prestige_manager_spend_echoes:
 * @self: an #LpPrestigeManager
 * @amount: Echoes to spend
 *
 * Spends Echoes.
 *
 * Returns: %TRUE if successfully spent
 */
gboolean
lp_prestige_manager_spend_echoes (LpPrestigeManager  *self,
                                  const LrgBigNumber *amount)
{
    LpPrestigeManagerPrivate *priv;
    g_autoptr(LrgBigNumber) new_echoes = NULL;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), FALSE);
    g_return_val_if_fail (amount != NULL, FALSE);

    priv = lp_prestige_manager_get_instance_private (self);

    /* Check if we have enough */
    if (lrg_big_number_compare (priv->echoes, amount) < 0)
        return FALSE;

    new_echoes = lrg_big_number_subtract (priv->echoes, amount);
    lrg_big_number_free (priv->echoes);
    priv->echoes = g_steal_pointer (&new_echoes);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ECHOES]);

    return TRUE;
}

/**
 * lp_prestige_manager_get_times_prestiged:
 * @self: an #LpPrestigeManager
 *
 * Gets how many times prestige has been performed.
 *
 * Returns: Prestige count
 */
guint64
lp_prestige_manager_get_times_prestiged (LpPrestigeManager *self)
{
    LpPrestigeManagerPrivate *priv;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), 0);

    priv = lp_prestige_manager_get_instance_private (self);
    return priv->times_prestiged;
}

/* ==========================================================================
 * Echo Specialization Trees
 * ========================================================================== */

/**
 * lp_prestige_manager_get_echo_tree:
 * @self: an #LpPrestigeManager
 * @tree: Which specialization tree to get
 *
 * Gets the unlock tree for a specific Echo specialization.
 *
 * Returns: (transfer none): The unlock tree
 */
LrgUnlockTree *
lp_prestige_manager_get_echo_tree (LpPrestigeManager *self,
                                   LpEchoTree         tree)
{
    LpPrestigeManagerPrivate *priv;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), NULL);
    g_return_val_if_fail (tree < NUM_ECHO_TREES, NULL);

    priv = lp_prestige_manager_get_instance_private (self);
    return priv->echo_trees[tree];
}

/**
 * lp_prestige_manager_unlock_upgrade:
 * @self: an #LpPrestigeManager
 * @tree: Which specialization tree
 * @upgrade_id: ID of the upgrade to unlock
 *
 * Attempts to unlock an upgrade in an Echo tree.
 *
 * Returns: %TRUE if successfully unlocked
 */
gboolean
lp_prestige_manager_unlock_upgrade (LpPrestigeManager *self,
                                    LpEchoTree         tree,
                                    const gchar       *upgrade_id)
{
    LpPrestigeManagerPrivate *priv;
    LrgUnlockTree *unlock_tree;
    LrgUnlockNode *node;
    const LrgBigNumber *cost;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), FALSE);
    g_return_val_if_fail (tree < NUM_ECHO_TREES, FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    priv = lp_prestige_manager_get_instance_private (self);
    unlock_tree = priv->echo_trees[tree];

    /* Check if can unlock */
    if (!lrg_unlock_tree_can_unlock (unlock_tree, upgrade_id, priv->echoes))
        return FALSE;

    /* Get the cost */
    node = lrg_unlock_tree_get_node (unlock_tree, upgrade_id);
    if (node == NULL)
        return FALSE;

    cost = lrg_unlock_node_get_cost (node);

    /* Spend echoes and unlock */
    if (!lp_prestige_manager_spend_echoes (self, cost))
        return FALSE;

    if (!lrg_unlock_tree_unlock (unlock_tree, upgrade_id))
        return FALSE;

    lp_log_info ("Unlocked Echo upgrade: tree=%d, upgrade=%s", tree, upgrade_id);

    g_signal_emit (self, signals[SIGNAL_UPGRADE_UNLOCKED], 0, tree, upgrade_id);

    return TRUE;
}

/**
 * lp_prestige_manager_has_upgrade:
 * @self: an #LpPrestigeManager
 * @tree: Which specialization tree
 * @upgrade_id: ID of the upgrade to check
 *
 * Checks if an upgrade is unlocked.
 *
 * Returns: %TRUE if unlocked
 */
gboolean
lp_prestige_manager_has_upgrade (LpPrestigeManager *self,
                                 LpEchoTree         tree,
                                 const gchar       *upgrade_id)
{
    LpPrestigeManagerPrivate *priv;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), FALSE);
    g_return_val_if_fail (tree < NUM_ECHO_TREES, FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    priv = lp_prestige_manager_get_instance_private (self);
    return lrg_unlock_tree_is_unlocked (priv->echo_trees[tree], upgrade_id);
}

/* ==========================================================================
 * Prestige Operations
 * ========================================================================== */

/**
 * lp_prestige_manager_calculate_echo_reward:
 * @self: an #LpPrestigeManager
 * @total_gold: Total gold accumulated this run
 * @years_played: Years played this run
 *
 * Calculates how many Echoes would be gained from prestige.
 *
 * Returns: (transfer full): Pending Echoes
 */
LrgBigNumber *
lp_prestige_manager_calculate_echo_reward (LpPrestigeManager  *self,
                                           const LrgBigNumber *total_gold,
                                           guint64             years_played)
{
    LpPrestigeManagerClass *klass;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), NULL);
    g_return_val_if_fail (total_gold != NULL, NULL);

    klass = LP_PRESTIGE_MANAGER_GET_CLASS (self);
    g_return_val_if_fail (klass->calculate_echo_reward != NULL, NULL);

    return klass->calculate_echo_reward (self, total_gold, years_played);
}

/**
 * lp_prestige_manager_can_prestige:
 * @self: an #LpPrestigeManager
 * @total_gold: Total gold accumulated this run
 * @years_played: Years played this run
 *
 * Checks if prestige is available.
 *
 * Returns: %TRUE if prestige requirements are met
 */
gboolean
lp_prestige_manager_can_prestige (LpPrestigeManager  *self,
                                  const LrgBigNumber *total_gold,
                                  guint64             years_played)
{
    LpPrestigeManagerClass *klass;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), FALSE);
    g_return_val_if_fail (total_gold != NULL, FALSE);

    klass = LP_PRESTIGE_MANAGER_GET_CLASS (self);
    g_return_val_if_fail (klass->can_prestige != NULL, FALSE);

    return klass->can_prestige (self, total_gold, years_played);
}

/**
 * lp_prestige_manager_perform_prestige:
 * @self: an #LpPrestigeManager
 * @total_gold: Total gold accumulated this run
 * @years_played: Years played this run
 *
 * Performs prestige, adding reward to Echoes.
 *
 * Returns: (transfer full): Echoes awarded (for display)
 */
LrgBigNumber *
lp_prestige_manager_perform_prestige (LpPrestigeManager  *self,
                                      const LrgBigNumber *total_gold,
                                      guint64             years_played)
{
    LpPrestigeManagerPrivate *priv;
    LpPrestigeManagerClass *klass;
    g_autoptr(LrgBigNumber) reward = NULL;
    g_autoptr(LrgBigNumber) new_echoes = NULL;
    g_autoptr(LrgBigNumber) new_total = NULL;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), NULL);
    g_return_val_if_fail (total_gold != NULL, NULL);

    priv = lp_prestige_manager_get_instance_private (self);
    klass = LP_PRESTIGE_MANAGER_GET_CLASS (self);

    if (!lp_prestige_manager_can_prestige (self, total_gold, years_played))
    {
        lp_log_debug ("Prestige attempted but requirements not met");
        return NULL;
    }

    reward = lp_prestige_manager_calculate_echo_reward (self, total_gold, years_played);

    /* Add to echoes */
    new_echoes = lrg_big_number_add (priv->echoes, reward);
    lrg_big_number_free (priv->echoes);
    priv->echoes = lrg_big_number_copy (new_echoes);

    /* Update total */
    new_total = lrg_big_number_add (priv->total_echoes_earned, reward);
    lrg_big_number_free (priv->total_echoes_earned);
    priv->total_echoes_earned = lrg_big_number_copy (new_total);

    /* Increment count */
    priv->times_prestiged++;

    /* Call virtual on_prestige */
    if (klass->on_prestige != NULL)
        klass->on_prestige (self, reward);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ECHOES]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TOTAL_ECHOES_EARNED]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TIMES_PRESTIGED]);

    g_signal_emit (self, signals[SIGNAL_PRESTIGE_PERFORMED], 0, reward);

    return g_steal_pointer (&reward);
}

/**
 * lp_prestige_manager_get_bonus_multiplier:
 * @self: an #LpPrestigeManager
 *
 * Gets the current bonus multiplier from prestige.
 *
 * Returns: Bonus multiplier (1.0 = no bonus)
 */
gdouble
lp_prestige_manager_get_bonus_multiplier (LpPrestigeManager *self)
{
    LpPrestigeManagerClass *klass;

    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), 1.0);

    klass = LP_PRESTIGE_MANAGER_GET_CLASS (self);
    g_return_val_if_fail (klass->get_bonus_multiplier != NULL, 1.0);

    return klass->get_bonus_multiplier (self);
}

/* ==========================================================================
 * Bonus Queries (from Echo Trees)
 * ========================================================================== */

/**
 * lp_prestige_manager_get_starting_gold_multiplier:
 * @self: an #LpPrestigeManager
 *
 * Gets starting gold multiplier from Economist tree.
 *
 * Returns: Multiplier (1.0 = no bonus)
 */
gdouble
lp_prestige_manager_get_starting_gold_multiplier (LpPrestigeManager *self)
{
    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), 1.0);

    /* Startup Capital gives 2x starting gold */
    if (lp_prestige_manager_has_upgrade (self, LP_ECHO_TREE_ECONOMIST, "startup-capital"))
        return 2.0;

    return 1.0;
}

/**
 * lp_prestige_manager_get_compound_interest_bonus:
 * @self: an #LpPrestigeManager
 *
 * Gets bonus to compound interest from Economist tree.
 *
 * Returns: Additive bonus (e.g., 0.02 = +2%)
 */
gdouble
lp_prestige_manager_get_compound_interest_bonus (LpPrestigeManager *self)
{
    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), 0.0);

    /* Compound Master gives +2% base interest */
    if (lp_prestige_manager_has_upgrade (self, LP_ECHO_TREE_ECONOMIST, "compound-master"))
        return 0.02;

    return 0.0;
}

/**
 * lp_prestige_manager_get_ledger_retention:
 * @self: an #LpPrestigeManager
 *
 * Gets fraction of Ledger entries to keep on prestige from Scholar tree.
 *
 * Returns: Retention (0.0 = keep none, 1.0 = keep all)
 */
gdouble
lp_prestige_manager_get_ledger_retention (LpPrestigeManager *self)
{
    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), 0.0);

    /* Omniscience keeps full ledger */
    if (lp_prestige_manager_has_upgrade (self, LP_ECHO_TREE_SCHOLAR, "omniscience"))
        return 1.0;

    /* Memory Fragments keeps 25% */
    if (lp_prestige_manager_has_upgrade (self, LP_ECHO_TREE_SCHOLAR, "memory-fragments"))
        return 0.25;

    return 0.0;
}

/**
 * lp_prestige_manager_get_gold_retention:
 * @self: an #LpPrestigeManager
 *
 * Gets fraction of gold to keep on prestige from Architect tree.
 *
 * Returns: Retention (0.0 = keep none, 1.0 = keep all)
 */
gdouble
lp_prestige_manager_get_gold_retention (LpPrestigeManager *self)
{
    g_return_val_if_fail (LP_IS_PRESTIGE_MANAGER (self), 0.0);

    /* Dimensional Vault keeps 50% gold */
    if (lp_prestige_manager_has_upgrade (self, LP_ECHO_TREE_ARCHITECT, "dimensional-vault"))
        return 0.5;

    return 0.0;
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_prestige_manager_reset:
 * @self: an #LpPrestigeManager
 *
 * Resets all prestige progress.
 */
void
lp_prestige_manager_reset (LpPrestigeManager *self)
{
    LpPrestigeManagerPrivate *priv;
    gint i;

    g_return_if_fail (LP_IS_PRESTIGE_MANAGER (self));

    priv = lp_prestige_manager_get_instance_private (self);

    lp_log_debug ("Resetting prestige manager");

    lrg_big_number_free (priv->echoes);
    priv->echoes = lrg_big_number_new (0.0);

    lrg_big_number_free (priv->total_echoes_earned);
    priv->total_echoes_earned = lrg_big_number_new (0.0);

    priv->times_prestiged = 0;

    for (i = 0; i < NUM_ECHO_TREES; i++)
        lrg_unlock_tree_reset (priv->echo_trees[i]);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ECHOES]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TOTAL_ECHOES_EARNED]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TIMES_PRESTIGED]);
}
