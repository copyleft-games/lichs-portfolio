/* test-chronicle.c - Tests for Event Chronicle System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include <libregnum.h>
#include "../src/core/lp-event-chronicle.h"
#include "../src/simulation/lp-event.h"
#include "../src/lp-enums.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpEventChronicle *chronicle;
} ChronicleFixture;

static void
chronicle_fixture_setup (ChronicleFixture *fixture,
                         gconstpointer     user_data)
{
    fixture->chronicle = lp_event_chronicle_get_default ();
    /* Reset to clean state for each test */
    lp_event_chronicle_reset (fixture->chronicle);
}

static void
chronicle_fixture_teardown (ChronicleFixture *fixture,
                            gconstpointer     user_data)
{
    lp_event_chronicle_reset (fixture->chronicle);
    /* Singleton - don't unref */
}

/* Helper to create a test event */
static LpEvent *
create_test_event (const gchar     *id,
                   const gchar     *name,
                   LpEventType      type,
                   LpEventSeverity  severity,
                   guint64          year)
{
    LpEvent *event;

    event = lp_event_new (id, name, type);
    lp_event_set_severity (event, severity);
    lp_event_set_year_occurred (event, year);
    lp_event_set_description (event, "Test event description");

    return event;
}

/* ==========================================================================
 * LpChronicleEntry Tests
 * ========================================================================== */

static void
test_chronicle_entry_new (void)
{
    g_autoptr(LpEvent) event = NULL;
    LpChronicleEntry *entry;

    event = create_test_event ("test-event", "Test Event",
                               LP_EVENT_TYPE_ECONOMIC,
                               LP_EVENT_SEVERITY_MODERATE,
                               850);

    entry = lp_chronicle_entry_new (event, 852);

    g_assert_nonnull (entry);
    g_assert_cmpstr (entry->event_id, ==, "test-event");
    g_assert_cmpstr (entry->event_name, ==, "Test Event");
    g_assert_cmpint (entry->event_type, ==, LP_EVENT_TYPE_ECONOMIC);
    g_assert_cmpint (entry->severity, ==, LP_EVENT_SEVERITY_MODERATE);
    g_assert_cmpuint (entry->year_occurred, ==, 850);
    g_assert_cmpuint (entry->year_resolved, ==, 852);
    g_assert_cmpstr (entry->description, ==, "Test event description");

    lp_chronicle_entry_free (entry);
}

static void
test_chronicle_entry_copy (void)
{
    g_autoptr(LpEvent) event = NULL;
    LpChronicleEntry *entry;
    LpChronicleEntry *copy;

    event = create_test_event ("copy-test", "Copy Test",
                               LP_EVENT_TYPE_POLITICAL,
                               LP_EVENT_SEVERITY_MAJOR,
                               900);

    entry = lp_chronicle_entry_new (event, 905);
    entry->outcome = g_strdup ("The kingdom fell");
    entry->gold_impact = -5000;
    entry->exposure_impact = 15.0f;

    copy = lp_chronicle_entry_copy (entry);

    g_assert_nonnull (copy);
    g_assert_cmpstr (copy->event_id, ==, "copy-test");
    g_assert_cmpstr (copy->event_name, ==, "Copy Test");
    g_assert_cmpint (copy->event_type, ==, LP_EVENT_TYPE_POLITICAL);
    g_assert_cmpuint (copy->year_occurred, ==, 900);
    g_assert_cmpuint (copy->year_resolved, ==, 905);
    g_assert_cmpstr (copy->outcome, ==, "The kingdom fell");
    g_assert_cmpint (copy->gold_impact, ==, -5000);
    g_assert_cmpfloat_with_epsilon (copy->exposure_impact, 15.0f, 0.01f);

    /* Verify they are separate copies */
    g_assert_true (copy != entry);
    g_assert_true (copy->event_id != entry->event_id);

    lp_chronicle_entry_free (entry);
    lp_chronicle_entry_free (copy);
}

static void
test_chronicle_entry_free_null (void)
{
    /* Should handle NULL gracefully */
    lp_chronicle_entry_free (NULL);
}

static void
test_chronicle_entry_copy_null (void)
{
    LpChronicleEntry *copy;

    copy = lp_chronicle_entry_copy (NULL);
    g_assert_null (copy);
}

/* ==========================================================================
 * LpEventChronicle Tests
 * ========================================================================== */

static void
test_chronicle_singleton (ChronicleFixture *fixture,
                          gconstpointer     user_data)
{
    LpEventChronicle *chronicle2;

    g_assert_nonnull (fixture->chronicle);
    g_assert_true (LP_IS_EVENT_CHRONICLE (fixture->chronicle));

    /* Verify singleton behavior */
    chronicle2 = lp_event_chronicle_get_default ();
    g_assert_true (fixture->chronicle == chronicle2);
}

static void
test_chronicle_record (ChronicleFixture *fixture,
                       gconstpointer     user_data)
{
    g_autoptr(LpEvent) event = NULL;

    g_assert_cmpuint (lp_event_chronicle_get_count (fixture->chronicle), ==, 0);

    event = create_test_event ("record-test", "Record Test",
                               LP_EVENT_TYPE_ECONOMIC,
                               LP_EVENT_SEVERITY_MINOR,
                               847);

    lp_event_chronicle_record (fixture->chronicle, event, 847,
                               "Markets stabilized", 1000, 0.5f);

    g_assert_cmpuint (lp_event_chronicle_get_count (fixture->chronicle), ==, 1);
}

static void
test_chronicle_record_with_choice (ChronicleFixture *fixture,
                                   gconstpointer     user_data)
{
    g_autoptr(LpEvent) event = NULL;
    GPtrArray *all;
    LpChronicleEntry *entry;

    event = create_test_event ("choice-test", "Choice Test",
                               LP_EVENT_TYPE_PERSONAL,
                               LP_EVENT_SEVERITY_MAJOR,
                               900);

    lp_event_chronicle_record_with_choice (fixture->chronicle, event, 901,
                                           "bribe_official",
                                           "The official looked the other way",
                                           -500, 2.0f);

    g_assert_cmpuint (lp_event_chronicle_get_count (fixture->chronicle), ==, 1);

    all = lp_event_chronicle_get_all (fixture->chronicle);
    entry = g_ptr_array_index (all, 0);

    g_assert_cmpstr (entry->player_choice, ==, "bribe_official");
    g_assert_cmpstr (entry->outcome, ==, "The official looked the other way");
    g_assert_cmpint (entry->gold_impact, ==, -500);
}

static void
test_chronicle_get_all (ChronicleFixture *fixture,
                        gconstpointer     user_data)
{
    g_autoptr(LpEvent) event1 = NULL;
    g_autoptr(LpEvent) event2 = NULL;
    g_autoptr(LpEvent) event3 = NULL;
    GPtrArray *all;

    event1 = create_test_event ("e1", "Event 1", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 850);
    event2 = create_test_event ("e2", "Event 2", LP_EVENT_TYPE_POLITICAL, LP_EVENT_SEVERITY_MODERATE, 860);
    event3 = create_test_event ("e3", "Event 3", LP_EVENT_TYPE_MAGICAL, LP_EVENT_SEVERITY_MAJOR, 870);

    lp_event_chronicle_record (fixture->chronicle, event1, 850, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event2, 862, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event3, 875, NULL, 0, 0);

    all = lp_event_chronicle_get_all (fixture->chronicle);
    g_assert_nonnull (all);
    g_assert_cmpuint (all->len, ==, 3);

    /* Most recent should be first */
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (all, 0))->event_id, ==, "e3");
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (all, 1))->event_id, ==, "e2");
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (all, 2))->event_id, ==, "e1");
}

static void
test_chronicle_get_by_type (ChronicleFixture *fixture,
                            gconstpointer     user_data)
{
    g_autoptr(LpEvent) event1 = NULL;
    g_autoptr(LpEvent) event2 = NULL;
    g_autoptr(LpEvent) event3 = NULL;
    g_autoptr(GPtrArray) economic = NULL;
    g_autoptr(GPtrArray) political = NULL;

    event1 = create_test_event ("eco1", "Economic 1", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 850);
    event2 = create_test_event ("pol1", "Political 1", LP_EVENT_TYPE_POLITICAL, LP_EVENT_SEVERITY_MODERATE, 860);
    event3 = create_test_event ("eco2", "Economic 2", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MAJOR, 870);

    lp_event_chronicle_record (fixture->chronicle, event1, 850, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event2, 862, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event3, 875, NULL, 0, 0);

    economic = lp_event_chronicle_get_by_type (fixture->chronicle, LP_EVENT_TYPE_ECONOMIC);
    g_assert_cmpuint (economic->len, ==, 2);

    political = lp_event_chronicle_get_by_type (fixture->chronicle, LP_EVENT_TYPE_POLITICAL);
    g_assert_cmpuint (political->len, ==, 1);
}

static void
test_chronicle_get_by_year_range (ChronicleFixture *fixture,
                                  gconstpointer     user_data)
{
    g_autoptr(LpEvent) event1 = NULL;
    g_autoptr(LpEvent) event2 = NULL;
    g_autoptr(LpEvent) event3 = NULL;
    g_autoptr(GPtrArray) range = NULL;

    event1 = create_test_event ("y850", "Year 850", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 850);
    event2 = create_test_event ("y900", "Year 900", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 900);
    event3 = create_test_event ("y950", "Year 950", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 950);

    lp_event_chronicle_record (fixture->chronicle, event1, 850, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event2, 900, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event3, 950, NULL, 0, 0);

    range = lp_event_chronicle_get_by_year_range (fixture->chronicle, 860, 920);
    g_assert_cmpuint (range->len, ==, 1);
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (range, 0))->event_id, ==, "y900");
}

static void
test_chronicle_get_by_kingdom (ChronicleFixture *fixture,
                               gconstpointer     user_data)
{
    g_autoptr(LpEvent) event1 = NULL;
    g_autoptr(LpEvent) event2 = NULL;
    g_autoptr(GPtrArray) verania = NULL;

    event1 = create_test_event ("ver1", "Verania Event", LP_EVENT_TYPE_POLITICAL, LP_EVENT_SEVERITY_MINOR, 850);
    lp_event_set_affects_kingdom_id (event1, "verania");

    event2 = create_test_event ("kha1", "Khadriel Event", LP_EVENT_TYPE_POLITICAL, LP_EVENT_SEVERITY_MINOR, 860);
    lp_event_set_affects_kingdom_id (event2, "khadriel");

    lp_event_chronicle_record (fixture->chronicle, event1, 850, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event2, 860, NULL, 0, 0);

    verania = lp_event_chronicle_get_by_kingdom (fixture->chronicle, "verania");
    g_assert_cmpuint (verania->len, ==, 1);
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (verania, 0))->event_id, ==, "ver1");
}

static void
test_chronicle_get_by_severity (ChronicleFixture *fixture,
                                gconstpointer     user_data)
{
    g_autoptr(LpEvent) event1 = NULL;
    g_autoptr(LpEvent) event2 = NULL;
    g_autoptr(LpEvent) event3 = NULL;
    g_autoptr(GPtrArray) major_plus = NULL;

    event1 = create_test_event ("minor", "Minor", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 850);
    event2 = create_test_event ("moderate", "Moderate", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MODERATE, 860);
    event3 = create_test_event ("major", "Major", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MAJOR, 870);

    lp_event_chronicle_record (fixture->chronicle, event1, 850, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event2, 860, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event3, 870, NULL, 0, 0);

    major_plus = lp_event_chronicle_get_by_severity (fixture->chronicle, LP_EVENT_SEVERITY_MAJOR);
    g_assert_cmpuint (major_plus->len, ==, 1);
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (major_plus, 0))->event_id, ==, "major");
}

static void
test_chronicle_get_recent (ChronicleFixture *fixture,
                           gconstpointer     user_data)
{
    g_autoptr(LpEvent) event1 = NULL;
    g_autoptr(LpEvent) event2 = NULL;
    g_autoptr(LpEvent) event3 = NULL;
    g_autoptr(GPtrArray) recent = NULL;

    event1 = create_test_event ("old", "Old", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 800);
    event2 = create_test_event ("mid", "Mid", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 850);
    event3 = create_test_event ("new", "New", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 900);

    lp_event_chronicle_record (fixture->chronicle, event1, 800, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event2, 850, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event3, 900, NULL, 0, 0);

    recent = lp_event_chronicle_get_recent (fixture->chronicle, 2);
    g_assert_cmpuint (recent->len, ==, 2);

    /* Most recent first */
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (recent, 0))->event_id, ==, "new");
    g_assert_cmpstr (((LpChronicleEntry *)g_ptr_array_index (recent, 1))->event_id, ==, "mid");
}

static void
test_chronicle_count_by_type (ChronicleFixture *fixture,
                              gconstpointer     user_data)
{
    g_autoptr(LpEvent) event1 = NULL;
    g_autoptr(LpEvent) event2 = NULL;
    g_autoptr(LpEvent) event3 = NULL;

    event1 = create_test_event ("e1", "E1", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 850);
    event2 = create_test_event ("e2", "E2", LP_EVENT_TYPE_ECONOMIC, LP_EVENT_SEVERITY_MINOR, 860);
    event3 = create_test_event ("e3", "E3", LP_EVENT_TYPE_POLITICAL, LP_EVENT_SEVERITY_MINOR, 870);

    lp_event_chronicle_record (fixture->chronicle, event1, 850, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event2, 860, NULL, 0, 0);
    lp_event_chronicle_record (fixture->chronicle, event3, 870, NULL, 0, 0);

    g_assert_cmpuint (lp_event_chronicle_get_count_by_type (fixture->chronicle, LP_EVENT_TYPE_ECONOMIC), ==, 2);
    g_assert_cmpuint (lp_event_chronicle_get_count_by_type (fixture->chronicle, LP_EVENT_TYPE_POLITICAL), ==, 1);
    g_assert_cmpuint (lp_event_chronicle_get_count_by_type (fixture->chronicle, LP_EVENT_TYPE_MAGICAL), ==, 0);
    g_assert_cmpuint (lp_event_chronicle_get_count_by_type (fixture->chronicle, LP_EVENT_TYPE_PERSONAL), ==, 0);
}

static void
test_chronicle_milestone (ChronicleFixture *fixture,
                          gconstpointer     user_data)
{
    /* Should not crash */
    lp_event_chronicle_add_milestone (fixture->chronicle, 1000,
                                      "First Millennium",
                                      "A thousand years have passed");

    lp_event_chronicle_add_milestone (fixture->chronicle, 847,
                                      "First Awakening",
                                      "The lich awakens for the first time");
}

static void
test_chronicle_reset (ChronicleFixture *fixture,
                      gconstpointer     user_data)
{
    g_autoptr(LpEvent) event = NULL;

    event = create_test_event ("reset-test", "Reset Test",
                               LP_EVENT_TYPE_ECONOMIC,
                               LP_EVENT_SEVERITY_MINOR,
                               847);

    lp_event_chronicle_record (fixture->chronicle, event, 847, NULL, 0, 0);
    g_assert_cmpuint (lp_event_chronicle_get_count (fixture->chronicle), ==, 1);

    lp_event_chronicle_reset (fixture->chronicle);
    g_assert_cmpuint (lp_event_chronicle_get_count (fixture->chronicle), ==, 0);
    g_assert_cmpuint (lp_event_chronicle_get_count_by_type (fixture->chronicle, LP_EVENT_TYPE_ECONOMIC), ==, 0);
}

/* ==========================================================================
 * Save/Load Tests
 * ========================================================================== */

static void
test_chronicle_saveable_interface (ChronicleFixture *fixture,
                                   gconstpointer     user_data)
{
    /* Verify the chronicle implements LrgSaveable */
    g_assert_true (LRG_IS_SAVEABLE (fixture->chronicle));
}

/* ==========================================================================
 * Main
 * ========================================================================== */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Chronicle entry tests */
    g_test_add_func ("/chronicle/entry/new", test_chronicle_entry_new);
    g_test_add_func ("/chronicle/entry/copy", test_chronicle_entry_copy);
    g_test_add_func ("/chronicle/entry/free-null", test_chronicle_entry_free_null);
    g_test_add_func ("/chronicle/entry/copy-null", test_chronicle_entry_copy_null);

    /* Chronicle tests */
    g_test_add ("/chronicle/singleton",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_singleton,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/record",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_record,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/record-with-choice",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_record_with_choice,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/get-all",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_get_all,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/get-by-type",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_get_by_type,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/get-by-year-range",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_get_by_year_range,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/get-by-kingdom",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_get_by_kingdom,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/get-by-severity",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_get_by_severity,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/get-recent",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_get_recent,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/count-by-type",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_count_by_type,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/milestone",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_milestone,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/reset",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_reset,
                chronicle_fixture_teardown);

    g_test_add ("/chronicle/saveable-interface",
                ChronicleFixture, NULL,
                chronicle_fixture_setup,
                test_chronicle_saveable_interface,
                chronicle_fixture_teardown);

    return g_test_run ();
}
