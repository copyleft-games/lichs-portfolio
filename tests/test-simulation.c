/* test-simulation.c - World Simulation Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests the Phase 4 world simulation system:
 * - LpRegion
 * - LpKingdom
 * - LpEvent (base class and subclasses)
 * - LpEventGenerator
 * - LpCompetitor
 * - LpWorldSimulation (integration)
 */

#include <glib.h>
#include <libregnum.h>
#include "simulation/lp-region.h"
#include "simulation/lp-kingdom.h"
#include "simulation/lp-event.h"
#include "simulation/lp-event-economic.h"
#include "simulation/lp-event-political.h"
#include "simulation/lp-event-magical.h"
#include "simulation/lp-event-personal.h"
#include "simulation/lp-event-generator.h"
#include "simulation/lp-competitor.h"
#include "simulation/lp-world-simulation.h"

/* ==========================================================================
 * Region Fixture
 * ========================================================================== */

typedef struct
{
    LpRegion *region;
} RegionFixture;

static void
region_fixture_set_up (RegionFixture *fixture,
                       gconstpointer  user_data)
{
    (void)user_data;

    fixture->region = lp_region_new ("region-001", "Northern Plains", LP_GEOGRAPHY_TYPE_INLAND);
    g_assert_nonnull (fixture->region);
}

static void
region_fixture_tear_down (RegionFixture *fixture,
                          gconstpointer  user_data)
{
    (void)user_data;

    g_clear_object (&fixture->region);
}

/* ==========================================================================
 * Kingdom Fixture
 * ========================================================================== */

typedef struct
{
    LpKingdom *kingdom;
} KingdomFixture;

static void
kingdom_fixture_set_up (KingdomFixture *fixture,
                        gconstpointer   user_data)
{
    (void)user_data;

    fixture->kingdom = lp_kingdom_new ("kingdom-001", "Valdoria");
    g_assert_nonnull (fixture->kingdom);
}

static void
kingdom_fixture_tear_down (KingdomFixture *fixture,
                           gconstpointer   user_data)
{
    (void)user_data;

    g_clear_object (&fixture->kingdom);
}

/* ==========================================================================
 * Event Fixture
 * ========================================================================== */

typedef struct
{
    LpEventEconomic *economic;
    LpEventPolitical *political;
    LpEventMagical *magical;
    LpEventPersonal *personal;
} EventFixture;

static void
event_fixture_set_up (EventFixture *fixture,
                      gconstpointer user_data)
{
    (void)user_data;

    fixture->economic = lp_event_economic_new ("econ-001", "Trade Fair");
    fixture->political = lp_event_political_new ("poli-001", "Royal Decree");
    fixture->magical = lp_event_magical_new ("magi-001", "Strange Lights");
    fixture->personal = lp_event_personal_new ("pers-001", "Agent Report");

    g_assert_nonnull (fixture->economic);
    g_assert_nonnull (fixture->political);
    g_assert_nonnull (fixture->magical);
    g_assert_nonnull (fixture->personal);
}

static void
event_fixture_tear_down (EventFixture *fixture,
                         gconstpointer user_data)
{
    (void)user_data;

    g_clear_object (&fixture->economic);
    g_clear_object (&fixture->political);
    g_clear_object (&fixture->magical);
    g_clear_object (&fixture->personal);
}

/* ==========================================================================
 * Competitor Fixture
 * ========================================================================== */

typedef struct
{
    LpCompetitor *competitor;
} CompetitorFixture;

static void
competitor_fixture_set_up (CompetitorFixture *fixture,
                           gconstpointer      user_data)
{
    (void)user_data;

    fixture->competitor = lp_competitor_new ("comp-001", "Drakorath",
                                              LP_COMPETITOR_TYPE_DRAGON);
    g_assert_nonnull (fixture->competitor);
}

static void
competitor_fixture_tear_down (CompetitorFixture *fixture,
                              gconstpointer      user_data)
{
    (void)user_data;

    g_clear_object (&fixture->competitor);
}

/* ==========================================================================
 * World Simulation Fixture
 * ========================================================================== */

typedef struct
{
    LpWorldSimulation *simulation;
} SimulationFixture;

static void
simulation_fixture_set_up (SimulationFixture *fixture,
                           gconstpointer      user_data)
{
    (void)user_data;

    fixture->simulation = lp_world_simulation_new ();
    g_assert_nonnull (fixture->simulation);
}

static void
simulation_fixture_tear_down (SimulationFixture *fixture,
                              gconstpointer      user_data)
{
    (void)user_data;

    g_clear_object (&fixture->simulation);
}

/* ==========================================================================
 * Region Tests
 * ========================================================================== */

static void
test_region_new (RegionFixture *fixture,
                 gconstpointer  user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_REGION (fixture->region));
    g_assert_true (LRG_IS_SAVEABLE (fixture->region));
}

static void
test_region_id (RegionFixture *fixture,
                gconstpointer  user_data)
{
    const gchar *id;

    (void)user_data;

    id = lp_region_get_id (fixture->region);
    g_assert_cmpstr (id, ==, "region-001");
}

static void
test_region_name (RegionFixture *fixture,
                  gconstpointer  user_data)
{
    const gchar *name;

    (void)user_data;

    name = lp_region_get_name (fixture->region);
    g_assert_cmpstr (name, ==, "Northern Plains");

    lp_region_set_name (fixture->region, "Southern Hills");
    name = lp_region_get_name (fixture->region);
    g_assert_cmpstr (name, ==, "Southern Hills");
}

static void
test_region_geography (RegionFixture *fixture,
                       gconstpointer  user_data)
{
    LpGeographyType geography;

    (void)user_data;

    /* Geography type is set at construction time (construct-only) */
    geography = lp_region_get_geography_type (fixture->region);
    g_assert_cmpint (geography, ==, LP_GEOGRAPHY_TYPE_INLAND);
}

static void
test_region_population (RegionFixture *fixture,
                        gconstpointer  user_data)
{
    guint population;

    (void)user_data;

    lp_region_set_population (fixture->region, 50000);
    population = lp_region_get_population (fixture->region);
    g_assert_cmpuint (population, ==, 50000);
}

static void
test_region_trade_routes (RegionFixture *fixture,
                          gconstpointer  user_data)
{
    gboolean has;

    (void)user_data;

    lp_region_add_trade_route (fixture->region, "route-001");
    has = lp_region_has_trade_route_to (fixture->region, "route-001");
    g_assert_true (has);

    has = lp_region_has_trade_route_to (fixture->region, "nonexistent");
    g_assert_false (has);

    lp_region_remove_trade_route (fixture->region, "route-001");
    has = lp_region_has_trade_route_to (fixture->region, "route-001");
    g_assert_false (has);
}

static void
test_region_geography_bonuses (RegionFixture *fixture,
                               gconstpointer  user_data)
{
    g_autoptr(LpRegion) coastal = NULL;
    g_autoptr(LpRegion) mountain = NULL;
    g_autoptr(LpRegion) swamp = NULL;
    gdouble trade_bonus;
    gdouble resource_bonus;
    gdouble conceal_bonus;

    (void)user_data;
    (void)fixture;

    /* Coastal regions have trade bonus */
    coastal = lp_region_new ("coastal", "Coastal Region", LP_GEOGRAPHY_TYPE_COASTAL);
    trade_bonus = lp_region_get_geography_trade_bonus (coastal);
    g_assert_cmpfloat (trade_bonus, >, 1.0);

    /* Mountain regions have resource bonus */
    mountain = lp_region_new ("mountain", "Mountain Region", LP_GEOGRAPHY_TYPE_MOUNTAIN);
    resource_bonus = lp_region_get_geography_resource_bonus (mountain);
    g_assert_cmpfloat (resource_bonus, >, 1.0);

    /* Swamp regions have concealment bonus */
    swamp = lp_region_new ("swamp", "Swamp Region", LP_GEOGRAPHY_TYPE_SWAMP);
    conceal_bonus = lp_region_get_geography_concealment_bonus (swamp);
    g_assert_cmpfloat (conceal_bonus, >, 1.0);
}

/* ==========================================================================
 * Kingdom Tests
 * ========================================================================== */

static void
test_kingdom_new (KingdomFixture *fixture,
                  gconstpointer   user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_KINGDOM (fixture->kingdom));
    g_assert_true (LRG_IS_SAVEABLE (fixture->kingdom));
}

static void
test_kingdom_id (KingdomFixture *fixture,
                 gconstpointer   user_data)
{
    const gchar *id;

    (void)user_data;

    id = lp_kingdom_get_id (fixture->kingdom);
    g_assert_cmpstr (id, ==, "kingdom-001");
}

static void
test_kingdom_core_attributes (KingdomFixture *fixture,
                              gconstpointer   user_data)
{
    gint stability;
    gint prosperity;
    gint military;
    gint culture;
    gint tolerance;

    (void)user_data;

    lp_kingdom_set_stability (fixture->kingdom, 75);
    stability = lp_kingdom_get_stability (fixture->kingdom);
    g_assert_cmpint (stability, ==, 75);

    lp_kingdom_set_prosperity (fixture->kingdom, 60);
    prosperity = lp_kingdom_get_prosperity (fixture->kingdom);
    g_assert_cmpint (prosperity, ==, 60);

    lp_kingdom_set_military (fixture->kingdom, 80);
    military = lp_kingdom_get_military (fixture->kingdom);
    g_assert_cmpint (military, ==, 80);

    lp_kingdom_set_culture (fixture->kingdom, 70);
    culture = lp_kingdom_get_culture (fixture->kingdom);
    g_assert_cmpint (culture, ==, 70);

    lp_kingdom_set_tolerance (fixture->kingdom, 40);
    tolerance = lp_kingdom_get_tolerance (fixture->kingdom);
    g_assert_cmpint (tolerance, ==, 40);
}

static void
test_kingdom_attribute_clamping (KingdomFixture *fixture,
                                 gconstpointer   user_data)
{
    gint stability;

    (void)user_data;

    /* Test clamping to 0-100 */
    lp_kingdom_set_stability (fixture->kingdom, 150);
    stability = lp_kingdom_get_stability (fixture->kingdom);
    g_assert_cmpint (stability, ==, 100);

    lp_kingdom_set_stability (fixture->kingdom, -50);
    stability = lp_kingdom_get_stability (fixture->kingdom);
    g_assert_cmpint (stability, ==, 0);
}

static void
test_kingdom_ruler (KingdomFixture *fixture,
                    gconstpointer   user_data)
{
    const gchar *ruler;

    (void)user_data;

    lp_kingdom_set_ruler_name (fixture->kingdom, "King Edmund III");
    ruler = lp_kingdom_get_ruler_name (fixture->kingdom);
    g_assert_cmpstr (ruler, ==, "King Edmund III");
}

static void
test_kingdom_dynasty_years (KingdomFixture *fixture,
                            gconstpointer   user_data)
{
    guint years;

    (void)user_data;

    lp_kingdom_set_dynasty_years (fixture->kingdom, 150);
    years = lp_kingdom_get_dynasty_years (fixture->kingdom);
    g_assert_cmpuint (years, ==, 150);
}

static void
test_kingdom_relations (KingdomFixture *fixture,
                        gconstpointer   user_data)
{
    LpKingdomRelation relation;

    (void)user_data;

    lp_kingdom_set_relation (fixture->kingdom, "other-kingdom", LP_KINGDOM_RELATION_ALLIANCE);
    relation = lp_kingdom_get_relation (fixture->kingdom, "other-kingdom");
    g_assert_cmpint (relation, ==, LP_KINGDOM_RELATION_ALLIANCE);

    /* Unknown kingdom should be neutral */
    relation = lp_kingdom_get_relation (fixture->kingdom, "unknown");
    g_assert_cmpint (relation, ==, LP_KINGDOM_RELATION_NEUTRAL);
}

static void
test_kingdom_tick_year (KingdomFixture *fixture,
                        gconstpointer   user_data)
{
    gint stability_before;
    guint dynasty_before;

    (void)user_data;

    stability_before = lp_kingdom_get_stability (fixture->kingdom);
    dynasty_before = lp_kingdom_get_dynasty_years (fixture->kingdom);

    lp_kingdom_tick_year (fixture->kingdom);

    /* Dynasty years should increase */
    g_assert_cmpuint (lp_kingdom_get_dynasty_years (fixture->kingdom),
                      ==, dynasty_before + 1);

    /* Stability might drift slightly */
    (void)stability_before;  /* May or may not change due to randomness */
}

/* ==========================================================================
 * Event Tests
 * ========================================================================== */

static void
test_event_economic_new (EventFixture *fixture,
                         gconstpointer user_data)
{
    LpEventType type;

    (void)user_data;

    g_assert_true (LP_IS_EVENT_ECONOMIC (fixture->economic));
    g_assert_true (LP_IS_EVENT (fixture->economic));
    g_assert_true (LRG_IS_SAVEABLE (fixture->economic));

    type = lp_event_get_event_type (LP_EVENT (fixture->economic));
    g_assert_cmpint (type, ==, LP_EVENT_TYPE_ECONOMIC);
}

static void
test_event_economic_modifier (EventFixture *fixture,
                              gconstpointer user_data)
{
    gdouble modifier;

    (void)user_data;

    lp_event_economic_set_market_modifier (fixture->economic, 1.25);
    modifier = lp_event_economic_get_market_modifier (fixture->economic);
    g_assert_cmpfloat (modifier, ==, 1.25);
}

static void
test_event_political_new (EventFixture *fixture,
                          gconstpointer user_data)
{
    LpEventType type;

    (void)user_data;

    g_assert_true (LP_IS_EVENT_POLITICAL (fixture->political));
    g_assert_true (LP_IS_EVENT (fixture->political));

    type = lp_event_get_event_type (LP_EVENT (fixture->political));
    g_assert_cmpint (type, ==, LP_EVENT_TYPE_POLITICAL);
}

static void
test_event_political_stability_impact (EventFixture *fixture,
                                       gconstpointer user_data)
{
    gint impact;

    (void)user_data;

    lp_event_political_set_stability_impact (fixture->political, -25);
    impact = lp_event_political_get_stability_impact (fixture->political);
    g_assert_cmpint (impact, ==, -25);
}

static void
test_event_political_war (EventFixture *fixture,
                          gconstpointer user_data)
{
    gboolean causes_war;

    (void)user_data;

    lp_event_political_set_causes_war (fixture->political, TRUE);
    causes_war = lp_event_political_get_causes_war (fixture->political);
    g_assert_true (causes_war);
}

static void
test_event_magical_new (EventFixture *fixture,
                        gconstpointer user_data)
{
    LpEventType type;

    (void)user_data;

    g_assert_true (LP_IS_EVENT_MAGICAL (fixture->magical));
    g_assert_true (LP_IS_EVENT (fixture->magical));

    type = lp_event_get_event_type (LP_EVENT (fixture->magical));
    g_assert_cmpint (type, ==, LP_EVENT_TYPE_MAGICAL);
}

static void
test_event_magical_exposure (EventFixture *fixture,
                             gconstpointer user_data)
{
    gint exposure;

    (void)user_data;

    lp_event_magical_set_exposure_impact (fixture->magical, 30);
    exposure = lp_event_magical_get_exposure_impact (fixture->magical);
    g_assert_cmpint (exposure, ==, 30);
}

static void
test_event_personal_new (EventFixture *fixture,
                         gconstpointer user_data)
{
    LpEventType type;

    (void)user_data;

    g_assert_true (LP_IS_EVENT_PERSONAL (fixture->personal));
    g_assert_true (LP_IS_EVENT (fixture->personal));

    type = lp_event_get_event_type (LP_EVENT (fixture->personal));
    g_assert_cmpint (type, ==, LP_EVENT_TYPE_PERSONAL);
}

static void
test_event_personal_betrayal (EventFixture *fixture,
                              gconstpointer user_data)
{
    gboolean is_betrayal;

    (void)user_data;

    lp_event_personal_set_is_betrayal (fixture->personal, TRUE);
    is_betrayal = lp_event_personal_get_is_betrayal (fixture->personal);
    g_assert_true (is_betrayal);
}

static void
test_event_personal_death (EventFixture *fixture,
                           gconstpointer user_data)
{
    gboolean is_death;

    (void)user_data;

    lp_event_personal_set_is_death (fixture->personal, TRUE);
    is_death = lp_event_personal_get_is_death (fixture->personal);
    g_assert_true (is_death);
}

static void
test_event_severity (EventFixture *fixture,
                     gconstpointer user_data)
{
    LpEventSeverity severity;

    (void)user_data;

    lp_event_set_severity (LP_EVENT (fixture->economic), LP_EVENT_SEVERITY_MAJOR);
    severity = lp_event_get_severity (LP_EVENT (fixture->economic));
    g_assert_cmpint (severity, ==, LP_EVENT_SEVERITY_MAJOR);
}

static void
test_event_duration (EventFixture *fixture,
                     gconstpointer user_data)
{
    guint duration;

    (void)user_data;

    lp_event_set_duration_years (LP_EVENT (fixture->political), 5);
    duration = lp_event_get_duration_years (LP_EVENT (fixture->political));
    g_assert_cmpuint (duration, ==, 5);
}

/* ==========================================================================
 * Event Generator Tests
 * ========================================================================== */

static void
test_event_generator_singleton (void)
{
    LpEventGenerator *gen1;
    LpEventGenerator *gen2;

    gen1 = lp_event_generator_get_default ();
    gen2 = lp_event_generator_get_default ();

    g_assert_nonnull (gen1);
    g_assert_true (gen1 == gen2);
}

static void
test_event_generator_chances (void)
{
    LpEventGenerator *gen;
    gdouble yearly;
    gdouble decade;
    gdouble era;

    gen = lp_event_generator_get_default ();

    yearly = lp_event_generator_get_base_yearly_event_chance (gen);
    g_assert_cmpfloat (yearly, >=, 0.0);
    g_assert_cmpfloat (yearly, <=, 1.0);

    decade = lp_event_generator_get_base_decade_event_chance (gen);
    g_assert_cmpfloat (decade, >=, 0.0);
    g_assert_cmpfloat (decade, <=, 1.0);

    era = lp_event_generator_get_base_era_event_chance (gen);
    g_assert_cmpfloat (era, >=, 0.0);
    g_assert_cmpfloat (era, <=, 1.0);
}

static void
test_event_generator_create_economic (void)
{
    LpEventGenerator *gen;
    g_autoptr(LpEvent) event = NULL;

    gen = lp_event_generator_get_default ();
    event = lp_event_generator_create_economic_event (gen, LP_EVENT_SEVERITY_MODERATE);

    g_assert_nonnull (event);
    g_assert_true (LP_IS_EVENT_ECONOMIC (event));
    g_assert_cmpint (lp_event_get_severity (event), ==, LP_EVENT_SEVERITY_MODERATE);
}

static void
test_event_generator_create_political (void)
{
    LpEventGenerator *gen;
    g_autoptr(LpEvent) event = NULL;

    gen = lp_event_generator_get_default ();
    event = lp_event_generator_create_political_event (gen, LP_EVENT_SEVERITY_MAJOR);

    g_assert_nonnull (event);
    g_assert_true (LP_IS_EVENT_POLITICAL (event));
}

static void
test_event_generator_create_magical (void)
{
    LpEventGenerator *gen;
    g_autoptr(LpEvent) event = NULL;

    gen = lp_event_generator_get_default ();
    event = lp_event_generator_create_magical_event (gen, LP_EVENT_SEVERITY_MINOR);

    g_assert_nonnull (event);
    g_assert_true (LP_IS_EVENT_MAGICAL (event));
}

static void
test_event_generator_create_personal (void)
{
    LpEventGenerator *gen;
    g_autoptr(LpEvent) event = NULL;

    gen = lp_event_generator_get_default ();
    event = lp_event_generator_create_personal_event (gen, LP_EVENT_SEVERITY_MODERATE);

    g_assert_nonnull (event);
    g_assert_true (LP_IS_EVENT_PERSONAL (event));
}

/* ==========================================================================
 * Competitor Tests
 * ========================================================================== */

static void
test_competitor_new (CompetitorFixture *fixture,
                     gconstpointer      user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_COMPETITOR (fixture->competitor));
    g_assert_true (LRG_IS_SAVEABLE (fixture->competitor));
}

static void
test_competitor_type (CompetitorFixture *fixture,
                      gconstpointer      user_data)
{
    LpCompetitorType type;

    (void)user_data;

    type = lp_competitor_get_competitor_type (fixture->competitor);
    g_assert_cmpint (type, ==, LP_COMPETITOR_TYPE_DRAGON);
}

static void
test_competitor_stance (CompetitorFixture *fixture,
                        gconstpointer      user_data)
{
    LpCompetitorStance stance;

    (void)user_data;

    /* Default should be unknown */
    stance = lp_competitor_get_stance (fixture->competitor);
    g_assert_cmpint (stance, ==, LP_COMPETITOR_STANCE_UNKNOWN);

    lp_competitor_set_stance (fixture->competitor, LP_COMPETITOR_STANCE_HOSTILE);
    stance = lp_competitor_get_stance (fixture->competitor);
    g_assert_cmpint (stance, ==, LP_COMPETITOR_STANCE_HOSTILE);
}

static void
test_competitor_traits (CompetitorFixture *fixture,
                        gconstpointer      user_data)
{
    gint power;
    gint aggression;
    gint greed;
    gint cunning;

    (void)user_data;

    lp_competitor_set_power_level (fixture->competitor, 80);
    power = lp_competitor_get_power_level (fixture->competitor);
    g_assert_cmpint (power, ==, 80);

    lp_competitor_set_aggression (fixture->competitor, 70);
    aggression = lp_competitor_get_aggression (fixture->competitor);
    g_assert_cmpint (aggression, ==, 70);

    lp_competitor_set_greed (fixture->competitor, 60);
    greed = lp_competitor_get_greed (fixture->competitor);
    g_assert_cmpint (greed, ==, 60);

    lp_competitor_set_cunning (fixture->competitor, 90);
    cunning = lp_competitor_get_cunning (fixture->competitor);
    g_assert_cmpint (cunning, ==, 90);
}

static void
test_competitor_territory (CompetitorFixture *fixture,
                           gconstpointer      user_data)
{
    gboolean has;

    (void)user_data;

    lp_competitor_add_territory (fixture->competitor, "region-001");
    has = lp_competitor_has_territory (fixture->competitor, "region-001");
    g_assert_true (has);

    has = lp_competitor_has_territory (fixture->competitor, "nonexistent");
    g_assert_false (has);

    lp_competitor_remove_territory (fixture->competitor, "region-001");
    has = lp_competitor_has_territory (fixture->competitor, "region-001");
    g_assert_false (has);
}

static void
test_competitor_discovery (CompetitorFixture *fixture,
                           gconstpointer      user_data)
{
    gboolean is_known;

    (void)user_data;

    is_known = lp_competitor_get_is_known (fixture->competitor);
    g_assert_false (is_known);

    lp_competitor_discover (fixture->competitor);
    is_known = lp_competitor_get_is_known (fixture->competitor);
    g_assert_true (is_known);
}

static void
test_competitor_active (CompetitorFixture *fixture,
                        gconstpointer      user_data)
{
    gboolean is_active;

    (void)user_data;

    is_active = lp_competitor_get_is_active (fixture->competitor);
    g_assert_true (is_active);

    lp_competitor_destroy (fixture->competitor);
    is_active = lp_competitor_get_is_active (fixture->competitor);
    g_assert_false (is_active);
}

/* ==========================================================================
 * World Simulation Tests
 * ========================================================================== */

static void
test_simulation_new (SimulationFixture *fixture,
                     gconstpointer      user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_WORLD_SIMULATION (fixture->simulation));
    g_assert_true (LRG_IS_SAVEABLE (fixture->simulation));
}

static void
test_simulation_year (SimulationFixture *fixture,
                      gconstpointer      user_data)
{
    guint64 year;

    (void)user_data;

    year = lp_world_simulation_get_current_year (fixture->simulation);
    g_assert_cmpuint (year, ==, 847);

    lp_world_simulation_set_current_year (fixture->simulation, 900);
    year = lp_world_simulation_get_current_year (fixture->simulation);
    g_assert_cmpuint (year, ==, 900);
}

static void
test_simulation_add_kingdom (SimulationFixture *fixture,
                             gconstpointer      user_data)
{
    LpKingdom *kingdom;
    guint count;

    (void)user_data;

    kingdom = lp_kingdom_new ("sim-kingdom", "Test Kingdom");
    lp_world_simulation_add_kingdom (fixture->simulation, kingdom);

    count = lp_world_simulation_get_kingdom_count (fixture->simulation);
    g_assert_cmpuint (count, ==, 1);
}

static void
test_simulation_get_kingdom_by_id (SimulationFixture *fixture,
                                   gconstpointer      user_data)
{
    LpKingdom *kingdom;
    LpKingdom *found;

    (void)user_data;

    kingdom = lp_kingdom_new ("find-kingdom", "Find Kingdom");
    lp_world_simulation_add_kingdom (fixture->simulation, kingdom);

    found = lp_world_simulation_get_kingdom_by_id (fixture->simulation, "find-kingdom");
    g_assert_nonnull (found);
    g_assert_true (kingdom == found);

    found = lp_world_simulation_get_kingdom_by_id (fixture->simulation, "not-found");
    g_assert_null (found);
}

static void
test_simulation_add_region (SimulationFixture *fixture,
                            gconstpointer      user_data)
{
    LpRegion *region;
    guint count;

    (void)user_data;

    region = lp_region_new ("sim-region", "Test Region", LP_GEOGRAPHY_TYPE_INLAND);
    lp_world_simulation_add_region (fixture->simulation, region);

    count = lp_world_simulation_get_region_count (fixture->simulation);
    g_assert_cmpuint (count, ==, 1);
}

static void
test_simulation_add_competitor (SimulationFixture *fixture,
                                gconstpointer      user_data)
{
    LpCompetitor *competitor;
    guint count;

    (void)user_data;

    competitor = lp_competitor_new ("sim-comp", "Test Competitor", LP_COMPETITOR_TYPE_VAMPIRE);
    lp_world_simulation_add_competitor (fixture->simulation, competitor);

    count = lp_world_simulation_get_competitor_count (fixture->simulation);
    g_assert_cmpuint (count, ==, 1);
}

static void
test_simulation_advance_year (SimulationFixture *fixture,
                              gconstpointer      user_data)
{
    guint64 year_before;
    guint64 year_after;
    g_autolist(LpEvent) events = NULL;

    (void)user_data;

    year_before = lp_world_simulation_get_current_year (fixture->simulation);

    events = lp_world_simulation_advance_year (fixture->simulation);

    year_after = lp_world_simulation_get_current_year (fixture->simulation);
    g_assert_cmpuint (year_after, ==, year_before + 1);
}

static void
test_simulation_advance_years (SimulationFixture *fixture,
                               gconstpointer      user_data)
{
    guint64 year_before;
    guint64 year_after;
    g_autolist(LpEvent) events = NULL;

    (void)user_data;

    year_before = lp_world_simulation_get_current_year (fixture->simulation);

    events = lp_world_simulation_advance_years (fixture->simulation, 10);

    year_after = lp_world_simulation_get_current_year (fixture->simulation);
    g_assert_cmpuint (year_after, ==, year_before + 10);
}

static void
test_simulation_economic_cycle (SimulationFixture *fixture,
                                gconstpointer      user_data)
{
    guint phase;
    gdouble rate;

    (void)user_data;

    phase = lp_world_simulation_get_economic_cycle_phase (fixture->simulation);
    g_assert_cmpuint (phase, <=, 3);

    rate = lp_world_simulation_get_base_growth_rate (fixture->simulation);
    g_assert_cmpfloat (rate, >=, 0.9);
    g_assert_cmpfloat (rate, <=, 1.1);
}

static void
test_simulation_reset (SimulationFixture *fixture,
                       gconstpointer      user_data)
{
    guint64 year;
    guint count;

    (void)user_data;

    /* Add some data */
    lp_world_simulation_add_kingdom (fixture->simulation,
        lp_kingdom_new ("reset-kingdom", "Reset Kingdom"));
    lp_world_simulation_advance_years (fixture->simulation, 50);

    /* Reset */
    lp_world_simulation_reset (fixture->simulation, 1000);

    year = lp_world_simulation_get_current_year (fixture->simulation);
    g_assert_cmpuint (year, ==, 1000);

    count = lp_world_simulation_get_kingdom_count (fixture->simulation);
    g_assert_cmpuint (count, ==, 0);
}

static void
test_simulation_event_generator (SimulationFixture *fixture,
                                 gconstpointer      user_data)
{
    LpEventGenerator *gen;

    (void)user_data;

    gen = lp_world_simulation_get_event_generator (fixture->simulation);
    g_assert_nonnull (gen);
    g_assert_true (LP_IS_EVENT_GENERATOR (gen));
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Region tests */
    g_test_add ("/simulation/region/new",
                RegionFixture, NULL,
                region_fixture_set_up, test_region_new, region_fixture_tear_down);
    g_test_add ("/simulation/region/id",
                RegionFixture, NULL,
                region_fixture_set_up, test_region_id, region_fixture_tear_down);
    g_test_add ("/simulation/region/name",
                RegionFixture, NULL,
                region_fixture_set_up, test_region_name, region_fixture_tear_down);
    g_test_add ("/simulation/region/geography",
                RegionFixture, NULL,
                region_fixture_set_up, test_region_geography, region_fixture_tear_down);
    g_test_add ("/simulation/region/population",
                RegionFixture, NULL,
                region_fixture_set_up, test_region_population, region_fixture_tear_down);
    g_test_add ("/simulation/region/trade-routes",
                RegionFixture, NULL,
                region_fixture_set_up, test_region_trade_routes, region_fixture_tear_down);
    g_test_add ("/simulation/region/geography-bonuses",
                RegionFixture, NULL,
                region_fixture_set_up, test_region_geography_bonuses, region_fixture_tear_down);

    /* Kingdom tests */
    g_test_add ("/simulation/kingdom/new",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_new, kingdom_fixture_tear_down);
    g_test_add ("/simulation/kingdom/id",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_id, kingdom_fixture_tear_down);
    g_test_add ("/simulation/kingdom/core-attributes",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_core_attributes, kingdom_fixture_tear_down);
    g_test_add ("/simulation/kingdom/attribute-clamping",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_attribute_clamping, kingdom_fixture_tear_down);
    g_test_add ("/simulation/kingdom/ruler",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_ruler, kingdom_fixture_tear_down);
    g_test_add ("/simulation/kingdom/dynasty-years",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_dynasty_years, kingdom_fixture_tear_down);
    g_test_add ("/simulation/kingdom/relations",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_relations, kingdom_fixture_tear_down);
    g_test_add ("/simulation/kingdom/tick-year",
                KingdomFixture, NULL,
                kingdom_fixture_set_up, test_kingdom_tick_year, kingdom_fixture_tear_down);

    /* Event tests */
    g_test_add ("/simulation/event/economic/new",
                EventFixture, NULL,
                event_fixture_set_up, test_event_economic_new, event_fixture_tear_down);
    g_test_add ("/simulation/event/economic/modifier",
                EventFixture, NULL,
                event_fixture_set_up, test_event_economic_modifier, event_fixture_tear_down);
    g_test_add ("/simulation/event/political/new",
                EventFixture, NULL,
                event_fixture_set_up, test_event_political_new, event_fixture_tear_down);
    g_test_add ("/simulation/event/political/stability-impact",
                EventFixture, NULL,
                event_fixture_set_up, test_event_political_stability_impact, event_fixture_tear_down);
    g_test_add ("/simulation/event/political/war",
                EventFixture, NULL,
                event_fixture_set_up, test_event_political_war, event_fixture_tear_down);
    g_test_add ("/simulation/event/magical/new",
                EventFixture, NULL,
                event_fixture_set_up, test_event_magical_new, event_fixture_tear_down);
    g_test_add ("/simulation/event/magical/exposure",
                EventFixture, NULL,
                event_fixture_set_up, test_event_magical_exposure, event_fixture_tear_down);
    g_test_add ("/simulation/event/personal/new",
                EventFixture, NULL,
                event_fixture_set_up, test_event_personal_new, event_fixture_tear_down);
    g_test_add ("/simulation/event/personal/betrayal",
                EventFixture, NULL,
                event_fixture_set_up, test_event_personal_betrayal, event_fixture_tear_down);
    g_test_add ("/simulation/event/personal/death",
                EventFixture, NULL,
                event_fixture_set_up, test_event_personal_death, event_fixture_tear_down);
    g_test_add ("/simulation/event/severity",
                EventFixture, NULL,
                event_fixture_set_up, test_event_severity, event_fixture_tear_down);
    g_test_add ("/simulation/event/duration",
                EventFixture, NULL,
                event_fixture_set_up, test_event_duration, event_fixture_tear_down);

    /* Event generator tests */
    g_test_add_func ("/simulation/generator/singleton", test_event_generator_singleton);
    g_test_add_func ("/simulation/generator/chances", test_event_generator_chances);
    g_test_add_func ("/simulation/generator/create-economic", test_event_generator_create_economic);
    g_test_add_func ("/simulation/generator/create-political", test_event_generator_create_political);
    g_test_add_func ("/simulation/generator/create-magical", test_event_generator_create_magical);
    g_test_add_func ("/simulation/generator/create-personal", test_event_generator_create_personal);

    /* Competitor tests */
    g_test_add ("/simulation/competitor/new",
                CompetitorFixture, NULL,
                competitor_fixture_set_up, test_competitor_new, competitor_fixture_tear_down);
    g_test_add ("/simulation/competitor/type",
                CompetitorFixture, NULL,
                competitor_fixture_set_up, test_competitor_type, competitor_fixture_tear_down);
    g_test_add ("/simulation/competitor/stance",
                CompetitorFixture, NULL,
                competitor_fixture_set_up, test_competitor_stance, competitor_fixture_tear_down);
    g_test_add ("/simulation/competitor/traits",
                CompetitorFixture, NULL,
                competitor_fixture_set_up, test_competitor_traits, competitor_fixture_tear_down);
    g_test_add ("/simulation/competitor/territory",
                CompetitorFixture, NULL,
                competitor_fixture_set_up, test_competitor_territory, competitor_fixture_tear_down);
    g_test_add ("/simulation/competitor/discovery",
                CompetitorFixture, NULL,
                competitor_fixture_set_up, test_competitor_discovery, competitor_fixture_tear_down);
    g_test_add ("/simulation/competitor/active",
                CompetitorFixture, NULL,
                competitor_fixture_set_up, test_competitor_active, competitor_fixture_tear_down);

    /* World simulation tests */
    g_test_add ("/simulation/world/new",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_new, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/year",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_year, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/add-kingdom",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_add_kingdom, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/get-kingdom-by-id",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_get_kingdom_by_id, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/add-region",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_add_region, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/add-competitor",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_add_competitor, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/advance-year",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_advance_year, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/advance-years",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_advance_years, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/economic-cycle",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_economic_cycle, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/reset",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_reset, simulation_fixture_tear_down);
    g_test_add ("/simulation/world/event-generator",
                SimulationFixture, NULL,
                simulation_fixture_set_up, test_simulation_event_generator, simulation_fixture_tear_down);

    return g_test_run ();
}
