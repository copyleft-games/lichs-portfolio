/* lp-ledger.h - Discovery Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The Ledger tracks player discoveries and unlocks.
 * Discoveries persist across prestige and provide permanent bonuses
 * or unlock new content.
 *
 * Some discoveries require multiple occurrences to fully unlock
 * (e.g., experiencing the same event type twice).
 *
 * Discovery methods:
 * - Agent Reports: Random per cycle per agent
 * - Event Survival: Triggered by specific events
 * - Competitor Interaction: Through alliance/conflict
 * - Achievement Completion: Specific achievements
 * - Investment Milestones: Portfolio thresholds
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
 * Discovery Methods - How entries are discovered
 * ========================================================================== */

/**
 * LpDiscoveryMethod:
 * @LP_DISCOVERY_METHOD_MANUAL: Manually triggered (debug/testing)
 * @LP_DISCOVERY_METHOD_AGENT_REPORT: Random discovery from agent activity
 * @LP_DISCOVERY_METHOD_EVENT_SURVIVAL: Surviving specific world events
 * @LP_DISCOVERY_METHOD_COMPETITOR: Interaction with immortal competitors
 * @LP_DISCOVERY_METHOD_ACHIEVEMENT: Completing specific achievements
 * @LP_DISCOVERY_METHOD_MILESTONE: Reaching portfolio/investment milestones
 *
 * Methods by which ledger entries can be discovered.
 */
typedef enum
{
    LP_DISCOVERY_METHOD_MANUAL,
    LP_DISCOVERY_METHOD_AGENT_REPORT,
    LP_DISCOVERY_METHOD_EVENT_SURVIVAL,
    LP_DISCOVERY_METHOD_COMPETITOR,
    LP_DISCOVERY_METHOD_ACHIEVEMENT,
    LP_DISCOVERY_METHOD_MILESTONE
} LpDiscoveryMethod;

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
 * Entry Registration (for entries that need progress tracking)
 * ========================================================================== */

/**
 * lp_ledger_register_entry:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 * @category: the category of the discovery
 * @occurrences_required: number of occurrences needed for full discovery
 *
 * Registers an entry that requires multiple occurrences to discover.
 * If occurrences_required is 1, this is equivalent to a simple discovery.
 *
 * Call this before using lp_ledger_progress_entry().
 */
void
lp_ledger_register_entry (LpLedger         *self,
                          const gchar      *entry_id,
                          LpLedgerCategory  category,
                          guint             occurrences_required);

/**
 * lp_ledger_is_registered:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 *
 * Checks if an entry is registered.
 *
 * Returns: %TRUE if the entry is registered
 */
gboolean
lp_ledger_is_registered (LpLedger    *self,
                         const gchar *entry_id);

/* ==========================================================================
 * Progress Tracking
 * ========================================================================== */

/**
 * lp_ledger_progress_entry:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 * @method: how this progress was triggered
 *
 * Advances progress on a discovery entry by one occurrence.
 * If this brings the entry to full discovery, emits ::entry-discovered.
 * If this is progress but not full discovery, emits ::entry-progressed.
 *
 * For unregistered entries, this auto-registers with occurrences_required=1.
 *
 * Returns: %TRUE if progress was made (not already fully discovered)
 */
gboolean
lp_ledger_progress_entry (LpLedger          *self,
                          const gchar       *entry_id,
                          LpDiscoveryMethod  method);

/**
 * lp_ledger_get_progress:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 *
 * Gets the current progress on an entry.
 *
 * Returns: Number of occurrences recorded, or 0 if not started
 */
guint
lp_ledger_get_progress (LpLedger    *self,
                        const gchar *entry_id);

/**
 * lp_ledger_get_required_occurrences:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 *
 * Gets the number of occurrences required for full discovery.
 *
 * Returns: Required occurrences, or 0 if not registered
 */
guint
lp_ledger_get_required_occurrences (LpLedger    *self,
                                    const gchar *entry_id);

/**
 * lp_ledger_get_progress_fraction:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 *
 * Gets the discovery progress as a fraction.
 *
 * Returns: Progress from 0.0 to 1.0, or 0.0 if not started
 */
gfloat
lp_ledger_get_progress_fraction (LpLedger    *self,
                                 const gchar *entry_id);

/* ==========================================================================
 * Discovery Queries
 * ========================================================================== */

/**
 * lp_ledger_has_discovered:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 *
 * Checks if an entry has been fully discovered.
 *
 * Returns: %TRUE if the entry is fully discovered
 */
gboolean
lp_ledger_has_discovered (LpLedger    *self,
                          const gchar *entry_id);

/**
 * lp_ledger_has_started:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 *
 * Checks if any progress has been made on an entry.
 *
 * Returns: %TRUE if progress > 0
 */
gboolean
lp_ledger_has_started (LpLedger    *self,
                       const gchar *entry_id);

/**
 * lp_ledger_discover:
 * @self: an #LpLedger
 * @entry_id: the discovery entry ID
 * @category: the category of the discovery
 *
 * Immediately fully discovers an entry (bypasses progress).
 * Used for simple discoveries or cheat/debug.
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
 * Gets the total number of fully discovered entries.
 *
 * Returns: Number of full discoveries
 */
guint
lp_ledger_get_discovered_count (LpLedger *self);

/**
 * lp_ledger_get_in_progress_count:
 * @self: an #LpLedger
 *
 * Gets the number of entries with progress but not fully discovered.
 *
 * Returns: Number of in-progress entries
 */
guint
lp_ledger_get_in_progress_count (LpLedger *self);

/**
 * lp_ledger_get_discovered_in_category:
 * @self: an #LpLedger
 * @category: the category to count
 *
 * Gets the number of fully discovered entries in a specific category.
 *
 * Returns: Number of full discoveries in the category
 */
guint
lp_ledger_get_discovered_in_category (LpLedger         *self,
                                      LpLedgerCategory  category);

/**
 * lp_ledger_get_all_discoveries:
 * @self: an #LpLedger
 *
 * Gets a list of all fully discovered entry IDs.
 *
 * Returns: (transfer container) (element-type utf8): List of entry IDs
 */
GList *
lp_ledger_get_all_discoveries (LpLedger *self);

/**
 * lp_ledger_get_all_in_progress:
 * @self: an #LpLedger
 *
 * Gets a list of entry IDs that are started but not fully discovered.
 *
 * Returns: (transfer container) (element-type utf8): List of entry IDs
 */
GList *
lp_ledger_get_all_in_progress (LpLedger *self);

/**
 * lp_ledger_get_discoveries_by_category:
 * @self: an #LpLedger
 * @category: the category to filter by
 *
 * Gets a list of fully discovered entry IDs in a specific category.
 *
 * Returns: (transfer container) (element-type utf8): List of entry IDs
 */
GList *
lp_ledger_get_discoveries_by_category (LpLedger         *self,
                                       LpLedgerCategory  category);

/* ==========================================================================
 * Prestige Retention
 * ========================================================================== */

/**
 * lp_ledger_apply_retention:
 * @self: an #LpLedger
 * @retention: fraction to retain (0.0 to 1.0)
 *
 * Applies prestige retention - keeps only a fraction of entries.
 * This is called during prestige if the player has Scholar tree bonuses.
 *
 * Fully discovered entries are retained based on retention fraction.
 * In-progress entries are fully lost unless retention >= 1.0.
 */
void
lp_ledger_apply_retention (LpLedger *self,
                           gdouble   retention);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_ledger_clear_all:
 * @self: an #LpLedger
 *
 * Clears all discoveries and progress. Used when starting a completely
 * new game, NOT during prestige (use apply_retention for prestige).
 */
void
lp_ledger_clear_all (LpLedger *self);

G_END_DECLS

#endif /* LP_LEDGER_H */
