/* lp-portfolio-history.c - Portfolio History Tracking
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-portfolio-history.h"

/* ==========================================================================
 * LpPortfolioSnapshot GBoxed Type
 * ========================================================================== */

struct _LpPortfolioSnapshot
{
    guint64       year;
    LrgBigNumber *total_value;
    LrgBigNumber *gold;
    LrgBigNumber *investment_value;
};

G_DEFINE_BOXED_TYPE (LpPortfolioSnapshot,
                     lp_portfolio_snapshot,
                     lp_portfolio_snapshot_copy,
                     lp_portfolio_snapshot_free)

LpPortfolioSnapshot *
lp_portfolio_snapshot_new (guint64       year,
                           LrgBigNumber *total_value,
                           LrgBigNumber *gold,
                           LrgBigNumber *investment_value)
{
    LpPortfolioSnapshot *snapshot;

    snapshot = g_new0 (LpPortfolioSnapshot, 1);
    snapshot->year = year;
    snapshot->total_value = total_value ? lrg_big_number_copy (total_value)
                                         : lrg_big_number_new (0.0);
    snapshot->gold = gold ? lrg_big_number_copy (gold)
                          : lrg_big_number_new (0.0);
    snapshot->investment_value = investment_value ? lrg_big_number_copy (investment_value)
                                                   : lrg_big_number_new (0.0);

    return snapshot;
}

LpPortfolioSnapshot *
lp_portfolio_snapshot_copy (const LpPortfolioSnapshot *snapshot)
{
    g_return_val_if_fail (snapshot != NULL, NULL);

    return lp_portfolio_snapshot_new (snapshot->year,
                                       snapshot->total_value,
                                       snapshot->gold,
                                       snapshot->investment_value);
}

void
lp_portfolio_snapshot_free (LpPortfolioSnapshot *snapshot)
{
    if (snapshot == NULL)
        return;

    lrg_big_number_free (snapshot->total_value);
    lrg_big_number_free (snapshot->gold);
    lrg_big_number_free (snapshot->investment_value);
    g_free (snapshot);
}

guint64
lp_portfolio_snapshot_get_year (const LpPortfolioSnapshot *snapshot)
{
    g_return_val_if_fail (snapshot != NULL, 0);
    return snapshot->year;
}

LrgBigNumber *
lp_portfolio_snapshot_get_total_value (const LpPortfolioSnapshot *snapshot)
{
    g_return_val_if_fail (snapshot != NULL, NULL);
    return snapshot->total_value;
}

LrgBigNumber *
lp_portfolio_snapshot_get_gold (const LpPortfolioSnapshot *snapshot)
{
    g_return_val_if_fail (snapshot != NULL, NULL);
    return snapshot->gold;
}

LrgBigNumber *
lp_portfolio_snapshot_get_investment_value (const LpPortfolioSnapshot *snapshot)
{
    g_return_val_if_fail (snapshot != NULL, NULL);
    return snapshot->investment_value;
}

/* ==========================================================================
 * LpPortfolioHistory GObject
 * ========================================================================== */

struct _LpPortfolioHistory
{
    GObject parent_instance;

    GPtrArray *snapshots;  /* Array of LpPortfolioSnapshot */
};

/* Forward declarations for LrgSaveable interface */
static void lp_portfolio_history_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpPortfolioHistory, lp_portfolio_history, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_portfolio_history_saveable_init))

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_portfolio_history_get_save_id (LrgSaveable *saveable)
{
    (void)saveable;
    return "portfolio-history";
}

static gboolean
lp_portfolio_history_save (LrgSaveable    *saveable,
                           LrgSaveContext *context,
                           GError        **error)
{
    LpPortfolioHistory *self = LP_PORTFOLIO_HISTORY (saveable);
    guint i;

    (void)error;

    lrg_save_context_write_uint (context, "snapshot-count", self->snapshots->len);

    lrg_save_context_begin_section (context, "snapshots");
    for (i = 0; i < self->snapshots->len; i++)
    {
        LpPortfolioSnapshot *snapshot;
        gchar section_name[32];

        snapshot = g_ptr_array_index (self->snapshots, i);
        g_snprintf (section_name, sizeof (section_name), "snapshot-%u", i);

        lrg_save_context_begin_section (context, section_name);
        lrg_save_context_write_uint (context, "year", snapshot->year);

        /* Save big numbers as mantissa/exponent pairs */
        lrg_save_context_write_double (context, "total-value-mantissa",
                                        lrg_big_number_get_mantissa (snapshot->total_value));
        lrg_save_context_write_int (context, "total-value-exponent",
                                     lrg_big_number_get_exponent (snapshot->total_value));

        lrg_save_context_write_double (context, "gold-mantissa",
                                        lrg_big_number_get_mantissa (snapshot->gold));
        lrg_save_context_write_int (context, "gold-exponent",
                                     lrg_big_number_get_exponent (snapshot->gold));

        lrg_save_context_write_double (context, "investment-value-mantissa",
                                        lrg_big_number_get_mantissa (snapshot->investment_value));
        lrg_save_context_write_int (context, "investment-value-exponent",
                                     lrg_big_number_get_exponent (snapshot->investment_value));

        lrg_save_context_end_section (context);
    }
    lrg_save_context_end_section (context);

    return TRUE;
}

static gboolean
lp_portfolio_history_load (LrgSaveable    *saveable,
                           LrgSaveContext *context,
                           GError        **error)
{
    LpPortfolioHistory *self = LP_PORTFOLIO_HISTORY (saveable);
    guint count;
    guint i;

    (void)error;

    /* Clear existing snapshots */
    g_ptr_array_set_size (self->snapshots, 0);

    count = (guint)lrg_save_context_read_uint (context, "snapshot-count", 0);

    if (lrg_save_context_enter_section (context, "snapshots"))
    {
        for (i = 0; i < count; i++)
        {
            gchar section_name[32];
            guint64 year;
            g_autoptr(LrgBigNumber) total_value = NULL;
            g_autoptr(LrgBigNumber) gold = NULL;
            g_autoptr(LrgBigNumber) investment_value = NULL;
            LpPortfolioSnapshot *snapshot;

            g_snprintf (section_name, sizeof (section_name), "snapshot-%u", i);

            if (lrg_save_context_enter_section (context, section_name))
            {
                gdouble mantissa;
                gint exponent;

                year = lrg_save_context_read_uint (context, "year", 0);

                /* Load big numbers from mantissa/exponent pairs */
                mantissa = lrg_save_context_read_double (context, "total-value-mantissa", 0.0);
                exponent = (gint)lrg_save_context_read_int (context, "total-value-exponent", 0);
                total_value = lrg_big_number_new_from_parts (mantissa, exponent);

                mantissa = lrg_save_context_read_double (context, "gold-mantissa", 0.0);
                exponent = (gint)lrg_save_context_read_int (context, "gold-exponent", 0);
                gold = lrg_big_number_new_from_parts (mantissa, exponent);

                mantissa = lrg_save_context_read_double (context, "investment-value-mantissa", 0.0);
                exponent = (gint)lrg_save_context_read_int (context, "investment-value-exponent", 0);
                investment_value = lrg_big_number_new_from_parts (mantissa, exponent);

                snapshot = lp_portfolio_snapshot_new (year, total_value, gold, investment_value);
                g_ptr_array_add (self->snapshots, snapshot);

                lrg_save_context_leave_section (context);
            }
        }
        lrg_save_context_leave_section (context);
    }

    lp_log_info ("Loaded %u portfolio history snapshots", self->snapshots->len);

    return TRUE;
}

static void
lp_portfolio_history_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_portfolio_history_get_save_id;
    iface->save = lp_portfolio_history_save;
    iface->load = lp_portfolio_history_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_portfolio_history_dispose (GObject *object)
{
    LpPortfolioHistory *self = LP_PORTFOLIO_HISTORY (object);

    g_clear_pointer (&self->snapshots, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_portfolio_history_parent_class)->dispose (object);
}

static void
lp_portfolio_history_class_init (LpPortfolioHistoryClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = lp_portfolio_history_dispose;
}

static void
lp_portfolio_history_init (LpPortfolioHistory *self)
{
    self->snapshots = g_ptr_array_new_with_free_func (
        (GDestroyNotify)lp_portfolio_snapshot_free);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_portfolio_history_new:
 *
 * Creates a new portfolio history tracker.
 *
 * Returns: (transfer full): a new #LpPortfolioHistory
 */
LpPortfolioHistory *
lp_portfolio_history_new (void)
{
    return g_object_new (LP_TYPE_PORTFOLIO_HISTORY, NULL);
}

/**
 * lp_portfolio_history_record_snapshot:
 * @self: an #LpPortfolioHistory
 * @year: the year of the snapshot
 * @total_value: the total portfolio value
 * @gold: the gold amount
 * @investment_value: the investment value
 *
 * Records a new snapshot at the specified year.
 */
void
lp_portfolio_history_record_snapshot (LpPortfolioHistory *self,
                                       guint64             year,
                                       LrgBigNumber       *total_value,
                                       LrgBigNumber       *gold,
                                       LrgBigNumber       *investment_value)
{
    LpPortfolioSnapshot *snapshot;

    g_return_if_fail (LP_IS_PORTFOLIO_HISTORY (self));

    snapshot = lp_portfolio_snapshot_new (year, total_value, gold, investment_value);
    g_ptr_array_add (self->snapshots, snapshot);

    lp_log_debug ("Recorded portfolio snapshot for year %lu (total: %s)",
                  (gulong)year,
                  total_value ? lrg_big_number_format_short (total_value) : "0");
}

/**
 * lp_portfolio_history_get_snapshots:
 * @self: an #LpPortfolioHistory
 *
 * Gets all recorded snapshots.
 *
 * Returns: (transfer none) (element-type LpPortfolioSnapshot): the snapshots array
 */
GPtrArray *
lp_portfolio_history_get_snapshots (LpPortfolioHistory *self)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO_HISTORY (self), NULL);

    return self->snapshots;
}

/**
 * lp_portfolio_history_get_snapshot_count:
 * @self: an #LpPortfolioHistory
 *
 * Gets the number of recorded snapshots.
 *
 * Returns: the snapshot count
 */
guint
lp_portfolio_history_get_snapshot_count (LpPortfolioHistory *self)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO_HISTORY (self), 0);

    return self->snapshots->len;
}

/**
 * lp_portfolio_history_clear:
 * @self: an #LpPortfolioHistory
 *
 * Clears all recorded snapshots. Called on prestige reset.
 */
void
lp_portfolio_history_clear (LpPortfolioHistory *self)
{
    g_return_if_fail (LP_IS_PORTFOLIO_HISTORY (self));

    g_ptr_array_set_size (self->snapshots, 0);

    lp_log_info ("Cleared portfolio history");
}

/**
 * lp_portfolio_history_get_latest_snapshot:
 * @self: an #LpPortfolioHistory
 *
 * Gets the most recent snapshot.
 *
 * Returns: (transfer none) (nullable): the latest snapshot or %NULL if empty
 */
const LpPortfolioSnapshot *
lp_portfolio_history_get_latest_snapshot (LpPortfolioHistory *self)
{
    g_return_val_if_fail (LP_IS_PORTFOLIO_HISTORY (self), NULL);

    if (self->snapshots->len == 0)
        return NULL;

    return g_ptr_array_index (self->snapshots, self->snapshots->len - 1);
}
