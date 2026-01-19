/* test-help.c - Tests for Tooltip and Help System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include <libregnum.h>
#include "../src/ui/lp-tooltip.h"
#include "../src/ui/lp-help-system.h"

/* ==========================================================================
 * LpTooltip Tests
 * ========================================================================== */

typedef struct
{
    LpTooltip *tooltip;
} TooltipFixture;

static void
tooltip_fixture_setup (TooltipFixture *fixture,
                       gconstpointer   user_data)
{
    fixture->tooltip = lp_tooltip_new ();
}

static void
tooltip_fixture_teardown (TooltipFixture *fixture,
                          gconstpointer   user_data)
{
    g_clear_object (&fixture->tooltip);
}

static void
test_tooltip_new (TooltipFixture *fixture,
                  gconstpointer   user_data)
{
    g_assert_nonnull (fixture->tooltip);
    g_assert_true (LP_IS_TOOLTIP (fixture->tooltip));
}

static void
test_tooltip_title (TooltipFixture *fixture,
                    gconstpointer   user_data)
{
    /* Initially NULL */
    g_assert_null (lp_tooltip_get_title (fixture->tooltip));

    /* Set and verify */
    lp_tooltip_set_title (fixture->tooltip, "Test Title");
    g_assert_cmpstr (lp_tooltip_get_title (fixture->tooltip), ==, "Test Title");

    /* Update */
    lp_tooltip_set_title (fixture->tooltip, "New Title");
    g_assert_cmpstr (lp_tooltip_get_title (fixture->tooltip), ==, "New Title");

    /* Clear with NULL */
    lp_tooltip_set_title (fixture->tooltip, NULL);
    g_assert_null (lp_tooltip_get_title (fixture->tooltip));
}

static void
test_tooltip_text (TooltipFixture *fixture,
                   gconstpointer   user_data)
{
    g_assert_null (lp_tooltip_get_text (fixture->tooltip));

    lp_tooltip_set_text (fixture->tooltip, "Description text here.");
    g_assert_cmpstr (lp_tooltip_get_text (fixture->tooltip), ==, "Description text here.");
}

static void
test_tooltip_hint (TooltipFixture *fixture,
                   gconstpointer   user_data)
{
    g_assert_null (lp_tooltip_get_hint (fixture->tooltip));

    lp_tooltip_set_hint (fixture->tooltip, "Pro tip: do the thing.");
    g_assert_cmpstr (lp_tooltip_get_hint (fixture->tooltip), ==, "Pro tip: do the thing.");
}

static void
test_tooltip_visibility (TooltipFixture *fixture,
                         gconstpointer   user_data)
{
    /* Initially hidden */
    g_assert_false (lp_tooltip_is_visible (fixture->tooltip));

    /* Show at position */
    lp_tooltip_show_at (fixture->tooltip, 100.0f, 200.0f, LP_TOOLTIP_POSITION_AUTO);
    g_assert_true (lp_tooltip_is_visible (fixture->tooltip));

    /* Hide */
    lp_tooltip_hide (fixture->tooltip);
    g_assert_false (lp_tooltip_is_visible (fixture->tooltip));
}

static void
test_tooltip_position_modes (TooltipFixture *fixture,
                             gconstpointer   user_data)
{
    /* Test all position modes don't crash */
    lp_tooltip_show_at (fixture->tooltip, 100.0f, 100.0f, LP_TOOLTIP_POSITION_AUTO);
    g_assert_true (lp_tooltip_is_visible (fixture->tooltip));

    lp_tooltip_show_at (fixture->tooltip, 100.0f, 100.0f, LP_TOOLTIP_POSITION_ABOVE);
    g_assert_true (lp_tooltip_is_visible (fixture->tooltip));

    lp_tooltip_show_at (fixture->tooltip, 100.0f, 100.0f, LP_TOOLTIP_POSITION_BELOW);
    g_assert_true (lp_tooltip_is_visible (fixture->tooltip));

    lp_tooltip_show_at (fixture->tooltip, 100.0f, 100.0f, LP_TOOLTIP_POSITION_LEFT);
    g_assert_true (lp_tooltip_is_visible (fixture->tooltip));

    lp_tooltip_show_at (fixture->tooltip, 100.0f, 100.0f, LP_TOOLTIP_POSITION_RIGHT);
    g_assert_true (lp_tooltip_is_visible (fixture->tooltip));
}

static void
test_tooltip_max_width (TooltipFixture *fixture,
                        gconstpointer   user_data)
{
    /* Set max width - should not crash */
    lp_tooltip_set_max_width (fixture->tooltip, 400.0f);
    lp_tooltip_set_max_width (fixture->tooltip, 0.0f);  /* Disable */
}

static void
test_tooltip_delay (TooltipFixture *fixture,
                    gconstpointer   user_data)
{
    /* Set delay - should not crash */
    lp_tooltip_set_delay (fixture->tooltip, 1000);
    lp_tooltip_set_delay (fixture->tooltip, 0);
}

/* ==========================================================================
 * LpHelpSystem Tests
 * ========================================================================== */

typedef struct
{
    LpHelpSystem *help;
} HelpSystemFixture;

static void
help_system_fixture_setup (HelpSystemFixture *fixture,
                           gconstpointer      user_data)
{
    fixture->help = lp_help_system_get_default ();
}

static void
help_system_fixture_teardown (HelpSystemFixture *fixture,
                              gconstpointer      user_data)
{
    /* Singleton - don't unref */
}

static void
test_help_system_singleton (HelpSystemFixture *fixture,
                            gconstpointer      user_data)
{
    LpHelpSystem *help2;

    g_assert_nonnull (fixture->help);
    g_assert_true (LP_IS_HELP_SYSTEM (fixture->help));

    /* Verify singleton behavior */
    help2 = lp_help_system_get_default ();
    g_assert_true (fixture->help == help2);
}

static void
test_help_system_load (HelpSystemFixture *fixture,
                       gconstpointer      user_data)
{
    g_autoptr(GError) error = NULL;
    gboolean success;

    /* Loading should succeed (or skip if no files) */
    success = lp_help_system_load (fixture->help, &error);
    g_assert_true (success);
    g_assert_no_error (error);

    /* Loading again should return immediately */
    success = lp_help_system_load (fixture->help, &error);
    g_assert_true (success);
}

static void
test_help_system_get_entry_null (HelpSystemFixture *fixture,
                                 gconstpointer      user_data)
{
    const LpHelpEntry *entry;

    /* Non-existent entry should return NULL */
    entry = lp_help_system_get_entry (fixture->help, "nonexistent_entry_id");
    g_assert_null (entry);
}

static void
test_help_system_get_categories (HelpSystemFixture *fixture,
                                 gconstpointer      user_data)
{
    GList *categories;

    /* Should return a list (possibly empty) */
    categories = lp_help_system_get_categories (fixture->help);
    /* List can be NULL if no entries loaded */
    g_list_free (categories);
}

static void
test_help_system_convenience_functions (HelpSystemFixture *fixture,
                                        gconstpointer      user_data)
{
    const gchar *title;
    const gchar *desc;
    const LpHelpEntry *entry;

    /* Non-existent entry should return NULL */
    title = lp_help_title ("nonexistent");
    g_assert_null (title);

    desc = lp_help_desc ("nonexistent");
    g_assert_null (desc);

    entry = lp_help_get ("nonexistent");
    g_assert_null (entry);
}

/* ==========================================================================
 * LpHelpEntry Accessor Tests
 * ========================================================================== */

static void
test_help_entry_accessors_null (void)
{
    /*
     * Test that accessor functions properly return NULL for NULL input
     * while logging the expected critical warning.
     */

    /* Expect the critical messages from g_return_val_if_fail */
    g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                           "*assertion*entry*!=*NULL*failed*");
    g_assert_null (lp_help_entry_get_id (NULL));

    g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                           "*assertion*entry*!=*NULL*failed*");
    g_assert_null (lp_help_entry_get_title (NULL));

    g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                           "*assertion*entry*!=*NULL*failed*");
    g_assert_null (lp_help_entry_get_description (NULL));

    g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                           "*assertion*entry*!=*NULL*failed*");
    g_assert_null (lp_help_entry_get_hint (NULL));

    g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                           "*assertion*entry*!=*NULL*failed*");
    g_assert_null (lp_help_entry_get_category (NULL));
}

/* ==========================================================================
 * Main
 * ========================================================================== */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Tooltip tests */
    g_test_add ("/tooltip/new",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_new,
                tooltip_fixture_teardown);

    g_test_add ("/tooltip/title",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_title,
                tooltip_fixture_teardown);

    g_test_add ("/tooltip/text",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_text,
                tooltip_fixture_teardown);

    g_test_add ("/tooltip/hint",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_hint,
                tooltip_fixture_teardown);

    g_test_add ("/tooltip/visibility",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_visibility,
                tooltip_fixture_teardown);

    g_test_add ("/tooltip/position-modes",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_position_modes,
                tooltip_fixture_teardown);

    g_test_add ("/tooltip/max-width",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_max_width,
                tooltip_fixture_teardown);

    g_test_add ("/tooltip/delay",
                TooltipFixture, NULL,
                tooltip_fixture_setup,
                test_tooltip_delay,
                tooltip_fixture_teardown);

    /* Help system tests */
    g_test_add ("/help-system/singleton",
                HelpSystemFixture, NULL,
                help_system_fixture_setup,
                test_help_system_singleton,
                help_system_fixture_teardown);

    g_test_add ("/help-system/load",
                HelpSystemFixture, NULL,
                help_system_fixture_setup,
                test_help_system_load,
                help_system_fixture_teardown);

    g_test_add ("/help-system/get-entry-null",
                HelpSystemFixture, NULL,
                help_system_fixture_setup,
                test_help_system_get_entry_null,
                help_system_fixture_teardown);

    g_test_add ("/help-system/get-categories",
                HelpSystemFixture, NULL,
                help_system_fixture_setup,
                test_help_system_get_categories,
                help_system_fixture_teardown);

    g_test_add ("/help-system/convenience-functions",
                HelpSystemFixture, NULL,
                help_system_fixture_setup,
                test_help_system_convenience_functions,
                help_system_fixture_teardown);

    /* Help entry accessor tests */
    g_test_add_func ("/help-entry/accessors-null",
                     test_help_entry_accessors_null);

    return g_test_run ();
}
