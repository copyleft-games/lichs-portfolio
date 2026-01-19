/* lp-event-chronicle.c - Historical Record of World Events
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-event-chronicle.h"
#include "../simulation/lp-event.h"

/* ==========================================================================
 * LpChronicleEntry - Boxed Type Implementation
 * ========================================================================== */

LpChronicleEntry *
lp_chronicle_entry_new (LpEvent *event,
                        guint64  year_resolved)
{
    LpChronicleEntry *entry;

    g_return_val_if_fail (LP_IS_EVENT (event), NULL);

    entry = g_slice_new0 (LpChronicleEntry);
    entry->event_id = g_strdup (lp_event_get_id (event));
    entry->event_name = g_strdup (lp_event_get_name (event));
    entry->event_type = lp_event_get_event_type (event);
    entry->severity = lp_event_get_severity (event);
    entry->year_occurred = lp_event_get_year_occurred (event);
    entry->year_resolved = year_resolved;
    entry->description = g_strdup (lp_event_get_description (event));
    entry->outcome = NULL;
    entry->affected_region = g_strdup (lp_event_get_affects_region_id (event));
    entry->affected_kingdom = g_strdup (lp_event_get_affects_kingdom_id (event));
    entry->player_choice = NULL;
    entry->gold_impact = 0;
    entry->exposure_impact = 0.0f;

    return entry;
}

LpChronicleEntry *
lp_chronicle_entry_copy (const LpChronicleEntry *entry)
{
    LpChronicleEntry *copy;

    if (entry == NULL)
        return NULL;

    copy = g_slice_new0 (LpChronicleEntry);
    copy->event_id = g_strdup (entry->event_id);
    copy->event_name = g_strdup (entry->event_name);
    copy->event_type = entry->event_type;
    copy->severity = entry->severity;
    copy->year_occurred = entry->year_occurred;
    copy->year_resolved = entry->year_resolved;
    copy->description = g_strdup (entry->description);
    copy->outcome = g_strdup (entry->outcome);
    copy->affected_region = g_strdup (entry->affected_region);
    copy->affected_kingdom = g_strdup (entry->affected_kingdom);
    copy->player_choice = g_strdup (entry->player_choice);
    copy->gold_impact = entry->gold_impact;
    copy->exposure_impact = entry->exposure_impact;

    return copy;
}

void
lp_chronicle_entry_free (LpChronicleEntry *entry)
{
    if (entry == NULL)
        return;

    g_free (entry->event_id);
    g_free (entry->event_name);
    g_free (entry->description);
    g_free (entry->outcome);
    g_free (entry->affected_region);
    g_free (entry->affected_kingdom);
    g_free (entry->player_choice);
    g_slice_free (LpChronicleEntry, entry);
}

G_DEFINE_BOXED_TYPE (LpChronicleEntry, lp_chronicle_entry,
                     lp_chronicle_entry_copy,
                     lp_chronicle_entry_free)

/* ==========================================================================
 * LpEventChronicle - Private Data
 * ========================================================================== */

struct _LpEventChronicle
{
    GObject parent_instance;

    /* All entries, most recent first */
    GPtrArray *entries;

    /* Milestone entries (special events like era changes, prestiges) */
    GPtrArray *milestones;

    /* Statistics */
    guint count_by_type[4];  /* Indexed by LpEventType */
};

static void lp_event_chronicle_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpEventChronicle, lp_event_chronicle, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_event_chronicle_saveable_init))

/* Singleton instance */
static LpEventChronicle *default_instance = NULL;

/* ==========================================================================
 * LrgSaveable Implementation
 * ========================================================================== */

static const gchar *
lp_event_chronicle_get_save_id (LrgSaveable *saveable)
{
    return "event-chronicle";
}

static gboolean
lp_event_chronicle_save (LrgSaveable    *saveable,
                         LrgSaveContext *ctx,
                         GError        **error)
{
    LpEventChronicle *self = LP_EVENT_CHRONICLE (saveable);
    guint i;

    lrg_save_context_write_uint (ctx, "entry-count", self->entries->len);

    lrg_save_context_begin_section (ctx, "entries");
    for (i = 0; i < self->entries->len; i++)
    {
        LpChronicleEntry *entry = g_ptr_array_index (self->entries, i);
        g_autofree gchar *key = g_strdup_printf ("entry-%u", i);

        lrg_save_context_begin_section (ctx, key);

        lrg_save_context_write_string (ctx, "event-id", entry->event_id);
        lrg_save_context_write_string (ctx, "event-name", entry->event_name);
        lrg_save_context_write_int (ctx, "event-type", entry->event_type);
        lrg_save_context_write_int (ctx, "severity", entry->severity);
        lrg_save_context_write_uint (ctx, "year-occurred", entry->year_occurred);
        lrg_save_context_write_uint (ctx, "year-resolved", entry->year_resolved);
        lrg_save_context_write_string (ctx, "description", entry->description);
        if (entry->outcome)
            lrg_save_context_write_string (ctx, "outcome", entry->outcome);
        if (entry->affected_region)
            lrg_save_context_write_string (ctx, "affected-region", entry->affected_region);
        if (entry->affected_kingdom)
            lrg_save_context_write_string (ctx, "affected-kingdom", entry->affected_kingdom);
        if (entry->player_choice)
            lrg_save_context_write_string (ctx, "player-choice", entry->player_choice);
        lrg_save_context_write_int (ctx, "gold-impact", entry->gold_impact);
        lrg_save_context_write_double (ctx, "exposure-impact", (gdouble)entry->exposure_impact);

        lrg_save_context_end_section (ctx);
    }
    lrg_save_context_end_section (ctx);

    /* Save milestones */
    lrg_save_context_write_uint (ctx, "milestone-count", self->milestones->len);

    lrg_save_context_begin_section (ctx, "milestones");
    for (i = 0; i < self->milestones->len; i++)
    {
        LpChronicleEntry *entry = g_ptr_array_index (self->milestones, i);
        g_autofree gchar *key = g_strdup_printf ("milestone-%u", i);

        lrg_save_context_begin_section (ctx, key);
        lrg_save_context_write_uint (ctx, "year", entry->year_occurred);
        lrg_save_context_write_string (ctx, "title", entry->event_name);
        lrg_save_context_write_string (ctx, "description", entry->description);
        lrg_save_context_end_section (ctx);
    }
    lrg_save_context_end_section (ctx);

    return TRUE;
}

static gboolean
lp_event_chronicle_load (LrgSaveable    *saveable,
                         LrgSaveContext *ctx,
                         GError        **error)
{
    LpEventChronicle *self = LP_EVENT_CHRONICLE (saveable);
    guint entry_count, milestone_count;
    guint i;

    /* Clear existing data */
    g_ptr_array_set_size (self->entries, 0);
    g_ptr_array_set_size (self->milestones, 0);
    memset (self->count_by_type, 0, sizeof (self->count_by_type));

    entry_count = lrg_save_context_read_uint (ctx, "entry-count", 0);

    if (lrg_save_context_enter_section (ctx, "entries"))
    {
        for (i = 0; i < entry_count; i++)
        {
            LpChronicleEntry *entry;
            g_autofree gchar *key = g_strdup_printf ("entry-%u", i);

            if (!lrg_save_context_enter_section (ctx, key))
                continue;

            entry = g_slice_new0 (LpChronicleEntry);
            entry->event_id = g_strdup (lrg_save_context_read_string (ctx, "event-id", "unknown"));
            entry->event_name = g_strdup (lrg_save_context_read_string (ctx, "event-name", "Unknown Event"));
            entry->event_type = lrg_save_context_read_int (ctx, "event-type", LP_EVENT_TYPE_ECONOMIC);
            entry->severity = lrg_save_context_read_int (ctx, "severity", LP_EVENT_SEVERITY_MINOR);
            entry->year_occurred = lrg_save_context_read_uint (ctx, "year-occurred", 847);
            entry->year_resolved = lrg_save_context_read_uint (ctx, "year-resolved", 847);
            entry->description = g_strdup (lrg_save_context_read_string (ctx, "description", ""));
            entry->outcome = g_strdup (lrg_save_context_read_string (ctx, "outcome", NULL));
            entry->affected_region = g_strdup (lrg_save_context_read_string (ctx, "affected-region", NULL));
            entry->affected_kingdom = g_strdup (lrg_save_context_read_string (ctx, "affected-kingdom", NULL));
            entry->player_choice = g_strdup (lrg_save_context_read_string (ctx, "player-choice", NULL));
            entry->gold_impact = lrg_save_context_read_int (ctx, "gold-impact", 0);
            entry->exposure_impact = (gfloat)lrg_save_context_read_double (ctx, "exposure-impact", 0.0);

            g_ptr_array_add (self->entries, entry);

            /* Update type counts */
            if (entry->event_type >= 0 && entry->event_type < 4)
                self->count_by_type[entry->event_type]++;

            lrg_save_context_leave_section (ctx);
        }
        lrg_save_context_leave_section (ctx);
    }

    /* Load milestones */
    milestone_count = lrg_save_context_read_uint (ctx, "milestone-count", 0);

    if (lrg_save_context_enter_section (ctx, "milestones"))
    {
        for (i = 0; i < milestone_count; i++)
        {
            LpChronicleEntry *entry;
            g_autofree gchar *key = g_strdup_printf ("milestone-%u", i);

            if (!lrg_save_context_enter_section (ctx, key))
                continue;

            entry = g_slice_new0 (LpChronicleEntry);
            entry->year_occurred = lrg_save_context_read_uint (ctx, "year", 847);
            entry->event_name = g_strdup (lrg_save_context_read_string (ctx, "title", "Milestone"));
            entry->description = g_strdup (lrg_save_context_read_string (ctx, "description", ""));

            g_ptr_array_add (self->milestones, entry);
            lrg_save_context_leave_section (ctx);
        }
        lrg_save_context_leave_section (ctx);
    }

    lp_log_debug ("Loaded chronicle with %u entries and %u milestones",
                  self->entries->len, self->milestones->len);

    return TRUE;
}

static void
lp_event_chronicle_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_event_chronicle_get_save_id;
    iface->save = lp_event_chronicle_save;
    iface->load = lp_event_chronicle_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_event_chronicle_dispose (GObject *object)
{
    LpEventChronicle *self = LP_EVENT_CHRONICLE (object);

    g_clear_pointer (&self->entries, g_ptr_array_unref);
    g_clear_pointer (&self->milestones, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_event_chronicle_parent_class)->dispose (object);
}

static void
lp_event_chronicle_class_init (LpEventChronicleClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = lp_event_chronicle_dispose;
}

static void
lp_event_chronicle_init (LpEventChronicle *self)
{
    self->entries = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_chronicle_entry_free);
    self->milestones = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_chronicle_entry_free);
    memset (self->count_by_type, 0, sizeof (self->count_by_type));
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

LpEventChronicle *
lp_event_chronicle_get_default (void)
{
    if (default_instance == NULL)
    {
        default_instance = g_object_new (LP_TYPE_EVENT_CHRONICLE, NULL);
        lp_log_info ("Created default LpEventChronicle instance");
    }

    return default_instance;
}

void
lp_event_chronicle_record (LpEventChronicle *self,
                           LpEvent          *event,
                           guint64           year_resolved,
                           const gchar      *outcome,
                           gint64            gold_impact,
                           gfloat            exposure_impact)
{
    LpChronicleEntry *entry;

    g_return_if_fail (LP_IS_EVENT_CHRONICLE (self));
    g_return_if_fail (LP_IS_EVENT (event));

    entry = lp_chronicle_entry_new (event, year_resolved);
    entry->outcome = g_strdup (outcome);
    entry->gold_impact = gold_impact;
    entry->exposure_impact = exposure_impact;

    /* Insert at beginning (most recent first) */
    g_ptr_array_insert (self->entries, 0, entry);

    /* Update type count */
    if (entry->event_type >= 0 && entry->event_type < 4)
        self->count_by_type[entry->event_type]++;

    lp_log_debug ("Chronicled event: %s (%s, year %lu)",
                  entry->event_name,
                  entry->event_type == LP_EVENT_TYPE_ECONOMIC ? "economic" :
                  entry->event_type == LP_EVENT_TYPE_POLITICAL ? "political" :
                  entry->event_type == LP_EVENT_TYPE_MAGICAL ? "magical" : "personal",
                  (unsigned long)entry->year_occurred);
}

void
lp_event_chronicle_record_with_choice (LpEventChronicle *self,
                                       LpEvent          *event,
                                       guint64           year_resolved,
                                       const gchar      *choice_id,
                                       const gchar      *outcome,
                                       gint64            gold_impact,
                                       gfloat            exposure_impact)
{
    LpChronicleEntry *entry;

    g_return_if_fail (LP_IS_EVENT_CHRONICLE (self));
    g_return_if_fail (LP_IS_EVENT (event));

    entry = lp_chronicle_entry_new (event, year_resolved);
    entry->player_choice = g_strdup (choice_id);
    entry->outcome = g_strdup (outcome);
    entry->gold_impact = gold_impact;
    entry->exposure_impact = exposure_impact;

    g_ptr_array_insert (self->entries, 0, entry);

    if (entry->event_type >= 0 && entry->event_type < 4)
        self->count_by_type[entry->event_type]++;

    lp_log_debug ("Chronicled event with choice %s: %s", choice_id, entry->event_name);
}

GPtrArray *
lp_event_chronicle_get_all (LpEventChronicle *self)
{
    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), NULL);
    return self->entries;
}

GPtrArray *
lp_event_chronicle_get_by_type (LpEventChronicle *self,
                                LpEventType       event_type)
{
    GPtrArray *result;
    guint i;

    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), NULL);

    result = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_chronicle_entry_free);

    for (i = 0; i < self->entries->len; i++)
    {
        LpChronicleEntry *entry = g_ptr_array_index (self->entries, i);
        if (entry->event_type == event_type)
            g_ptr_array_add (result, lp_chronicle_entry_copy (entry));
    }

    return result;
}

GPtrArray *
lp_event_chronicle_get_by_year_range (LpEventChronicle *self,
                                      guint64           start_year,
                                      guint64           end_year)
{
    GPtrArray *result;
    guint i;

    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), NULL);

    result = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_chronicle_entry_free);

    for (i = 0; i < self->entries->len; i++)
    {
        LpChronicleEntry *entry = g_ptr_array_index (self->entries, i);
        if (entry->year_occurred >= start_year && entry->year_occurred <= end_year)
            g_ptr_array_add (result, lp_chronicle_entry_copy (entry));
    }

    return result;
}

GPtrArray *
lp_event_chronicle_get_by_kingdom (LpEventChronicle *self,
                                   const gchar      *kingdom_id)
{
    GPtrArray *result;
    guint i;

    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), NULL);
    g_return_val_if_fail (kingdom_id != NULL, NULL);

    result = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_chronicle_entry_free);

    for (i = 0; i < self->entries->len; i++)
    {
        LpChronicleEntry *entry = g_ptr_array_index (self->entries, i);
        if (entry->affected_kingdom != NULL &&
            g_strcmp0 (entry->affected_kingdom, kingdom_id) == 0)
        {
            g_ptr_array_add (result, lp_chronicle_entry_copy (entry));
        }
    }

    return result;
}

GPtrArray *
lp_event_chronicle_get_by_severity (LpEventChronicle *self,
                                    LpEventSeverity   min_severity)
{
    GPtrArray *result;
    guint i;

    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), NULL);

    result = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_chronicle_entry_free);

    for (i = 0; i < self->entries->len; i++)
    {
        LpChronicleEntry *entry = g_ptr_array_index (self->entries, i);
        if (entry->severity >= min_severity)
            g_ptr_array_add (result, lp_chronicle_entry_copy (entry));
    }

    return result;
}

GPtrArray *
lp_event_chronicle_get_recent (LpEventChronicle *self,
                               guint             count)
{
    GPtrArray *result;
    guint i;
    guint actual_count;

    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), NULL);

    result = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_chronicle_entry_free);

    actual_count = MIN (count, self->entries->len);

    for (i = 0; i < actual_count; i++)
    {
        LpChronicleEntry *entry = g_ptr_array_index (self->entries, i);
        g_ptr_array_add (result, lp_chronicle_entry_copy (entry));
    }

    return result;
}

guint
lp_event_chronicle_get_count (LpEventChronicle *self)
{
    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), 0);
    return self->entries->len;
}

guint
lp_event_chronicle_get_count_by_type (LpEventChronicle *self,
                                      LpEventType       event_type)
{
    g_return_val_if_fail (LP_IS_EVENT_CHRONICLE (self), 0);

    if (event_type < 0 || event_type >= 4)
        return 0;

    return self->count_by_type[event_type];
}

void
lp_event_chronicle_add_milestone (LpEventChronicle *self,
                                  guint64           year,
                                  const gchar      *title,
                                  const gchar      *description)
{
    LpChronicleEntry *entry;

    g_return_if_fail (LP_IS_EVENT_CHRONICLE (self));
    g_return_if_fail (title != NULL);

    entry = g_slice_new0 (LpChronicleEntry);
    entry->year_occurred = year;
    entry->event_name = g_strdup (title);
    entry->description = g_strdup (description);

    g_ptr_array_add (self->milestones, entry);

    lp_log_info ("Chronicle milestone: %s (year %lu)", title, (unsigned long)year);
}

void
lp_event_chronicle_reset (LpEventChronicle *self)
{
    g_return_if_fail (LP_IS_EVENT_CHRONICLE (self));

    g_ptr_array_set_size (self->entries, 0);
    g_ptr_array_set_size (self->milestones, 0);
    memset (self->count_by_type, 0, sizeof (self->count_by_type));

    lp_log_debug ("Chronicle reset");
}
