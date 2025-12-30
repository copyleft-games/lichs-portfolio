/* lp-phylactery.c - Upgrade Tree System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-phylactery.h"

/* Number of upgrade categories */
#define N_UPGRADE_CATEGORIES 5

/* Base values before upgrades */
#define BASE_MAX_SLUMBER_YEARS    100
#define BASE_MAX_AGENTS           3

struct _LpPhylactery
{
    GObject parent_instance;

    guint64     points;              /* Available phylactery points */
    guint64     total_points_earned; /* All-time points earned */

    /* Upgrade trees for each category */
    LrgUnlockTree *temporal_tree;    /* Temporal Mastery */
    LrgUnlockTree *network_tree;     /* Network Expansion */
    LrgUnlockTree *divination_tree;  /* Divination */
    LrgUnlockTree *resilience_tree;  /* Resilience */
    LrgUnlockTree *dark_arts_tree;   /* Dark Arts */
};

enum
{
    PROP_0,
    PROP_POINTS,
    PROP_TOTAL_POINTS_EARNED,
    PROP_LEVEL,
    N_PROPS
};

enum
{
    SIGNAL_POINTS_CHANGED,
    SIGNAL_UPGRADE_PURCHASED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_phylactery_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpPhylactery, lp_phylactery, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_phylactery_saveable_init))

/* ==========================================================================
 * Upgrade Tree Creation
 * ========================================================================== */

/*
 * Creates the Temporal Mastery upgrade tree.
 * Upgrades: longer slumber, time efficiency
 *
 * Structure:
 *   extended-slumber-1 (1) -> extended-slumber-2 (3) -> extended-slumber-3 (8)
 *   time-compression-1 (2) -> time-compression-2 (5) -> temporal-mastery (15)
 */
static LrgUnlockTree *
create_temporal_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Extended Slumber branch - increases max slumber years */
    node = lrg_unlock_node_new ("extended-slumber-1", "Extended Slumber I");
    lrg_unlock_node_set_description (node, "Increase max slumber to 150 years");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("extended-slumber-2", "Extended Slumber II");
    lrg_unlock_node_set_description (node, "Increase max slumber to 250 years");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "extended-slumber-2", "extended-slumber-1");

    node = lrg_unlock_node_new ("extended-slumber-3", "Extended Slumber III");
    lrg_unlock_node_set_description (node, "Increase max slumber to 500 years");
    lrg_unlock_node_set_cost_simple (node, 8.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "extended-slumber-3", "extended-slumber-2");

    /* Time Compression branch - increases income efficiency */
    node = lrg_unlock_node_new ("time-compression-1", "Time Compression I");
    lrg_unlock_node_set_description (node, "+10% income per slumber year");
    lrg_unlock_node_set_cost_simple (node, 2.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("time-compression-2", "Time Compression II");
    lrg_unlock_node_set_description (node, "+25% income per slumber year");
    lrg_unlock_node_set_cost_simple (node, 5.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "time-compression-2", "time-compression-1");

    node = lrg_unlock_node_new ("temporal-mastery", "Temporal Mastery");
    lrg_unlock_node_set_description (node, "+50% income per slumber year");
    lrg_unlock_node_set_cost_simple (node, 15.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "temporal-mastery", "time-compression-2");
    lrg_unlock_tree_add_requirement (tree, "temporal-mastery", "extended-slumber-2");

    return tree;
}

/*
 * Creates the Network Expansion upgrade tree.
 * Upgrades: more agents, family agents, cult agents
 *
 * Structure:
 *   additional-agents-1 (1) -> additional-agents-2 (4) -> additional-agents-3 (10)
 *   family-legacy (3) -> bloodline-mastery (8)
 *   cult-initiation (5) -> (requires family-legacy) -> eternal-congregation (12)
 */
static LrgUnlockTree *
create_network_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Additional Agents branch */
    node = lrg_unlock_node_new ("additional-agents-1", "Expanded Network I");
    lrg_unlock_node_set_description (node, "+2 agent slots (5 total)");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("additional-agents-2", "Expanded Network II");
    lrg_unlock_node_set_description (node, "+3 agent slots (8 total)");
    lrg_unlock_node_set_cost_simple (node, 4.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "additional-agents-2", "additional-agents-1");

    node = lrg_unlock_node_new ("additional-agents-3", "Vast Network");
    lrg_unlock_node_set_description (node, "+4 agent slots (12 total)");
    lrg_unlock_node_set_cost_simple (node, 10.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "additional-agents-3", "additional-agents-2");

    /* Family branch */
    node = lrg_unlock_node_new ("family-legacy", "Family Legacy");
    lrg_unlock_node_set_description (node, "Unlock family agents with bloodline traits");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("bloodline-mastery", "Bloodline Mastery");
    lrg_unlock_node_set_description (node, "Improved trait inheritance for families");
    lrg_unlock_node_set_cost_simple (node, 8.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "bloodline-mastery", "family-legacy");

    /* Cult branch */
    node = lrg_unlock_node_new ("cult-initiation", "Cult Initiation");
    lrg_unlock_node_set_description (node, "Unlock cult agents");
    lrg_unlock_node_set_cost_simple (node, 5.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "cult-initiation", "family-legacy");

    node = lrg_unlock_node_new ("eternal-congregation", "Eternal Congregation");
    lrg_unlock_node_set_description (node, "Cults persist indefinitely and grow faster");
    lrg_unlock_node_set_cost_simple (node, 12.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "eternal-congregation", "cult-initiation");

    return tree;
}

/*
 * Creates the Divination upgrade tree.
 * Upgrades: better predictions, early warnings
 *
 * Structure:
 *   basic-scrying (1) -> improved-scrying (3) -> perfect-foresight (12)
 *   event-sensing (2) -> prophetic-visions (6) -> omniscience (20)
 */
static LrgUnlockTree *
create_divination_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Scrying branch - prediction accuracy */
    node = lrg_unlock_node_new ("basic-scrying", "Basic Scrying");
    lrg_unlock_node_set_description (node, "+15% event prediction accuracy");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("improved-scrying", "Improved Scrying");
    lrg_unlock_node_set_description (node, "+30% event prediction accuracy");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "improved-scrying", "basic-scrying");

    node = lrg_unlock_node_new ("perfect-foresight", "Perfect Foresight");
    lrg_unlock_node_set_description (node, "+50% event prediction accuracy");
    lrg_unlock_node_set_cost_simple (node, 12.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "perfect-foresight", "improved-scrying");

    /* Warning branch - advance notice of events */
    node = lrg_unlock_node_new ("event-sensing", "Event Sensing");
    lrg_unlock_node_set_description (node, "10 years warning before major events");
    lrg_unlock_node_set_cost_simple (node, 2.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("prophetic-visions", "Prophetic Visions");
    lrg_unlock_node_set_description (node, "25 years warning before major events");
    lrg_unlock_node_set_cost_simple (node, 6.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "prophetic-visions", "event-sensing");

    node = lrg_unlock_node_new ("omniscience", "Omniscience");
    lrg_unlock_node_set_description (node, "50 years warning, see all event outcomes");
    lrg_unlock_node_set_cost_simple (node, 20.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "omniscience", "prophetic-visions");
    lrg_unlock_tree_add_requirement (tree, "omniscience", "perfect-foresight");

    return tree;
}

/*
 * Creates the Resilience upgrade tree.
 * Upgrades: survive disasters, faster recovery, exposure decay
 *
 * Structure:
 *   contingency-plans (1) -> disaster-proofing (4) -> indestructible (15)
 *   quick-recovery (2) -> rapid-rebuilding (5)
 *   shadow-presence (3) -> unseen-hand (8) -> invisible (18)
 */
static LrgUnlockTree *
create_resilience_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Disaster survival branch */
    node = lrg_unlock_node_new ("contingency-plans", "Contingency Plans");
    lrg_unlock_node_set_description (node, "20% chance to avoid disaster losses");
    lrg_unlock_node_set_cost_simple (node, 1.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("disaster-proofing", "Disaster Proofing");
    lrg_unlock_node_set_description (node, "40% chance to avoid disaster losses");
    lrg_unlock_node_set_cost_simple (node, 4.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "disaster-proofing", "contingency-plans");

    node = lrg_unlock_node_new ("indestructible", "Indestructible");
    lrg_unlock_node_set_description (node, "70% chance to avoid disaster losses");
    lrg_unlock_node_set_cost_simple (node, 15.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "indestructible", "disaster-proofing");

    /* Recovery branch */
    node = lrg_unlock_node_new ("quick-recovery", "Quick Recovery");
    lrg_unlock_node_set_description (node, "50% faster recovery from disasters");
    lrg_unlock_node_set_cost_simple (node, 2.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("rapid-rebuilding", "Rapid Rebuilding");
    lrg_unlock_node_set_description (node, "100% faster recovery from disasters");
    lrg_unlock_node_set_cost_simple (node, 5.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "rapid-rebuilding", "quick-recovery");

    /* Exposure decay branch */
    node = lrg_unlock_node_new ("shadow-presence", "Shadow Presence");
    lrg_unlock_node_set_description (node, "+5 exposure decay per decade");
    lrg_unlock_node_set_cost_simple (node, 3.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    node = lrg_unlock_node_new ("unseen-hand", "Unseen Hand");
    lrg_unlock_node_set_description (node, "+10 exposure decay per decade");
    lrg_unlock_node_set_cost_simple (node, 8.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "unseen-hand", "shadow-presence");

    node = lrg_unlock_node_new ("invisible", "Invisible");
    lrg_unlock_node_set_description (node, "+20 exposure decay per decade");
    lrg_unlock_node_set_cost_simple (node, 18.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "invisible", "unseen-hand");

    return tree;
}

/*
 * Creates the Dark Arts upgrade tree.
 * Upgrades: dark investments, bound agents, dark income
 *
 * Structure:
 *   forbidden-knowledge (5) -> dark-investments (10)
 *                          -> soul-binding (12) -> legion-of-undead (25)
 *   dark-efficiency (8) -> shadow-economy (15) -> absolute-corruption (30)
 */
static LrgUnlockTree *
create_dark_arts_tree (void)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;

    tree = lrg_unlock_tree_new ();

    /* Gateway node */
    node = lrg_unlock_node_new ("forbidden-knowledge", "Forbidden Knowledge");
    lrg_unlock_node_set_description (node, "Begin studying the dark arts");
    lrg_unlock_node_set_cost_simple (node, 5.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);

    /* Dark investments branch */
    node = lrg_unlock_node_new ("dark-investments", "Dark Investments");
    lrg_unlock_node_set_description (node, "Unlock dark investment class");
    lrg_unlock_node_set_cost_simple (node, 10.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "dark-investments", "forbidden-knowledge");

    /* Bound agents branch */
    node = lrg_unlock_node_new ("soul-binding", "Soul Binding");
    lrg_unlock_node_set_description (node, "Unlock bound (undead) agents");
    lrg_unlock_node_set_cost_simple (node, 12.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "soul-binding", "forbidden-knowledge");

    node = lrg_unlock_node_new ("legion-of-undead", "Legion of Undead");
    lrg_unlock_node_set_description (node, "No limit on bound agents");
    lrg_unlock_node_set_cost_simple (node, 25.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "legion-of-undead", "soul-binding");

    /* Dark income branch */
    node = lrg_unlock_node_new ("dark-efficiency", "Dark Efficiency");
    lrg_unlock_node_set_description (node, "+25% dark investment income");
    lrg_unlock_node_set_cost_simple (node, 8.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "dark-efficiency", "forbidden-knowledge");

    node = lrg_unlock_node_new ("shadow-economy", "Shadow Economy");
    lrg_unlock_node_set_description (node, "+50% dark investment income");
    lrg_unlock_node_set_cost_simple (node, 15.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "shadow-economy", "dark-efficiency");
    lrg_unlock_tree_add_requirement (tree, "shadow-economy", "dark-investments");

    node = lrg_unlock_node_new ("absolute-corruption", "Absolute Corruption");
    lrg_unlock_node_set_description (node, "+100% dark investment income");
    lrg_unlock_node_set_cost_simple (node, 30.0);
    lrg_unlock_tree_add_node (tree, node);
    lrg_unlock_node_free (node);
    lrg_unlock_tree_add_requirement (tree, "absolute-corruption", "shadow-economy");

    return tree;
}

/* ==========================================================================
 * Helper Functions
 * ========================================================================== */

static LrgUnlockTree *
get_tree_for_category (LpPhylactery      *self,
                       LpUpgradeCategory  category)
{
    switch (category)
    {
        case LP_UPGRADE_CATEGORY_TEMPORAL:
            return self->temporal_tree;
        case LP_UPGRADE_CATEGORY_NETWORK:
            return self->network_tree;
        case LP_UPGRADE_CATEGORY_DIVINATION:
            return self->divination_tree;
        case LP_UPGRADE_CATEGORY_RESILIENCE:
            return self->resilience_tree;
        case LP_UPGRADE_CATEGORY_DARK_ARTS:
            return self->dark_arts_tree;
        default:
            g_warning ("Unknown upgrade category: %d", category);
            return NULL;
    }
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_phylactery_get_save_id (LrgSaveable *saveable)
{
    return "phylactery";
}

/*
 * save_unlock_tree_state:
 * @tree: The unlock tree to save
 * @context: The save context
 *
 * Saves the unlock state of a tree by storing the list of unlocked node IDs.
 */
static void
save_unlock_tree_state (LrgUnlockTree  *tree,
                        LrgSaveContext *context)
{
    g_autoptr(GPtrArray) unlocked = NULL;
    guint i;

    unlocked = lrg_unlock_tree_get_unlocked (tree);

    lrg_save_context_write_uint (context, "count", unlocked->len);

    for (i = 0; i < unlocked->len; i++)
    {
        LrgUnlockNode *node = g_ptr_array_index (unlocked, i);
        const gchar *node_id = lrg_unlock_node_get_id (node);
        g_autofree gchar *key = g_strdup_printf ("node-%u", i);
        lrg_save_context_write_string (context, key, node_id);
    }
}

/*
 * load_unlock_tree_state:
 * @tree: The unlock tree to load into
 * @context: The save context
 *
 * Loads the unlock state of a tree by restoring the list of unlocked node IDs.
 */
static void
load_unlock_tree_state (LrgUnlockTree  *tree,
                        LrgSaveContext *context)
{
    guint count;
    guint i;

    /* Reset the tree before loading */
    lrg_unlock_tree_reset (tree);

    count = lrg_save_context_read_uint (context, "count", 0);

    for (i = 0; i < count; i++)
    {
        g_autofree gchar *key = g_strdup_printf ("node-%u", i);
        g_autofree gchar *node_id = lrg_save_context_read_string (context, key, NULL);

        if (node_id != NULL)
        {
            /* Force unlock without currency check since we're restoring state */
            lrg_unlock_tree_unlock (tree, node_id);
        }
    }
}

static gboolean
lp_phylactery_save (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpPhylactery *self;

    (void)error;  /* Not used but required by interface */

    self = LP_PHYLACTERY (saveable);

    lrg_save_context_write_uint (context, "points", self->points);
    lrg_save_context_write_uint (context, "total-points-earned",
                                 self->total_points_earned);

    /* Save each upgrade tree's unlock state */
    lrg_save_context_begin_section (context, "temporal-tree");
    save_unlock_tree_state (self->temporal_tree, context);
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "network-tree");
    save_unlock_tree_state (self->network_tree, context);
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "divination-tree");
    save_unlock_tree_state (self->divination_tree, context);
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "resilience-tree");
    save_unlock_tree_state (self->resilience_tree, context);
    lrg_save_context_end_section (context);

    lrg_save_context_begin_section (context, "dark-arts-tree");
    save_unlock_tree_state (self->dark_arts_tree, context);
    lrg_save_context_end_section (context);

    return TRUE;
}

static gboolean
lp_phylactery_load (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpPhylactery *self;

    (void)error;  /* Not used but required by interface */

    self = LP_PHYLACTERY (saveable);

    self->points = lrg_save_context_read_uint (context, "points", 0);
    self->total_points_earned = lrg_save_context_read_uint (
        context, "total-points-earned", 0);

    /* Load each upgrade tree's unlock state */
    if (lrg_save_context_enter_section (context, "temporal-tree"))
    {
        load_unlock_tree_state (self->temporal_tree, context);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "network-tree"))
    {
        load_unlock_tree_state (self->network_tree, context);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "divination-tree"))
    {
        load_unlock_tree_state (self->divination_tree, context);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "resilience-tree"))
    {
        load_unlock_tree_state (self->resilience_tree, context);
        lrg_save_context_leave_section (context);
    }

    if (lrg_save_context_enter_section (context, "dark-arts-tree"))
    {
        load_unlock_tree_state (self->dark_arts_tree, context);
        lrg_save_context_leave_section (context);
    }

    lp_log_debug ("Loaded phylactery: %lu points available, %lu total earned, level %u",
                  self->points, self->total_points_earned,
                  lp_phylactery_get_level (self));

    return TRUE;
}

static void
lp_phylactery_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_phylactery_get_save_id;
    iface->save = lp_phylactery_save;
    iface->load = lp_phylactery_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_phylactery_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    LpPhylactery *self;

    self = LP_PHYLACTERY (object);

    switch (prop_id)
    {
        case PROP_POINTS:
            g_value_set_uint64 (value, self->points);
            break;
        case PROP_TOTAL_POINTS_EARNED:
            g_value_set_uint64 (value, self->total_points_earned);
            break;
        case PROP_LEVEL:
            g_value_set_uint (value, lp_phylactery_get_level (self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
lp_phylactery_finalize (GObject *object)
{
    LpPhylactery *self;

    self = LP_PHYLACTERY (object);

    lp_log_debug ("Finalizing phylactery");

    g_clear_object (&self->temporal_tree);
    g_clear_object (&self->network_tree);
    g_clear_object (&self->divination_tree);
    g_clear_object (&self->resilience_tree);
    g_clear_object (&self->dark_arts_tree);

    G_OBJECT_CLASS (lp_phylactery_parent_class)->finalize (object);
}

static void
lp_phylactery_class_init (LpPhylacteryClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_phylactery_get_property;
    object_class->finalize = lp_phylactery_finalize;

    /**
     * LpPhylactery:points:
     *
     * Available phylactery points.
     */
    properties[PROP_POINTS] =
        g_param_spec_uint64 ("points",
                             "Points",
                             "Available phylactery points",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * LpPhylactery:total-points-earned:
     *
     * Total phylactery points earned all-time.
     */
    properties[PROP_TOTAL_POINTS_EARNED] =
        g_param_spec_uint64 ("total-points-earned",
                             "Total Points Earned",
                             "Total phylactery points earned all-time",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * LpPhylactery:level:
     *
     * Phylactery level derived from total upgrades.
     */
    properties[PROP_LEVEL] =
        g_param_spec_uint ("level",
                           "Level",
                           "Phylactery level derived from upgrades",
                           0, G_MAXUINT, 0,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpPhylactery::points-changed:
     * @self: the #LpPhylactery
     * @old_points: previous point count
     * @new_points: new point count
     *
     * Emitted when available points change.
     */
    signals[SIGNAL_POINTS_CHANGED] =
        g_signal_new ("points-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_UINT64,
                      G_TYPE_UINT64);

    /**
     * LpPhylactery::upgrade-purchased:
     * @self: the #LpPhylactery
     * @category: the upgrade category
     * @upgrade_id: the purchased upgrade ID
     *
     * Emitted when an upgrade is purchased.
     */
    signals[SIGNAL_UPGRADE_PURCHASED] =
        g_signal_new ("upgrade-purchased",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 2,
                      LP_TYPE_UPGRADE_CATEGORY,
                      G_TYPE_STRING);
}

static void
lp_phylactery_init (LpPhylactery *self)
{
    self->points = 0;
    self->total_points_earned = 0;

    /* Create all upgrade trees */
    self->temporal_tree = create_temporal_tree ();
    self->network_tree = create_network_tree ();
    self->divination_tree = create_divination_tree ();
    self->resilience_tree = create_resilience_tree ();
    self->dark_arts_tree = create_dark_arts_tree ();
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpPhylactery *
lp_phylactery_new (void)
{
    return g_object_new (LP_TYPE_PHYLACTERY, NULL);
}

/* ==========================================================================
 * Points Management
 * ========================================================================== */

guint64
lp_phylactery_get_points (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);
    return self->points;
}

guint64
lp_phylactery_get_total_points_earned (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);
    return self->total_points_earned;
}

void
lp_phylactery_add_points (LpPhylactery *self,
                          guint64       points)
{
    guint64 old_points;

    g_return_if_fail (LP_IS_PHYLACTERY (self));

    if (points == 0)
        return;

    old_points = self->points;
    self->points += points;
    self->total_points_earned += points;

    lp_log_info ("Added %lu phylactery points (now: %lu)",
                 points, self->points);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POINTS]);
    g_signal_emit (self, signals[SIGNAL_POINTS_CHANGED], 0,
                   old_points, self->points);
}

guint
lp_phylactery_get_level (LpPhylactery *self)
{
    guint total_upgrades;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    total_upgrades = lp_phylactery_get_upgrade_count (self);

    /* Level formula: every 3 upgrades = 1 level, minimum level 1 */
    return 1 + (total_upgrades / 3);
}

/* ==========================================================================
 * Upgrade Tree Access
 * ========================================================================== */

LrgUnlockTree *
lp_phylactery_get_upgrade_tree (LpPhylactery      *self,
                                LpUpgradeCategory  category)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), NULL);
    return get_tree_for_category (self, category);
}

guint
lp_phylactery_get_upgrade_count (LpPhylactery *self)
{
    guint count;
    GPtrArray *unlocked;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    count = 0;

    unlocked = lrg_unlock_tree_get_unlocked (self->temporal_tree);
    count += unlocked->len;
    g_ptr_array_unref (unlocked);

    unlocked = lrg_unlock_tree_get_unlocked (self->network_tree);
    count += unlocked->len;
    g_ptr_array_unref (unlocked);

    unlocked = lrg_unlock_tree_get_unlocked (self->divination_tree);
    count += unlocked->len;
    g_ptr_array_unref (unlocked);

    unlocked = lrg_unlock_tree_get_unlocked (self->resilience_tree);
    count += unlocked->len;
    g_ptr_array_unref (unlocked);

    unlocked = lrg_unlock_tree_get_unlocked (self->dark_arts_tree);
    count += unlocked->len;
    g_ptr_array_unref (unlocked);

    return count;
}

guint
lp_phylactery_get_category_upgrade_count (LpPhylactery      *self,
                                          LpUpgradeCategory  category)
{
    LrgUnlockTree *tree;
    GPtrArray *unlocked;
    guint count;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    tree = get_tree_for_category (self, category);
    if (tree == NULL)
        return 0;

    unlocked = lrg_unlock_tree_get_unlocked (tree);
    count = unlocked->len;
    g_ptr_array_unref (unlocked);

    return count;
}

gboolean
lp_phylactery_has_upgrade (LpPhylactery *self,
                           const gchar  *upgrade_id)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    if (lrg_unlock_tree_is_unlocked (self->temporal_tree, upgrade_id))
        return TRUE;
    if (lrg_unlock_tree_is_unlocked (self->network_tree, upgrade_id))
        return TRUE;
    if (lrg_unlock_tree_is_unlocked (self->divination_tree, upgrade_id))
        return TRUE;
    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, upgrade_id))
        return TRUE;
    if (lrg_unlock_tree_is_unlocked (self->dark_arts_tree, upgrade_id))
        return TRUE;

    return FALSE;
}

gboolean
lp_phylactery_has_category_upgrade (LpPhylactery      *self,
                                    LpUpgradeCategory  category,
                                    const gchar       *upgrade_id)
{
    LrgUnlockTree *tree;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    tree = get_tree_for_category (self, category);
    if (tree == NULL)
        return FALSE;

    return lrg_unlock_tree_is_unlocked (tree, upgrade_id);
}

gboolean
lp_phylactery_can_purchase_upgrade (LpPhylactery      *self,
                                    LpUpgradeCategory  category,
                                    const gchar       *upgrade_id)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;
    const LrgBigNumber *cost;
    g_autoptr(LrgBigNumber) available = NULL;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    tree = get_tree_for_category (self, category);
    if (tree == NULL)
        return FALSE;

    node = lrg_unlock_tree_get_node (tree, upgrade_id);
    if (node == NULL)
        return FALSE;

    /* Check if already unlocked */
    if (lrg_unlock_tree_is_unlocked (tree, upgrade_id))
        return FALSE;

    /* Check prerequisites and affordability with lrg_unlock_tree_can_unlock */
    available = lrg_big_number_new ((gdouble) self->points);
    if (!lrg_unlock_tree_can_unlock (tree, upgrade_id, available))
        return FALSE;

    /* Double-check cost (can_unlock should handle this, but be explicit) */
    cost = lrg_unlock_node_get_cost (node);
    return self->points >= (guint64) lrg_big_number_to_double (cost);
}

gboolean
lp_phylactery_purchase_upgrade (LpPhylactery      *self,
                                LpUpgradeCategory  category,
                                const gchar       *upgrade_id)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;
    const LrgBigNumber *cost;
    gdouble cost_value;
    guint64 old_points;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    g_return_val_if_fail (upgrade_id != NULL, FALSE);

    if (!lp_phylactery_can_purchase_upgrade (self, category, upgrade_id))
        return FALSE;

    tree = get_tree_for_category (self, category);
    node = lrg_unlock_tree_get_node (tree, upgrade_id);
    cost = lrg_unlock_node_get_cost (node);
    cost_value = lrg_big_number_to_double (cost);

    /* Deduct cost */
    old_points = self->points;
    self->points -= (guint64) cost_value;

    /* Unlock the upgrade */
    lrg_unlock_tree_unlock (tree, upgrade_id);

    lp_log_info ("Purchased upgrade '%s' in category %d for %.0f points",
                 upgrade_id, category, cost_value);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POINTS]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LEVEL]);
    g_signal_emit (self, signals[SIGNAL_POINTS_CHANGED], 0,
                   old_points, self->points);
    g_signal_emit (self, signals[SIGNAL_UPGRADE_PURCHASED], 0,
                   category, upgrade_id);

    return TRUE;
}

guint64
lp_phylactery_get_upgrade_cost (LpPhylactery      *self,
                                LpUpgradeCategory  category,
                                const gchar       *upgrade_id)
{
    LrgUnlockTree *tree;
    LrgUnlockNode *node;
    const LrgBigNumber *cost;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);
    g_return_val_if_fail (upgrade_id != NULL, 0);

    tree = get_tree_for_category (self, category);
    if (tree == NULL)
        return 0;

    node = lrg_unlock_tree_get_node (tree, upgrade_id);
    if (node == NULL)
        return 0;

    cost = lrg_unlock_node_get_cost (node);
    return (guint64) lrg_big_number_to_double (cost);
}

/* ==========================================================================
 * Bonus Calculation - Temporal Mastery
 * ========================================================================== */

guint
lp_phylactery_get_max_slumber_years (LpPhylactery *self)
{
    guint max_years;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), BASE_MAX_SLUMBER_YEARS);

    max_years = BASE_MAX_SLUMBER_YEARS;

    if (lrg_unlock_tree_is_unlocked (self->temporal_tree, "extended-slumber-1"))
        max_years = 150;
    if (lrg_unlock_tree_is_unlocked (self->temporal_tree, "extended-slumber-2"))
        max_years = 250;
    if (lrg_unlock_tree_is_unlocked (self->temporal_tree, "extended-slumber-3"))
        max_years = 500;

    return max_years;
}

gdouble
lp_phylactery_get_time_efficiency_bonus (LpPhylactery *self)
{
    gdouble bonus;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 1.0);

    bonus = 1.0;

    if (lrg_unlock_tree_is_unlocked (self->temporal_tree, "time-compression-1"))
        bonus += 0.10;
    if (lrg_unlock_tree_is_unlocked (self->temporal_tree, "time-compression-2"))
        bonus += 0.15;  /* Total: 0.25 */
    if (lrg_unlock_tree_is_unlocked (self->temporal_tree, "temporal-mastery"))
        bonus += 0.25;  /* Total: 0.50 */

    return bonus;
}

/* ==========================================================================
 * Bonus Calculation - Network Expansion
 * ========================================================================== */

guint
lp_phylactery_get_max_agents (LpPhylactery *self)
{
    guint max_agents;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), BASE_MAX_AGENTS);

    max_agents = BASE_MAX_AGENTS;

    if (lrg_unlock_tree_is_unlocked (self->network_tree, "additional-agents-1"))
        max_agents = 5;
    if (lrg_unlock_tree_is_unlocked (self->network_tree, "additional-agents-2"))
        max_agents = 8;
    if (lrg_unlock_tree_is_unlocked (self->network_tree, "additional-agents-3"))
        max_agents = 12;

    return max_agents;
}

gboolean
lp_phylactery_has_family_agents (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    return lrg_unlock_tree_is_unlocked (self->network_tree, "family-legacy");
}

gboolean
lp_phylactery_has_cult_agents (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    return lrg_unlock_tree_is_unlocked (self->network_tree, "cult-initiation");
}

/* ==========================================================================
 * Bonus Calculation - Divination
 * ========================================================================== */

guint
lp_phylactery_get_prediction_bonus (LpPhylactery *self)
{
    guint bonus;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    bonus = 0;

    if (lrg_unlock_tree_is_unlocked (self->divination_tree, "basic-scrying"))
        bonus += 15;
    if (lrg_unlock_tree_is_unlocked (self->divination_tree, "improved-scrying"))
        bonus += 15;  /* Total: 30 */
    if (lrg_unlock_tree_is_unlocked (self->divination_tree, "perfect-foresight"))
        bonus += 20;  /* Total: 50 */

    return bonus;
}

guint
lp_phylactery_get_warning_years (LpPhylactery *self)
{
    guint years;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    years = 0;

    if (lrg_unlock_tree_is_unlocked (self->divination_tree, "event-sensing"))
        years = 10;
    if (lrg_unlock_tree_is_unlocked (self->divination_tree, "prophetic-visions"))
        years = 25;
    if (lrg_unlock_tree_is_unlocked (self->divination_tree, "omniscience"))
        years = 50;

    return years;
}

/* ==========================================================================
 * Bonus Calculation - Resilience
 * ========================================================================== */

guint
lp_phylactery_get_disaster_survival_bonus (LpPhylactery *self)
{
    guint bonus;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    bonus = 0;

    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "contingency-plans"))
        bonus = 20;
    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "disaster-proofing"))
        bonus = 40;
    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "indestructible"))
        bonus = 70;

    return bonus;
}

gdouble
lp_phylactery_get_recovery_bonus (LpPhylactery *self)
{
    gdouble bonus;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 1.0);

    bonus = 1.0;

    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "quick-recovery"))
        bonus += 0.50;
    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "rapid-rebuilding"))
        bonus += 0.50;  /* Total: 2.0 */

    return bonus;
}

guint
lp_phylactery_get_exposure_decay_bonus (LpPhylactery *self)
{
    guint bonus;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 0);

    bonus = 0;

    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "shadow-presence"))
        bonus += 5;
    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "unseen-hand"))
        bonus += 5;   /* Total: 10 */
    if (lrg_unlock_tree_is_unlocked (self->resilience_tree, "invisible"))
        bonus += 10;  /* Total: 20 */

    return bonus;
}

/* ==========================================================================
 * Bonus Calculation - Dark Arts
 * ========================================================================== */

gboolean
lp_phylactery_has_dark_investments (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    return lrg_unlock_tree_is_unlocked (self->dark_arts_tree, "dark-investments");
}

gboolean
lp_phylactery_has_bound_agents (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), FALSE);
    return lrg_unlock_tree_is_unlocked (self->dark_arts_tree, "soul-binding");
}

gdouble
lp_phylactery_get_dark_income_bonus (LpPhylactery *self)
{
    gdouble bonus;

    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 1.0);

    bonus = 1.0;

    if (lrg_unlock_tree_is_unlocked (self->dark_arts_tree, "dark-efficiency"))
        bonus += 0.25;
    if (lrg_unlock_tree_is_unlocked (self->dark_arts_tree, "shadow-economy"))
        bonus += 0.25;  /* Total: 1.50 */
    if (lrg_unlock_tree_is_unlocked (self->dark_arts_tree, "absolute-corruption"))
        bonus += 0.50;  /* Total: 2.00 */

    return bonus;
}

/* ==========================================================================
 * Legacy Bonus Calculation
 * ========================================================================== */

gdouble
lp_phylactery_get_starting_gold_bonus (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 1.0);

    /* No direct starting gold bonus in phylactery - use time efficiency */
    return 1.0;
}

gdouble
lp_phylactery_get_income_bonus (LpPhylactery *self)
{
    g_return_val_if_fail (LP_IS_PHYLACTERY (self), 1.0);

    /* Combine time efficiency and dark bonuses for general income */
    return lp_phylactery_get_time_efficiency_bonus (self);
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

void
lp_phylactery_reset_upgrades (LpPhylactery *self)
{
    g_return_if_fail (LP_IS_PHYLACTERY (self));

    lp_log_debug ("Resetting phylactery upgrades");

    /* Reset all trees */
    lrg_unlock_tree_reset (self->temporal_tree);
    lrg_unlock_tree_reset (self->network_tree);
    lrg_unlock_tree_reset (self->divination_tree);
    lrg_unlock_tree_reset (self->resilience_tree);
    lrg_unlock_tree_reset (self->dark_arts_tree);

    /* Refund all spent points */
    self->points = self->total_points_earned;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POINTS]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LEVEL]);
}

void
lp_phylactery_reset (LpPhylactery *self)
{
    g_return_if_fail (LP_IS_PHYLACTERY (self));

    lp_log_debug ("Full phylactery reset");

    lp_phylactery_reset_upgrades (self);
    self->points = 0;
    self->total_points_earned = 0;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POINTS]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TOTAL_POINTS_EARNED]);
}
