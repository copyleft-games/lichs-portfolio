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
