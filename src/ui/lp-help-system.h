/* lp-help-system.h - Contextual Help Content Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Manages help content for tooltips and the help glossary.
 * Content is loaded from YAML files for easy editing.
 */

#ifndef LP_HELP_SYSTEM_H
#define LP_HELP_SYSTEM_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_HELP_SYSTEM (lp_help_system_get_type ())

G_DECLARE_FINAL_TYPE (LpHelpSystem, lp_help_system, LP, HELP_SYSTEM, GObject)

/**
 * LpHelpEntry:
 * @id: unique identifier
 * @title: display title
 * @description: main help text
 * @hint: optional short tip
 * @category: category for glossary grouping
 * @related: comma-separated related entry IDs
 *
 * A single help entry for a game concept.
 */
typedef struct _LpHelpEntry LpHelpEntry;

/**
 * lp_help_system_get_default:
 *
 * Gets the singleton help system instance.
 *
 * Returns: (transfer none): The default #LpHelpSystem
 */
LpHelpSystem * lp_help_system_get_default (void);

/* ==========================================================================
 * Loading
 * ========================================================================== */

/**
 * lp_help_system_load:
 * @self: an #LpHelpSystem
 * @error: (nullable): return location for error
 *
 * Loads all help content from data/help/ YAML files.
 *
 * Returns: %TRUE on success
 */
gboolean lp_help_system_load (LpHelpSystem  *self,
                              GError       **error);

/* ==========================================================================
 * Entry Lookup
 * ========================================================================== */

/**
 * lp_help_system_get_entry:
 * @self: an #LpHelpSystem
 * @id: the entry identifier
 *
 * Gets a help entry by ID.
 *
 * Returns: (transfer none) (nullable): The #LpHelpEntry or %NULL
 */
const LpHelpEntry * lp_help_system_get_entry (LpHelpSystem *self,
                                              const gchar  *id);

/**
 * lp_help_system_get_entries_by_category:
 * @self: an #LpHelpSystem
 * @category: the category name
 *
 * Gets all entries in a category.
 *
 * Returns: (transfer container) (element-type LpHelpEntry): List of entries
 */
GList * lp_help_system_get_entries_by_category (LpHelpSystem *self,
                                                const gchar  *category);

/**
 * lp_help_system_get_categories:
 * @self: an #LpHelpSystem
 *
 * Gets a list of all categories.
 *
 * Returns: (transfer container) (element-type utf8): List of category names
 */
GList * lp_help_system_get_categories (LpHelpSystem *self);

/* ==========================================================================
 * Entry Accessors
 * ========================================================================== */

/**
 * lp_help_entry_get_id:
 * @entry: an #LpHelpEntry
 *
 * Returns: (transfer none): The entry ID
 */
const gchar * lp_help_entry_get_id (const LpHelpEntry *entry);

/**
 * lp_help_entry_get_title:
 * @entry: an #LpHelpEntry
 *
 * Returns: (transfer none): The entry title
 */
const gchar * lp_help_entry_get_title (const LpHelpEntry *entry);

/**
 * lp_help_entry_get_description:
 * @entry: an #LpHelpEntry
 *
 * Returns: (transfer none): The entry description
 */
const gchar * lp_help_entry_get_description (const LpHelpEntry *entry);

/**
 * lp_help_entry_get_hint:
 * @entry: an #LpHelpEntry
 *
 * Returns: (transfer none) (nullable): The entry hint
 */
const gchar * lp_help_entry_get_hint (const LpHelpEntry *entry);

/**
 * lp_help_entry_get_category:
 * @entry: an #LpHelpEntry
 *
 * Returns: (transfer none): The entry category
 */
const gchar * lp_help_entry_get_category (const LpHelpEntry *entry);

/* ==========================================================================
 * Quick Access Helpers
 * ========================================================================== */

/**
 * lp_help_get:
 * @id: the entry identifier
 *
 * Convenience function to get an entry from the default help system.
 *
 * Returns: (transfer none) (nullable): The #LpHelpEntry or %NULL
 */
const LpHelpEntry * lp_help_get (const gchar *id);

/**
 * lp_help_title:
 * @id: the entry identifier
 *
 * Convenience function to get an entry's title.
 *
 * Returns: (transfer none) (nullable): The title or %NULL
 */
const gchar * lp_help_title (const gchar *id);

/**
 * lp_help_desc:
 * @id: the entry identifier
 *
 * Convenience function to get an entry's description.
 *
 * Returns: (transfer none) (nullable): The description or %NULL
 */
const gchar * lp_help_desc (const gchar *id);

G_END_DECLS

#endif /* LP_HELP_SYSTEM_H */
