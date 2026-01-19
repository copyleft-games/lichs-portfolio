/* lp-event-chronicle.h - Historical Record of World Events
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The event chronicle maintains a permanent record of all significant
 * events that have occurred throughout the lich's existence. Unlike
 * active events which are transient, chronicled events are preserved
 * forever and can be reviewed at any time.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_EVENT_CHRONICLE_H
#define LP_EVENT_CHRONICLE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

/* ==========================================================================
 * LpChronicleEntry - Boxed type for historical event records
 * ========================================================================== */

#define LP_TYPE_CHRONICLE_ENTRY (lp_chronicle_entry_get_type ())

/**
 * LpChronicleEntry:
 * @event_id: the original event ID
 * @event_name: display name of the event
 * @event_type: the #LpEventType
 * @severity: the #LpEventSeverity
 * @year_occurred: year the event started
 * @year_resolved: year the event ended (or same as occurred if instant)
 * @description: event description
 * @outcome: what happened as a result
 * @affected_region: region that was affected (or NULL)
 * @affected_kingdom: kingdom that was affected (or NULL)
 * @player_choice: ID of choice made, if any
 * @gold_impact: net gold change from this event
 * @exposure_impact: net exposure change from this event
 *
 * A permanent record of an event that occurred in the world.
 */
struct _LpChronicleEntry
{
    gchar            *event_id;
    gchar            *event_name;
    LpEventType       event_type;
    LpEventSeverity   severity;
    guint64           year_occurred;
    guint64           year_resolved;
    gchar            *description;
    gchar            *outcome;
    gchar            *affected_region;
    gchar            *affected_kingdom;
    gchar            *player_choice;
    gint64            gold_impact;
    gfloat            exposure_impact;
};

GType
lp_chronicle_entry_get_type (void) G_GNUC_CONST;

/**
 * lp_chronicle_entry_new:
 * @event: the #LpEvent to chronicle
 * @year_resolved: the year the event ended
 *
 * Creates a chronicle entry from an event.
 *
 * Returns: (transfer full): A new #LpChronicleEntry
 */
LpChronicleEntry *
lp_chronicle_entry_new (LpEvent *event,
                        guint64  year_resolved);

/**
 * lp_chronicle_entry_copy:
 * @entry: an #LpChronicleEntry
 *
 * Creates a copy of a chronicle entry.
 *
 * Returns: (transfer full): A copy of @entry
 */
LpChronicleEntry *
lp_chronicle_entry_copy (const LpChronicleEntry *entry);

/**
 * lp_chronicle_entry_free:
 * @entry: an #LpChronicleEntry
 *
 * Frees a chronicle entry.
 */
void
lp_chronicle_entry_free (LpChronicleEntry *entry);

/* ==========================================================================
 * LpEventChronicle - Singleton manager
 * ========================================================================== */

#define LP_TYPE_EVENT_CHRONICLE (lp_event_chronicle_get_type ())

G_DECLARE_FINAL_TYPE (LpEventChronicle, lp_event_chronicle, LP, EVENT_CHRONICLE, GObject)

/**
 * lp_event_chronicle_get_default:
 *
 * Gets the default event chronicle instance.
 *
 * Returns: (transfer none): The default #LpEventChronicle
 */
LpEventChronicle *
lp_event_chronicle_get_default (void);

/* ==========================================================================
 * Recording Events
 * ========================================================================== */

/**
 * lp_event_chronicle_record:
 * @self: an #LpEventChronicle
 * @event: the #LpEvent that occurred
 * @year_resolved: the year the event ended
 * @outcome: (nullable): text describing the outcome
 * @gold_impact: net gold change from this event
 * @exposure_impact: net exposure change
 *
 * Records an event in the chronicle.
 */
void
lp_event_chronicle_record (LpEventChronicle *self,
                           LpEvent          *event,
                           guint64           year_resolved,
                           const gchar      *outcome,
                           gint64            gold_impact,
                           gfloat            exposure_impact);

/**
 * lp_event_chronicle_record_with_choice:
 * @self: an #LpEventChronicle
 * @event: the #LpEvent that occurred
 * @year_resolved: the year the event ended
 * @choice_id: the choice the player made
 * @outcome: text describing the outcome
 * @gold_impact: net gold change
 * @exposure_impact: net exposure change
 *
 * Records an event with a player choice.
 */
void
lp_event_chronicle_record_with_choice (LpEventChronicle *self,
                                       LpEvent          *event,
                                       guint64           year_resolved,
                                       const gchar      *choice_id,
                                       const gchar      *outcome,
                                       gint64            gold_impact,
                                       gfloat            exposure_impact);

/* ==========================================================================
 * Querying the Chronicle
 * ========================================================================== */

/**
 * lp_event_chronicle_get_all:
 * @self: an #LpEventChronicle
 *
 * Gets all chronicle entries, most recent first.
 *
 * Returns: (transfer none) (element-type LpChronicleEntry): All entries
 */
GPtrArray *
lp_event_chronicle_get_all (LpEventChronicle *self);

/**
 * lp_event_chronicle_get_by_type:
 * @self: an #LpEventChronicle
 * @event_type: the #LpEventType to filter by
 *
 * Gets entries of a specific type.
 *
 * Returns: (transfer full) (element-type LpChronicleEntry): Filtered entries
 */
GPtrArray *
lp_event_chronicle_get_by_type (LpEventChronicle *self,
                                LpEventType       event_type);

/**
 * lp_event_chronicle_get_by_year_range:
 * @self: an #LpEventChronicle
 * @start_year: earliest year to include
 * @end_year: latest year to include
 *
 * Gets entries within a year range.
 *
 * Returns: (transfer full) (element-type LpChronicleEntry): Filtered entries
 */
GPtrArray *
lp_event_chronicle_get_by_year_range (LpEventChronicle *self,
                                      guint64           start_year,
                                      guint64           end_year);

/**
 * lp_event_chronicle_get_by_kingdom:
 * @self: an #LpEventChronicle
 * @kingdom_id: the kingdom ID to filter by
 *
 * Gets entries affecting a specific kingdom.
 *
 * Returns: (transfer full) (element-type LpChronicleEntry): Filtered entries
 */
GPtrArray *
lp_event_chronicle_get_by_kingdom (LpEventChronicle *self,
                                   const gchar      *kingdom_id);

/**
 * lp_event_chronicle_get_by_severity:
 * @self: an #LpEventChronicle
 * @min_severity: minimum severity level
 *
 * Gets entries at or above a severity threshold.
 *
 * Returns: (transfer full) (element-type LpChronicleEntry): Filtered entries
 */
GPtrArray *
lp_event_chronicle_get_by_severity (LpEventChronicle *self,
                                    LpEventSeverity   min_severity);

/**
 * lp_event_chronicle_get_recent:
 * @self: an #LpEventChronicle
 * @count: maximum number to return
 *
 * Gets the most recent entries.
 *
 * Returns: (transfer full) (element-type LpChronicleEntry): Recent entries
 */
GPtrArray *
lp_event_chronicle_get_recent (LpEventChronicle *self,
                               guint             count);

/**
 * lp_event_chronicle_get_count:
 * @self: an #LpEventChronicle
 *
 * Gets the total number of recorded events.
 *
 * Returns: Number of events
 */
guint
lp_event_chronicle_get_count (LpEventChronicle *self);

/**
 * lp_event_chronicle_get_count_by_type:
 * @self: an #LpEventChronicle
 * @event_type: the #LpEventType
 *
 * Gets the count of events of a specific type.
 *
 * Returns: Number of events of that type
 */
guint
lp_event_chronicle_get_count_by_type (LpEventChronicle *self,
                                      LpEventType       event_type);

/* ==========================================================================
 * Era/Milestone Markers
 * ========================================================================== */

/**
 * lp_event_chronicle_add_milestone:
 * @self: an #LpEventChronicle
 * @year: the year of the milestone
 * @title: milestone title
 * @description: milestone description
 *
 * Adds a milestone marker (like "First Prestige" or "Century Passed").
 */
void
lp_event_chronicle_add_milestone (LpEventChronicle *self,
                                  guint64           year,
                                  const gchar      *title,
                                  const gchar      *description);

/**
 * lp_event_chronicle_reset:
 * @self: an #LpEventChronicle
 *
 * Clears all chronicle entries (for new game).
 * Call this when starting a completely new run.
 */
void
lp_event_chronicle_reset (LpEventChronicle *self);

G_END_DECLS

#endif /* LP_EVENT_CHRONICLE_H */
