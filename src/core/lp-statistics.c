/* lp-statistics.c - Comprehensive Lifetime Statistics Tracking
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-statistics.h"

/* ==========================================================================
 * Singleton Instance
 * ========================================================================== */

static LpStatistics *default_statistics = NULL;

/* ==========================================================================
 * Private Structure
 * ========================================================================== */

struct _LpStatistics
{
    GObject parent_instance;

    /* Wealth statistics */
    LrgBigNumber *lifetime_gold_earned;
    LrgBigNumber *lifetime_gold_spent;
    LrgBigNumber *peak_net_worth;
    guint64       peak_net_worth_year;

    /* Investment statistics */
    guint64       investments_purchased;
    guint64       investments_sold;
    guint64       investments_lost;
    LrgBigNumber *total_investment_returns;
    guint         longest_investment_held;

    /* Agent statistics */
    guint64       agents_recruited;
    guint64       agent_deaths;
    guint64       agent_betrayals;
    guint         highest_family_generation;
    guint64       total_agent_years_served;

    /* Time statistics */
    guint64       total_years_slumbered;
    guint         longest_slumber;
    guint64       total_awakenings;

    /* World statistics */
    guint64       events_witnessed;
    guint64       kingdoms_collapsed;
    guint64       crusades_survived;
    guint64       competitors_defeated;

    /* Prestige statistics */
    guint64       prestige_count;
    LrgBigNumber *total_phylactery_points_earned;
    LrgBigNumber *best_prestige_run;

    /* Session statistics */
    guint64       total_play_time_seconds;
    guint64       session_count;
    gint64        first_play_timestamp;

    /* Dark arts statistics */
    guint64       soul_trades_completed;
    guint64       dark_investments_owned;
};

/* Forward declarations for LrgSaveable interface */
static void lp_statistics_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpStatistics, lp_statistics, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_statistics_saveable_init))

/* ==========================================================================
 * Helper: Save/Load BigNumber
 * ========================================================================== */

static void
save_big_number (LrgSaveContext *context,
                 const gchar    *prefix,
                 LrgBigNumber   *number)
{
    gchar key_mantissa[64];
    gchar key_exponent[64];

    g_snprintf (key_mantissa, sizeof (key_mantissa), "%s-mantissa", prefix);
    g_snprintf (key_exponent, sizeof (key_exponent), "%s-exponent", prefix);

    lrg_save_context_write_double (context, key_mantissa,
                                   lrg_big_number_get_mantissa (number));
    lrg_save_context_write_int (context, key_exponent,
                                lrg_big_number_get_exponent (number));
}

static LrgBigNumber *
load_big_number (LrgSaveContext *context,
                 const gchar    *prefix)
{
    gchar key_mantissa[64];
    gchar key_exponent[64];
    gdouble mantissa;
    gint exponent;

    g_snprintf (key_mantissa, sizeof (key_mantissa), "%s-mantissa", prefix);
    g_snprintf (key_exponent, sizeof (key_exponent), "%s-exponent", prefix);

    mantissa = lrg_save_context_read_double (context, key_mantissa, 0.0);
    exponent = (gint)lrg_save_context_read_int (context, key_exponent, 0);

    return lrg_big_number_new_from_parts (mantissa, exponent);
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_statistics_get_save_id (LrgSaveable *saveable)
{
    (void)saveable;
    return "statistics";
}

static gboolean
lp_statistics_save (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpStatistics *self = LP_STATISTICS (saveable);

    (void)error;

    /* Wealth statistics */
    lrg_save_context_begin_section (context, "wealth");
    save_big_number (context, "lifetime-gold-earned", self->lifetime_gold_earned);
    save_big_number (context, "lifetime-gold-spent", self->lifetime_gold_spent);
    save_big_number (context, "peak-net-worth", self->peak_net_worth);
    lrg_save_context_write_uint (context, "peak-net-worth-year", self->peak_net_worth_year);
    lrg_save_context_end_section (context);

    /* Investment statistics */
    lrg_save_context_begin_section (context, "investments");
    lrg_save_context_write_uint (context, "purchased", self->investments_purchased);
    lrg_save_context_write_uint (context, "sold", self->investments_sold);
    lrg_save_context_write_uint (context, "lost", self->investments_lost);
    save_big_number (context, "total-returns", self->total_investment_returns);
    lrg_save_context_write_uint (context, "longest-held", self->longest_investment_held);
    lrg_save_context_end_section (context);

    /* Agent statistics */
    lrg_save_context_begin_section (context, "agents");
    lrg_save_context_write_uint (context, "recruited", self->agents_recruited);
    lrg_save_context_write_uint (context, "deaths", self->agent_deaths);
    lrg_save_context_write_uint (context, "betrayals", self->agent_betrayals);
    lrg_save_context_write_uint (context, "highest-generation", self->highest_family_generation);
    lrg_save_context_write_uint (context, "total-years-served", self->total_agent_years_served);
    lrg_save_context_end_section (context);

    /* Time statistics */
    lrg_save_context_begin_section (context, "time");
    lrg_save_context_write_uint (context, "total-years-slumbered", self->total_years_slumbered);
    lrg_save_context_write_uint (context, "longest-slumber", self->longest_slumber);
    lrg_save_context_write_uint (context, "total-awakenings", self->total_awakenings);
    lrg_save_context_end_section (context);

    /* World statistics */
    lrg_save_context_begin_section (context, "world");
    lrg_save_context_write_uint (context, "events-witnessed", self->events_witnessed);
    lrg_save_context_write_uint (context, "kingdoms-collapsed", self->kingdoms_collapsed);
    lrg_save_context_write_uint (context, "crusades-survived", self->crusades_survived);
    lrg_save_context_write_uint (context, "competitors-defeated", self->competitors_defeated);
    lrg_save_context_end_section (context);

    /* Prestige statistics */
    lrg_save_context_begin_section (context, "prestige");
    lrg_save_context_write_uint (context, "count", self->prestige_count);
    save_big_number (context, "total-points", self->total_phylactery_points_earned);
    save_big_number (context, "best-run", self->best_prestige_run);
    lrg_save_context_end_section (context);

    /* Session statistics */
    lrg_save_context_begin_section (context, "session");
    lrg_save_context_write_uint (context, "total-play-time", self->total_play_time_seconds);
    lrg_save_context_write_uint (context, "session-count", self->session_count);
    lrg_save_context_write_int (context, "first-play-timestamp", self->first_play_timestamp);
    lrg_save_context_end_section (context);

    /* Dark arts statistics */
    lrg_save_context_begin_section (context, "dark-arts");
    lrg_save_context_write_uint (context, "soul-trades", self->soul_trades_completed);
    lrg_save_context_write_uint (context, "dark-investments", self->dark_investments_owned);
    lrg_save_context_end_section (context);

    lp_log_info ("Statistics saved successfully");

    return TRUE;
}

static gboolean
lp_statistics_load (LrgSaveable    *saveable,
                    LrgSaveContext *context,
                    GError        **error)
{
    LpStatistics *self = LP_STATISTICS (saveable);

    (void)error;

    /* Wealth statistics */
    if (lrg_save_context_enter_section (context, "wealth"))
    {
        g_clear_pointer (&self->lifetime_gold_earned, lrg_big_number_free);
        g_clear_pointer (&self->lifetime_gold_spent, lrg_big_number_free);
        g_clear_pointer (&self->peak_net_worth, lrg_big_number_free);

        self->lifetime_gold_earned = load_big_number (context, "lifetime-gold-earned");
        self->lifetime_gold_spent = load_big_number (context, "lifetime-gold-spent");
        self->peak_net_worth = load_big_number (context, "peak-net-worth");
        self->peak_net_worth_year = lrg_save_context_read_uint (context, "peak-net-worth-year", 0);

        lrg_save_context_leave_section (context);
    }

    /* Investment statistics */
    if (lrg_save_context_enter_section (context, "investments"))
    {
        g_clear_pointer (&self->total_investment_returns, lrg_big_number_free);

        self->investments_purchased = lrg_save_context_read_uint (context, "purchased", 0);
        self->investments_sold = lrg_save_context_read_uint (context, "sold", 0);
        self->investments_lost = lrg_save_context_read_uint (context, "lost", 0);
        self->total_investment_returns = load_big_number (context, "total-returns");
        self->longest_investment_held = (guint)lrg_save_context_read_uint (context, "longest-held", 0);

        lrg_save_context_leave_section (context);
    }

    /* Agent statistics */
    if (lrg_save_context_enter_section (context, "agents"))
    {
        self->agents_recruited = lrg_save_context_read_uint (context, "recruited", 0);
        self->agent_deaths = lrg_save_context_read_uint (context, "deaths", 0);
        self->agent_betrayals = lrg_save_context_read_uint (context, "betrayals", 0);
        self->highest_family_generation = (guint)lrg_save_context_read_uint (context, "highest-generation", 0);
        self->total_agent_years_served = lrg_save_context_read_uint (context, "total-years-served", 0);

        lrg_save_context_leave_section (context);
    }

    /* Time statistics */
    if (lrg_save_context_enter_section (context, "time"))
    {
        self->total_years_slumbered = lrg_save_context_read_uint (context, "total-years-slumbered", 0);
        self->longest_slumber = (guint)lrg_save_context_read_uint (context, "longest-slumber", 0);
        self->total_awakenings = lrg_save_context_read_uint (context, "total-awakenings", 0);

        lrg_save_context_leave_section (context);
    }

    /* World statistics */
    if (lrg_save_context_enter_section (context, "world"))
    {
        self->events_witnessed = lrg_save_context_read_uint (context, "events-witnessed", 0);
        self->kingdoms_collapsed = lrg_save_context_read_uint (context, "kingdoms-collapsed", 0);
        self->crusades_survived = lrg_save_context_read_uint (context, "crusades-survived", 0);
        self->competitors_defeated = lrg_save_context_read_uint (context, "competitors-defeated", 0);

        lrg_save_context_leave_section (context);
    }

    /* Prestige statistics */
    if (lrg_save_context_enter_section (context, "prestige"))
    {
        g_clear_pointer (&self->total_phylactery_points_earned, lrg_big_number_free);
        g_clear_pointer (&self->best_prestige_run, lrg_big_number_free);

        self->prestige_count = lrg_save_context_read_uint (context, "count", 0);
        self->total_phylactery_points_earned = load_big_number (context, "total-points");
        self->best_prestige_run = load_big_number (context, "best-run");

        lrg_save_context_leave_section (context);
    }

    /* Session statistics */
    if (lrg_save_context_enter_section (context, "session"))
    {
        self->total_play_time_seconds = lrg_save_context_read_uint (context, "total-play-time", 0);
        self->session_count = lrg_save_context_read_uint (context, "session-count", 0);
        self->first_play_timestamp = lrg_save_context_read_int (context, "first-play-timestamp", 0);

        lrg_save_context_leave_section (context);
    }

    /* Dark arts statistics */
    if (lrg_save_context_enter_section (context, "dark-arts"))
    {
        self->soul_trades_completed = lrg_save_context_read_uint (context, "soul-trades", 0);
        self->dark_investments_owned = lrg_save_context_read_uint (context, "dark-investments", 0);

        lrg_save_context_leave_section (context);
    }

    lp_log_info ("Statistics loaded: %lu total years slumbered, %lu sessions",
                 (gulong)self->total_years_slumbered,
                 (gulong)self->session_count);

    return TRUE;
}

static void
lp_statistics_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_statistics_get_save_id;
    iface->save = lp_statistics_save;
    iface->load = lp_statistics_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_statistics_dispose (GObject *object)
{
    LpStatistics *self = LP_STATISTICS (object);

    g_clear_pointer (&self->lifetime_gold_earned, lrg_big_number_free);
    g_clear_pointer (&self->lifetime_gold_spent, lrg_big_number_free);
    g_clear_pointer (&self->peak_net_worth, lrg_big_number_free);
    g_clear_pointer (&self->total_investment_returns, lrg_big_number_free);
    g_clear_pointer (&self->total_phylactery_points_earned, lrg_big_number_free);
    g_clear_pointer (&self->best_prestige_run, lrg_big_number_free);

    G_OBJECT_CLASS (lp_statistics_parent_class)->dispose (object);
}

static void
lp_statistics_class_init (LpStatisticsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = lp_statistics_dispose;
}

static void
lp_statistics_init (LpStatistics *self)
{
    /* Initialize all BigNumbers to zero */
    self->lifetime_gold_earned = lrg_big_number_new (0.0);
    self->lifetime_gold_spent = lrg_big_number_new (0.0);
    self->peak_net_worth = lrg_big_number_new (0.0);
    self->total_investment_returns = lrg_big_number_new (0.0);
    self->total_phylactery_points_earned = lrg_big_number_new (0.0);
    self->best_prestige_run = lrg_big_number_new (0.0);

    /* All other fields default to zero */
}

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_statistics_get_default:
 *
 * Gets the default statistics instance.
 *
 * Returns: (transfer none): The default #LpStatistics instance
 */
LpStatistics *
lp_statistics_get_default (void)
{
    if (default_statistics == NULL)
    {
        default_statistics = g_object_new (LP_TYPE_STATISTICS, NULL);
        lp_log_info ("Created default LpStatistics instance");
    }

    return default_statistics;
}

/* ==========================================================================
 * Wealth Statistics Getters
 * ========================================================================== */

LrgBigNumber *
lp_statistics_get_lifetime_gold_earned (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), NULL);
    return lrg_big_number_copy (self->lifetime_gold_earned);
}

LrgBigNumber *
lp_statistics_get_lifetime_gold_spent (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), NULL);
    return lrg_big_number_copy (self->lifetime_gold_spent);
}

LrgBigNumber *
lp_statistics_get_peak_net_worth (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), NULL);
    return lrg_big_number_copy (self->peak_net_worth);
}

guint64
lp_statistics_get_peak_net_worth_year (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->peak_net_worth_year;
}

/* ==========================================================================
 * Investment Statistics Getters
 * ========================================================================== */

guint64
lp_statistics_get_investments_purchased (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->investments_purchased;
}

guint64
lp_statistics_get_investments_sold (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->investments_sold;
}

guint64
lp_statistics_get_investments_lost (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->investments_lost;
}

LrgBigNumber *
lp_statistics_get_total_investment_returns (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), NULL);
    return lrg_big_number_copy (self->total_investment_returns);
}

guint
lp_statistics_get_longest_investment_held (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->longest_investment_held;
}

/* ==========================================================================
 * Agent Statistics Getters
 * ========================================================================== */

guint64
lp_statistics_get_agents_recruited (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->agents_recruited;
}

guint64
lp_statistics_get_agent_deaths (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->agent_deaths;
}

guint64
lp_statistics_get_agent_betrayals (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->agent_betrayals;
}

guint
lp_statistics_get_highest_family_generation (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->highest_family_generation;
}

guint64
lp_statistics_get_total_agent_years_served (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->total_agent_years_served;
}

/* ==========================================================================
 * Time Statistics Getters
 * ========================================================================== */

guint64
lp_statistics_get_total_years_slumbered (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->total_years_slumbered;
}

guint
lp_statistics_get_longest_slumber (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->longest_slumber;
}

guint64
lp_statistics_get_total_awakenings (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->total_awakenings;
}

/* ==========================================================================
 * World Statistics Getters
 * ========================================================================== */

guint64
lp_statistics_get_events_witnessed (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->events_witnessed;
}

guint64
lp_statistics_get_kingdoms_collapsed (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->kingdoms_collapsed;
}

guint64
lp_statistics_get_crusades_survived (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->crusades_survived;
}

guint64
lp_statistics_get_competitors_defeated (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->competitors_defeated;
}

/* ==========================================================================
 * Prestige Statistics Getters
 * ========================================================================== */

guint64
lp_statistics_get_prestige_count (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->prestige_count;
}

LrgBigNumber *
lp_statistics_get_total_phylactery_points_earned (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), NULL);
    return lrg_big_number_copy (self->total_phylactery_points_earned);
}

LrgBigNumber *
lp_statistics_get_best_prestige_run (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), NULL);
    return lrg_big_number_copy (self->best_prestige_run);
}

/* ==========================================================================
 * Session Statistics Getters
 * ========================================================================== */

guint64
lp_statistics_get_total_play_time_seconds (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->total_play_time_seconds;
}

guint64
lp_statistics_get_session_count (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->session_count;
}

gint64
lp_statistics_get_first_play_timestamp (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->first_play_timestamp;
}

/* ==========================================================================
 * Dark Arts Statistics Getters
 * ========================================================================== */

guint64
lp_statistics_get_soul_trades_completed (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->soul_trades_completed;
}

guint64
lp_statistics_get_dark_investments_owned (LpStatistics *self)
{
    g_return_val_if_fail (LP_IS_STATISTICS (self), 0);
    return self->dark_investments_owned;
}

/* ==========================================================================
 * Game Event Hooks
 * ========================================================================== */

void
lp_statistics_on_gold_earned (LpStatistics *self,
                              LrgBigNumber *amount)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    g_return_if_fail (amount != NULL);

    lrg_big_number_add_in_place (self->lifetime_gold_earned, amount);
}

void
lp_statistics_on_gold_spent (LpStatistics *self,
                             LrgBigNumber *amount)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    g_return_if_fail (amount != NULL);

    lrg_big_number_add_in_place (self->lifetime_gold_spent, amount);
}

void
lp_statistics_on_net_worth_changed (LpStatistics *self,
                                    LrgBigNumber *net_worth,
                                    guint64       year)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    g_return_if_fail (net_worth != NULL);

    if (lrg_big_number_compare (net_worth, self->peak_net_worth) > 0)
    {
        g_clear_pointer (&self->peak_net_worth, lrg_big_number_free);
        self->peak_net_worth = lrg_big_number_copy (net_worth);
        self->peak_net_worth_year = year;

        lp_log_debug ("New peak net worth: %s in year %lu",
                      lrg_big_number_format_short (net_worth),
                      (gulong)year);
    }
}

void
lp_statistics_on_investment_purchased (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->investments_purchased++;
}

void
lp_statistics_on_investment_sold (LpStatistics *self,
                                  LrgBigNumber *returns)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    self->investments_sold++;

    if (returns != NULL)
        lrg_big_number_add_in_place (self->total_investment_returns, returns);
}

void
lp_statistics_on_investment_lost (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->investments_lost++;
}

void
lp_statistics_on_investment_held (LpStatistics *self,
                                  guint         years)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    if (years > self->longest_investment_held)
        self->longest_investment_held = years;
}

void
lp_statistics_on_agent_recruited (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->agents_recruited++;
}

void
lp_statistics_on_agent_death (LpStatistics *self,
                              guint         years_served)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    self->agent_deaths++;
    self->total_agent_years_served += years_served;
}

void
lp_statistics_on_agent_betrayal (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->agent_betrayals++;
}

void
lp_statistics_on_family_succession (LpStatistics *self,
                                    guint         generation)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    if (generation > self->highest_family_generation)
        self->highest_family_generation = generation;
}

void
lp_statistics_on_slumber_complete (LpStatistics *self,
                                   guint         years)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    self->total_years_slumbered += years;
    self->total_awakenings++;

    if (years > self->longest_slumber)
        self->longest_slumber = years;
}

void
lp_statistics_on_event_witnessed (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->events_witnessed++;
}

void
lp_statistics_on_kingdom_collapsed (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->kingdoms_collapsed++;
}

void
lp_statistics_on_crusade_survived (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->crusades_survived++;
}

void
lp_statistics_on_competitor_defeated (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->competitors_defeated++;
}

void
lp_statistics_on_prestige (LpStatistics *self,
                           LrgBigNumber *points)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    g_return_if_fail (points != NULL);

    self->prestige_count++;
    lrg_big_number_add_in_place (self->total_phylactery_points_earned, points);

    if (lrg_big_number_compare (points, self->best_prestige_run) > 0)
    {
        g_clear_pointer (&self->best_prestige_run, lrg_big_number_free);
        self->best_prestige_run = lrg_big_number_copy (points);
    }
}

void
lp_statistics_on_session_start (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    self->session_count++;

    if (self->first_play_timestamp == 0)
        self->first_play_timestamp = g_get_real_time () / G_USEC_PER_SEC;

    lp_log_debug ("Session %lu started", (gulong)self->session_count);
}

void
lp_statistics_on_session_end (LpStatistics *self,
                              guint64       duration_seconds)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    self->total_play_time_seconds += duration_seconds;

    lp_log_debug ("Session ended after %lu seconds (total: %lu)",
                  (gulong)duration_seconds,
                  (gulong)self->total_play_time_seconds);
}

void
lp_statistics_on_soul_trade (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->soul_trades_completed++;
}

void
lp_statistics_on_dark_investment (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));
    self->dark_investments_owned++;
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

void
lp_statistics_reset (LpStatistics *self)
{
    g_return_if_fail (LP_IS_STATISTICS (self));

    lp_log_debug ("Resetting all statistics");

    /* Reset BigNumbers */
    g_clear_pointer (&self->lifetime_gold_earned, lrg_big_number_free);
    g_clear_pointer (&self->lifetime_gold_spent, lrg_big_number_free);
    g_clear_pointer (&self->peak_net_worth, lrg_big_number_free);
    g_clear_pointer (&self->total_investment_returns, lrg_big_number_free);
    g_clear_pointer (&self->total_phylactery_points_earned, lrg_big_number_free);
    g_clear_pointer (&self->best_prestige_run, lrg_big_number_free);

    self->lifetime_gold_earned = lrg_big_number_new (0.0);
    self->lifetime_gold_spent = lrg_big_number_new (0.0);
    self->peak_net_worth = lrg_big_number_new (0.0);
    self->total_investment_returns = lrg_big_number_new (0.0);
    self->total_phylactery_points_earned = lrg_big_number_new (0.0);
    self->best_prestige_run = lrg_big_number_new (0.0);

    /* Reset all counters */
    self->peak_net_worth_year = 0;
    self->investments_purchased = 0;
    self->investments_sold = 0;
    self->investments_lost = 0;
    self->longest_investment_held = 0;
    self->agents_recruited = 0;
    self->agent_deaths = 0;
    self->agent_betrayals = 0;
    self->highest_family_generation = 0;
    self->total_agent_years_served = 0;
    self->total_years_slumbered = 0;
    self->longest_slumber = 0;
    self->total_awakenings = 0;
    self->events_witnessed = 0;
    self->kingdoms_collapsed = 0;
    self->crusades_survived = 0;
    self->competitors_defeated = 0;
    self->prestige_count = 0;
    self->total_play_time_seconds = 0;
    self->session_count = 0;
    self->first_play_timestamp = 0;
    self->soul_trades_completed = 0;
    self->dark_investments_owned = 0;
}
