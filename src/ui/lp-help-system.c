/* lp-help-system.c - Contextual Help Content Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-help-system.h"
#include <yaml-glib.h>

struct _LpHelpEntry
{
    gchar *id;
    gchar *title;
    gchar *description;
    gchar *hint;
    gchar *category;
    gchar *related;
};

struct _LpHelpSystem
{
    GObject parent_instance;

    /* Entry storage: id -> LpHelpEntry* */
    GHashTable *entries;

    /* Category index: category -> GList of LpHelpEntry* */
    GHashTable *categories;

    gboolean loaded;
};

G_DEFINE_TYPE (LpHelpSystem, lp_help_system, G_TYPE_OBJECT)

/* Singleton instance */
static LpHelpSystem *default_instance = NULL;

/* ==========================================================================
 * LpHelpEntry Management
 * ========================================================================== */

static LpHelpEntry *
lp_help_entry_new (void)
{
    return g_slice_new0 (LpHelpEntry);
}

static void
lp_help_entry_free (LpHelpEntry *entry)
{
    if (entry == NULL)
        return;

    g_free (entry->id);
    g_free (entry->title);
    g_free (entry->description);
    g_free (entry->hint);
    g_free (entry->category);
    g_free (entry->related);
    g_slice_free (LpHelpEntry, entry);
}

/* ==========================================================================
 * YAML Loading
 * ========================================================================== */

static LpHelpEntry *
parse_help_entry (YamlMapping *mapping)
{
    LpHelpEntry *entry;
    YamlNode *node;

    if (mapping == NULL)
        return NULL;

    entry = lp_help_entry_new ();

    node = yaml_mapping_get_member (mapping, "id");
    if (node != NULL && yaml_node_get_node_type (node) == YAML_NODE_SCALAR)
        entry->id = g_strdup (yaml_node_get_string (node));

    node = yaml_mapping_get_member (mapping, "title");
    if (node != NULL && yaml_node_get_node_type (node) == YAML_NODE_SCALAR)
        entry->title = g_strdup (yaml_node_get_string (node));

    node = yaml_mapping_get_member (mapping, "description");
    if (node != NULL && yaml_node_get_node_type (node) == YAML_NODE_SCALAR)
        entry->description = g_strdup (yaml_node_get_string (node));

    node = yaml_mapping_get_member (mapping, "hint");
    if (node != NULL && yaml_node_get_node_type (node) == YAML_NODE_SCALAR)
        entry->hint = g_strdup (yaml_node_get_string (node));

    node = yaml_mapping_get_member (mapping, "category");
    if (node != NULL && yaml_node_get_node_type (node) == YAML_NODE_SCALAR)
        entry->category = g_strdup (yaml_node_get_string (node));
    else
        entry->category = g_strdup ("general");

    node = yaml_mapping_get_member (mapping, "related");
    if (node != NULL && yaml_node_get_node_type (node) == YAML_NODE_SCALAR)
        entry->related = g_strdup (yaml_node_get_string (node));

    /* Validate required fields */
    if (entry->id == NULL || entry->title == NULL || entry->description == NULL)
    {
        lp_log_warning ("Help entry missing required fields (id, title, description)");
        lp_help_entry_free (entry);
        return NULL;
    }

    return entry;
}

static gboolean
load_help_file (LpHelpSystem *self,
                const gchar  *path,
                GError      **error)
{
    g_autoptr(YamlParser) parser = NULL;
    YamlNode *root;
    YamlMapping *root_map;
    YamlNode *entries_node;
    YamlSequence *entries_seq;
    guint i, len;

    parser = yaml_parser_new ();
    if (!yaml_parser_load_from_file (parser, path, error))
        return FALSE;

    root = yaml_parser_get_root (parser);
    if (root == NULL)
        return TRUE;  /* Empty file is OK */

    if (yaml_node_get_node_type (root) != YAML_NODE_MAPPING)
        return TRUE;  /* Not a mapping, skip */

    root_map = yaml_node_get_mapping (root);

    /* Look for "entries" list */
    entries_node = yaml_mapping_get_member (root_map, "entries");
    if (entries_node == NULL || yaml_node_get_node_type (entries_node) != YAML_NODE_SEQUENCE)
    {
        /* File doesn't have entries list, skip */
        return TRUE;
    }

    entries_seq = yaml_node_get_sequence (entries_node);
    len = yaml_sequence_get_length (entries_seq);

    for (i = 0; i < len; i++)
    {
        LpHelpEntry *entry;
        YamlNode *entry_node;
        YamlMapping *entry_map;
        GList *cat_list;

        entry_node = yaml_sequence_get_element (entries_seq, i);
        if (entry_node == NULL || yaml_node_get_node_type (entry_node) != YAML_NODE_MAPPING)
            continue;

        entry_map = yaml_node_get_mapping (entry_node);
        entry = parse_help_entry (entry_map);
        if (entry == NULL)
            continue;

        /* Add to main hash table */
        g_hash_table_insert (self->entries, g_strdup (entry->id), entry);

        /* Add to category index */
        cat_list = g_hash_table_lookup (self->categories, entry->category);
        cat_list = g_list_append (cat_list, entry);
        g_hash_table_insert (self->categories, g_strdup (entry->category), cat_list);

        lp_log_debug ("Loaded help entry: %s (%s)", entry->id, entry->category);
    }

    return TRUE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_help_system_dispose (GObject *object)
{
    LpHelpSystem *self = LP_HELP_SYSTEM (object);

    g_clear_pointer (&self->entries, g_hash_table_unref);

    /* Category lists are just pointers to entries, don't free the entries */
    if (self->categories != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init (&iter, self->categories);
        while (g_hash_table_iter_next (&iter, &key, &value))
        {
            g_list_free ((GList *)value);
        }
        g_clear_pointer (&self->categories, g_hash_table_unref);
    }

    G_OBJECT_CLASS (lp_help_system_parent_class)->dispose (object);
}

static void
lp_help_system_class_init (LpHelpSystemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = lp_help_system_dispose;
}

static void
lp_help_system_init (LpHelpSystem *self)
{
    self->entries = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           g_free, (GDestroyNotify)lp_help_entry_free);
    self->categories = g_hash_table_new_full (g_str_hash, g_str_equal,
                                              g_free, NULL);
    self->loaded = FALSE;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

LpHelpSystem *
lp_help_system_get_default (void)
{
    if (default_instance == NULL)
    {
        default_instance = g_object_new (LP_TYPE_HELP_SYSTEM, NULL);
        lp_log_info ("Created default LpHelpSystem instance");
    }

    return default_instance;
}

gboolean
lp_help_system_load (LpHelpSystem  *self,
                     GError       **error)
{
    g_autoptr(GDir) dir = NULL;
    const gchar *filename;
    g_autofree gchar *help_dir = NULL;

    g_return_val_if_fail (LP_IS_HELP_SYSTEM (self), FALSE);

    if (self->loaded)
        return TRUE;

    help_dir = g_build_filename ("data", "help", NULL);

    /* Check if help directory exists */
    if (!g_file_test (help_dir, G_FILE_TEST_IS_DIR))
    {
        lp_log_info ("Help directory not found: %s (skipping)", help_dir);
        self->loaded = TRUE;
        return TRUE;
    }

    dir = g_dir_open (help_dir, 0, error);
    if (dir == NULL)
        return FALSE;

    while ((filename = g_dir_read_name (dir)) != NULL)
    {
        g_autofree gchar *path = NULL;

        if (!g_str_has_suffix (filename, ".yaml") &&
            !g_str_has_suffix (filename, ".yml"))
            continue;

        path = g_build_filename (help_dir, filename, NULL);

        if (!load_help_file (self, path, error))
        {
            lp_log_warning ("Failed to load help file: %s", path);
            /* Continue loading other files */
        }
    }

    self->loaded = TRUE;
    lp_log_info ("Loaded %u help entries across %u categories",
                 g_hash_table_size (self->entries),
                 g_hash_table_size (self->categories));

    return TRUE;
}

const LpHelpEntry *
lp_help_system_get_entry (LpHelpSystem *self,
                          const gchar  *id)
{
    g_return_val_if_fail (LP_IS_HELP_SYSTEM (self), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    return g_hash_table_lookup (self->entries, id);
}

GList *
lp_help_system_get_entries_by_category (LpHelpSystem *self,
                                        const gchar  *category)
{
    GList *entries;

    g_return_val_if_fail (LP_IS_HELP_SYSTEM (self), NULL);
    g_return_val_if_fail (category != NULL, NULL);

    entries = g_hash_table_lookup (self->categories, category);
    return g_list_copy (entries);
}

GList *
lp_help_system_get_categories (LpHelpSystem *self)
{
    g_return_val_if_fail (LP_IS_HELP_SYSTEM (self), NULL);
    return g_hash_table_get_keys (self->categories);
}

/* Entry accessors */
const gchar *
lp_help_entry_get_id (const LpHelpEntry *entry)
{
    g_return_val_if_fail (entry != NULL, NULL);
    return entry->id;
}

const gchar *
lp_help_entry_get_title (const LpHelpEntry *entry)
{
    g_return_val_if_fail (entry != NULL, NULL);
    return entry->title;
}

const gchar *
lp_help_entry_get_description (const LpHelpEntry *entry)
{
    g_return_val_if_fail (entry != NULL, NULL);
    return entry->description;
}

const gchar *
lp_help_entry_get_hint (const LpHelpEntry *entry)
{
    g_return_val_if_fail (entry != NULL, NULL);
    return entry->hint;
}

const gchar *
lp_help_entry_get_category (const LpHelpEntry *entry)
{
    g_return_val_if_fail (entry != NULL, NULL);
    return entry->category;
}

/* Convenience functions */
const LpHelpEntry *
lp_help_get (const gchar *id)
{
    return lp_help_system_get_entry (lp_help_system_get_default (), id);
}

const gchar *
lp_help_title (const gchar *id)
{
    const LpHelpEntry *entry = lp_help_get (id);
    return entry ? entry->title : NULL;
}

const gchar *
lp_help_desc (const gchar *id)
{
    const LpHelpEntry *entry = lp_help_get (id);
    return entry ? entry->description : NULL;
}
