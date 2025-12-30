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

/* ==========================================================================
 * LpGeographyType - Region geography classifications
 * ========================================================================== */

/**
 * LpGeographyType:
 * @LP_GEOGRAPHY_TYPE_COASTAL: Coastal region (trade bonus)
 * @LP_GEOGRAPHY_TYPE_INLAND: Inland plains (agriculture bonus)
 * @LP_GEOGRAPHY_TYPE_MOUNTAIN: Mountain terrain (mining bonus)
 * @LP_GEOGRAPHY_TYPE_FOREST: Forest territory (lumber bonus)
 * @LP_GEOGRAPHY_TYPE_DESERT: Desert wasteland (magical bonus)
 * @LP_GEOGRAPHY_TYPE_SWAMP: Swampland (hidden, dark arts bonus)
 *
 * Types of regional geography affecting resources and trade.
 */
typedef enum
{
    LP_GEOGRAPHY_TYPE_COASTAL,
    LP_GEOGRAPHY_TYPE_INLAND,
    LP_GEOGRAPHY_TYPE_MOUNTAIN,
    LP_GEOGRAPHY_TYPE_FOREST,
    LP_GEOGRAPHY_TYPE_DESERT,
    LP_GEOGRAPHY_TYPE_SWAMP
} LpGeographyType;

GType lp_geography_type_get_type (void) G_GNUC_CONST;
#define LP_TYPE_GEOGRAPHY_TYPE (lp_geography_type_get_type ())

/* ==========================================================================
 * LpKingdomRelation - Diplomatic relations between kingdoms
 * ========================================================================== */

/**
 * LpKingdomRelation:
 * @LP_KINGDOM_RELATION_ALLIANCE: Allied kingdoms
 * @LP_KINGDOM_RELATION_NEUTRAL: Neutral stance
 * @LP_KINGDOM_RELATION_RIVALRY: Economic/political rivalry
 * @LP_KINGDOM_RELATION_WAR: Active warfare
 * @LP_KINGDOM_RELATION_VASSALAGE: One is vassal of other
 *
 * Diplomatic relations between kingdoms.
 */
typedef enum
{
    LP_KINGDOM_RELATION_ALLIANCE,
    LP_KINGDOM_RELATION_NEUTRAL,
    LP_KINGDOM_RELATION_RIVALRY,
    LP_KINGDOM_RELATION_WAR,
    LP_KINGDOM_RELATION_VASSALAGE
} LpKingdomRelation;

GType lp_kingdom_relation_get_type (void) G_GNUC_CONST;
#define LP_TYPE_KINGDOM_RELATION (lp_kingdom_relation_get_type ())

/* ==========================================================================
 * LpCompetitorType - Types of immortal competitors
 * ========================================================================== */

/**
 * LpCompetitorType:
 * @LP_COMPETITOR_TYPE_DRAGON: Ancient dragon with hoard
 * @LP_COMPETITOR_TYPE_VAMPIRE: Elder vampire with network
 * @LP_COMPETITOR_TYPE_LICH: Rival lich
 * @LP_COMPETITOR_TYPE_FAE: Fae lord with holdings
 * @LP_COMPETITOR_TYPE_DEMON: Bound demon with cultists
 *
 * Types of immortal competitors in the world.
 */
typedef enum
{
    LP_COMPETITOR_TYPE_DRAGON,
    LP_COMPETITOR_TYPE_VAMPIRE,
    LP_COMPETITOR_TYPE_LICH,
    LP_COMPETITOR_TYPE_FAE,
    LP_COMPETITOR_TYPE_DEMON
} LpCompetitorType;

GType lp_competitor_type_get_type (void) G_GNUC_CONST;
#define LP_TYPE_COMPETITOR_TYPE (lp_competitor_type_get_type ())

/* ==========================================================================
 * LpCompetitorStance - Competitor attitude toward player
 * ========================================================================== */

/**
 * LpCompetitorStance:
 * @LP_COMPETITOR_STANCE_UNKNOWN: Not yet encountered
 * @LP_COMPETITOR_STANCE_WARY: Cautious observation
 * @LP_COMPETITOR_STANCE_NEUTRAL: No strong feelings
 * @LP_COMPETITOR_STANCE_FRIENDLY: Potential ally
 * @LP_COMPETITOR_STANCE_HOSTILE: Active opposition
 * @LP_COMPETITOR_STANCE_ALLIED: Formal alliance
 *
 * How a competitor views the player.
 */
typedef enum
{
    LP_COMPETITOR_STANCE_UNKNOWN,
    LP_COMPETITOR_STANCE_WARY,
    LP_COMPETITOR_STANCE_NEUTRAL,
    LP_COMPETITOR_STANCE_FRIENDLY,
    LP_COMPETITOR_STANCE_HOSTILE,
    LP_COMPETITOR_STANCE_ALLIED
} LpCompetitorStance;

GType lp_competitor_stance_get_type (void) G_GNUC_CONST;
#define LP_TYPE_COMPETITOR_STANCE (lp_competitor_stance_get_type ())

/* ==========================================================================
 * LpEventSeverity - Event impact level
 * ========================================================================== */

/**
 * LpEventSeverity:
 * @LP_EVENT_SEVERITY_MINOR: Small impact, common
 * @LP_EVENT_SEVERITY_MODERATE: Medium impact
 * @LP_EVENT_SEVERITY_MAJOR: Significant impact
 * @LP_EVENT_SEVERITY_CATASTROPHIC: World-changing impact, rare
 *
 * Severity level of world events.
 */
typedef enum
{
    LP_EVENT_SEVERITY_MINOR,
    LP_EVENT_SEVERITY_MODERATE,
    LP_EVENT_SEVERITY_MAJOR,
    LP_EVENT_SEVERITY_CATASTROPHIC
} LpEventSeverity;

GType lp_event_severity_get_type (void) G_GNUC_CONST;
#define LP_TYPE_EVENT_SEVERITY (lp_event_severity_get_type ())

/* ==========================================================================
 * LpUpgradeCategory - Phylactery upgrade categories
 * ========================================================================== */

/**
 * LpUpgradeCategory:
 * @LP_UPGRADE_CATEGORY_TEMPORAL: Temporal Mastery - longer slumber, time efficiency
 * @LP_UPGRADE_CATEGORY_NETWORK: Network Expansion - more agents, family/cult mechanics
 * @LP_UPGRADE_CATEGORY_DIVINATION: Divination - better predictions, early warnings
 * @LP_UPGRADE_CATEGORY_RESILIENCE: Resilience - survive disasters, faster recovery
 * @LP_UPGRADE_CATEGORY_DARK_ARTS: Dark Arts - unlock dark investments (hidden)
 *
 * Categories of upgrades in the phylactery tree.
 */
typedef enum
{
    LP_UPGRADE_CATEGORY_TEMPORAL,
    LP_UPGRADE_CATEGORY_NETWORK,
    LP_UPGRADE_CATEGORY_DIVINATION,
    LP_UPGRADE_CATEGORY_RESILIENCE,
    LP_UPGRADE_CATEGORY_DARK_ARTS
} LpUpgradeCategory;

GType lp_upgrade_category_get_type (void) G_GNUC_CONST;
#define LP_TYPE_UPGRADE_CATEGORY (lp_upgrade_category_get_type ())

/* ==========================================================================
 * LpMegaprojectState - Megaproject progress states
 * ========================================================================== */

/**
 * LpMegaprojectState:
 * @LP_MEGAPROJECT_STATE_LOCKED: Not yet unlocked
 * @LP_MEGAPROJECT_STATE_AVAILABLE: Unlocked but not started
 * @LP_MEGAPROJECT_STATE_ACTIVE: Currently in progress
 * @LP_MEGAPROJECT_STATE_PAUSED: Temporarily paused
 * @LP_MEGAPROJECT_STATE_DISCOVERED: Discovered by enemies (at risk)
 * @LP_MEGAPROJECT_STATE_COMPLETE: Successfully completed
 * @LP_MEGAPROJECT_STATE_DESTROYED: Destroyed by enemies
 *
 * States a megaproject can be in during its lifecycle.
 */
typedef enum
{
    LP_MEGAPROJECT_STATE_LOCKED,
    LP_MEGAPROJECT_STATE_AVAILABLE,
    LP_MEGAPROJECT_STATE_ACTIVE,
    LP_MEGAPROJECT_STATE_PAUSED,
    LP_MEGAPROJECT_STATE_DISCOVERED,
    LP_MEGAPROJECT_STATE_COMPLETE,
    LP_MEGAPROJECT_STATE_DESTROYED
} LpMegaprojectState;

GType lp_megaproject_state_get_type (void) G_GNUC_CONST;
#define LP_TYPE_MEGAPROJECT_STATE (lp_megaproject_state_get_type ())

/* ==========================================================================
 * LpEchoTree - Echo specialization tree types
 * ========================================================================== */

/**
 * LpEchoTree:
 * @LP_ECHO_TREE_ECONOMIST: The Economist - financial and compound bonuses
 * @LP_ECHO_TREE_MANIPULATOR: The Manipulator - agent and political bonuses
 * @LP_ECHO_TREE_SCHOLAR: The Scholar - ledger and discovery bonuses
 * @LP_ECHO_TREE_ARCHITECT: The Architect - preservation and project bonuses
 *
 * Specialization trees purchased with Echo points after prestige.
 */
typedef enum
{
    LP_ECHO_TREE_ECONOMIST,
    LP_ECHO_TREE_MANIPULATOR,
    LP_ECHO_TREE_SCHOLAR,
    LP_ECHO_TREE_ARCHITECT
} LpEchoTree;

GType lp_echo_tree_get_type (void) G_GNUC_CONST;
#define LP_TYPE_ECHO_TREE (lp_echo_tree_get_type ())

G_END_DECLS

#endif /* LP_ENUMS_H */
