/* lp-portfolio-history.h - Portfolio History Tracking
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>
#include <libregnum.h>

G_BEGIN_DECLS

/*
 * LpPortfolioSnapshot:
 *
 * A snapshot of portfolio values at a specific point in time.
 * This is a GBoxed type for efficient storage in arrays.
 */
typedef struct _LpPortfolioSnapshot LpPortfolioSnapshot;

#define LP_TYPE_PORTFOLIO_SNAPSHOT (lp_portfolio_snapshot_get_type ())

GType lp_portfolio_snapshot_get_type (void) G_GNUC_CONST;

/**
 * lp_portfolio_snapshot_new:
 * @year: the year of the snapshot
 * @total_value: the total portfolio value
 * @gold: the gold amount
 * @investment_value: the investment value
 *
 * Creates a new portfolio snapshot.
 *
 * Returns: (transfer full): a new #LpPortfolioSnapshot
 */
LpPortfolioSnapshot *lp_portfolio_snapshot_new (guint64       year,
                                                 LrgBigNumber *total_value,
                                                 LrgBigNumber *gold,
                                                 LrgBigNumber *investment_value);

/**
 * lp_portfolio_snapshot_copy:
 * @snapshot: a #LpPortfolioSnapshot
 *
 * Creates a copy of the snapshot.
 *
 * Returns: (transfer full): a copy of @snapshot
 */
LpPortfolioSnapshot *lp_portfolio_snapshot_copy (const LpPortfolioSnapshot *snapshot);

/**
 * lp_portfolio_snapshot_free:
 * @snapshot: a #LpPortfolioSnapshot
 *
 * Frees the snapshot.
 */
void lp_portfolio_snapshot_free (LpPortfolioSnapshot *snapshot);

/**
 * lp_portfolio_snapshot_get_year:
 * @snapshot: a #LpPortfolioSnapshot
 *
 * Gets the year of the snapshot.
 *
 * Returns: the year
 */
guint64 lp_portfolio_snapshot_get_year (const LpPortfolioSnapshot *snapshot);

/**
 * lp_portfolio_snapshot_get_total_value:
 * @snapshot: a #LpPortfolioSnapshot
 *
 * Gets the total portfolio value at the snapshot.
 *
 * Returns: (transfer none): the total value
 */
LrgBigNumber *lp_portfolio_snapshot_get_total_value (const LpPortfolioSnapshot *snapshot);

/**
 * lp_portfolio_snapshot_get_gold:
 * @snapshot: a #LpPortfolioSnapshot
 *
 * Gets the gold amount at the snapshot.
 *
 * Returns: (transfer none): the gold amount
 */
LrgBigNumber *lp_portfolio_snapshot_get_gold (const LpPortfolioSnapshot *snapshot);

/**
 * lp_portfolio_snapshot_get_investment_value:
 * @snapshot: a #LpPortfolioSnapshot
 *
 * Gets the investment value at the snapshot.
 *
 * Returns: (transfer none): the investment value
 */
LrgBigNumber *lp_portfolio_snapshot_get_investment_value (const LpPortfolioSnapshot *snapshot);

/*
 * LpPortfolioHistory:
 *
 * Tracks portfolio value snapshots over time for charting.
 * History is reset on prestige - only tracks current run's progress.
 */
#define LP_TYPE_PORTFOLIO_HISTORY (lp_portfolio_history_get_type ())
G_DECLARE_FINAL_TYPE (LpPortfolioHistory, lp_portfolio_history, LP, PORTFOLIO_HISTORY, GObject)

/**
 * lp_portfolio_history_new:
 *
 * Creates a new portfolio history tracker.
 *
 * Returns: (transfer full): a new #LpPortfolioHistory
 */
LpPortfolioHistory *lp_portfolio_history_new (void);

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
void lp_portfolio_history_record_snapshot (LpPortfolioHistory *self,
                                            guint64             year,
                                            LrgBigNumber       *total_value,
                                            LrgBigNumber       *gold,
                                            LrgBigNumber       *investment_value);

/**
 * lp_portfolio_history_get_snapshots:
 * @self: an #LpPortfolioHistory
 *
 * Gets all recorded snapshots.
 *
 * Returns: (transfer none) (element-type LpPortfolioSnapshot): the snapshots array
 */
GPtrArray *lp_portfolio_history_get_snapshots (LpPortfolioHistory *self);

/**
 * lp_portfolio_history_get_snapshot_count:
 * @self: an #LpPortfolioHistory
 *
 * Gets the number of recorded snapshots.
 *
 * Returns: the snapshot count
 */
guint lp_portfolio_history_get_snapshot_count (LpPortfolioHistory *self);

/**
 * lp_portfolio_history_clear:
 * @self: an #LpPortfolioHistory
 *
 * Clears all recorded snapshots. Called on prestige reset.
 */
void lp_portfolio_history_clear (LpPortfolioHistory *self);

/**
 * lp_portfolio_history_get_latest_snapshot:
 * @self: an #LpPortfolioHistory
 *
 * Gets the most recent snapshot.
 *
 * Returns: (transfer none) (nullable): the latest snapshot or %NULL if empty
 */
const LpPortfolioSnapshot *lp_portfolio_history_get_latest_snapshot (LpPortfolioHistory *self);

G_END_DECLS
