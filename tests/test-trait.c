/* test-trait.c - Bloodline Trait System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include "agent/lp-trait.h"
#include "lp-enums.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpTrait *trait;
} TraitFixture;

static void
fixture_set_up (TraitFixture  *fixture,
                gconstpointer  user_data)
{
    (void)user_data;

    fixture->trait = lp_trait_new ("test-trait", "Test Trait");
    g_assert_nonnull (fixture->trait);
}

static void
fixture_tear_down (TraitFixture  *fixture,
                   gconstpointer  user_data)
{
    (void)user_data;

    g_clear_object (&fixture->trait);
}

/* ==========================================================================
 * Construction Tests
 * ========================================================================== */

static void
test_trait_new_basic (TraitFixture  *fixture,
                      gconstpointer  user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_TRAIT (fixture->trait));
    g_assert_cmpstr (lp_trait_get_id (fixture->trait), ==, "test-trait");
    g_assert_cmpstr (lp_trait_get_name (fixture->trait), ==, "Test Trait");
}

static void
test_trait_new_full (void)
{
    g_autoptr(LpTrait) trait = NULL;

    trait = lp_trait_new_full ("wealthy-bloodline",
                               "Wealthy Bloodline",
                               "This family has deep pockets.",
                               0.75f,   /* 75% inheritance */
                               1.15f,   /* +15% income */
                               10,      /* +10 loyalty */
                               0.9f);   /* -10% discovery */

    g_assert_nonnull (trait);
    g_assert_cmpstr (lp_trait_get_id (trait), ==, "wealthy-bloodline");
    g_assert_cmpstr (lp_trait_get_name (trait), ==, "Wealthy Bloodline");
    g_assert_cmpstr (lp_trait_get_description (trait), ==,
                     "This family has deep pockets.");
    g_assert_cmpfloat (lp_trait_get_inheritance_chance (trait), ==, 0.75f);
    g_assert_cmpfloat (lp_trait_get_income_modifier (trait), ==, 1.15f);
    g_assert_cmpint (lp_trait_get_loyalty_modifier (trait), ==, 10);
    g_assert_cmpfloat (lp_trait_get_discovery_modifier (trait), ==, 0.9f);
}

/* ==========================================================================
 * Property Tests
 * ========================================================================== */

static void
test_trait_id_getter (TraitFixture  *fixture,
                      gconstpointer  user_data)
{
    (void)user_data;

    /* ID should be immutable once set */
    g_assert_cmpstr (lp_trait_get_id (fixture->trait), ==, "test-trait");
}

static void
test_trait_name_getter_setter (TraitFixture  *fixture,
                               gconstpointer  user_data)
{
    (void)user_data;

    lp_trait_set_name (fixture->trait, "Renamed Trait");
    g_assert_cmpstr (lp_trait_get_name (fixture->trait), ==, "Renamed Trait");
}

static void
test_trait_description_getter_setter (TraitFixture  *fixture,
                                      gconstpointer  user_data)
{
    (void)user_data;

    /* Initially NULL */
    g_assert_null (lp_trait_get_description (fixture->trait));

    lp_trait_set_description (fixture->trait, "A test description.");
    g_assert_cmpstr (lp_trait_get_description (fixture->trait), ==,
                     "A test description.");

    /* Can be set back to NULL */
    lp_trait_set_description (fixture->trait, NULL);
    g_assert_null (lp_trait_get_description (fixture->trait));
}

static void
test_trait_inheritance_chance (TraitFixture  *fixture,
                               gconstpointer  user_data)
{
    (void)user_data;

    /* Default should be reasonable (0.5 or similar) */
    lp_trait_set_inheritance_chance (fixture->trait, 0.8f);
    g_assert_cmpfloat (lp_trait_get_inheritance_chance (fixture->trait), ==, 0.8f);

    /* Test clamping at boundaries */
    lp_trait_set_inheritance_chance (fixture->trait, 0.0f);
    g_assert_cmpfloat (lp_trait_get_inheritance_chance (fixture->trait), ==, 0.0f);

    lp_trait_set_inheritance_chance (fixture->trait, 1.0f);
    g_assert_cmpfloat (lp_trait_get_inheritance_chance (fixture->trait), ==, 1.0f);
}

static void
test_trait_income_modifier (TraitFixture  *fixture,
                            gconstpointer  user_data)
{
    (void)user_data;

    lp_trait_set_income_modifier (fixture->trait, 1.25f);
    g_assert_cmpfloat (lp_trait_get_income_modifier (fixture->trait), ==, 1.25f);

    /* Can be less than 1.0 for negative traits */
    lp_trait_set_income_modifier (fixture->trait, 0.75f);
    g_assert_cmpfloat (lp_trait_get_income_modifier (fixture->trait), ==, 0.75f);
}

static void
test_trait_loyalty_modifier (TraitFixture  *fixture,
                             gconstpointer  user_data)
{
    (void)user_data;

    lp_trait_set_loyalty_modifier (fixture->trait, 15);
    g_assert_cmpint (lp_trait_get_loyalty_modifier (fixture->trait), ==, 15);

    /* Can be negative for disloyal traits */
    lp_trait_set_loyalty_modifier (fixture->trait, -20);
    g_assert_cmpint (lp_trait_get_loyalty_modifier (fixture->trait), ==, -20);
}

static void
test_trait_discovery_modifier (TraitFixture  *fixture,
                               gconstpointer  user_data)
{
    (void)user_data;

    /* Lower is better (less likely to be discovered) */
    lp_trait_set_discovery_modifier (fixture->trait, 0.5f);
    g_assert_cmpfloat (lp_trait_get_discovery_modifier (fixture->trait), ==, 0.5f);

    /* Higher means more likely to reveal the lich */
    lp_trait_set_discovery_modifier (fixture->trait, 1.5f);
    g_assert_cmpfloat (lp_trait_get_discovery_modifier (fixture->trait), ==, 1.5f);
}

/* ==========================================================================
 * Conflict Tests
 * ========================================================================== */

static void
test_trait_add_conflict (TraitFixture  *fixture,
                         gconstpointer  user_data)
{
    GPtrArray *conflicts;

    (void)user_data;

    lp_trait_add_conflict (fixture->trait, "rival-trait");
    conflicts = lp_trait_get_conflicts_with (fixture->trait);

    g_assert_nonnull (conflicts);
    g_assert_cmpuint (conflicts->len, ==, 1);
    g_assert_cmpstr (g_ptr_array_index (conflicts, 0), ==, "rival-trait");
}

static void
test_trait_conflicts_with_id (TraitFixture  *fixture,
                              gconstpointer  user_data)
{
    (void)user_data;

    lp_trait_add_conflict (fixture->trait, "conflicting-trait");

    g_assert_true (lp_trait_conflicts_with_id (fixture->trait, "conflicting-trait"));
    g_assert_false (lp_trait_conflicts_with_id (fixture->trait, "other-trait"));
}

static void
test_trait_conflicts_with_object (void)
{
    g_autoptr(LpTrait) trait1 = NULL;
    g_autoptr(LpTrait) trait2 = NULL;
    g_autoptr(LpTrait) trait3 = NULL;

    trait1 = lp_trait_new ("trait-1", "Trait 1");
    trait2 = lp_trait_new ("trait-2", "Trait 2");
    trait3 = lp_trait_new ("trait-3", "Trait 3");

    lp_trait_add_conflict (trait1, "trait-2");

    g_assert_true (lp_trait_conflicts_with (trait1, trait2));
    g_assert_false (lp_trait_conflicts_with (trait1, trait3));
}

/* ==========================================================================
 * Inheritance Tests
 * ========================================================================== */

static void
test_trait_roll_inheritance_zero_chance (TraitFixture  *fixture,
                                         gconstpointer  user_data)
{
    gboolean inherited;
    guint i;

    (void)user_data;

    /*
     * With 0% base chance and generation=0, should never inherit.
     * Note: generation adds +2% per level, so we use generation=0
     * to test pure 0% chance.
     */
    lp_trait_set_inheritance_chance (fixture->trait, 0.0f);

    for (i = 0; i < 100; i++)
    {
        inherited = lp_trait_roll_inheritance (fixture->trait, 0);
        g_assert_false (inherited);
    }
}

static void
test_trait_roll_inheritance_high_chance (TraitFixture  *fixture,
                                         gconstpointer  user_data)
{
    guint inherited_count;
    guint i;

    (void)user_data;

    /*
     * With 95% chance, should inherit most of the time.
     * We test for at least 80 out of 100 to account for RNG.
     */
    lp_trait_set_inheritance_chance (fixture->trait, 0.95f);
    inherited_count = 0;

    for (i = 0; i < 100; i++)
    {
        if (lp_trait_roll_inheritance (fixture->trait, 1))
        {
            inherited_count++;
        }
    }

    /* Should inherit at least 80% of the time */
    g_assert_cmpuint (inherited_count, >=, 80);
}

/* ==========================================================================
 * Copy Tests
 * ========================================================================== */

static void
test_trait_copy (TraitFixture  *fixture,
                 gconstpointer  user_data)
{
    g_autoptr(LpTrait) copy = NULL;

    (void)user_data;

    lp_trait_set_description (fixture->trait, "Original description");
    lp_trait_set_inheritance_chance (fixture->trait, 0.65f);
    lp_trait_set_income_modifier (fixture->trait, 1.1f);

    copy = lp_trait_copy (fixture->trait);

    g_assert_nonnull (copy);
    g_assert_true (copy != fixture->trait);
    g_assert_cmpstr (lp_trait_get_id (copy), ==, lp_trait_get_id (fixture->trait));
    g_assert_cmpstr (lp_trait_get_name (copy), ==, lp_trait_get_name (fixture->trait));
    g_assert_cmpstr (lp_trait_get_description (copy), ==,
                     lp_trait_get_description (fixture->trait));
    g_assert_cmpfloat (lp_trait_get_inheritance_chance (copy), ==,
                       lp_trait_get_inheritance_chance (fixture->trait));
    g_assert_cmpfloat (lp_trait_get_income_modifier (copy), ==,
                       lp_trait_get_income_modifier (fixture->trait));
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Construction tests */
    g_test_add ("/trait/new-basic",
                TraitFixture, NULL,
                fixture_set_up, test_trait_new_basic, fixture_tear_down);

    g_test_add_func ("/trait/new-full", test_trait_new_full);

    /* Property tests */
    g_test_add ("/trait/id-getter",
                TraitFixture, NULL,
                fixture_set_up, test_trait_id_getter, fixture_tear_down);

    g_test_add ("/trait/name-getter-setter",
                TraitFixture, NULL,
                fixture_set_up, test_trait_name_getter_setter, fixture_tear_down);

    g_test_add ("/trait/description-getter-setter",
                TraitFixture, NULL,
                fixture_set_up, test_trait_description_getter_setter, fixture_tear_down);

    g_test_add ("/trait/inheritance-chance",
                TraitFixture, NULL,
                fixture_set_up, test_trait_inheritance_chance, fixture_tear_down);

    g_test_add ("/trait/income-modifier",
                TraitFixture, NULL,
                fixture_set_up, test_trait_income_modifier, fixture_tear_down);

    g_test_add ("/trait/loyalty-modifier",
                TraitFixture, NULL,
                fixture_set_up, test_trait_loyalty_modifier, fixture_tear_down);

    g_test_add ("/trait/discovery-modifier",
                TraitFixture, NULL,
                fixture_set_up, test_trait_discovery_modifier, fixture_tear_down);

    /* Conflict tests */
    g_test_add ("/trait/add-conflict",
                TraitFixture, NULL,
                fixture_set_up, test_trait_add_conflict, fixture_tear_down);

    g_test_add ("/trait/conflicts-with-id",
                TraitFixture, NULL,
                fixture_set_up, test_trait_conflicts_with_id, fixture_tear_down);

    g_test_add_func ("/trait/conflicts-with-object", test_trait_conflicts_with_object);

    /* Inheritance tests */
    g_test_add ("/trait/roll-inheritance-zero-chance",
                TraitFixture, NULL,
                fixture_set_up, test_trait_roll_inheritance_zero_chance, fixture_tear_down);

    g_test_add ("/trait/roll-inheritance-high-chance",
                TraitFixture, NULL,
                fixture_set_up, test_trait_roll_inheritance_high_chance, fixture_tear_down);

    /* Copy tests */
    g_test_add ("/trait/copy",
                TraitFixture, NULL,
                fixture_set_up, test_trait_copy, fixture_tear_down);

    return g_test_run ();
}
