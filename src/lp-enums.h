/* lp-enums.h - Game Enumerations for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Game-specific enumerations with GType registration for GObject integration.
 */

#ifndef LP_ENUMS_H
#define LP_ENUMS_H

#include <glib-object.h>

G_BEGIN_DECLS

/* ==========================================================================
 * LpAssetClass - Investment asset categories
 * ========================================================================== */

/**
 * LpAssetClass:
 * @LP_ASSET_CLASS_PROPERTY: Real property (land, buildings, mines)
 * @LP_ASSET_CLASS_TRADE: Trade and commerce (routes, guilds, shipping)
 * @LP_ASSET_CLASS_FINANCIAL: Financial instruments (bonds, notes, insurance)
 * @LP_ASSET_CLASS_MAGICAL: Magical assets (artifacts, components, creatures)
 * @LP_ASSET_CLASS_POLITICAL: Political influence (noble backing, spy networks)
 * @LP_ASSET_CLASS_DARK: Dark investments (undead labor, soul trading) - hidden
 *
 * Categories of investment assets in the portfolio.
 */
typedef enum
{
    LP_ASSET_CLASS_PROPERTY,
    LP_ASSET_CLASS_TRADE,
    LP_ASSET_CLASS_FINANCIAL,
    LP_ASSET_CLASS_MAGICAL,
    LP_ASSET_CLASS_POLITICAL,
    LP_ASSET_CLASS_DARK
} LpAssetClass;

GType lp_asset_class_get_type (void) G_GNUC_CONST;
#define LP_TYPE_ASSET_CLASS (lp_asset_class_get_type ())

/* ==========================================================================
 * LpAgentType - Agent categories
 * ========================================================================== */

/**
 * LpAgentType:
 * @LP_AGENT_TYPE_INDIVIDUAL: Single mortal agent
 * @LP_AGENT_TYPE_FAMILY: Bloodline dynasty of agents
 * @LP_AGENT_TYPE_CULT: Religious organization of followers
 * @LP_AGENT_TYPE_BOUND: Undead or magically bound servants
 *
 * Types of agents that serve the lich.
 */
typedef enum
{
    LP_AGENT_TYPE_INDIVIDUAL,
    LP_AGENT_TYPE_FAMILY,
    LP_AGENT_TYPE_CULT,
    LP_AGENT_TYPE_BOUND
} LpAgentType;

GType lp_agent_type_get_type (void) G_GNUC_CONST;
#define LP_TYPE_AGENT_TYPE (lp_agent_type_get_type ())

/* ==========================================================================
 * LpRiskLevel - Investment risk levels
 * ========================================================================== */

/**
 * LpRiskLevel:
 * @LP_RISK_LEVEL_LOW: Low risk, low return (property)
 * @LP_RISK_LEVEL_MEDIUM: Medium risk, medium return (trade)
 * @LP_RISK_LEVEL_HIGH: High risk, high return (magical)
 * @LP_RISK_LEVEL_EXTREME: Extreme risk, extreme return (dark)
 *
 * Risk classification for investments.
 */
typedef enum
{
    LP_RISK_LEVEL_LOW,
    LP_RISK_LEVEL_MEDIUM,
    LP_RISK_LEVEL_HIGH,
    LP_RISK_LEVEL_EXTREME
} LpRiskLevel;

GType lp_risk_level_get_type (void) G_GNUC_CONST;
#define LP_TYPE_RISK_LEVEL (lp_risk_level_get_type ())

/* ==========================================================================
 * LpEventType - World event categories
 * ========================================================================== */

/**
 * LpEventType:
 * @LP_EVENT_TYPE_ECONOMIC: Economic events (market crashes, discoveries)
 * @LP_EVENT_TYPE_POLITICAL: Political events (wars, successions)
 * @LP_EVENT_TYPE_MAGICAL: Magical events (artifacts, divine intervention)
 * @LP_EVENT_TYPE_PERSONAL: Personal events (agent deaths, discovery attempts)
 *
 * Categories of world events that occur during slumber.
 */
typedef enum
{
    LP_EVENT_TYPE_ECONOMIC,
    LP_EVENT_TYPE_POLITICAL,
    LP_EVENT_TYPE_MAGICAL,
    LP_EVENT_TYPE_PERSONAL
} LpEventType;

GType lp_event_type_get_type (void) G_GNUC_CONST;
#define LP_TYPE_EVENT_TYPE (lp_event_type_get_type ())

/* ==========================================================================
 * LpExposureLevel - Visibility to mortal institutions
 * ========================================================================== */

/**
 * LpExposureLevel:
 * @LP_EXPOSURE_LEVEL_HIDDEN: Completely hidden (0-24%)
 * @LP_EXPOSURE_LEVEL_SCRUTINY: Under scrutiny (25-49%)
 * @LP_EXPOSURE_LEVEL_SUSPICION: Suspected (50-74%)
 * @LP_EXPOSURE_LEVEL_HUNT: Actively hunted (75-99%)
 * @LP_EXPOSURE_LEVEL_CRUSADE: Crusade launched (100%)
 *
 * How visible the lich's activities are to mortal institutions.
 */
typedef enum
{
    LP_EXPOSURE_LEVEL_HIDDEN,
    LP_EXPOSURE_LEVEL_SCRUTINY,
    LP_EXPOSURE_LEVEL_SUSPICION,
    LP_EXPOSURE_LEVEL_HUNT,
    LP_EXPOSURE_LEVEL_CRUSADE
} LpExposureLevel;

GType lp_exposure_level_get_type (void) G_GNUC_CONST;
#define LP_TYPE_EXPOSURE_LEVEL (lp_exposure_level_get_type ())

/* ==========================================================================
 * LpLedgerCategory - Categories of discoverable information
 * ========================================================================== */

/**
 * LpLedgerCategory:
 * @LP_LEDGER_CATEGORY_ECONOMIC: Market patterns, cycles, trade secrets
 * @LP_LEDGER_CATEGORY_AGENT: Bloodline secrets, family histories
 * @LP_LEDGER_CATEGORY_COMPETITOR: Other immortals' weaknesses
 * @LP_LEDGER_CATEGORY_HIDDEN: Hidden game mechanics
 *
 * Categories of entries in the ledger (discovery system).
 */
typedef enum
{
    LP_LEDGER_CATEGORY_ECONOMIC,
    LP_LEDGER_CATEGORY_AGENT,
    LP_LEDGER_CATEGORY_COMPETITOR,
    LP_LEDGER_CATEGORY_HIDDEN
} LpLedgerCategory;

GType lp_ledger_category_get_type (void) G_GNUC_CONST;
#define LP_TYPE_LEDGER_CATEGORY (lp_ledger_category_get_type ())

/* ==========================================================================
 * LpCoverStatus - Agent cover status
 * ========================================================================== */

/**
 * LpCoverStatus:
 * @LP_COVER_STATUS_SECURE: Cover is secure
 * @LP_COVER_STATUS_SUSPICIOUS: Cover is under suspicion
 * @LP_COVER_STATUS_COMPROMISED: Cover is compromised
 * @LP_COVER_STATUS_EXPOSED: Fully exposed
 *
 * Status of an agent's cover identity.
 */
typedef enum
{
    LP_COVER_STATUS_SECURE,
    LP_COVER_STATUS_SUSPICIOUS,
    LP_COVER_STATUS_COMPROMISED,
    LP_COVER_STATUS_EXPOSED
} LpCoverStatus;

GType lp_cover_status_get_type (void) G_GNUC_CONST;
#define LP_TYPE_COVER_STATUS (lp_cover_status_get_type ())

/* ==========================================================================
 * LpKnowledgeLevel - Agent knowledge of true master
 * ========================================================================== */

/**
 * LpKnowledgeLevel:
 * @LP_KNOWLEDGE_LEVEL_NONE: Believes they serve a mortal benefactor
 * @LP_KNOWLEDGE_LEVEL_SUSPICIOUS: Suspects something supernatural
 * @LP_KNOWLEDGE_LEVEL_AWARE: Knows they serve an immortal
 * @LP_KNOWLEDGE_LEVEL_FULL: Knows they serve a lich specifically
 *
 * How much an agent knows about their true master.
 */
typedef enum
{
    LP_KNOWLEDGE_LEVEL_NONE,
    LP_KNOWLEDGE_LEVEL_SUSPICIOUS,
    LP_KNOWLEDGE_LEVEL_AWARE,
    LP_KNOWLEDGE_LEVEL_FULL
} LpKnowledgeLevel;

GType lp_knowledge_level_get_type (void) G_GNUC_CONST;
#define LP_TYPE_KNOWLEDGE_LEVEL (lp_knowledge_level_get_type ())

G_END_DECLS

#endif /* LP_ENUMS_H */
