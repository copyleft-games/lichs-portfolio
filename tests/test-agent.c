/* test-agent.c - Agent System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests the agent base class, traits, and all agent subclasses:
 * - LpAgent (base class)
 * - LpTrait
 * - LpAgentIndividual
 * - LpAgentFamily
 * - LpAgentManager
 */

#include <glib.h>
#include <libregnum.h>
#include "agent/lp-agent.h"
#include "agent/lp-trait.h"
#include "agent/lp-agent-individual.h"
#include "agent/lp-agent-family.h"
#include "agent/lp-agent-manager.h"

/* ==========================================================================
 * Agent Base Class Fixture
 * ========================================================================== */

typedef struct
{
    LpAgent *agent;
} AgentFixture;

static void
agent_fixture_set_up (AgentFixture *fixture,
                      gconstpointer user_data)
{
    (void)user_data;

    fixture->agent = lp_agent_new ("agent-001", "Test Agent");
    g_assert_nonnull (fixture->agent);
}

static void
agent_fixture_tear_down (AgentFixture *fixture,
                         gconstpointer user_data)
{
    (void)user_data;

    g_clear_object (&fixture->agent);
}

/* ==========================================================================
 * Trait Fixture
 * ========================================================================== */

typedef struct
{
    LpTrait *trait;
} TraitFixture;

static void
trait_fixture_set_up (TraitFixture *fixture,
                      gconstpointer user_data)
{
    (void)user_data;

    fixture->trait = lp_trait_new ("trait-keen", "Keen Mind");
    g_assert_nonnull (fixture->trait);
}

static void
trait_fixture_tear_down (TraitFixture *fixture,
                         gconstpointer user_data)
{
    (void)user_data;

    g_clear_object (&fixture->trait);
}

/* ==========================================================================
 * Individual Agent Fixture
 * ========================================================================== */

typedef struct
{
    LpAgentIndividual *individual;
} IndividualFixture;

static void
individual_fixture_set_up (IndividualFixture *fixture,
                           gconstpointer      user_data)
{
    (void)user_data;

    fixture->individual = lp_agent_individual_new ("ind-001", "Marcus Steward");
    g_assert_nonnull (fixture->individual);
}

static void
individual_fixture_tear_down (IndividualFixture *fixture,
                              gconstpointer      user_data)
{
    (void)user_data;

    g_clear_object (&fixture->individual);
}

/* ==========================================================================
 * Family Agent Fixture
 * ========================================================================== */

typedef struct
{
    LpAgentFamily *family;
} FamilyFixture;

static void
family_fixture_set_up (FamilyFixture *fixture,
                       gconstpointer  user_data)
{
    (void)user_data;

    fixture->family = lp_agent_family_new ("fam-001", "von Richter", 847);
    g_assert_nonnull (fixture->family);
}

static void
family_fixture_tear_down (FamilyFixture *fixture,
                          gconstpointer  user_data)
{
    (void)user_data;

    g_clear_object (&fixture->family);
}

/* ==========================================================================
 * Agent Manager Fixture
 * ========================================================================== */

typedef struct
{
    LpAgentManager *manager;
} ManagerFixture;

static void
manager_fixture_set_up (ManagerFixture *fixture,
                        gconstpointer   user_data)
{
    (void)user_data;

    fixture->manager = lp_agent_manager_new ();
    g_assert_nonnull (fixture->manager);
}

static void
manager_fixture_tear_down (ManagerFixture *fixture,
                           gconstpointer   user_data)
{
    (void)user_data;

    g_clear_object (&fixture->manager);
}

/* ==========================================================================
 * Agent Base Class Tests
 * ========================================================================== */

static void
test_agent_new (AgentFixture *fixture,
                gconstpointer user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_AGENT (fixture->agent));
    g_assert_true (LRG_IS_SAVEABLE (fixture->agent));
}

static void
test_agent_id (AgentFixture *fixture,
               gconstpointer user_data)
{
    const gchar *id;

    (void)user_data;

    id = lp_agent_get_id (fixture->agent);
    g_assert_cmpstr (id, ==, "agent-001");
}

static void
test_agent_name (AgentFixture *fixture,
                 gconstpointer user_data)
{
    const gchar *name;

    (void)user_data;

    name = lp_agent_get_name (fixture->agent);
    g_assert_cmpstr (name, ==, "Test Agent");

    lp_agent_set_name (fixture->agent, "New Name");
    name = lp_agent_get_name (fixture->agent);
    g_assert_cmpstr (name, ==, "New Name");
}

static void
test_agent_age (AgentFixture *fixture,
                gconstpointer user_data)
{
    guint age;

    (void)user_data;

    /* Default age should be reasonable */
    age = lp_agent_get_age (fixture->agent);
    g_assert_cmpuint (age, >=, 18);
    g_assert_cmpuint (age, <=, 70);

    lp_agent_set_age (fixture->agent, 35);
    age = lp_agent_get_age (fixture->agent);
    g_assert_cmpuint (age, ==, 35);
}

static void
test_agent_max_age (AgentFixture *fixture,
                    gconstpointer user_data)
{
    guint max_age;

    (void)user_data;

    max_age = lp_agent_get_max_age (fixture->agent);
    g_assert_cmpuint (max_age, >=, 60);
    g_assert_cmpuint (max_age, <=, 90);

    lp_agent_set_max_age (fixture->agent, 80);
    max_age = lp_agent_get_max_age (fixture->agent);
    g_assert_cmpuint (max_age, ==, 80);
}

static void
test_agent_loyalty (AgentFixture *fixture,
                    gconstpointer user_data)
{
    gint loyalty;

    (void)user_data;

    lp_agent_set_loyalty (fixture->agent, 75);
    loyalty = lp_agent_get_loyalty (fixture->agent);
    g_assert_cmpint (loyalty, ==, 75);

    /* Test clamping to 0-100 */
    lp_agent_set_loyalty (fixture->agent, 150);
    loyalty = lp_agent_get_loyalty (fixture->agent);
    g_assert_cmpint (loyalty, ==, 100);

    lp_agent_set_loyalty (fixture->agent, -50);
    loyalty = lp_agent_get_loyalty (fixture->agent);
    g_assert_cmpint (loyalty, ==, 0);
}

static void
test_agent_competence (AgentFixture *fixture,
                       gconstpointer user_data)
{
    gint competence;

    (void)user_data;

    lp_agent_set_competence (fixture->agent, 60);
    competence = lp_agent_get_competence (fixture->agent);
    g_assert_cmpint (competence, ==, 60);

    /* Test clamping to 0-100 */
    lp_agent_set_competence (fixture->agent, 200);
    competence = lp_agent_get_competence (fixture->agent);
    g_assert_cmpint (competence, ==, 100);
}

static void
test_agent_cover_status (AgentFixture *fixture,
                         gconstpointer user_data)
{
    LpCoverStatus status;

    (void)user_data;

    /* Default should be secure */
    status = lp_agent_get_cover_status (fixture->agent);
    g_assert_cmpint (status, ==, LP_COVER_STATUS_SECURE);

    lp_agent_set_cover_status (fixture->agent, LP_COVER_STATUS_SUSPICIOUS);
    status = lp_agent_get_cover_status (fixture->agent);
    g_assert_cmpint (status, ==, LP_COVER_STATUS_SUSPICIOUS);
}

static void
test_agent_knowledge_level (AgentFixture *fixture,
                            gconstpointer user_data)
{
    LpKnowledgeLevel level;

    (void)user_data;

    /* Default should be none */
    level = lp_agent_get_knowledge_level (fixture->agent);
    g_assert_cmpint (level, ==, LP_KNOWLEDGE_LEVEL_NONE);

    lp_agent_set_knowledge_level (fixture->agent, LP_KNOWLEDGE_LEVEL_SUSPICIOUS);
    level = lp_agent_get_knowledge_level (fixture->agent);
    g_assert_cmpint (level, ==, LP_KNOWLEDGE_LEVEL_SUSPICIOUS);
}

static void
test_agent_is_alive (AgentFixture *fixture,
                     gconstpointer user_data)
{
    (void)user_data;

    lp_agent_set_age (fixture->agent, 30);
    lp_agent_set_max_age (fixture->agent, 70);
    g_assert_true (lp_agent_is_alive (fixture->agent));

    lp_agent_set_age (fixture->agent, 70);
    g_assert_false (lp_agent_is_alive (fixture->agent));
}

static void
test_agent_years_remaining (AgentFixture *fixture,
                            gconstpointer user_data)
{
    guint remaining;

    (void)user_data;

    lp_agent_set_age (fixture->agent, 30);
    lp_agent_set_max_age (fixture->agent, 70);

    remaining = lp_agent_get_years_remaining (fixture->agent);
    g_assert_cmpuint (remaining, ==, 40);
}

static void
test_agent_exposure_contribution (AgentFixture *fixture,
                                  gconstpointer user_data)
{
    guint exposure;

    (void)user_data;

    /* Secure cover with no knowledge = low exposure */
    lp_agent_set_cover_status (fixture->agent, LP_COVER_STATUS_SECURE);
    lp_agent_set_knowledge_level (fixture->agent, LP_KNOWLEDGE_LEVEL_NONE);
    exposure = lp_agent_get_exposure_contribution (fixture->agent);
    g_assert_cmpuint (exposure, <=, 10);

    /* Exposed with full knowledge = high exposure (10 base * 3 multiplier = 30) */
    lp_agent_set_cover_status (fixture->agent, LP_COVER_STATUS_EXPOSED);
    lp_agent_set_knowledge_level (fixture->agent, LP_KNOWLEDGE_LEVEL_FULL);
    exposure = lp_agent_get_exposure_contribution (fixture->agent);
    g_assert_cmpuint (exposure, >=, 25);
}

/* ==========================================================================
 * Trait Tests
 * ========================================================================== */

static void
test_trait_new (TraitFixture *fixture,
                gconstpointer user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_TRAIT (fixture->trait));
    g_assert_true (LRG_IS_SAVEABLE (fixture->trait));
}

static void
test_trait_id (TraitFixture *fixture,
               gconstpointer user_data)
{
    const gchar *id;

    (void)user_data;

    id = lp_trait_get_id (fixture->trait);
    g_assert_cmpstr (id, ==, "trait-keen");
}

static void
test_trait_name (TraitFixture *fixture,
                 gconstpointer user_data)
{
    const gchar *name;

    (void)user_data;

    name = lp_trait_get_name (fixture->trait);
    g_assert_cmpstr (name, ==, "Keen Mind");

    lp_trait_set_name (fixture->trait, "Sharp Mind");
    name = lp_trait_get_name (fixture->trait);
    g_assert_cmpstr (name, ==, "Sharp Mind");
}

static void
test_trait_modifiers (TraitFixture *fixture,
                      gconstpointer user_data)
{
    gfloat income_mod;
    gint loyalty_mod;
    gfloat discovery_mod;

    (void)user_data;

    lp_trait_set_income_modifier (fixture->trait, 1.15f);
    income_mod = lp_trait_get_income_modifier (fixture->trait);
    g_assert_cmpfloat (income_mod, ==, 1.15f);

    lp_trait_set_loyalty_modifier (fixture->trait, 10);
    loyalty_mod = lp_trait_get_loyalty_modifier (fixture->trait);
    g_assert_cmpint (loyalty_mod, ==, 10);

    lp_trait_set_discovery_modifier (fixture->trait, 0.8f);
    discovery_mod = lp_trait_get_discovery_modifier (fixture->trait);
    g_assert_cmpfloat (discovery_mod, ==, 0.8f);
}

static void
test_trait_inheritance_chance (TraitFixture *fixture,
                               gconstpointer user_data)
{
    gfloat chance;

    (void)user_data;

    lp_trait_set_inheritance_chance (fixture->trait, 0.7f);
    chance = lp_trait_get_inheritance_chance (fixture->trait);
    g_assert_cmpfloat (chance, ==, 0.7f);
}

static void
test_trait_conflicts (TraitFixture *fixture,
                      gconstpointer user_data)
{
    gboolean conflicts;

    (void)user_data;

    lp_trait_add_conflict (fixture->trait, "trait-slow");

    conflicts = lp_trait_conflicts_with_id (fixture->trait, "trait-slow");
    g_assert_true (conflicts);

    conflicts = lp_trait_conflicts_with_id (fixture->trait, "trait-fast");
    g_assert_false (conflicts);
}

static void
test_trait_copy (TraitFixture *fixture,
                 gconstpointer user_data)
{
    g_autoptr(LpTrait) copy = NULL;
    const gchar *id;
    const gchar *name;

    (void)user_data;

    lp_trait_set_income_modifier (fixture->trait, 1.2f);
    copy = lp_trait_copy (fixture->trait);

    g_assert_nonnull (copy);
    g_assert_true (LP_IS_TRAIT (copy));

    id = lp_trait_get_id (copy);
    g_assert_cmpstr (id, ==, "trait-keen");

    name = lp_trait_get_name (copy);
    g_assert_cmpstr (name, ==, "Keen Mind");

    g_assert_cmpfloat (lp_trait_get_income_modifier (copy), ==, 1.2f);
}

static void
test_trait_new_full (void)
{
    g_autoptr(LpTrait) trait = NULL;

    trait = lp_trait_new_full ("trait-custom", "Custom Trait",
                                "A custom test trait",
                                0.65f, 1.1f, 5, 0.9f);
    g_assert_nonnull (trait);
    g_assert_cmpstr (lp_trait_get_id (trait), ==, "trait-custom");
    g_assert_cmpstr (lp_trait_get_name (trait), ==, "Custom Trait");
    g_assert_cmpstr (lp_trait_get_description (trait), ==, "A custom test trait");
    g_assert_cmpfloat (lp_trait_get_inheritance_chance (trait), ==, 0.65f);
    g_assert_cmpfloat (lp_trait_get_income_modifier (trait), ==, 1.1f);
    g_assert_cmpint (lp_trait_get_loyalty_modifier (trait), ==, 5);
    g_assert_cmpfloat (lp_trait_get_discovery_modifier (trait), ==, 0.9f);
}

/* ==========================================================================
 * Agent Trait Integration Tests
 * ========================================================================== */

static void
test_agent_add_trait (AgentFixture *fixture,
                      gconstpointer user_data)
{
    g_autoptr(LpTrait) trait = NULL;
    GPtrArray *traits;

    (void)user_data;

    trait = lp_trait_new ("trait-test", "Test Trait");
    lp_agent_add_trait (fixture->agent, trait);

    traits = lp_agent_get_traits (fixture->agent);
    g_assert_cmpuint (traits->len, ==, 1);
}

static void
test_agent_has_trait (AgentFixture *fixture,
                      gconstpointer user_data)
{
    g_autoptr(LpTrait) trait = NULL;
    gboolean has;

    (void)user_data;

    trait = lp_trait_new ("trait-check", "Check Trait");
    lp_agent_add_trait (fixture->agent, trait);

    has = lp_agent_has_trait (fixture->agent, "trait-check");
    g_assert_true (has);

    has = lp_agent_has_trait (fixture->agent, "nonexistent");
    g_assert_false (has);
}

static void
test_agent_remove_trait (AgentFixture *fixture,
                         gconstpointer user_data)
{
    g_autoptr(LpTrait) trait = NULL;
    GPtrArray *traits;
    gboolean removed;

    (void)user_data;

    trait = lp_trait_new ("trait-remove", "Remove Trait");
    lp_agent_add_trait (fixture->agent, trait);

    traits = lp_agent_get_traits (fixture->agent);
    g_assert_cmpuint (traits->len, ==, 1);

    removed = lp_agent_remove_trait (fixture->agent, trait);
    g_assert_true (removed);

    traits = lp_agent_get_traits (fixture->agent);
    g_assert_cmpuint (traits->len, ==, 0);
}

/* ==========================================================================
 * Individual Agent Tests
 * ========================================================================== */

static void
test_individual_new (IndividualFixture *fixture,
                     gconstpointer      user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_AGENT_INDIVIDUAL (fixture->individual));
    g_assert_true (LP_IS_AGENT (fixture->individual));
}

static void
test_individual_agent_type (IndividualFixture *fixture,
                            gconstpointer      user_data)
{
    LpAgentType type;

    (void)user_data;

    type = lp_agent_get_agent_type (LP_AGENT (fixture->individual));
    g_assert_cmpint (type, ==, LP_AGENT_TYPE_INDIVIDUAL);
}

static void
test_individual_successor (IndividualFixture *fixture,
                           gconstpointer      user_data)
{
    LpAgentIndividual *successor;
    LpAgentIndividual *retrieved;

    (void)user_data;

    /* Initially no successor */
    retrieved = lp_agent_individual_get_successor (fixture->individual);
    g_assert_null (retrieved);

    /* Set successor */
    successor = lp_agent_individual_new ("succ-001", "Successor");
    lp_agent_individual_set_successor (fixture->individual, successor);

    retrieved = lp_agent_individual_get_successor (fixture->individual);
    g_assert_nonnull (retrieved);
    g_assert_cmpstr (lp_agent_get_id (LP_AGENT (retrieved)), ==, "succ-001");

    g_object_unref (successor);
}

static void
test_individual_training_progress (IndividualFixture *fixture,
                                   gconstpointer      user_data)
{
    gfloat progress;

    (void)user_data;

    /* Initially 0 */
    progress = lp_agent_individual_get_training_progress (fixture->individual);
    g_assert_cmpfloat (progress, ==, 0.0f);

    lp_agent_individual_set_training_progress (fixture->individual, 0.5f);
    progress = lp_agent_individual_get_training_progress (fixture->individual);
    g_assert_cmpfloat (progress, ==, 0.5f);
}

static void
test_individual_has_trained_successor (IndividualFixture *fixture,
                                       gconstpointer      user_data)
{
    g_autoptr(LpAgentIndividual) successor = NULL;
    gboolean trained;

    (void)user_data;

    successor = lp_agent_individual_new ("succ-002", "Trained One");
    lp_agent_individual_set_successor (fixture->individual, successor);

    /* Not trained yet */
    trained = lp_agent_individual_has_trained_successor (fixture->individual);
    g_assert_false (trained);

    /* Fully trained */
    lp_agent_individual_set_training_progress (fixture->individual, 1.0f);
    trained = lp_agent_individual_has_trained_successor (fixture->individual);
    g_assert_true (trained);
}

static void
test_individual_skill_retention (IndividualFixture *fixture,
                                 gconstpointer      user_data)
{
    g_autoptr(LpAgentIndividual) successor = NULL;
    gfloat retention;

    (void)user_data;

    /* No successor = 25% retention */
    retention = lp_agent_individual_get_skill_retention (fixture->individual);
    g_assert_cmpfloat (retention, >=, 0.24f);
    g_assert_cmpfloat (retention, <=, 0.26f);

    /* Add trained successor = 75% retention */
    successor = lp_agent_individual_new ("s", "S");
    lp_agent_individual_set_successor (fixture->individual, successor);
    lp_agent_individual_set_training_progress (fixture->individual, 1.0f);

    retention = lp_agent_individual_get_skill_retention (fixture->individual);
    g_assert_cmpfloat (retention, >=, 0.74f);
    g_assert_cmpfloat (retention, <=, 0.76f);
}

static void
test_individual_new_full (void)
{
    g_autoptr(LpAgentIndividual) ind = NULL;

    ind = lp_agent_individual_new_full ("full-001", "Full Agent",
                                         30, 75, 80, 65);
    g_assert_nonnull (ind);
    g_assert_cmpuint (lp_agent_get_age (LP_AGENT (ind)), ==, 30);
    g_assert_cmpuint (lp_agent_get_max_age (LP_AGENT (ind)), ==, 75);
    g_assert_cmpint (lp_agent_get_loyalty (LP_AGENT (ind)), ==, 80);
    g_assert_cmpint (lp_agent_get_competence (LP_AGENT (ind)), ==, 65);
}

/* ==========================================================================
 * Family Agent Tests
 * ========================================================================== */

static void
test_family_new (FamilyFixture *fixture,
                 gconstpointer  user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_AGENT_FAMILY (fixture->family));
    g_assert_true (LP_IS_AGENT (fixture->family));
}

static void
test_family_agent_type (FamilyFixture *fixture,
                        gconstpointer  user_data)
{
    LpAgentType type;

    (void)user_data;

    type = lp_agent_get_agent_type (LP_AGENT (fixture->family));
    g_assert_cmpint (type, ==, LP_AGENT_TYPE_FAMILY);
}

static void
test_family_name (FamilyFixture *fixture,
                  gconstpointer  user_data)
{
    const gchar *family_name;

    (void)user_data;

    family_name = lp_agent_family_get_family_name (fixture->family);
    g_assert_cmpstr (family_name, ==, "von Richter");
}

static void
test_family_generation (FamilyFixture *fixture,
                        gconstpointer  user_data)
{
    guint generation;

    (void)user_data;

    /* Starts at generation 1 */
    generation = lp_agent_family_get_generation (fixture->family);
    g_assert_cmpuint (generation, ==, 1);
}

static void
test_family_founding_year (FamilyFixture *fixture,
                           gconstpointer  user_data)
{
    guint64 founding;

    (void)user_data;

    founding = lp_agent_family_get_founding_year (fixture->family);
    g_assert_cmpuint (founding, ==, 847);
}

static void
test_family_bloodline_trait (FamilyFixture *fixture,
                             gconstpointer  user_data)
{
    g_autoptr(LpTrait) trait = NULL;
    GPtrArray *bloodline;
    gboolean has;

    (void)user_data;

    trait = lp_trait_new ("trait-blood", "Blood Trait");
    lp_agent_family_add_bloodline_trait (fixture->family, trait);

    bloodline = lp_agent_family_get_bloodline_traits (fixture->family);
    g_assert_cmpuint (bloodline->len, ==, 1);

    has = lp_agent_family_has_bloodline_trait (fixture->family, "trait-blood");
    g_assert_true (has);

    has = lp_agent_family_has_bloodline_trait (fixture->family, "nonexistent");
    g_assert_false (has);
}

static void
test_family_remove_bloodline_trait (FamilyFixture *fixture,
                                    gconstpointer  user_data)
{
    g_autoptr(LpTrait) trait = NULL;
    GPtrArray *bloodline;
    gboolean removed;

    (void)user_data;

    trait = lp_trait_new ("trait-rm", "Remove Blood Trait");
    lp_agent_family_add_bloodline_trait (fixture->family, trait);

    bloodline = lp_agent_family_get_bloodline_traits (fixture->family);
    g_assert_cmpuint (bloodline->len, ==, 1);

    removed = lp_agent_family_remove_bloodline_trait (fixture->family, trait);
    g_assert_true (removed);

    bloodline = lp_agent_family_get_bloodline_traits (fixture->family);
    g_assert_cmpuint (bloodline->len, ==, 0);
}

static void
test_family_advance_generation (FamilyFixture *fixture,
                                gconstpointer  user_data)
{
    guint gen_before;
    guint gen_after;

    (void)user_data;

    gen_before = lp_agent_family_get_generation (fixture->family);

    /* Force death to trigger generation advance */
    lp_agent_set_age (LP_AGENT (fixture->family), 100);
    lp_agent_set_max_age (LP_AGENT (fixture->family), 80);
    lp_agent_family_advance_generation (fixture->family);

    gen_after = lp_agent_family_get_generation (fixture->family);
    g_assert_cmpuint (gen_after, ==, gen_before + 1);

    /* Age should be reset to young */
    g_assert_cmpuint (lp_agent_get_age (LP_AGENT (fixture->family)), <, 30);
}

static void
test_family_years_established (FamilyFixture *fixture,
                               gconstpointer  user_data)
{
    guint64 years;

    (void)user_data;

    years = lp_agent_family_get_years_established (fixture->family, 947);
    g_assert_cmpuint (years, ==, 100);
}

static void
test_family_new_with_head (void)
{
    g_autoptr(LpAgentFamily) family = NULL;

    family = lp_agent_family_new_with_head ("fam-head", "House Blackwood",
                                             "Lord Edmund", 800, 45, 80);
    g_assert_nonnull (family);
    g_assert_cmpstr (lp_agent_family_get_family_name (family), ==, "House Blackwood");
    g_assert_cmpuint (lp_agent_get_age (LP_AGENT (family)), ==, 45);
    g_assert_cmpuint (lp_agent_get_max_age (LP_AGENT (family)), ==, 80);
}

/* ==========================================================================
 * Agent Manager Tests
 * ========================================================================== */

static void
test_manager_new (ManagerFixture *fixture,
                  gconstpointer   user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_AGENT_MANAGER (fixture->manager));
    g_assert_true (LRG_IS_SAVEABLE (fixture->manager));
}

static void
test_manager_add_agent (ManagerFixture *fixture,
                        gconstpointer   user_data)
{
    LpAgentIndividual *agent;
    guint count;

    (void)user_data;

    agent = lp_agent_individual_new ("mgr-agent-001", "Manager Agent");
    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (agent));

    count = lp_agent_manager_get_agent_count (fixture->manager);
    g_assert_cmpuint (count, ==, 1);
}

static void
test_manager_get_by_id (ManagerFixture *fixture,
                        gconstpointer   user_data)
{
    LpAgentIndividual *agent;
    LpAgent *found;

    (void)user_data;

    agent = lp_agent_individual_new ("find-me", "Find Me");
    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (agent));

    found = lp_agent_manager_get_agent_by_id (fixture->manager, "find-me");
    g_assert_nonnull (found);
    g_assert_true (LP_AGENT (agent) == found);

    found = lp_agent_manager_get_agent_by_id (fixture->manager, "not-found");
    g_assert_null (found);
}

static void
test_manager_remove_agent (ManagerFixture *fixture,
                           gconstpointer   user_data)
{
    LpAgentIndividual *agent;
    gboolean removed;
    guint count;

    (void)user_data;

    agent = lp_agent_individual_new ("remove-me", "Remove Me");
    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (agent));

    count = lp_agent_manager_get_agent_count (fixture->manager);
    g_assert_cmpuint (count, ==, 1);

    removed = lp_agent_manager_remove_agent (fixture->manager, LP_AGENT (agent));
    g_assert_true (removed);

    count = lp_agent_manager_get_agent_count (fixture->manager);
    g_assert_cmpuint (count, ==, 0);
}

static void
test_manager_get_agents_by_type (ManagerFixture *fixture,
                                 gconstpointer   user_data)
{
    LpAgentIndividual *ind;
    LpAgentFamily *fam;
    g_autoptr(GList) individuals = NULL;
    g_autoptr(GList) families = NULL;

    (void)user_data;

    ind = lp_agent_individual_new ("type-ind", "Individual");
    fam = lp_agent_family_new ("type-fam", "Family", 847);

    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (ind));
    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (fam));

    individuals = lp_agent_manager_get_agents_by_type (fixture->manager,
                                                        LP_AGENT_TYPE_INDIVIDUAL);
    g_assert_cmpuint (g_list_length (individuals), ==, 1);

    families = lp_agent_manager_get_agents_by_type (fixture->manager,
                                                     LP_AGENT_TYPE_FAMILY);
    g_assert_cmpuint (g_list_length (families), ==, 1);
}

static void
test_manager_get_available_agents (ManagerFixture *fixture,
                                   gconstpointer   user_data)
{
    LpAgentIndividual *agent;
    g_autoptr(GList) available = NULL;

    (void)user_data;

    agent = lp_agent_individual_new ("avail", "Available");
    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (agent));

    /* Agent with no investments should be available */
    available = lp_agent_manager_get_available_agents (fixture->manager);
    g_assert_cmpuint (g_list_length (available), ==, 1);
}

static void
test_manager_statistics (ManagerFixture *fixture,
                         gconstpointer   user_data)
{
    LpAgentIndividual *agent1;
    LpAgentIndividual *agent2;
    gint avg_loyalty;
    gint avg_competence;

    (void)user_data;

    agent1 = lp_agent_individual_new_full ("stat1", "Stat1", 30, 70, 80, 60);
    agent2 = lp_agent_individual_new_full ("stat2", "Stat2", 30, 70, 60, 80);

    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (agent1));
    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (agent2));

    avg_loyalty = lp_agent_manager_get_average_loyalty (fixture->manager);
    g_assert_cmpint (avg_loyalty, ==, 70);  /* (80 + 60) / 2 */

    avg_competence = lp_agent_manager_get_average_competence (fixture->manager);
    g_assert_cmpint (avg_competence, ==, 70);  /* (60 + 80) / 2 */
}

static void
test_manager_reset (ManagerFixture *fixture,
                    gconstpointer   user_data)
{
    LpAgentIndividual *agent;
    guint count;

    (void)user_data;

    agent = lp_agent_individual_new ("reset-me", "Reset Me");
    lp_agent_manager_add_agent (fixture->manager, LP_AGENT (agent));

    count = lp_agent_manager_get_agent_count (fixture->manager);
    g_assert_cmpuint (count, ==, 1);

    lp_agent_manager_reset (fixture->manager);

    count = lp_agent_manager_get_agent_count (fixture->manager);
    g_assert_cmpuint (count, ==, 0);
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Agent base class tests */
    g_test_add ("/agent/base/new",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_new, agent_fixture_tear_down);
    g_test_add ("/agent/base/id",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_id, agent_fixture_tear_down);
    g_test_add ("/agent/base/name",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_name, agent_fixture_tear_down);
    g_test_add ("/agent/base/age",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_age, agent_fixture_tear_down);
    g_test_add ("/agent/base/max-age",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_max_age, agent_fixture_tear_down);
    g_test_add ("/agent/base/loyalty",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_loyalty, agent_fixture_tear_down);
    g_test_add ("/agent/base/competence",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_competence, agent_fixture_tear_down);
    g_test_add ("/agent/base/cover-status",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_cover_status, agent_fixture_tear_down);
    g_test_add ("/agent/base/knowledge-level",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_knowledge_level, agent_fixture_tear_down);
    g_test_add ("/agent/base/is-alive",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_is_alive, agent_fixture_tear_down);
    g_test_add ("/agent/base/years-remaining",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_years_remaining, agent_fixture_tear_down);
    g_test_add ("/agent/base/exposure-contribution",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_exposure_contribution, agent_fixture_tear_down);

    /* Trait tests */
    g_test_add ("/agent/trait/new",
                TraitFixture, NULL,
                trait_fixture_set_up, test_trait_new, trait_fixture_tear_down);
    g_test_add ("/agent/trait/id",
                TraitFixture, NULL,
                trait_fixture_set_up, test_trait_id, trait_fixture_tear_down);
    g_test_add ("/agent/trait/name",
                TraitFixture, NULL,
                trait_fixture_set_up, test_trait_name, trait_fixture_tear_down);
    g_test_add ("/agent/trait/modifiers",
                TraitFixture, NULL,
                trait_fixture_set_up, test_trait_modifiers, trait_fixture_tear_down);
    g_test_add ("/agent/trait/inheritance-chance",
                TraitFixture, NULL,
                trait_fixture_set_up, test_trait_inheritance_chance, trait_fixture_tear_down);
    g_test_add ("/agent/trait/conflicts",
                TraitFixture, NULL,
                trait_fixture_set_up, test_trait_conflicts, trait_fixture_tear_down);
    g_test_add ("/agent/trait/copy",
                TraitFixture, NULL,
                trait_fixture_set_up, test_trait_copy, trait_fixture_tear_down);
    g_test_add_func ("/agent/trait/new-full", test_trait_new_full);

    /* Agent trait integration tests */
    g_test_add ("/agent/trait/add",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_add_trait, agent_fixture_tear_down);
    g_test_add ("/agent/trait/has",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_has_trait, agent_fixture_tear_down);
    g_test_add ("/agent/trait/remove",
                AgentFixture, NULL,
                agent_fixture_set_up, test_agent_remove_trait, agent_fixture_tear_down);

    /* Individual agent tests */
    g_test_add ("/agent/individual/new",
                IndividualFixture, NULL,
                individual_fixture_set_up, test_individual_new, individual_fixture_tear_down);
    g_test_add ("/agent/individual/agent-type",
                IndividualFixture, NULL,
                individual_fixture_set_up, test_individual_agent_type, individual_fixture_tear_down);
    g_test_add ("/agent/individual/successor",
                IndividualFixture, NULL,
                individual_fixture_set_up, test_individual_successor, individual_fixture_tear_down);
    g_test_add ("/agent/individual/training-progress",
                IndividualFixture, NULL,
                individual_fixture_set_up, test_individual_training_progress, individual_fixture_tear_down);
    g_test_add ("/agent/individual/has-trained-successor",
                IndividualFixture, NULL,
                individual_fixture_set_up, test_individual_has_trained_successor, individual_fixture_tear_down);
    g_test_add ("/agent/individual/skill-retention",
                IndividualFixture, NULL,
                individual_fixture_set_up, test_individual_skill_retention, individual_fixture_tear_down);
    g_test_add_func ("/agent/individual/new-full", test_individual_new_full);

    /* Family agent tests */
    g_test_add ("/agent/family/new",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_new, family_fixture_tear_down);
    g_test_add ("/agent/family/agent-type",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_agent_type, family_fixture_tear_down);
    g_test_add ("/agent/family/name",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_name, family_fixture_tear_down);
    g_test_add ("/agent/family/generation",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_generation, family_fixture_tear_down);
    g_test_add ("/agent/family/founding-year",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_founding_year, family_fixture_tear_down);
    g_test_add ("/agent/family/bloodline-trait",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_bloodline_trait, family_fixture_tear_down);
    g_test_add ("/agent/family/remove-bloodline-trait",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_remove_bloodline_trait, family_fixture_tear_down);
    g_test_add ("/agent/family/advance-generation",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_advance_generation, family_fixture_tear_down);
    g_test_add ("/agent/family/years-established",
                FamilyFixture, NULL,
                family_fixture_set_up, test_family_years_established, family_fixture_tear_down);
    g_test_add_func ("/agent/family/new-with-head", test_family_new_with_head);

    /* Agent manager tests */
    g_test_add ("/agent/manager/new",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_new, manager_fixture_tear_down);
    g_test_add ("/agent/manager/add-agent",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_add_agent, manager_fixture_tear_down);
    g_test_add ("/agent/manager/get-by-id",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_get_by_id, manager_fixture_tear_down);
    g_test_add ("/agent/manager/remove-agent",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_remove_agent, manager_fixture_tear_down);
    g_test_add ("/agent/manager/get-by-type",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_get_agents_by_type, manager_fixture_tear_down);
    g_test_add ("/agent/manager/get-available",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_get_available_agents, manager_fixture_tear_down);
    g_test_add ("/agent/manager/statistics",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_statistics, manager_fixture_tear_down);
    g_test_add ("/agent/manager/reset",
                ManagerFixture, NULL,
                manager_fixture_set_up, test_manager_reset, manager_fixture_tear_down);

    return g_test_run ();
}
