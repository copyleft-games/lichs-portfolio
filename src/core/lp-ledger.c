/* lp-ledger.c - Discovery Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-ledger.h"

/* Internal structure for a ledger entry */
typedef struct
{
    gchar           *entry_id;
    LpLedgerCategory category;
} LedgerEntry;

static void
ledger_entry_free (LedgerEntry *entry)
{
    if (entry != NULL)
    {
        g_free (entry->entry_id);
        g_free (entry);
    }
}

struct _LpLedger
{
    GObject parent_instance;

    GHashTable *discoveries;     /* entry_id -> LedgerEntry */
    guint       category_counts[4];  /* Count per category */
};

enum
{
    SIGNAL_ENTRY_DISCOVERED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/* Forward declarations for LrgSaveable interface */
static void lp_ledger_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpLedger, lp_ledger, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_ledger_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_ledger_get_save_id (LrgSaveable *saveable)
{
    return "ledger";
}

static gboolean
lp_ledger_save (LrgSaveable    *saveable,
                LrgSaveContext *context,
                GError        **error)
{
    LpLedger *self = LP_LEDGER (saveable);
    GHashTableIter iter;
    gpointer key, value;
    guint count;

    g_hash_table_iter_init (&iter, self->discoveries);
    count = 0;

    /* Write each discovery as a section */
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry = (LedgerEntry *)value;
        g_autofree gchar *section_name = g_strdup_printf ("entry-%u", count);

        lrg_save_context_begin_section (context, section_name);
        lrg_save_context_write_string (context, "id", entry->entry_id);
        lrg_save_context_write_int (context, "category", (gint64)entry->category);
        lrg_save_context_end_section (context);

        count++;
    }

    lrg_save_context_write_uint (context, "discovery-count", count);

    return TRUE;
}

static gboolean
lp_ledger_load (LrgSaveable    *saveable,
                LrgSaveContext *context,
                GError        **error)
{
    LpLedger *self = LP_LEDGER (saveable);
    guint count;
    guint i;

    /* Clear existing discoveries */
    g_hash_table_remove_all (self->discoveries);
    for (i = 0; i < 4; i++)
        self->category_counts[i] = 0;

    count = (guint)lrg_save_context_read_uint (context, "discovery-count", 0);

    /* Load each discovery */
    for (i = 0; i < count; i++)
    {
        g_autofree gchar *section_name = g_strdup_printf ("entry-%u", i);
        const gchar *entry_id;
        LpLedgerCategory category;

        if (!lrg_save_context_enter_section (context, section_name))
            continue;

        entry_id = lrg_save_context_read_string (context, "id", NULL);
        category = (LpLedgerCategory)lrg_save_context_read_int (context, "category", 0);

        if (entry_id != NULL)
        {
            LedgerEntry *entry = g_new0 (LedgerEntry, 1);
            entry->entry_id = g_strdup (entry_id);
            entry->category = category;

            g_hash_table_insert (self->discoveries,
                                 g_strdup (entry_id),
                                 entry);

            if (category < 4)
                self->category_counts[category]++;
        }

        lrg_save_context_leave_section (context);
    }

    lp_log_debug ("Loaded %u discoveries from save", count);

    return TRUE;
}

static void
lp_ledger_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_ledger_get_save_id;
    iface->save = lp_ledger_save;
    iface->load = lp_ledger_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_ledger_finalize (GObject *object)
{
    LpLedger *self = LP_LEDGER (object);

    lp_log_debug ("Finalizing ledger");

    g_clear_pointer (&self->discoveries, g_hash_table_unref);

    G_OBJECT_CLASS (lp_ledger_parent_class)->finalize (object);
}

static void
lp_ledger_class_init (LpLedgerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_ledger_finalize;

    /**
     * LpLedger::entry-discovered:
     * @self: the #LpLedger
     * @entry_id: the ID of the discovered entry
     * @category: the category of the discovery
     *
     * Emitted when a new entry is discovered.
     */
    signals[SIGNAL_ENTRY_DISCOVERED] =
        g_signal_new ("entry-discovered",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_STRING,
                      LP_TYPE_LEDGER_CATEGORY);
}

static void
lp_ledger_init (LpLedger *self)
{
    guint i;

    self->discoveries = g_hash_table_new_full (g_str_hash,
                                                g_str_equal,
                                                g_free,
                                                (GDestroyNotify)ledger_entry_free);

    for (i = 0; i < 4; i++)
        self->category_counts[i] = 0;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_ledger_new:
 *
 * Creates a new ledger instance.
 *
 * Returns: (transfer full): A new #LpLedger
 */
LpLedger *
lp_ledger_new (void)
{
    return g_object_new (LP_TYPE_LEDGER, NULL);
}

/* ==========================================================================
 * Discovery Tracking
 * ========================================================================== */

/**
 * lp_ledger_has_discovered:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 *
 * Checks if an entry has been discovered.
 *
 * Returns: %TRUE if the entry is discovered
 */
gboolean
lp_ledger_has_discovered (LpLedger    *self,
                          const gchar *entry_id)
{
    g_return_val_if_fail (LP_IS_LEDGER (self), FALSE);
    g_return_val_if_fail (entry_id != NULL, FALSE);

    return g_hash_table_contains (self->discoveries, entry_id);
}

/**
 * lp_ledger_discover:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 * @category: the category of the discovery
 *
 * Records a new discovery.
 *
 * Returns: %TRUE if this was a new discovery
 */
gboolean
lp_ledger_discover (LpLedger         *self,
                    const gchar      *entry_id,
                    LpLedgerCategory  category)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), FALSE);
    g_return_val_if_fail (entry_id != NULL, FALSE);

    /* Already discovered */
    if (g_hash_table_contains (self->discoveries, entry_id))
        return FALSE;

    /* Create new entry */
    entry = g_new0 (LedgerEntry, 1);
    entry->entry_id = g_strdup (entry_id);
    entry->category = category;

    g_hash_table_insert (self->discoveries,
                         g_strdup (entry_id),
                         entry);

    if (category < 4)
        self->category_counts[category]++;

    lp_log_info ("New discovery: %s (category: %d)", entry_id, category);

    g_signal_emit (self, signals[SIGNAL_ENTRY_DISCOVERED], 0,
                   entry_id, category);

    return TRUE;
}

/**
 * lp_ledger_get_discovered_count:
 * @self: an #LpLedger
 *
 * Gets the total number of discovered entries.
 *
 * Returns: Number of discoveries
 */
guint
lp_ledger_get_discovered_count (LpLedger *self)
{
    g_return_val_if_fail (LP_IS_LEDGER (self), 0);

    return g_hash_table_size (self->discoveries);
}

/**
 * lp_ledger_get_discovered_in_category:
 * @self: an #LpLedger
 * @category: the category to count
 *
 * Gets the number of discoveries in a specific category.
 *
 * Returns: Number of discoveries in the category
 */
guint
lp_ledger_get_discovered_in_category (LpLedger         *self,
                                      LpLedgerCategory  category)
{
    g_return_val_if_fail (LP_IS_LEDGER (self), 0);
    g_return_val_if_fail (category < 4, 0);

    return self->category_counts[category];
}

/**
 * lp_ledger_get_all_discoveries:
 * @self: an #LpLedger
 *
 * Gets a list of all discovered entry IDs.
 *
 * Returns: (transfer container) (element-type utf8): List of entry IDs
 */
GList *
lp_ledger_get_all_discoveries (LpLedger *self)
{
    g_return_val_if_fail (LP_IS_LEDGER (self), NULL);

    return g_hash_table_get_keys (self->discoveries);
}

/**
 * lp_ledger_get_discoveries_by_category:
 * @self: an #LpLedger
 * @category: the category to filter by
 *
 * Gets a list of discovered entry IDs in a specific category.
 *
 * Returns: (transfer container) (element-type utf8): List of entry IDs
 */
GList *
lp_ledger_get_discoveries_by_category (LpLedger         *self,
                                       LpLedgerCategory  category)
{
    GHashTableIter iter;
    gpointer key, value;
    GList *result = NULL;

    g_return_val_if_fail (LP_IS_LEDGER (self), NULL);

    g_hash_table_iter_init (&iter, self->discoveries);

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry = (LedgerEntry *)value;

        if (entry->category == category)
            result = g_list_prepend (result, key);
    }

    return result;
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_ledger_clear_all:
 * @self: an #LpLedger
 *
 * Clears all discoveries.
 */
void
lp_ledger_clear_all (LpLedger *self)
{
    guint i;

    g_return_if_fail (LP_IS_LEDGER (self));

    lp_log_debug ("Clearing all ledger discoveries");

    g_hash_table_remove_all (self->discoveries);

    for (i = 0; i < 4; i++)
        self->category_counts[i] = 0;
}
