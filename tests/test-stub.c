/* test-stub.c - Infrastructure Verification Test
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * A simple test to verify the test infrastructure is working.
 * This will be expanded as more game systems are implemented.
 */

#include <glib.h>

/* ==========================================================================
 * Basic Infrastructure Tests
 * ========================================================================== */

static void
test_stub_passes (void)
{
    /* This test always passes - verifies infrastructure */
    g_assert_true (TRUE);
}

static void
test_glib_version (void)
{
    /* Verify we have a recent enough GLib */
    g_assert_cmpuint (glib_major_version, >=, 2);
    g_assert_cmpuint (glib_minor_version, >=, 50);
}

static void
test_memory_allocation (void)
{
    /* Verify g_autoptr works correctly */
    g_autofree gchar *str = g_strdup ("test");
    g_assert_nonnull (str);
    g_assert_cmpstr (str, ==, "test");
}

static void
test_array_operations (void)
{
    /* Verify GPtrArray works correctly */
    g_autoptr(GPtrArray) array = g_ptr_array_new ();
    g_assert_nonnull (array);
    g_assert_cmpuint (array->len, ==, 0);

    g_ptr_array_add (array, g_strdup ("one"));
    g_ptr_array_add (array, g_strdup ("two"));
    g_assert_cmpuint (array->len, ==, 2);

    /* Clean up strings manually since we didn't set a free func */
    g_free (g_ptr_array_index (array, 0));
    g_free (g_ptr_array_index (array, 1));
}

static void
test_hash_table (void)
{
    /* Verify GHashTable works correctly */
    const gchar *value;
    g_autoptr(GHashTable) table = g_hash_table_new_full (g_str_hash,
                                                          g_str_equal,
                                                          g_free,
                                                          g_free);
    g_assert_nonnull (table);

    g_hash_table_insert (table, g_strdup ("key"), g_strdup ("value"));
    g_assert_cmpuint (g_hash_table_size (table), ==, 1);

    value = g_hash_table_lookup (table, "key");
    g_assert_cmpstr (value, ==, "value");
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Infrastructure tests */
    g_test_add_func ("/stub/passes", test_stub_passes);
    g_test_add_func ("/stub/glib-version", test_glib_version);
    g_test_add_func ("/stub/memory-allocation", test_memory_allocation);
    g_test_add_func ("/stub/array-operations", test_array_operations);
    g_test_add_func ("/stub/hash-table", test_hash_table);

    return g_test_run ();
}
