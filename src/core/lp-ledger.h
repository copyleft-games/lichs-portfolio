/* lp-ledger.h - Discovery Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The Ledger tracks player discoveries and unlocks.
 * Discoveries persist across prestige and provide permanent bonuses
 * or unlock new content.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_LEDGER_H
#define LP_LEDGER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_LEDGER (lp_ledger_get_type ())

G_DECLARE_FINAL_TYPE (LpLedger, lp_ledger, LP, LEDGER, GObject)

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
lp_ledger_new (void);

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
                          const gchar *entry_id);

/**
 * lp_ledger_discover:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 * @category: the category of the discovery
 *
 * Records a new discovery. Does nothing if already discovered.
 * Emits ::entry-discovered signal if this is a new discovery.
 *
 * Returns: %TRUE if this was a new discovery
 */
gboolean
lp_ledger_discover (LpLedger         *self,
                    const gchar      *entry_id,
                    LpLedgerCategory  category);

/**
 * lp_ledger_get_discovered_count:
 * @self: an #LpLedger
 *
 * Gets the total number of discovered entries.
 *
 * Returns: Number of discoveries
 */
guint
lp_ledger_get_discovered_count (LpLedger *self);

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
                                      LpLedgerCategory  category);

/**
 * lp_ledger_get_all_discoveries:
 * @self: an #LpLedger
 *
 * Gets a list of all discovered entry IDs.
 *
 * Returns: (transfer container) (element-type utf8): List of entry IDs
 */
GList *
lp_ledger_get_all_discoveries (LpLedger *self);

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
                                       LpLedgerCategory  category);

/* ==========================================================================
 * Reset (for new game - discoveries persist across prestige)
 * ========================================================================== */

/**
 * lp_ledger_clear_all:
 * @self: an #LpLedger
 *
 * Clears all discoveries. Used when starting a completely new game,
 * NOT during prestige (discoveries persist across prestige).
 */
void
lp_ledger_clear_all (LpLedger *self);

G_END_DECLS

#endif /* LP_LEDGER_H */
