/* lp-enums.c - GType Registration for Game Enumerations
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GType registration for game-specific enumerations.
 */

#include "lp-enums.h"

/* ==========================================================================
 * LpAssetClass
 * ========================================================================== */

GType
lp_asset_class_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_ASSET_CLASS_PROPERTY,  "LP_ASSET_CLASS_PROPERTY",  "property" },
            { LP_ASSET_CLASS_TRADE,     "LP_ASSET_CLASS_TRADE",     "trade" },
            { LP_ASSET_CLASS_FINANCIAL, "LP_ASSET_CLASS_FINANCIAL", "financial" },
            { LP_ASSET_CLASS_MAGICAL,   "LP_ASSET_CLASS_MAGICAL",   "magical" },
            { LP_ASSET_CLASS_POLITICAL, "LP_ASSET_CLASS_POLITICAL", "political" },
            { LP_ASSET_CLASS_DARK,      "LP_ASSET_CLASS_DARK",      "dark" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpAssetClass", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpAgentType
 * ========================================================================== */

GType
lp_agent_type_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_AGENT_TYPE_INDIVIDUAL, "LP_AGENT_TYPE_INDIVIDUAL", "individual" },
            { LP_AGENT_TYPE_FAMILY,     "LP_AGENT_TYPE_FAMILY",     "family" },
            { LP_AGENT_TYPE_CULT,       "LP_AGENT_TYPE_CULT",       "cult" },
            { LP_AGENT_TYPE_BOUND,      "LP_AGENT_TYPE_BOUND",      "bound" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpAgentType", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpRiskLevel
 * ========================================================================== */

GType
lp_risk_level_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_RISK_LEVEL_LOW,     "LP_RISK_LEVEL_LOW",     "low" },
            { LP_RISK_LEVEL_MEDIUM,  "LP_RISK_LEVEL_MEDIUM",  "medium" },
            { LP_RISK_LEVEL_HIGH,    "LP_RISK_LEVEL_HIGH",    "high" },
            { LP_RISK_LEVEL_EXTREME, "LP_RISK_LEVEL_EXTREME", "extreme" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpRiskLevel", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpEventType
 * ========================================================================== */

GType
lp_event_type_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_EVENT_TYPE_ECONOMIC,  "LP_EVENT_TYPE_ECONOMIC",  "economic" },
            { LP_EVENT_TYPE_POLITICAL, "LP_EVENT_TYPE_POLITICAL", "political" },
            { LP_EVENT_TYPE_MAGICAL,   "LP_EVENT_TYPE_MAGICAL",   "magical" },
            { LP_EVENT_TYPE_PERSONAL,  "LP_EVENT_TYPE_PERSONAL",  "personal" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpEventType", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpExposureLevel
 * ========================================================================== */

GType
lp_exposure_level_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_EXPOSURE_LEVEL_HIDDEN,    "LP_EXPOSURE_LEVEL_HIDDEN",    "hidden" },
            { LP_EXPOSURE_LEVEL_SCRUTINY,  "LP_EXPOSURE_LEVEL_SCRUTINY",  "scrutiny" },
            { LP_EXPOSURE_LEVEL_SUSPICION, "LP_EXPOSURE_LEVEL_SUSPICION", "suspicion" },
            { LP_EXPOSURE_LEVEL_HUNT,      "LP_EXPOSURE_LEVEL_HUNT",      "hunt" },
            { LP_EXPOSURE_LEVEL_CRUSADE,   "LP_EXPOSURE_LEVEL_CRUSADE",   "crusade" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpExposureLevel", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpLedgerCategory
 * ========================================================================== */

GType
lp_ledger_category_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_LEDGER_CATEGORY_ECONOMIC,   "LP_LEDGER_CATEGORY_ECONOMIC",   "economic" },
            { LP_LEDGER_CATEGORY_AGENT,      "LP_LEDGER_CATEGORY_AGENT",      "agent" },
            { LP_LEDGER_CATEGORY_COMPETITOR, "LP_LEDGER_CATEGORY_COMPETITOR", "competitor" },
            { LP_LEDGER_CATEGORY_HIDDEN,     "LP_LEDGER_CATEGORY_HIDDEN",     "hidden" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpLedgerCategory", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpCoverStatus
 * ========================================================================== */

GType
lp_cover_status_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_COVER_STATUS_SECURE,      "LP_COVER_STATUS_SECURE",      "secure" },
            { LP_COVER_STATUS_SUSPICIOUS,  "LP_COVER_STATUS_SUSPICIOUS",  "suspicious" },
            { LP_COVER_STATUS_COMPROMISED, "LP_COVER_STATUS_COMPROMISED", "compromised" },
            { LP_COVER_STATUS_EXPOSED,     "LP_COVER_STATUS_EXPOSED",     "exposed" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpCoverStatus", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpKnowledgeLevel
 * ========================================================================== */

GType
lp_knowledge_level_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_KNOWLEDGE_LEVEL_NONE,       "LP_KNOWLEDGE_LEVEL_NONE",       "none" },
            { LP_KNOWLEDGE_LEVEL_SUSPICIOUS, "LP_KNOWLEDGE_LEVEL_SUSPICIOUS", "suspicious" },
            { LP_KNOWLEDGE_LEVEL_AWARE,      "LP_KNOWLEDGE_LEVEL_AWARE",      "aware" },
            { LP_KNOWLEDGE_LEVEL_FULL,       "LP_KNOWLEDGE_LEVEL_FULL",       "full" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpKnowledgeLevel", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpGeographyType
 * ========================================================================== */

GType
lp_geography_type_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_GEOGRAPHY_TYPE_COASTAL,  "LP_GEOGRAPHY_TYPE_COASTAL",  "coastal" },
            { LP_GEOGRAPHY_TYPE_INLAND,   "LP_GEOGRAPHY_TYPE_INLAND",   "inland" },
            { LP_GEOGRAPHY_TYPE_MOUNTAIN, "LP_GEOGRAPHY_TYPE_MOUNTAIN", "mountain" },
            { LP_GEOGRAPHY_TYPE_FOREST,   "LP_GEOGRAPHY_TYPE_FOREST",   "forest" },
            { LP_GEOGRAPHY_TYPE_DESERT,   "LP_GEOGRAPHY_TYPE_DESERT",   "desert" },
            { LP_GEOGRAPHY_TYPE_SWAMP,    "LP_GEOGRAPHY_TYPE_SWAMP",    "swamp" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpGeographyType", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpKingdomRelation
 * ========================================================================== */

GType
lp_kingdom_relation_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_KINGDOM_RELATION_ALLIANCE,  "LP_KINGDOM_RELATION_ALLIANCE",  "alliance" },
            { LP_KINGDOM_RELATION_NEUTRAL,   "LP_KINGDOM_RELATION_NEUTRAL",   "neutral" },
            { LP_KINGDOM_RELATION_RIVALRY,   "LP_KINGDOM_RELATION_RIVALRY",   "rivalry" },
            { LP_KINGDOM_RELATION_WAR,       "LP_KINGDOM_RELATION_WAR",       "war" },
            { LP_KINGDOM_RELATION_VASSALAGE, "LP_KINGDOM_RELATION_VASSALAGE", "vassalage" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpKingdomRelation", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpCompetitorType
 * ========================================================================== */

GType
lp_competitor_type_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_COMPETITOR_TYPE_DRAGON,  "LP_COMPETITOR_TYPE_DRAGON",  "dragon" },
            { LP_COMPETITOR_TYPE_VAMPIRE, "LP_COMPETITOR_TYPE_VAMPIRE", "vampire" },
            { LP_COMPETITOR_TYPE_LICH,    "LP_COMPETITOR_TYPE_LICH",    "lich" },
            { LP_COMPETITOR_TYPE_FAE,     "LP_COMPETITOR_TYPE_FAE",     "fae" },
            { LP_COMPETITOR_TYPE_DEMON,   "LP_COMPETITOR_TYPE_DEMON",   "demon" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpCompetitorType", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpCompetitorStance
 * ========================================================================== */

GType
lp_competitor_stance_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_COMPETITOR_STANCE_UNKNOWN,  "LP_COMPETITOR_STANCE_UNKNOWN",  "unknown" },
            { LP_COMPETITOR_STANCE_WARY,     "LP_COMPETITOR_STANCE_WARY",     "wary" },
            { LP_COMPETITOR_STANCE_NEUTRAL,  "LP_COMPETITOR_STANCE_NEUTRAL",  "neutral" },
            { LP_COMPETITOR_STANCE_FRIENDLY, "LP_COMPETITOR_STANCE_FRIENDLY", "friendly" },
            { LP_COMPETITOR_STANCE_HOSTILE,  "LP_COMPETITOR_STANCE_HOSTILE",  "hostile" },
            { LP_COMPETITOR_STANCE_ALLIED,   "LP_COMPETITOR_STANCE_ALLIED",   "allied" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpCompetitorStance", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpEventSeverity
 * ========================================================================== */

GType
lp_event_severity_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_EVENT_SEVERITY_MINOR,       "LP_EVENT_SEVERITY_MINOR",       "minor" },
            { LP_EVENT_SEVERITY_MODERATE,    "LP_EVENT_SEVERITY_MODERATE",    "moderate" },
            { LP_EVENT_SEVERITY_MAJOR,       "LP_EVENT_SEVERITY_MAJOR",       "major" },
            { LP_EVENT_SEVERITY_CATASTROPHIC, "LP_EVENT_SEVERITY_CATASTROPHIC", "catastrophic" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpEventSeverity", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpUpgradeCategory
 * ========================================================================== */

GType
lp_upgrade_category_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_UPGRADE_CATEGORY_TEMPORAL,   "LP_UPGRADE_CATEGORY_TEMPORAL",   "temporal" },
            { LP_UPGRADE_CATEGORY_NETWORK,    "LP_UPGRADE_CATEGORY_NETWORK",    "network" },
            { LP_UPGRADE_CATEGORY_DIVINATION, "LP_UPGRADE_CATEGORY_DIVINATION", "divination" },
            { LP_UPGRADE_CATEGORY_RESILIENCE, "LP_UPGRADE_CATEGORY_RESILIENCE", "resilience" },
            { LP_UPGRADE_CATEGORY_DARK_ARTS,  "LP_UPGRADE_CATEGORY_DARK_ARTS",  "dark-arts" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpUpgradeCategory", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpMegaprojectState
 * ========================================================================== */

GType
lp_megaproject_state_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_MEGAPROJECT_STATE_LOCKED,     "LP_MEGAPROJECT_STATE_LOCKED",     "locked" },
            { LP_MEGAPROJECT_STATE_AVAILABLE,  "LP_MEGAPROJECT_STATE_AVAILABLE",  "available" },
            { LP_MEGAPROJECT_STATE_ACTIVE,     "LP_MEGAPROJECT_STATE_ACTIVE",     "active" },
            { LP_MEGAPROJECT_STATE_PAUSED,     "LP_MEGAPROJECT_STATE_PAUSED",     "paused" },
            { LP_MEGAPROJECT_STATE_DISCOVERED, "LP_MEGAPROJECT_STATE_DISCOVERED", "discovered" },
            { LP_MEGAPROJECT_STATE_COMPLETE,   "LP_MEGAPROJECT_STATE_COMPLETE",   "complete" },
            { LP_MEGAPROJECT_STATE_DESTROYED,  "LP_MEGAPROJECT_STATE_DESTROYED",  "destroyed" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpMegaprojectState", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpEchoTree
 * ========================================================================== */

GType
lp_echo_tree_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_ECHO_TREE_ECONOMIST,   "LP_ECHO_TREE_ECONOMIST",   "economist" },
            { LP_ECHO_TREE_MANIPULATOR, "LP_ECHO_TREE_MANIPULATOR", "manipulator" },
            { LP_ECHO_TREE_SCHOLAR,     "LP_ECHO_TREE_SCHOLAR",     "scholar" },
            { LP_ECHO_TREE_ARCHITECT,   "LP_ECHO_TREE_ARCHITECT",   "architect" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpEchoTree", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}

/* ==========================================================================
 * LpGrowthIntensity
 * ========================================================================== */

GType
lp_growth_intensity_get_type (void)
{
    static volatile gsize g_type_id = 0;

    if (g_once_init_enter (&g_type_id))
    {
        static const GEnumValue values[] = {
            { LP_GROWTH_INTENSITY_MINOR,     "LP_GROWTH_INTENSITY_MINOR",     "minor" },
            { LP_GROWTH_INTENSITY_MODERATE,  "LP_GROWTH_INTENSITY_MODERATE",  "moderate" },
            { LP_GROWTH_INTENSITY_MAJOR,     "LP_GROWTH_INTENSITY_MAJOR",     "major" },
            { LP_GROWTH_INTENSITY_LEGENDARY, "LP_GROWTH_INTENSITY_LEGENDARY", "legendary" },
            { 0, NULL, NULL }
        };
        GType type_id = g_enum_register_static ("LpGrowthIntensity", values);
        g_once_init_leave (&g_type_id, type_id);
    }

    return g_type_id;
}
