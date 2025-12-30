/* test-game-data.c - Game Data Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <glib.h>
#include <libregnum.h>
#include "core/lp-game-data.h"
#include "investment/lp-portfolio.h"
#include "agent/lp-agent-manager.h"
#include "core/lp-phylactery.h"
#include "core/lp-ledger.h"
#include "simulation/lp-world-simulation.h"

/* ==========================================================================
 * Test Fixtures
 * ========================================================================== */

typedef struct
{
    LpGameData *game_data;
} GameDataFixture;

static void
fixture_set_up (GameDataFixture *fixture,
                gconstpointer    user_data)
{
    (void)user_data;

    fixture->game_data = lp_game_data_new ();
    g_assert_nonnull (fixture->game_data);
}

static void
fixture_tear_down (GameDataFixture *fixture,
                   gconstpointer    user_data)
{
    (void)user_data;

    g_clear_object (&fixture->game_data);
}

/* ==========================================================================
 * Tests
 * ========================================================================== */

static void
test_game_data_new (GameDataFixture *fixture,
                    gconstpointer    user_data)
{
    (void)user_data;

    g_assert_true (LP_IS_GAME_DATA (fixture->game_data));
}

static void
test_game_data_saveable_interface (GameDataFixture *fixture,
                                   gconstpointer    user_data)
{
    (void)user_data;

    g_assert_true (LRG_IS_SAVEABLE (fixture->game_data));
}

static void
test_game_data_save_id (GameDataFixture *fixture,
                        gconstpointer    user_data)
{
    const gchar *save_id;

    (void)user_data;

    save_id = lrg_saveable_get_save_id (LRG_SAVEABLE (fixture->game_data));
    g_assert_cmpstr (save_id, ==, "game-data");
}

static void
test_game_data_default_year (GameDataFixture *fixture,
                             gconstpointer    user_data)
{
    guint64 year;

    (void)user_data;

    year = lp_game_data_get_current_year (fixture->game_data);
    g_assert_cmpuint (year, ==, 847);
}

static void
test_game_data_set_year_via_world (GameDataFixture *fixture,
                                   gconstpointer    user_data)
{
    LpWorldSimulation *world;
    guint64            year;

    (void)user_data;

    /* Year is set via the world simulation, not game data directly */
    world = lp_game_data_get_world_simulation (fixture->game_data);
    lp_world_simulation_set_current_year (world, 1000);

    year = lp_game_data_get_current_year (fixture->game_data);
    g_assert_cmpuint (year, ==, 1000);
}

static void
test_game_data_total_years (GameDataFixture *fixture,
                            gconstpointer    user_data)
{
    guint64 total;

    (void)user_data;

    /* Initially 0 */
    total = lp_game_data_get_total_years_played (fixture->game_data);
    g_assert_cmpuint (total, ==, 0);

    /* Total years is incremented via slumber, so just verify initial value */
    /* (We can't test setter since it doesn't exist - internal property) */
}

static void
test_game_data_portfolio (GameDataFixture *fixture,
                          gconstpointer    user_data)
{
    LpPortfolio *portfolio;

    (void)user_data;

    portfolio = lp_game_data_get_portfolio (fixture->game_data);
    g_assert_nonnull (portfolio);
    g_assert_true (LP_IS_PORTFOLIO (portfolio));
}

static void
test_game_data_agent_manager (GameDataFixture *fixture,
                              gconstpointer    user_data)
{
    LpAgentManager *agent_manager;

    (void)user_data;

    agent_manager = lp_game_data_get_agent_manager (fixture->game_data);
    g_assert_nonnull (agent_manager);
    g_assert_true (LP_IS_AGENT_MANAGER (agent_manager));
}

static void
test_game_data_phylactery (GameDataFixture *fixture,
                           gconstpointer    user_data)
{
    LpPhylactery *phylactery;

    (void)user_data;

    phylactery = lp_game_data_get_phylactery (fixture->game_data);
    g_assert_nonnull (phylactery);
    g_assert_true (LP_IS_PHYLACTERY (phylactery));
}

static void
test_game_data_ledger (GameDataFixture *fixture,
                       gconstpointer    user_data)
{
    LpLedger *ledger;

    (void)user_data;

    ledger = lp_game_data_get_ledger (fixture->game_data);
    g_assert_nonnull (ledger);
    g_assert_true (LP_IS_LEDGER (ledger));
}

static void
test_game_data_world (GameDataFixture *fixture,
                      gconstpointer    user_data)
{
    LpWorldSimulation *world;

    (void)user_data;

    world = lp_game_data_get_world_simulation (fixture->game_data);
    g_assert_nonnull (world);
    g_assert_true (LP_IS_WORLD_SIMULATION (world));
}

static void
test_game_data_child_objects_different (GameDataFixture *fixture,
                                        gconstpointer    user_data)
{
    LpPortfolio       *portfolio;
    LpAgentManager    *agent_manager;
    LpPhylactery      *phylactery;
    LpLedger          *ledger;
    LpWorldSimulation *world;

    (void)user_data;

    /* Verify all child objects are different instances */
    portfolio = lp_game_data_get_portfolio (fixture->game_data);
    agent_manager = lp_game_data_get_agent_manager (fixture->game_data);
    phylactery = lp_game_data_get_phylactery (fixture->game_data);
    ledger = lp_game_data_get_ledger (fixture->game_data);
    world = lp_game_data_get_world_simulation (fixture->game_data);

    g_assert_true ((gpointer)portfolio != (gpointer)agent_manager);
    g_assert_true ((gpointer)portfolio != (gpointer)phylactery);
    g_assert_true ((gpointer)portfolio != (gpointer)ledger);
    g_assert_true ((gpointer)portfolio != (gpointer)world);
    g_assert_true ((gpointer)agent_manager != (gpointer)phylactery);
}

static void
test_game_data_multiple_instances (void)
{
    g_autoptr(LpGameData) data1 = NULL;
    g_autoptr(LpGameData) data2 = NULL;

    data1 = lp_game_data_new ();
    data2 = lp_game_data_new ();

    /* Different instances should have different addresses */
    g_assert_true (data1 != data2);

    /* But same default values */
    g_assert_cmpuint (lp_game_data_get_current_year (data1), ==,
                      lp_game_data_get_current_year (data2));
}

/* ==========================================================================
 * Test Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/game-data/new",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_new, fixture_tear_down);

    g_test_add ("/game-data/saveable-interface",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_saveable_interface, fixture_tear_down);

    g_test_add ("/game-data/save-id",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_save_id, fixture_tear_down);

    g_test_add ("/game-data/default-year",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_default_year, fixture_tear_down);

    g_test_add ("/game-data/set-year-via-world",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_set_year_via_world, fixture_tear_down);

    g_test_add ("/game-data/total-years",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_total_years, fixture_tear_down);

    g_test_add ("/game-data/portfolio",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_portfolio, fixture_tear_down);

    g_test_add ("/game-data/agent-manager",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_agent_manager, fixture_tear_down);

    g_test_add ("/game-data/phylactery",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_phylactery, fixture_tear_down);

    g_test_add ("/game-data/ledger",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_ledger, fixture_tear_down);

    g_test_add ("/game-data/world",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_world, fixture_tear_down);

    g_test_add ("/game-data/child-objects-different",
                GameDataFixture, NULL,
                fixture_set_up, test_game_data_child_objects_different, fixture_tear_down);

    g_test_add_func ("/game-data/multiple-instances",
                     test_game_data_multiple_instances);

    return g_test_run ();
}
