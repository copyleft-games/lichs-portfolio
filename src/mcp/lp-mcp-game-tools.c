/* lp-mcp-game-tools.c - Game-Specific MCP Debugging Tools
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implements MCP tools for inspecting and manipulating game state.
 */

#ifdef LP_MCP

#include "lp-mcp-game-tools.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../core/lp-exposure-manager.h"
#include "../core/lp-synergy-manager.h"
#include "../core/lp-phylactery.h"
#include "../investment/lp-portfolio.h"
#include "../investment/lp-investment.h"
#include "../agent/lp-agent-manager.h"
#include "../agent/lp-agent.h"
#include "../lp-enums.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <mcp.h>

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_MCP
#include "../lp-log.h"

/**
 * SECTION:lp-mcp-game-tools
 * @title: LpMcpGameTools
 * @short_description: MCP tools for Lich's Portfolio debugging
 *
 * #LpMcpGameTools provides MCP tools for inspecting and manipulating
 * game state in Lich's Portfolio. These tools enable AI-assisted
 * testing and debugging.
 */

struct _LpMcpGameTools
{
    LrgMcpToolGroup parent_instance;
};

G_DEFINE_FINAL_TYPE (LpMcpGameTools, lp_mcp_game_tools, LRG_TYPE_MCP_TOOL_GROUP)

/* ==========================================================================
 * Helper: Get Game Data (with error handling)
 * ========================================================================== */

static LpGameData *
get_game_data (GError **error)
{
    LpGame *game;
    LpGameData *data;

    /*
     * Get the current game instance via the static tracker.
     * lp_game_get_from_state() ignores its argument and returns
     * the current_game_instance static variable.
     */
    game = lp_game_get_from_state (NULL);
    if (game == NULL)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "Game not running");
        return NULL;
    }

    data = lp_game_get_game_data (game);
    if (data == NULL)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "No active game session");
        return NULL;
    }

    return data;
}

/* ==========================================================================
 * Helper: JSON Serialization
 * ========================================================================== */

static gchar *
json_builder_to_string (JsonBuilder *builder)
{
    g_autoptr(JsonGenerator) generator = NULL;
    g_autoptr(JsonNode) root = NULL;

    root = json_builder_get_root (builder);
    generator = json_generator_new ();
    json_generator_set_root (generator, root);
    json_generator_set_pretty (generator, TRUE);

    return json_generator_to_data (generator, NULL);
}

/* ==========================================================================
 * Tool Handlers
 * ========================================================================== */

static McpToolResult *
handle_get_game_state (LpMcpGameTools *self,
                       JsonObject     *arguments,
                       GError        **error)
{
    LpGameData *data;
    LpPortfolio *portfolio;
    LpAgentManager *agents;
    LpExposureManager *exposure;
    LpSynergyManager *synergy;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;
    g_autoptr(LrgBigNumber) total_value = NULL;
    g_autoptr(LrgBigNumber) gold = NULL;
    g_autofree gchar *gold_str = NULL;
    g_autofree gchar *total_str = NULL;

    data = get_game_data (error);
    if (data == NULL)
        return NULL;

    portfolio = lp_game_data_get_portfolio (data);
    agents = lp_game_data_get_agent_manager (data);
    exposure = lp_exposure_manager_get_default ();
    synergy = lp_synergy_manager_get_default ();

    total_value = lp_portfolio_get_total_value (portfolio);
    gold = lrg_big_number_copy (lp_portfolio_get_gold (portfolio));
    gold_str = lrg_big_number_format_short (gold);
    total_str = lrg_big_number_format_short (total_value);

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    /* Year info */
    json_builder_set_member_name (builder, "current_year");
    json_builder_add_int_value (builder,
        (gint64) lp_game_data_get_current_year (data));
    json_builder_set_member_name (builder, "total_years_played");
    json_builder_add_int_value (builder,
        (gint64) lp_game_data_get_total_years_played (data));

    /* Portfolio summary */
    json_builder_set_member_name (builder, "gold");
    json_builder_add_string_value (builder, gold_str);
    json_builder_set_member_name (builder, "total_value");
    json_builder_add_string_value (builder, total_str);
    json_builder_set_member_name (builder, "investment_count");
    json_builder_add_int_value (builder,
        (gint64) lp_portfolio_get_investment_count (portfolio));

    /* Agent summary */
    json_builder_set_member_name (builder, "agent_count");
    json_builder_add_int_value (builder,
        (gint64) lp_agent_manager_get_agent_count (agents));
    json_builder_set_member_name (builder, "average_loyalty");
    json_builder_add_int_value (builder,
        lp_agent_manager_get_average_loyalty (agents));

    /* Exposure */
    json_builder_set_member_name (builder, "exposure");
    json_builder_add_int_value (builder,
        (gint64) lp_exposure_manager_get_exposure (exposure));
    json_builder_set_member_name (builder, "exposure_level");
    json_builder_add_int_value (builder,
        (gint64) lp_exposure_manager_get_level (exposure));

    /* Synergies */
    json_builder_set_member_name (builder, "synergy_count");
    json_builder_add_int_value (builder,
        (gint64) lp_synergy_manager_get_synergy_count (synergy));
    json_builder_set_member_name (builder, "synergy_bonus");
    json_builder_add_double_value (builder,
        lp_synergy_manager_get_total_bonus (synergy));

    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

static McpToolResult *
handle_inspect_portfolio (LpMcpGameTools *self,
                          JsonObject     *arguments,
                          GError        **error)
{
    LpGameData *data;
    LpPortfolio *portfolio;
    GPtrArray *investments;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;
    g_autoptr(LrgBigNumber) total_value = NULL;
    g_autoptr(LrgBigNumber) inv_value = NULL;
    g_autofree gchar *gold_str = NULL;
    g_autofree gchar *inv_str = NULL;
    g_autofree gchar *total_str = NULL;
    guint i;

    data = get_game_data (error);
    if (data == NULL)
        return NULL;

    portfolio = lp_game_data_get_portfolio (data);
    investments = lp_portfolio_get_investments (portfolio);
    total_value = lp_portfolio_get_total_value (portfolio);
    inv_value = lp_portfolio_get_investment_value (portfolio);

    gold_str = lrg_big_number_format_short (lp_portfolio_get_gold (portfolio));
    inv_str = lrg_big_number_format_short (inv_value);
    total_str = lrg_big_number_format_short (total_value);

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "gold");
    json_builder_add_string_value (builder, gold_str);

    json_builder_set_member_name (builder, "investment_value");
    json_builder_add_string_value (builder, inv_str);

    json_builder_set_member_name (builder, "total_value");
    json_builder_add_string_value (builder, total_str);

    json_builder_set_member_name (builder, "investments");
    json_builder_begin_array (builder);

    for (i = 0; i < investments->len; i++)
    {
        LpInvestment *inv = g_ptr_array_index (investments, i);
        g_autoptr(LrgBigNumber) current = NULL;
        g_autofree gchar *current_str = NULL;

        current = lp_investment_get_current_value (inv);
        current_str = lrg_big_number_format_short (current);

        json_builder_begin_object (builder);
        json_builder_set_member_name (builder, "id");
        json_builder_add_string_value (builder,
            lp_investment_get_id (inv));
        json_builder_set_member_name (builder, "name");
        json_builder_add_string_value (builder,
            lp_investment_get_name (inv));
        json_builder_set_member_name (builder, "asset_class");
        json_builder_add_int_value (builder,
            (gint64) lp_investment_get_asset_class (inv));
        json_builder_set_member_name (builder, "risk_level");
        json_builder_add_int_value (builder,
            (gint64) lp_investment_get_risk_level (inv));
        json_builder_set_member_name (builder, "current_value");
        json_builder_add_string_value (builder, current_str);
        json_builder_end_object (builder);
    }

    json_builder_end_array (builder);
    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

static McpToolResult *
handle_inspect_agents (LpMcpGameTools *self,
                       JsonObject     *arguments,
                       GError        **error)
{
    LpGameData *data;
    LpAgentManager *manager;
    GPtrArray *agents;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;
    gint64 type_filter;
    guint i;

    data = get_game_data (error);
    if (data == NULL)
        return NULL;

    manager = lp_game_data_get_agent_manager (data);
    agents = lp_agent_manager_get_agents (manager);

    /* Get optional type filter (-1 = all) */
    type_filter = lrg_mcp_tool_group_get_int_arg (arguments, "type", -1);

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "total_count");
    json_builder_add_int_value (builder,
        (gint64) lp_agent_manager_get_agent_count (manager));
    json_builder_set_member_name (builder, "average_loyalty");
    json_builder_add_int_value (builder,
        lp_agent_manager_get_average_loyalty (manager));
    json_builder_set_member_name (builder, "average_competence");
    json_builder_add_int_value (builder,
        lp_agent_manager_get_average_competence (manager));

    json_builder_set_member_name (builder, "agents");
    json_builder_begin_array (builder);

    for (i = 0; i < agents->len; i++)
    {
        LpAgent *agent = g_ptr_array_index (agents, i);
        LpAgentType agent_type = lp_agent_get_agent_type (agent);

        /* Apply type filter if specified */
        if (type_filter >= 0 && (gint64) agent_type != type_filter)
            continue;

        json_builder_begin_object (builder);
        json_builder_set_member_name (builder, "id");
        json_builder_add_string_value (builder,
            lp_agent_get_id (agent));
        json_builder_set_member_name (builder, "name");
        json_builder_add_string_value (builder,
            lp_agent_get_name (agent));
        json_builder_set_member_name (builder, "type");
        json_builder_add_int_value (builder,
            (gint64) agent_type);
        json_builder_set_member_name (builder, "loyalty");
        json_builder_add_int_value (builder,
            (gint64) lp_agent_get_loyalty (agent));
        json_builder_set_member_name (builder, "competence");
        json_builder_add_int_value (builder,
            (gint64) lp_agent_get_competence (agent));
        json_builder_set_member_name (builder, "age");
        json_builder_add_int_value (builder,
            (gint64) lp_agent_get_age (agent));
        json_builder_end_object (builder);
    }

    json_builder_end_array (builder);
    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

static McpToolResult *
handle_inspect_exposure (LpMcpGameTools *self,
                         JsonObject     *arguments,
                         GError        **error)
{
    LpExposureManager *exposure;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;
    LpExposureLevel level;
    const gchar *level_name;

    exposure = lp_exposure_manager_get_default ();

    level = lp_exposure_manager_get_level (exposure);
    switch (level)
    {
    case LP_EXPOSURE_LEVEL_HIDDEN:
        level_name = "hidden";
        break;
    case LP_EXPOSURE_LEVEL_SCRUTINY:
        level_name = "scrutiny";
        break;
    case LP_EXPOSURE_LEVEL_SUSPICION:
        level_name = "suspicion";
        break;
    case LP_EXPOSURE_LEVEL_HUNT:
        level_name = "hunt";
        break;
    case LP_EXPOSURE_LEVEL_CRUSADE:
        level_name = "crusade";
        break;
    default:
        level_name = "unknown";
        break;
    }

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "value");
    json_builder_add_int_value (builder,
        (gint64) lp_exposure_manager_get_exposure (exposure));
    json_builder_set_member_name (builder, "level");
    json_builder_add_int_value (builder, (gint64) level);
    json_builder_set_member_name (builder, "level_name");
    json_builder_add_string_value (builder, level_name);
    json_builder_set_member_name (builder, "decay_rate");
    json_builder_add_int_value (builder,
        (gint64) lp_exposure_manager_get_decay_rate (exposure));

    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

static McpToolResult *
handle_inspect_synergies (LpMcpGameTools *self,
                          JsonObject     *arguments,
                          GError        **error)
{
    LpSynergyManager *synergy;
    GPtrArray *synergies;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;

    synergy = lp_synergy_manager_get_default ();
    synergies = lp_synergy_manager_get_active_synergies (synergy);

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "count");
    json_builder_add_int_value (builder,
        (gint64) lp_synergy_manager_get_synergy_count (synergy));
    json_builder_set_member_name (builder, "total_bonus");
    json_builder_add_double_value (builder,
        lp_synergy_manager_get_total_bonus (synergy));

    json_builder_set_member_name (builder, "synergies");
    json_builder_begin_array (builder);
    /* Synergy array would be populated here when synergy system is complete */
    (void) synergies;  /* Silence unused warning for skeleton */
    json_builder_end_array (builder);

    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

static McpToolResult *
handle_advance_years (LpMcpGameTools *self,
                      JsonObject     *arguments,
                      GError        **error)
{
    LpGameData *data;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;
    g_autoptr(GList) events = NULL;
    gint64 years;
    guint event_count;

    data = get_game_data (error);
    if (data == NULL)
        return NULL;

    years = lrg_mcp_tool_group_get_int_arg (arguments, "years", 1);
    if (years < 1 || years > 1000)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
                     "Years must be between 1 and 1000");
        return NULL;
    }

    /* Perform slumber */
    events = lp_game_data_slumber (data, (guint) years);
    event_count = g_list_length (events);

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "years_advanced");
    json_builder_add_int_value (builder, years);
    json_builder_set_member_name (builder, "new_year");
    json_builder_add_int_value (builder,
        (gint64) lp_game_data_get_current_year (data));
    json_builder_set_member_name (builder, "events_occurred");
    json_builder_add_int_value (builder, (gint64) event_count);

    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

static McpToolResult *
handle_set_gold (LpMcpGameTools *self,
                 JsonObject     *arguments,
                 GError        **error)
{
    LpGameData *data;
    LpPortfolio *portfolio;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;
    g_autoptr(LrgBigNumber) new_gold = NULL;
    g_autofree gchar *gold_str = NULL;
    gdouble amount;

    data = get_game_data (error);
    if (data == NULL)
        return NULL;

    /* Get amount as double (supports scientific notation via JSON parsing) */
    amount = lrg_mcp_tool_group_get_double_arg (arguments, "amount", -1.0);
    if (amount < 0.0)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
                     "amount parameter is required (positive number)");
        return NULL;
    }

    new_gold = lrg_big_number_new (amount);
    portfolio = lp_game_data_get_portfolio (data);
    lp_portfolio_set_gold (portfolio, g_steal_pointer (&new_gold));

    gold_str = lrg_big_number_format_short (lp_portfolio_get_gold (portfolio));

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "success");
    json_builder_add_boolean_value (builder, TRUE);
    json_builder_set_member_name (builder, "new_gold");
    json_builder_add_string_value (builder, gold_str);

    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

static LpUpgradeCategory
parse_upgrade_category (const gchar *category_str)
{
    if (category_str == NULL || g_strcmp0 (category_str, "temporal") == 0)
        return LP_UPGRADE_CATEGORY_TEMPORAL;
    if (g_strcmp0 (category_str, "network") == 0)
        return LP_UPGRADE_CATEGORY_NETWORK;
    if (g_strcmp0 (category_str, "divination") == 0)
        return LP_UPGRADE_CATEGORY_DIVINATION;
    if (g_strcmp0 (category_str, "resilience") == 0)
        return LP_UPGRADE_CATEGORY_RESILIENCE;
    if (g_strcmp0 (category_str, "dark-arts") == 0)
        return LP_UPGRADE_CATEGORY_DARK_ARTS;

    return LP_UPGRADE_CATEGORY_TEMPORAL;  /* Default */
}

static McpToolResult *
handle_unlock_phylactery (LpMcpGameTools *self,
                          JsonObject     *arguments,
                          GError        **error)
{
    LpGameData *data;
    LpPhylactery *phylactery;
    McpToolResult *result;
    g_autoptr(JsonBuilder) builder = NULL;
    g_autofree gchar *json_str = NULL;
    const gchar *upgrade_id;
    const gchar *category_str;
    LpUpgradeCategory category;
    guint64 cost;
    gboolean success;

    data = get_game_data (error);
    if (data == NULL)
        return NULL;

    upgrade_id = lrg_mcp_tool_group_get_string_arg (arguments, "upgrade_id", NULL);
    if (upgrade_id == NULL)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
                     "upgrade_id parameter is required");
        return NULL;
    }

    /* Parse optional category (defaults to temporal) */
    category_str = lrg_mcp_tool_group_get_string_arg (arguments, "category", NULL);
    category = parse_upgrade_category (category_str);

    phylactery = lp_game_data_get_phylactery (data);

    /* Add points to afford the upgrade, then purchase it */
    cost = lp_phylactery_get_upgrade_cost (phylactery, category, upgrade_id);
    lp_phylactery_add_points (phylactery, cost);
    success = lp_phylactery_purchase_upgrade (phylactery, category, upgrade_id);

    builder = json_builder_new ();
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "success");
    json_builder_add_boolean_value (builder, success);
    json_builder_set_member_name (builder, "upgrade_id");
    json_builder_add_string_value (builder, upgrade_id);
    json_builder_set_member_name (builder, "category");
    json_builder_add_string_value (builder,
        category_str != NULL ? category_str : "temporal");

    if (!success)
    {
        json_builder_set_member_name (builder, "message");
        json_builder_add_string_value (builder,
            "Upgrade not found or already unlocked");
    }

    json_builder_end_object (builder);

    json_str = json_builder_to_string (builder);
    result = mcp_tool_result_new (FALSE);
    mcp_tool_result_add_text (result, json_str);
    return result;
}

/* ==========================================================================
 * LrgMcpToolGroup Virtual Methods
 * ========================================================================== */

static const gchar *
lp_mcp_game_tools_get_group_name (LrgMcpToolGroup *group)
{
    return "lichs-portfolio";
}

/* Helper to build a tool input schema */
static JsonNode *
build_input_schema_1 (const gchar *prop_name,
                      const gchar *prop_type,
                      const gchar *prop_desc,
                      gboolean     required)
{
    g_autoptr(JsonBuilder) builder = json_builder_new ();

    json_builder_begin_object (builder);
    json_builder_set_member_name (builder, "type");
    json_builder_add_string_value (builder, "object");

    json_builder_set_member_name (builder, "properties");
    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, prop_name);
    json_builder_begin_object (builder);
    json_builder_set_member_name (builder, "type");
    json_builder_add_string_value (builder, prop_type);
    json_builder_set_member_name (builder, "description");
    json_builder_add_string_value (builder, prop_desc);
    json_builder_end_object (builder);

    json_builder_end_object (builder);

    if (required)
    {
        json_builder_set_member_name (builder, "required");
        json_builder_begin_array (builder);
        json_builder_add_string_value (builder, prop_name);
        json_builder_end_array (builder);
    }

    json_builder_end_object (builder);

    return json_builder_get_root (builder);
}

static void
lp_mcp_game_tools_register_tools (LrgMcpToolGroup *group)
{
    McpTool *tool;
    JsonNode *schema;

    /* lp_get_game_state */
    tool = mcp_tool_new ("lp_get_game_state",
        "Get a full summary of the current game state including year, "
        "gold, investments, agents, exposure, and synergies");
    lrg_mcp_tool_group_add_tool (group, tool);

    /* lp_inspect_portfolio */
    tool = mcp_tool_new ("lp_inspect_portfolio",
        "Get detailed information about the player's investment portfolio "
        "including gold, all investments with their values and types");
    lrg_mcp_tool_group_add_tool (group, tool);

    /* lp_inspect_agents */
    tool = mcp_tool_new ("lp_inspect_agents",
        "List all agents with their stats (loyalty, competence, age). "
        "Optionally filter by agent type (0=individual, 1=family)");
    schema = build_input_schema_1 ("type", "integer",
        "Filter by agent type (0=individual, 1=family)", FALSE);
    mcp_tool_set_input_schema (tool, schema);
    lrg_mcp_tool_group_add_tool (group, tool);

    /* lp_inspect_exposure */
    tool = mcp_tool_new ("lp_inspect_exposure",
        "Get the current exposure level and status. Exposure tracks how "
        "visible the lich is to mortal institutions (0-100)");
    lrg_mcp_tool_group_add_tool (group, tool);

    /* lp_inspect_synergies */
    tool = mcp_tool_new ("lp_inspect_synergies",
        "List active investment synergies and their bonus multipliers");
    lrg_mcp_tool_group_add_tool (group, tool);

    /* lp_advance_years */
    tool = mcp_tool_new ("lp_advance_years",
        "Simulate the passage of time (slumber). Advances the world "
        "simulation and triggers events. Use for testing.");
    schema = build_input_schema_1 ("years", "integer",
        "Number of years to advance (1-1000)", TRUE);
    mcp_tool_set_input_schema (tool, schema);
    lrg_mcp_tool_group_add_tool (group, tool);

    /* lp_set_gold */
    tool = mcp_tool_new ("lp_set_gold",
        "Set the player's gold amount directly (debug tool). "
        "Accepts numeric values");
    schema = build_input_schema_1 ("amount", "number",
        "Gold amount as a number", TRUE);
    mcp_tool_set_input_schema (tool, schema);
    lrg_mcp_tool_group_add_tool (group, tool);

    /* lp_unlock_phylactery */
    tool = mcp_tool_new ("lp_unlock_phylactery",
        "Unlock a phylactery upgrade by ID (debug tool)");
    schema = build_input_schema_1 ("upgrade_id", "string",
        "ID of the phylactery upgrade to unlock", TRUE);
    mcp_tool_set_input_schema (tool, schema);
    lrg_mcp_tool_group_add_tool (group, tool);
}

static McpToolResult *
lp_mcp_game_tools_handle_tool (LrgMcpToolGroup *group,
                               const gchar     *name,
                               JsonObject      *arguments,
                               GError         **error)
{
    LpMcpGameTools *self = LP_MCP_GAME_TOOLS (group);

    if (g_strcmp0 (name, "lp_get_game_state") == 0)
        return handle_get_game_state (self, arguments, error);
    if (g_strcmp0 (name, "lp_inspect_portfolio") == 0)
        return handle_inspect_portfolio (self, arguments, error);
    if (g_strcmp0 (name, "lp_inspect_agents") == 0)
        return handle_inspect_agents (self, arguments, error);
    if (g_strcmp0 (name, "lp_inspect_exposure") == 0)
        return handle_inspect_exposure (self, arguments, error);
    if (g_strcmp0 (name, "lp_inspect_synergies") == 0)
        return handle_inspect_synergies (self, arguments, error);
    if (g_strcmp0 (name, "lp_advance_years") == 0)
        return handle_advance_years (self, arguments, error);
    if (g_strcmp0 (name, "lp_set_gold") == 0)
        return handle_set_gold (self, arguments, error);
    if (g_strcmp0 (name, "lp_unlock_phylactery") == 0)
        return handle_unlock_phylactery (self, arguments, error);

    g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                 "Unknown tool: %s", name);
    return NULL;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_mcp_game_tools_class_init (LpMcpGameToolsClass *klass)
{
    LrgMcpToolGroupClass *group_class = LRG_MCP_TOOL_GROUP_CLASS (klass);

    group_class->get_group_name = lp_mcp_game_tools_get_group_name;
    group_class->register_tools = lp_mcp_game_tools_register_tools;
    group_class->handle_tool = lp_mcp_game_tools_handle_tool;
}

static void
lp_mcp_game_tools_init (LpMcpGameTools *self)
{
    /* Nothing to initialize */
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_mcp_game_tools_new:
 *
 * Creates a new game tools provider.
 *
 * Returns: (transfer full): A new #LpMcpGameTools
 */
LpMcpGameTools *
lp_mcp_game_tools_new (void)
{
    return g_object_new (LP_TYPE_MCP_GAME_TOOLS, NULL);
}

#endif /* LP_MCP */
