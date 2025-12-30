/* test-ledger.c - Ledger Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include <libregnum.h>
#include "core/lp-ledger.h"
#include "lp-enums.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpLedger *ledger;
} LedgerFixture;

static void
fixture_set_up (LedgerFixture *fixture,
                gconstpointer  user_data)
{
    (void)user_data;

    fixture->ledger = lp_ledger_new ();
    g_assert_nonnull (fixture->ledger);
}

static void
fixture_tear_down (LedgerFixture *fixture,
                   gconstpointer  user_data)
{
    (void)user_data;

    g_clear_object (&fixture->ledger);
}

/* ==========================================================================
 * Tests
 * ========================================================================== */

static void
test_ledger_new (LedgerFixture *fixture,
                 gconstpointer  user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_LEDGER (fixture->ledger));
}

static void
test_ledger_saveable_interface (LedgerFixture *fixture,
                                gconstpointer  user_data)
{
    (void)user_data;

    g_assert_true (LRG_IS_SAVEABLE (fixture->ledger));
}

static void
test_ledger_save_id (LedgerFixture *fixture,
                     gconstpointer  user_data)
{
    const gchar *save_id;

    (void)user_data;

    save_id = lrg_saveable_get_save_id (LRG_SAVEABLE (fixture->ledger));
    g_assert_cmpstr (save_id, ==, "ledger");
}

static void
test_ledger_discover_initially_unknown (LedgerFixture *fixture,
                                        gconstpointer  user_data)
{
    gboolean discovered;

    (void)user_data;

    discovered = lp_ledger_has_discovered (fixture->ledger, "test-entry");
    g_assert_false (discovered);
}

static void
test_ledger_discover_record (LedgerFixture *fixture,
                             gconstpointer  user_data)
{
    gboolean discovered;
    gboolean was_new;

    (void)user_data;

    /* Record a discovery */
    was_new = lp_ledger_discover (fixture->ledger,
                                   "property-real-estate",
                                   LP_LEDGER_CATEGORY_ECONOMIC);
    g_assert_true (was_new);

    /* Verify it's now discovered */
    discovered = lp_ledger_has_discovered (fixture->ledger, "property-real-estate");
    g_assert_true (discovered);
}

static void
test_ledger_discover_duplicate (LedgerFixture *fixture,
                                gconstpointer  user_data)
{
    gboolean was_new;

    (void)user_data;

    /* First discovery */
    was_new = lp_ledger_discover (fixture->ledger,
                                   "merchant-family",
                                   LP_LEDGER_CATEGORY_AGENT);
    g_assert_true (was_new);

    /* Same discovery again should return FALSE */
    was_new = lp_ledger_discover (fixture->ledger,
                                   "merchant-family",
                                   LP_LEDGER_CATEGORY_AGENT);
    g_assert_false (was_new);
}

static void
test_ledger_discovery_count (LedgerFixture *fixture,
                             gconstpointer  user_data)
{
    guint count;

    (void)user_data;

    /* Initially empty */
    count = lp_ledger_get_discovered_in_category (fixture->ledger,
                                                   LP_LEDGER_CATEGORY_COMPETITOR);
    g_assert_cmpuint (count, ==, 0);

    /* Add some discoveries */
    lp_ledger_discover (fixture->ledger,
                         "rival-lich",
                         LP_LEDGER_CATEGORY_COMPETITOR);
    lp_ledger_discover (fixture->ledger,
                         "vampire-lord",
                         LP_LEDGER_CATEGORY_COMPETITOR);
    lp_ledger_discover (fixture->ledger,
                         "ancient-dragon",
                         LP_LEDGER_CATEGORY_COMPETITOR);

    count = lp_ledger_get_discovered_in_category (fixture->ledger,
                                                   LP_LEDGER_CATEGORY_COMPETITOR);
    g_assert_cmpuint (count, ==, 3);
}

static void
test_ledger_total_discoveries (LedgerFixture *fixture,
                               gconstpointer  user_data)
{
    guint total;

    (void)user_data;

    /* Add discoveries across categories */
    lp_ledger_discover (fixture->ledger,
                         "bonds",
                         LP_LEDGER_CATEGORY_ECONOMIC);
    lp_ledger_discover (fixture->ledger,
                         "spy",
                         LP_LEDGER_CATEGORY_AGENT);
    lp_ledger_discover (fixture->ledger,
                         "secret-mechanic",
                         LP_LEDGER_CATEGORY_HIDDEN);

    total = lp_ledger_get_discovered_count (fixture->ledger);
    g_assert_cmpuint (total, ==, 3);
}

static void
test_ledger_multiple_categories (LedgerFixture *fixture,
                                 gconstpointer  user_data)
{
    guint econ_count;
    guint agent_count;

    (void)user_data;

    /* Add to different categories */
    lp_ledger_discover (fixture->ledger,
                         "market-cycle",
                         LP_LEDGER_CATEGORY_ECONOMIC);
    lp_ledger_discover (fixture->ledger,
                         "trade-route",
                         LP_LEDGER_CATEGORY_ECONOMIC);
    lp_ledger_discover (fixture->ledger,
                         "agent1",
                         LP_LEDGER_CATEGORY_AGENT);

    /* Verify counts */
    econ_count = lp_ledger_get_discovered_in_category (fixture->ledger,
                                                        LP_LEDGER_CATEGORY_ECONOMIC);
    agent_count = lp_ledger_get_discovered_in_category (fixture->ledger,
                                                         LP_LEDGER_CATEGORY_AGENT);

    g_assert_cmpuint (econ_count, ==, 2);
    g_assert_cmpuint (agent_count, ==, 1);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/ledger/new",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_new, fixture_tear_down);

    g_test_add ("/ledger/saveable-interface",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_saveable_interface, fixture_tear_down);

    g_test_add ("/ledger/save-id",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_save_id, fixture_tear_down);

    g_test_add ("/ledger/discover/initially-unknown",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_discover_initially_unknown, fixture_tear_down);

    g_test_add ("/ledger/discover/record",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_discover_record, fixture_tear_down);

    g_test_add ("/ledger/discover/duplicate",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_discover_duplicate, fixture_tear_down);

    g_test_add ("/ledger/discovery-count",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_discovery_count, fixture_tear_down);

    g_test_add ("/ledger/total-discoveries",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_total_discoveries, fixture_tear_down);

    g_test_add ("/ledger/multiple-categories",
                LedgerFixture, NULL,
                fixture_set_up, test_ledger_multiple_categories, fixture_tear_down);

    return g_test_run ();
}
