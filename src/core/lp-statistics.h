/* lp-statistics.h - Comprehensive Lifetime Statistics Tracking
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpStatistics tracks all lifetime statistics for the player.
 * Unlike LpPortfolioHistory (reset on prestige), these stats persist
 * across all runs and provide data for the statistics dashboard.
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_STATISTICS_H
#define LP_STATISTICS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATISTICS (lp_statistics_get_type ())

G_DECLARE_FINAL_TYPE (LpStatistics, lp_statistics, LP, STATISTICS, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_statistics_get_default:
 *
 * Gets the default statistics instance.
 * Creates it if it doesn't exist.
 *
 * Returns: (transfer none): The default #LpStatistics instance
 */
LpStatistics *
lp_statistics_get_default (void);

/* ==========================================================================
 * Wealth Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_lifetime_gold_earned:
 * @self: an #LpStatistics
 *
 * Gets the total gold earned across all runs.
 *
 * Returns: (transfer full): Lifetime gold earned as #LrgBigNumber
 */
LrgBigNumber *
lp_statistics_get_lifetime_gold_earned (LpStatistics *self);

/**
 * lp_statistics_get_lifetime_gold_spent:
 * @self: an #LpStatistics
 *
 * Gets the total gold spent across all runs.
 *
 * Returns: (transfer full): Lifetime gold spent as #LrgBigNumber
 */
LrgBigNumber *
lp_statistics_get_lifetime_gold_spent (LpStatistics *self);

/**
 * lp_statistics_get_peak_net_worth:
 * @self: an #LpStatistics
 *
 * Gets the highest net worth ever achieved.
 *
 * Returns: (transfer full): Peak net worth as #LrgBigNumber
 */
LrgBigNumber *
lp_statistics_get_peak_net_worth (LpStatistics *self);

/**
 * lp_statistics_get_peak_net_worth_year:
 * @self: an #LpStatistics
 *
 * Gets the year when peak net worth was achieved.
 *
 * Returns: The year
 */
guint64
lp_statistics_get_peak_net_worth_year (LpStatistics *self);

/* ==========================================================================
 * Investment Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_investments_purchased:
 * @self: an #LpStatistics
 *
 * Gets the total number of investments purchased.
 *
 * Returns: Number of investments purchased
 */
guint64
lp_statistics_get_investments_purchased (LpStatistics *self);

/**
 * lp_statistics_get_investments_sold:
 * @self: an #LpStatistics
 *
 * Gets the total number of investments sold.
 *
 * Returns: Number of investments sold
 */
guint64
lp_statistics_get_investments_sold (LpStatistics *self);

/**
 * lp_statistics_get_investments_lost:
 * @self: an #LpStatistics
 *
 * Gets the total number of investments lost (destroyed by events, etc.).
 *
 * Returns: Number of investments lost
 */
guint64
lp_statistics_get_investments_lost (LpStatistics *self);

/**
 * lp_statistics_get_total_investment_returns:
 * @self: an #LpStatistics
 *
 * Gets the total returns from all investments.
 *
 * Returns: (transfer full): Total returns as #LrgBigNumber
 */
LrgBigNumber *
lp_statistics_get_total_investment_returns (LpStatistics *self);

/**
 * lp_statistics_get_longest_investment_held:
 * @self: an #LpStatistics
 *
 * Gets the longest duration an investment was held (years).
 *
 * Returns: Years
 */
guint
lp_statistics_get_longest_investment_held (LpStatistics *self);

/* ==========================================================================
 * Agent Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_agents_recruited:
 * @self: an #LpStatistics
 *
 * Gets the total number of agents recruited.
 *
 * Returns: Number of agents recruited
 */
guint64
lp_statistics_get_agents_recruited (LpStatistics *self);

/**
 * lp_statistics_get_agent_deaths:
 * @self: an #LpStatistics
 *
 * Gets the total number of agent deaths.
 *
 * Returns: Number of agent deaths
 */
guint64
lp_statistics_get_agent_deaths (LpStatistics *self);

/**
 * lp_statistics_get_agent_betrayals:
 * @self: an #LpStatistics
 *
 * Gets the total number of agent betrayals.
 *
 * Returns: Number of betrayals
 */
guint64
lp_statistics_get_agent_betrayals (LpStatistics *self);

/**
 * lp_statistics_get_highest_family_generation:
 * @self: an #LpStatistics
 *
 * Gets the highest family generation reached.
 *
 * Returns: Highest generation
 */
guint
lp_statistics_get_highest_family_generation (LpStatistics *self);

/**
 * lp_statistics_get_total_agent_years_served:
 * @self: an #LpStatistics
 *
 * Gets the total years of agent service.
 *
 * Returns: Total years served
 */
guint64
lp_statistics_get_total_agent_years_served (LpStatistics *self);

/* ==========================================================================
 * Time Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_total_years_slumbered:
 * @self: an #LpStatistics
 *
 * Gets the total years spent in slumber.
 *
 * Returns: Total years slumbered
 */
guint64
lp_statistics_get_total_years_slumbered (LpStatistics *self);

/**
 * lp_statistics_get_longest_slumber:
 * @self: an #LpStatistics
 *
 * Gets the longest single slumber duration.
 *
 * Returns: Longest slumber in years
 */
guint
lp_statistics_get_longest_slumber (LpStatistics *self);

/**
 * lp_statistics_get_total_awakenings:
 * @self: an #LpStatistics
 *
 * Gets the total number of awakenings from slumber.
 *
 * Returns: Number of awakenings
 */
guint64
lp_statistics_get_total_awakenings (LpStatistics *self);

/* ==========================================================================
 * World Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_events_witnessed:
 * @self: an #LpStatistics
 *
 * Gets the total number of world events witnessed.
 *
 * Returns: Number of events
 */
guint64
lp_statistics_get_events_witnessed (LpStatistics *self);

/**
 * lp_statistics_get_kingdoms_collapsed:
 * @self: an #LpStatistics
 *
 * Gets the total number of kingdoms that collapsed.
 *
 * Returns: Number of collapses
 */
guint64
lp_statistics_get_kingdoms_collapsed (LpStatistics *self);

/**
 * lp_statistics_get_crusades_survived:
 * @self: an #LpStatistics
 *
 * Gets the number of crusades survived.
 *
 * Returns: Number of crusades survived
 */
guint64
lp_statistics_get_crusades_survived (LpStatistics *self);

/**
 * lp_statistics_get_competitors_defeated:
 * @self: an #LpStatistics
 *
 * Gets the number of competitors defeated.
 *
 * Returns: Number of competitors defeated
 */
guint64
lp_statistics_get_competitors_defeated (LpStatistics *self);

/* ==========================================================================
 * Prestige Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_prestige_count:
 * @self: an #LpStatistics
 *
 * Gets the number of prestiges completed.
 *
 * Returns: Number of prestiges
 */
guint64
lp_statistics_get_prestige_count (LpStatistics *self);

/**
 * lp_statistics_get_total_phylactery_points_earned:
 * @self: an #LpStatistics
 *
 * Gets the total phylactery points earned.
 *
 * Returns: (transfer full): Total points as #LrgBigNumber
 */
LrgBigNumber *
lp_statistics_get_total_phylactery_points_earned (LpStatistics *self);

/**
 * lp_statistics_get_best_prestige_run:
 * @self: an #LpStatistics
 *
 * Gets the best prestige run (most points earned in a single run).
 *
 * Returns: (transfer full): Best run points as #LrgBigNumber
 */
LrgBigNumber *
lp_statistics_get_best_prestige_run (LpStatistics *self);

/* ==========================================================================
 * Session Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_total_play_time_seconds:
 * @self: an #LpStatistics
 *
 * Gets the total play time in seconds.
 *
 * Returns: Total play time in seconds
 */
guint64
lp_statistics_get_total_play_time_seconds (LpStatistics *self);

/**
 * lp_statistics_get_session_count:
 * @self: an #LpStatistics
 *
 * Gets the total number of play sessions.
 *
 * Returns: Number of sessions
 */
guint64
lp_statistics_get_session_count (LpStatistics *self);

/**
 * lp_statistics_get_first_play_timestamp:
 * @self: an #LpStatistics
 *
 * Gets the Unix timestamp of the first play session.
 *
 * Returns: Unix timestamp, or 0 if never played
 */
gint64
lp_statistics_get_first_play_timestamp (LpStatistics *self);

/* ==========================================================================
 * Dark Arts Statistics
 * ========================================================================== */

/**
 * lp_statistics_get_soul_trades_completed:
 * @self: an #LpStatistics
 *
 * Gets the number of soul trades completed.
 *
 * Returns: Number of soul trades
 */
guint64
lp_statistics_get_soul_trades_completed (LpStatistics *self);

/**
 * lp_statistics_get_dark_investments_owned:
 * @self: an #LpStatistics
 *
 * Gets the number of dark investments owned (ever).
 *
 * Returns: Number of dark investments
 */
guint64
lp_statistics_get_dark_investments_owned (LpStatistics *self);

/* ==========================================================================
 * Game Event Hooks
 *
 * These functions should be called by game systems when events occur.
 * ========================================================================== */

/**
 * lp_statistics_on_gold_earned:
 * @self: an #LpStatistics
 * @amount: gold amount earned
 *
 * Called when gold is earned.
 */
void
lp_statistics_on_gold_earned (LpStatistics *self,
                              LrgBigNumber *amount);

/**
 * lp_statistics_on_gold_spent:
 * @self: an #LpStatistics
 * @amount: gold amount spent
 *
 * Called when gold is spent.
 */
void
lp_statistics_on_gold_spent (LpStatistics *self,
                             LrgBigNumber *amount);

/**
 * lp_statistics_on_net_worth_changed:
 * @self: an #LpStatistics
 * @net_worth: current net worth
 * @year: current year
 *
 * Called when net worth changes. Updates peak tracking.
 */
void
lp_statistics_on_net_worth_changed (LpStatistics *self,
                                    LrgBigNumber *net_worth,
                                    guint64       year);

/**
 * lp_statistics_on_investment_purchased:
 * @self: an #LpStatistics
 *
 * Called when an investment is purchased.
 */
void
lp_statistics_on_investment_purchased (LpStatistics *self);

/**
 * lp_statistics_on_investment_sold:
 * @self: an #LpStatistics
 * @returns: returns from the sale
 *
 * Called when an investment is sold.
 */
void
lp_statistics_on_investment_sold (LpStatistics *self,
                                  LrgBigNumber *returns);

/**
 * lp_statistics_on_investment_lost:
 * @self: an #LpStatistics
 *
 * Called when an investment is lost (destroyed by events).
 */
void
lp_statistics_on_investment_lost (LpStatistics *self);

/**
 * lp_statistics_on_investment_held:
 * @self: an #LpStatistics
 * @years: years held
 *
 * Called to update longest investment hold record.
 */
void
lp_statistics_on_investment_held (LpStatistics *self,
                                  guint         years);

/**
 * lp_statistics_on_agent_recruited:
 * @self: an #LpStatistics
 *
 * Called when an agent is recruited.
 */
void
lp_statistics_on_agent_recruited (LpStatistics *self);

/**
 * lp_statistics_on_agent_death:
 * @self: an #LpStatistics
 * @years_served: years the agent served
 *
 * Called when an agent dies.
 */
void
lp_statistics_on_agent_death (LpStatistics *self,
                              guint         years_served);

/**
 * lp_statistics_on_agent_betrayal:
 * @self: an #LpStatistics
 *
 * Called when an agent betrays.
 */
void
lp_statistics_on_agent_betrayal (LpStatistics *self);

/**
 * lp_statistics_on_family_succession:
 * @self: an #LpStatistics
 * @generation: new generation number
 *
 * Called on family succession.
 */
void
lp_statistics_on_family_succession (LpStatistics *self,
                                    guint         generation);

/**
 * lp_statistics_on_slumber_complete:
 * @self: an #LpStatistics
 * @years: years slumbered
 *
 * Called when slumber completes.
 */
void
lp_statistics_on_slumber_complete (LpStatistics *self,
                                   guint         years);

/**
 * lp_statistics_on_event_witnessed:
 * @self: an #LpStatistics
 *
 * Called when a world event occurs.
 */
void
lp_statistics_on_event_witnessed (LpStatistics *self);

/**
 * lp_statistics_on_kingdom_collapsed:
 * @self: an #LpStatistics
 *
 * Called when a kingdom collapses.
 */
void
lp_statistics_on_kingdom_collapsed (LpStatistics *self);

/**
 * lp_statistics_on_crusade_survived:
 * @self: an #LpStatistics
 *
 * Called when a crusade is survived.
 */
void
lp_statistics_on_crusade_survived (LpStatistics *self);

/**
 * lp_statistics_on_competitor_defeated:
 * @self: an #LpStatistics
 *
 * Called when a competitor is defeated.
 */
void
lp_statistics_on_competitor_defeated (LpStatistics *self);

/**
 * lp_statistics_on_prestige:
 * @self: an #LpStatistics
 * @points: phylactery points earned
 *
 * Called when player prestiges.
 */
void
lp_statistics_on_prestige (LpStatistics *self,
                           LrgBigNumber *points);

/**
 * lp_statistics_on_session_start:
 * @self: an #LpStatistics
 *
 * Called when a play session starts.
 */
void
lp_statistics_on_session_start (LpStatistics *self);

/**
 * lp_statistics_on_session_end:
 * @self: an #LpStatistics
 * @duration_seconds: session duration in seconds
 *
 * Called when a play session ends.
 */
void
lp_statistics_on_session_end (LpStatistics *self,
                              guint64       duration_seconds);

/**
 * lp_statistics_on_soul_trade:
 * @self: an #LpStatistics
 *
 * Called when a soul trade is completed.
 */
void
lp_statistics_on_soul_trade (LpStatistics *self);

/**
 * lp_statistics_on_dark_investment:
 * @self: an #LpStatistics
 *
 * Called when a dark investment is acquired.
 */
void
lp_statistics_on_dark_investment (LpStatistics *self);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_statistics_reset:
 * @self: an #LpStatistics
 *
 * Resets all statistics to zero.
 * WARNING: This clears all lifetime stats. Use with caution.
 */
void
lp_statistics_reset (LpStatistics *self);

G_END_DECLS

#endif /* LP_STATISTICS_H */
