/* lp-ledger.c - Discovery Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-ledger.h"

/* Number of ledger categories */
#define N_LEDGER_CATEGORIES 4

/* Internal structure for a ledger entry */
typedef struct
{
    gchar            *entry_id;
    LpLedgerCategory  category;
    guint             occurrences_required;  /* How many occurrences for full discovery */
    guint             occurrences_current;   /* Current progress */
    gboolean          is_discovered;         /* TRUE when fully discovered */
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

    GHashTable *entries;             /* entry_id -> LedgerEntry */
    guint       discovered_counts[N_LEDGER_CATEGORIES];  /* Full discoveries per category */
};

enum
{
    SIGNAL_ENTRY_DISCOVERED,
    SIGNAL_ENTRY_PROGRESSED,
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
    LpLedger *self;
    GHashTableIter iter;
    gpointer key, value;
    guint count;

    self = LP_LEDGER (saveable);

    g_hash_table_iter_init (&iter, self->entries);
    count = 0;

    /* Write each entry as a section */
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry;
        g_autofree gchar *section_name = NULL;

        entry = (LedgerEntry *)value;
        section_name = g_strdup_printf ("entry-%u", count);

        lrg_save_context_begin_section (context, section_name);
        lrg_save_context_write_string (context, "id", entry->entry_id);
        lrg_save_context_write_int (context, "category", (gint64)entry->category);
        lrg_save_context_write_uint (context, "required", entry->occurrences_required);
        lrg_save_context_write_uint (context, "current", entry->occurrences_current);
        lrg_save_context_write_boolean (context, "discovered", entry->is_discovered);
        lrg_save_context_end_section (context);

        count++;
    }

    lrg_save_context_write_uint (context, "entry-count", count);

    return TRUE;
}

static gboolean
lp_ledger_load (LrgSaveable    *saveable,
                LrgSaveContext *context,
                GError        **error)
{
    LpLedger *self;
    guint count;
    guint i;

    self = LP_LEDGER (saveable);

    /* Clear existing entries */
    g_hash_table_remove_all (self->entries);
    for (i = 0; i < N_LEDGER_CATEGORIES; i++)
        self->discovered_counts[i] = 0;

    count = (guint)lrg_save_context_read_uint (context, "entry-count", 0);

    /* Load each entry */
    for (i = 0; i < count; i++)
    {
        g_autofree gchar *section_name = NULL;
        g_autofree gchar *entry_id = NULL;
        LpLedgerCategory category;
        guint required;
        guint current;
        gboolean discovered;

        section_name = g_strdup_printf ("entry-%u", i);

        if (!lrg_save_context_enter_section (context, section_name))
            continue;

        entry_id = lrg_save_context_read_string (context, "id", NULL);
        category = (LpLedgerCategory)lrg_save_context_read_int (context, "category", 0);
        required = (guint)lrg_save_context_read_uint (context, "required", 1);
        current = (guint)lrg_save_context_read_uint (context, "current", 0);
        discovered = lrg_save_context_read_boolean (context, "discovered", FALSE);

        if (entry_id != NULL)
        {
            LedgerEntry *entry;

            entry = g_new0 (LedgerEntry, 1);
            entry->entry_id = g_strdup (entry_id);
            entry->category = category;
            entry->occurrences_required = MAX (required, 1);
            entry->occurrences_current = current;
            entry->is_discovered = discovered;

            g_hash_table_insert (self->entries,
                                 g_strdup (entry_id),
                                 entry);

            if (discovered && category < N_LEDGER_CATEGORIES)
                self->discovered_counts[category]++;
        }

        lrg_save_context_leave_section (context);
    }

    lp_log_debug ("Loaded %u ledger entries from save", count);

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
    LpLedger *self;

    self = LP_LEDGER (object);

    lp_log_debug ("Finalizing ledger");

    g_clear_pointer (&self->entries, g_hash_table_unref);

    G_OBJECT_CLASS (lp_ledger_parent_class)->finalize (object);
}

static void
lp_ledger_class_init (LpLedgerClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_ledger_finalize;

    /**
     * LpLedger::entry-discovered:
     * @self: the #LpLedger
     * @entry_id: the ID of the discovered entry
     * @category: the category of the discovery
     *
     * Emitted when an entry becomes fully discovered.
     */
    signals[SIGNAL_ENTRY_DISCOVERED] =
        g_signal_new ("entry-discovered",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_STRING,
                      LP_TYPE_LEDGER_CATEGORY);

    /**
     * LpLedger::entry-progressed:
     * @self: the #LpLedger
     * @entry_id: the ID of the entry
     * @current: current progress
     * @required: required for full discovery
     *
     * Emitted when progress is made on an entry (but not yet fully discovered).
     */
    signals[SIGNAL_ENTRY_PROGRESSED] =
        g_signal_new ("entry-progressed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 3,
                      G_TYPE_STRING,
                      G_TYPE_UINT,
                      G_TYPE_UINT);
}

static void
lp_ledger_init (LpLedger *self)
{
    guint i;

    self->entries = g_hash_table_new_full (g_str_hash,
                                           g_str_equal,
                                           g_free,
                                           (GDestroyNotify)ledger_entry_free);

    for (i = 0; i < N_LEDGER_CATEGORIES; i++)
        self->discovered_counts[i] = 0;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

LpLedger *
lp_ledger_new (void)
{
    return g_object_new (LP_TYPE_LEDGER, NULL);
}

/* ==========================================================================
 * Entry Registration
 * ========================================================================== */

void
lp_ledger_register_entry (LpLedger         *self,
                          const gchar      *entry_id,
                          LpLedgerCategory  category,
                          guint             occurrences_required)
{
    LedgerEntry *entry;

    g_return_if_fail (LP_IS_LEDGER (self));
    g_return_if_fail (entry_id != NULL);
    g_return_if_fail (occurrences_required >= 1);

    /* Don't overwrite existing entries */
    if (g_hash_table_contains (self->entries, entry_id))
        return;

    entry = g_new0 (LedgerEntry, 1);
    entry->entry_id = g_strdup (entry_id);
    entry->category = category;
    entry->occurrences_required = occurrences_required;
    entry->occurrences_current = 0;
    entry->is_discovered = FALSE;

    g_hash_table_insert (self->entries,
                         g_strdup (entry_id),
                         entry);

    lp_log_debug ("Registered ledger entry: %s (requires %u occurrences)",
                  entry_id, occurrences_required);
}

gboolean
lp_ledger_is_registered (LpLedger    *self,
                         const gchar *entry_id)
{
    g_return_val_if_fail (LP_IS_LEDGER (self), FALSE);
    g_return_val_if_fail (entry_id != NULL, FALSE);

    return g_hash_table_contains (self->entries, entry_id);
}

/* ==========================================================================
 * Progress Tracking
 * ========================================================================== */

gboolean
lp_ledger_progress_entry (LpLedger          *self,
                          const gchar       *entry_id,
                          LpDiscoveryMethod  method)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), FALSE);
    g_return_val_if_fail (entry_id != NULL, FALSE);

    entry = g_hash_table_lookup (self->entries, entry_id);

    /* Auto-register if not found */
    if (entry == NULL)
    {
        lp_ledger_register_entry (self, entry_id,
                                  LP_LEDGER_CATEGORY_ECONOMIC, 1);
        entry = g_hash_table_lookup (self->entries, entry_id);
    }

    /* Already fully discovered */
    if (entry->is_discovered)
        return FALSE;

    /* Increment progress */
    entry->occurrences_current++;

    lp_log_debug ("Ledger progress: %s (%u/%u) via method %d",
                  entry_id, entry->occurrences_current,
                  entry->occurrences_required, method);

    /* Check if now fully discovered */
    if (entry->occurrences_current >= entry->occurrences_required)
    {
        entry->is_discovered = TRUE;

        if (entry->category < N_LEDGER_CATEGORIES)
            self->discovered_counts[entry->category]++;

        lp_log_info ("New discovery: %s (category: %d)", entry_id, entry->category);

        g_signal_emit (self, signals[SIGNAL_ENTRY_DISCOVERED], 0,
                       entry_id, entry->category);
    }
    else
    {
        /* Just progress, not full discovery */
        g_signal_emit (self, signals[SIGNAL_ENTRY_PROGRESSED], 0,
                       entry_id, entry->occurrences_current,
                       entry->occurrences_required);
    }

    return TRUE;
}

guint
lp_ledger_get_progress (LpLedger    *self,
                        const gchar *entry_id)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), 0);
    g_return_val_if_fail (entry_id != NULL, 0);

    entry = g_hash_table_lookup (self->entries, entry_id);
    if (entry == NULL)
        return 0;

    return entry->occurrences_current;
}

guint
lp_ledger_get_required_occurrences (LpLedger    *self,
                                    const gchar *entry_id)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), 0);
    g_return_val_if_fail (entry_id != NULL, 0);

    entry = g_hash_table_lookup (self->entries, entry_id);
    if (entry == NULL)
        return 0;

    return entry->occurrences_required;
}

gfloat
lp_ledger_get_progress_fraction (LpLedger    *self,
                                 const gchar *entry_id)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), 0.0f);
    g_return_val_if_fail (entry_id != NULL, 0.0f);

    entry = g_hash_table_lookup (self->entries, entry_id);
    if (entry == NULL || entry->occurrences_required == 0)
        return 0.0f;

    return (gfloat)entry->occurrences_current / (gfloat)entry->occurrences_required;
}

/* ==========================================================================
 * Discovery Queries
 * ========================================================================== */

gboolean
lp_ledger_has_discovered (LpLedger    *self,
                          const gchar *entry_id)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), FALSE);
    g_return_val_if_fail (entry_id != NULL, FALSE);

    entry = g_hash_table_lookup (self->entries, entry_id);
    if (entry == NULL)
        return FALSE;

    return entry->is_discovered;
}

gboolean
lp_ledger_has_started (LpLedger    *self,
                       const gchar *entry_id)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), FALSE);
    g_return_val_if_fail (entry_id != NULL, FALSE);

    entry = g_hash_table_lookup (self->entries, entry_id);
    if (entry == NULL)
        return FALSE;

    return entry->occurrences_current > 0;
}

gboolean
lp_ledger_discover (LpLedger         *self,
                    const gchar      *entry_id,
                    LpLedgerCategory  category)
{
    LedgerEntry *entry;

    g_return_val_if_fail (LP_IS_LEDGER (self), FALSE);
    g_return_val_if_fail (entry_id != NULL, FALSE);

    entry = g_hash_table_lookup (self->entries, entry_id);

    /* Create entry if not exists */
    if (entry == NULL)
    {
        entry = g_new0 (LedgerEntry, 1);
        entry->entry_id = g_strdup (entry_id);
        entry->category = category;
        entry->occurrences_required = 1;
        entry->occurrences_current = 0;
        entry->is_discovered = FALSE;

        g_hash_table_insert (self->entries,
                             g_strdup (entry_id),
                             entry);
    }

    /* Already discovered */
    if (entry->is_discovered)
        return FALSE;

    /* Mark as fully discovered */
    entry->is_discovered = TRUE;
    entry->occurrences_current = entry->occurrences_required;

    if (entry->category < N_LEDGER_CATEGORIES)
        self->discovered_counts[entry->category]++;

    lp_log_info ("New discovery (immediate): %s (category: %d)",
                 entry_id, entry->category);

    g_signal_emit (self, signals[SIGNAL_ENTRY_DISCOVERED], 0,
                   entry_id, entry->category);

    return TRUE;
}

guint
lp_ledger_get_discovered_count (LpLedger *self)
{
    guint total;
    guint i;

    g_return_val_if_fail (LP_IS_LEDGER (self), 0);

    total = 0;
    for (i = 0; i < N_LEDGER_CATEGORIES; i++)
        total += self->discovered_counts[i];

    return total;
}

guint
lp_ledger_get_in_progress_count (LpLedger *self)
{
    GHashTableIter iter;
    gpointer key, value;
    guint count;

    g_return_val_if_fail (LP_IS_LEDGER (self), 0);

    count = 0;
    g_hash_table_iter_init (&iter, self->entries);

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry;

        entry = (LedgerEntry *)value;
        if (!entry->is_discovered && entry->occurrences_current > 0)
            count++;
    }

    return count;
}

guint
lp_ledger_get_discovered_in_category (LpLedger         *self,
                                      LpLedgerCategory  category)
{
    g_return_val_if_fail (LP_IS_LEDGER (self), 0);
    g_return_val_if_fail (category < N_LEDGER_CATEGORIES, 0);

    return self->discovered_counts[category];
}

GList *
lp_ledger_get_all_discoveries (LpLedger *self)
{
    GHashTableIter iter;
    gpointer key, value;
    GList *result;

    g_return_val_if_fail (LP_IS_LEDGER (self), NULL);

    result = NULL;
    g_hash_table_iter_init (&iter, self->entries);

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry;

        entry = (LedgerEntry *)value;
        if (entry->is_discovered)
            result = g_list_prepend (result, key);
    }

    return result;
}

GList *
lp_ledger_get_all_in_progress (LpLedger *self)
{
    GHashTableIter iter;
    gpointer key, value;
    GList *result;

    g_return_val_if_fail (LP_IS_LEDGER (self), NULL);

    result = NULL;
    g_hash_table_iter_init (&iter, self->entries);

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry;

        entry = (LedgerEntry *)value;
        if (!entry->is_discovered && entry->occurrences_current > 0)
            result = g_list_prepend (result, key);
    }

    return result;
}

GList *
lp_ledger_get_discoveries_by_category (LpLedger         *self,
                                       LpLedgerCategory  category)
{
    GHashTableIter iter;
    gpointer key, value;
    GList *result;

    g_return_val_if_fail (LP_IS_LEDGER (self), NULL);

    result = NULL;
    g_hash_table_iter_init (&iter, self->entries);

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry;

        entry = (LedgerEntry *)value;
        if (entry->is_discovered && entry->category == category)
            result = g_list_prepend (result, key);
    }

    return result;
}

/* ==========================================================================
 * Prestige Retention
 * ========================================================================== */

void
lp_ledger_apply_retention (LpLedger *self,
                           gdouble   retention)
{
    GHashTableIter iter;
    gpointer key, value;
    GPtrArray *to_remove;
    guint i;

    g_return_if_fail (LP_IS_LEDGER (self));

    retention = CLAMP (retention, 0.0, 1.0);

    lp_log_debug ("Applying ledger retention: %.0f%%", retention * 100);

    /* If full retention, do nothing */
    if (retention >= 1.0)
        return;

    to_remove = g_ptr_array_new ();

    /* First pass: identify entries to remove */
    g_hash_table_iter_init (&iter, self->entries);
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        LedgerEntry *entry;
        gdouble roll;

        entry = (LedgerEntry *)value;

        /* In-progress entries are always lost (unless 100% retention) */
        if (!entry->is_discovered)
        {
            g_ptr_array_add (to_remove, g_strdup (key));
            continue;
        }

        /* Roll for discovered entries */
        roll = g_random_double ();
        if (roll >= retention)
        {
            /* Entry lost - update category count */
            if (entry->category < N_LEDGER_CATEGORIES)
                self->discovered_counts[entry->category]--;

            g_ptr_array_add (to_remove, g_strdup (key));
        }
    }

    /* Second pass: remove identified entries */
    for (i = 0; i < to_remove->len; i++)
    {
        gchar *entry_id;

        entry_id = g_ptr_array_index (to_remove, i);
        g_hash_table_remove (self->entries, entry_id);
        g_free (entry_id);
    }

    g_ptr_array_free (to_remove, TRUE);

    lp_log_info ("Ledger after retention: %u discoveries remain",
                 lp_ledger_get_discovered_count (self));
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

void
lp_ledger_clear_all (LpLedger *self)
{
    guint i;

    g_return_if_fail (LP_IS_LEDGER (self));

    lp_log_debug ("Clearing all ledger entries");

    g_hash_table_remove_all (self->entries);

    for (i = 0; i < N_LEDGER_CATEGORIES; i++)
        self->discovered_counts[i] = 0;
}
