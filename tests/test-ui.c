/* test-ui.c - Phase 6 UI Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for UI components: theme, widgets, screens, and dialogs.
 * Many tests skip if no display is available (headless environment).
 */

#include <glib.h>
#include <libregnum.h>

#include "ui/lp-theme.h"
#include "ui/lp-widget-exposure-meter.h"
#include "ui/lp-widget-synergy-indicator.h"
#include "ui/lp-screen-portfolio.h"
#include "ui/lp-screen-world-map.h"
#include "ui/lp-screen-agents.h"
#include "ui/lp-screen-intelligence.h"
#include "ui/lp-screen-slumber.h"
#include "ui/lp-screen-ledger.h"
#include "ui/lp-screen-megaprojects.h"
#include "ui/lp-dialog-event.h"
#include "core/lp-exposure-manager.h"
#include "core/lp-synergy-manager.h"

/* ==========================================================================
 * Theme Tests
 * ========================================================================== */

static void
test_theme_configure_default (void)
{
    LrgTheme *theme;

    /* Configure the theme */
    lp_theme_configure_default ();

    /* Verify theme is accessible */
    theme = lrg_theme_get_default ();
    g_assert_nonnull (theme);
}

static void
test_theme_custom_colors (void)
{
    const GrlColor *color;

    lp_theme_configure_default ();

    /* Test gold color */
    color = lp_theme_get_gold_color ();
    g_assert_nonnull (color);

    /* Test danger color */
    color = lp_theme_get_danger_color ();
    g_assert_nonnull (color);

    /* Test hidden color */
    color = lp_theme_get_hidden_color ();
    g_assert_nonnull (color);

    /* Test scrutiny color */
    color = lp_theme_get_scrutiny_color ();
    g_assert_nonnull (color);

    /* Test suspicion color */
    color = lp_theme_get_suspicion_color ();
    g_assert_nonnull (color);

    /* Test hunt color */
    color = lp_theme_get_hunt_color ();
    g_assert_nonnull (color);

    /* Test crusade color */
    color = lp_theme_get_crusade_color ();
    g_assert_nonnull (color);

    /* Test synergy color */
    color = lp_theme_get_synergy_color ();
    g_assert_nonnull (color);

    /* Test inactive color */
    color = lp_theme_get_inactive_color ();
    g_assert_nonnull (color);
}

/* ==========================================================================
 * Exposure Meter Tests
 * ========================================================================== */

static void
test_exposure_meter_new (void)
{
    g_autoptr(LpWidgetExposureMeter) meter = NULL;

    meter = lp_widget_exposure_meter_new ();
    g_assert_nonnull (meter);
    g_assert_true (LP_IS_WIDGET_EXPOSURE_METER (meter));
}

static void
test_exposure_meter_value (void)
{
    g_autoptr(LpWidgetExposureMeter) meter = NULL;

    meter = lp_widget_exposure_meter_new ();

    /* Test initial value */
    g_assert_cmpuint (lp_widget_exposure_meter_get_value (meter), ==, 0);

    /* Test setting value */
    lp_widget_exposure_meter_set_value (meter, 50);
    g_assert_cmpuint (lp_widget_exposure_meter_get_value (meter), ==, 50);

    /* Test clamping to max */
    lp_widget_exposure_meter_set_value (meter, 150);
    g_assert_cmpuint (lp_widget_exposure_meter_get_value (meter), ==, 100);
}

static void
test_exposure_meter_level (void)
{
    g_autoptr(LpWidgetExposureMeter) meter = NULL;

    meter = lp_widget_exposure_meter_new ();

    /* Test Hidden level (0-24) */
    lp_widget_exposure_meter_set_value (meter, 0);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_HIDDEN);

    lp_widget_exposure_meter_set_value (meter, 24);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_HIDDEN);

    /* Test Scrutiny level (25-49) */
    lp_widget_exposure_meter_set_value (meter, 25);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_SCRUTINY);

    lp_widget_exposure_meter_set_value (meter, 49);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_SCRUTINY);

    /* Test Suspicion level (50-74) */
    lp_widget_exposure_meter_set_value (meter, 50);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_SUSPICION);

    lp_widget_exposure_meter_set_value (meter, 74);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_SUSPICION);

    /* Test Hunt level (75-99) */
    lp_widget_exposure_meter_set_value (meter, 75);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_HUNT);

    lp_widget_exposure_meter_set_value (meter, 99);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_HUNT);

    /* Test Crusade level (100) */
    lp_widget_exposure_meter_set_value (meter, 100);
    g_assert_cmpint (lp_widget_exposure_meter_get_level (meter), ==, LP_EXPOSURE_LEVEL_CRUSADE);
}

static void
test_exposure_meter_options (void)
{
    g_autoptr(LpWidgetExposureMeter) meter = NULL;

    meter = lp_widget_exposure_meter_new ();

    /* Test show label */
    g_assert_true (lp_widget_exposure_meter_get_show_label (meter));  /* Default TRUE */
    lp_widget_exposure_meter_set_show_label (meter, FALSE);
    g_assert_false (lp_widget_exposure_meter_get_show_label (meter));

    /* Test show percentage */
    g_assert_true (lp_widget_exposure_meter_get_show_percentage (meter));  /* Default TRUE */
    lp_widget_exposure_meter_set_show_percentage (meter, FALSE);
    g_assert_false (lp_widget_exposure_meter_get_show_percentage (meter));

    /* Test orientation */
    g_assert_cmpint (lp_widget_exposure_meter_get_orientation (meter), ==,
                     LRG_ORIENTATION_HORIZONTAL);  /* Default */
    lp_widget_exposure_meter_set_orientation (meter, LRG_ORIENTATION_VERTICAL);
    g_assert_cmpint (lp_widget_exposure_meter_get_orientation (meter), ==,
                     LRG_ORIENTATION_VERTICAL);
}

/* ==========================================================================
 * Synergy Indicator Tests
 * ========================================================================== */

static void
test_synergy_indicator_new (void)
{
    g_autoptr(LpWidgetSynergyIndicator) indicator = NULL;

    indicator = lp_widget_synergy_indicator_new ();
    g_assert_nonnull (indicator);
    g_assert_true (LP_IS_WIDGET_SYNERGY_INDICATOR (indicator));
}

static void
test_synergy_indicator_values (void)
{
    g_autoptr(LpWidgetSynergyIndicator) indicator = NULL;

    indicator = lp_widget_synergy_indicator_new ();

    /* Test initial values (from skeleton manager) */
    g_assert_cmpuint (lp_widget_synergy_indicator_get_synergy_count (indicator), ==, 0);
    g_assert_cmpfloat (lp_widget_synergy_indicator_get_total_bonus (indicator), ==, 1.0);
}

static void
test_synergy_indicator_options (void)
{
    g_autoptr(LpWidgetSynergyIndicator) indicator = NULL;

    indicator = lp_widget_synergy_indicator_new ();

    /* Test show details */
    g_assert_false (lp_widget_synergy_indicator_get_show_details (indicator));
    lp_widget_synergy_indicator_set_show_details (indicator, TRUE);
    g_assert_true (lp_widget_synergy_indicator_get_show_details (indicator));

    /* Test compact mode */
    g_assert_false (lp_widget_synergy_indicator_get_compact (indicator));
    lp_widget_synergy_indicator_set_compact (indicator, TRUE);
    g_assert_true (lp_widget_synergy_indicator_get_compact (indicator));
}

/* ==========================================================================
 * Screen Tests
 * ========================================================================== */

static void
test_screen_portfolio_new (void)
{
    g_autoptr(LpScreenPortfolio) screen = NULL;

    screen = lp_screen_portfolio_new ();
    g_assert_nonnull (screen);
    g_assert_true (LP_IS_SCREEN_PORTFOLIO (screen));
}

static void
test_screen_portfolio_view_mode (void)
{
    g_autoptr(LpScreenPortfolio) screen = NULL;

    screen = lp_screen_portfolio_new ();

    /* Test default view mode */
    g_assert_cmpint (lp_screen_portfolio_get_view_mode (screen), ==,
                     LP_PORTFOLIO_VIEW_LIST);

    /* Test setting view mode */
    lp_screen_portfolio_set_view_mode (screen, LP_PORTFOLIO_VIEW_ALLOCATION);
    g_assert_cmpint (lp_screen_portfolio_get_view_mode (screen), ==,
                     LP_PORTFOLIO_VIEW_ALLOCATION);

    lp_screen_portfolio_set_view_mode (screen, LP_PORTFOLIO_VIEW_PERFORMANCE);
    g_assert_cmpint (lp_screen_portfolio_get_view_mode (screen), ==,
                     LP_PORTFOLIO_VIEW_PERFORMANCE);
}

static void
test_screen_world_map_new (void)
{
    g_autoptr(LpScreenWorldMap) screen = NULL;

    screen = lp_screen_world_map_new ();
    g_assert_nonnull (screen);
    g_assert_true (LP_IS_SCREEN_WORLD_MAP (screen));
}

static void
test_screen_agents_new (void)
{
    g_autoptr(LpScreenAgents) screen = NULL;

    screen = lp_screen_agents_new ();
    g_assert_nonnull (screen);
    g_assert_true (LP_IS_SCREEN_AGENTS (screen));
}

static void
test_screen_intelligence_new (void)
{
    g_autoptr(LpScreenIntelligence) screen = NULL;

    screen = lp_screen_intelligence_new ();
    g_assert_nonnull (screen);
    g_assert_true (LP_IS_SCREEN_INTELLIGENCE (screen));
}

static void
test_screen_slumber_new (void)
{
    g_autoptr(LpScreenSlumber) screen = NULL;

    screen = lp_screen_slumber_new ();
    g_assert_nonnull (screen);
    g_assert_true (LP_IS_SCREEN_SLUMBER (screen));
}

static void
test_screen_slumber_duration (void)
{
    g_autoptr(LpScreenSlumber) screen = NULL;

    screen = lp_screen_slumber_new ();

    /* Test default duration (25 years minimum) */
    g_assert_cmpuint (lp_screen_slumber_get_duration (screen), ==, 25);

    /* Test setting duration */
    lp_screen_slumber_set_duration (screen, 100);
    g_assert_cmpuint (lp_screen_slumber_get_duration (screen), ==, 100);

    /* Test minimum clamping */
    lp_screen_slumber_set_duration (screen, 10);  /* Below minimum */
    g_assert_cmpuint (lp_screen_slumber_get_duration (screen), >=, 25);
}

static void
test_screen_ledger_new (void)
{
    g_autoptr(LpScreenLedger) screen = NULL;

    screen = lp_screen_ledger_new ();
    g_assert_nonnull (screen);
    g_assert_true (LP_IS_SCREEN_LEDGER (screen));
}

static void
test_screen_megaprojects_new (void)
{
    g_autoptr(LpScreenMegaprojects) screen = NULL;

    screen = lp_screen_megaprojects_new ();
    g_assert_nonnull (screen);
    g_assert_true (LP_IS_SCREEN_MEGAPROJECTS (screen));
}

/* ==========================================================================
 * Dialog Tests
 * ========================================================================== */

static void
test_dialog_event_new (void)
{
    g_autoptr(LpDialogEvent) dialog = NULL;

    dialog = lp_dialog_event_new ();
    g_assert_nonnull (dialog);
    g_assert_true (LP_IS_DIALOG_EVENT (dialog));
}

static void
test_dialog_event_choice (void)
{
    g_autoptr(LpDialogEvent) dialog = NULL;

    dialog = lp_dialog_event_new ();

    /* Test initial choice */
    g_assert_cmpint (lp_dialog_event_get_selected_choice (dialog), ==, 0);

    /* Without an event, selecting choices does nothing significant */
    lp_dialog_event_select_choice (dialog, 2);
    /* Choice won't change without valid event with choices */
}

/* ==========================================================================
 * Main Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Initialize theme for all tests */
    lp_theme_configure_default ();

    /* Theme tests */
    g_test_add_func ("/ui/theme/configure-default", test_theme_configure_default);
    g_test_add_func ("/ui/theme/custom-colors", test_theme_custom_colors);

    /* Exposure meter tests */
    g_test_add_func ("/ui/exposure-meter/new", test_exposure_meter_new);
    g_test_add_func ("/ui/exposure-meter/value", test_exposure_meter_value);
    g_test_add_func ("/ui/exposure-meter/level", test_exposure_meter_level);
    g_test_add_func ("/ui/exposure-meter/options", test_exposure_meter_options);

    /* Synergy indicator tests */
    g_test_add_func ("/ui/synergy-indicator/new", test_synergy_indicator_new);
    g_test_add_func ("/ui/synergy-indicator/values", test_synergy_indicator_values);
    g_test_add_func ("/ui/synergy-indicator/options", test_synergy_indicator_options);

    /* Screen tests */
    g_test_add_func ("/ui/screen/portfolio/new", test_screen_portfolio_new);
    g_test_add_func ("/ui/screen/portfolio/view-mode", test_screen_portfolio_view_mode);
    g_test_add_func ("/ui/screen/world-map/new", test_screen_world_map_new);
    g_test_add_func ("/ui/screen/agents/new", test_screen_agents_new);
    g_test_add_func ("/ui/screen/intelligence/new", test_screen_intelligence_new);
    g_test_add_func ("/ui/screen/slumber/new", test_screen_slumber_new);
    g_test_add_func ("/ui/screen/slumber/duration", test_screen_slumber_duration);
    g_test_add_func ("/ui/screen/ledger/new", test_screen_ledger_new);
    g_test_add_func ("/ui/screen/megaprojects/new", test_screen_megaprojects_new);

    /* Dialog tests */
    g_test_add_func ("/ui/dialog/event/new", test_dialog_event_new);
    g_test_add_func ("/ui/dialog/event/choice", test_dialog_event_choice);

    return g_test_run ();
}
