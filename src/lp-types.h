/* lp-types.h - Forward Declarations for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file contains forward declarations for all Lp* types.
 * Include this to avoid circular dependencies between headers.
 */

#ifndef LP_TYPES_H
#define LP_TYPES_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/* ==========================================================================
 * Core Types (src/core/)
 * ========================================================================== */

typedef struct _LpApplication           LpApplication;
typedef struct _LpApplicationClass      LpApplicationClass;

typedef struct _LpGameData              LpGameData;
typedef struct _LpGameDataClass         LpGameDataClass;

typedef struct _LpPhylactery            LpPhylactery;
typedef struct _LpPhylacteryClass       LpPhylacteryClass;

typedef struct _LpPrestigeManager       LpPrestigeManager;
typedef struct _LpPrestigeManagerClass  LpPrestigeManagerClass;

typedef struct _LpExposureManager       LpExposureManager;
typedef struct _LpExposureManagerClass  LpExposureManagerClass;

typedef struct _LpSynergyManager        LpSynergyManager;
typedef struct _LpSynergyManagerClass   LpSynergyManagerClass;

typedef struct _LpSynergy               LpSynergy;
typedef struct _LpSynergyClass          LpSynergyClass;

typedef struct _LpLedger                LpLedger;
typedef struct _LpLedgerClass           LpLedgerClass;

typedef struct _LpMegaproject           LpMegaproject;
typedef struct _LpMegaprojectClass      LpMegaprojectClass;

/* ==========================================================================
 * Simulation Types (src/simulation/)
 * ========================================================================== */

typedef struct _LpWorldSimulation       LpWorldSimulation;
typedef struct _LpWorldSimulationClass  LpWorldSimulationClass;

typedef struct _LpKingdom               LpKingdom;
typedef struct _LpKingdomClass          LpKingdomClass;

typedef struct _LpRegion                LpRegion;
typedef struct _LpRegionClass           LpRegionClass;

typedef struct _LpEvent                 LpEvent;
typedef struct _LpEventClass            LpEventClass;

typedef struct _LpEventGenerator        LpEventGenerator;
typedef struct _LpEventGeneratorClass   LpEventGeneratorClass;

typedef struct _LpCompetitor            LpCompetitor;
typedef struct _LpCompetitorClass       LpCompetitorClass;

/* ==========================================================================
 * Investment Types (src/investment/)
 * ========================================================================== */

typedef struct _LpInvestment            LpInvestment;
typedef struct _LpInvestmentClass       LpInvestmentClass;

typedef struct _LpInvestmentProperty    LpInvestmentProperty;
typedef struct _LpInvestmentPropertyClass LpInvestmentPropertyClass;

typedef struct _LpInvestmentTrade       LpInvestmentTrade;
typedef struct _LpInvestmentTradeClass  LpInvestmentTradeClass;

typedef struct _LpInvestmentFinancial   LpInvestmentFinancial;
typedef struct _LpInvestmentFinancialClass LpInvestmentFinancialClass;

typedef struct _LpInvestmentMagical     LpInvestmentMagical;
typedef struct _LpInvestmentMagicalClass LpInvestmentMagicalClass;

typedef struct _LpInvestmentPolitical   LpInvestmentPolitical;
typedef struct _LpInvestmentPoliticalClass LpInvestmentPoliticalClass;

typedef struct _LpInvestmentDark        LpInvestmentDark;
typedef struct _LpInvestmentDarkClass   LpInvestmentDarkClass;

typedef struct _LpPortfolio             LpPortfolio;
typedef struct _LpPortfolioClass        LpPortfolioClass;

/* ==========================================================================
 * Agent Types (src/agent/)
 * ========================================================================== */

typedef struct _LpAgent                 LpAgent;
typedef struct _LpAgentClass            LpAgentClass;

typedef struct _LpAgentIndividual       LpAgentIndividual;
typedef struct _LpAgentIndividualClass  LpAgentIndividualClass;

typedef struct _LpAgentFamily           LpAgentFamily;
typedef struct _LpAgentFamilyClass      LpAgentFamilyClass;

typedef struct _LpAgentCult             LpAgentCult;
typedef struct _LpAgentCultClass        LpAgentCultClass;

typedef struct _LpAgentBound            LpAgentBound;
typedef struct _LpAgentBoundClass       LpAgentBoundClass;

typedef struct _LpAgentManager          LpAgentManager;
typedef struct _LpAgentManagerClass     LpAgentManagerClass;

typedef struct _LpTrait                 LpTrait;
typedef struct _LpTraitClass            LpTraitClass;

/* ==========================================================================
 * UI Types (src/ui/)
 * ========================================================================== */

typedef struct _LpScreenPortfolio       LpScreenPortfolio;
typedef struct _LpScreenPortfolioClass  LpScreenPortfolioClass;

typedef struct _LpScreenWorldMap        LpScreenWorldMap;
typedef struct _LpScreenWorldMapClass   LpScreenWorldMapClass;

typedef struct _LpScreenAgents          LpScreenAgents;
typedef struct _LpScreenAgentsClass     LpScreenAgentsClass;

typedef struct _LpScreenIntelligence    LpScreenIntelligence;
typedef struct _LpScreenIntelligenceClass LpScreenIntelligenceClass;

typedef struct _LpScreenSlumber         LpScreenSlumber;
typedef struct _LpScreenSlumberClass    LpScreenSlumberClass;

typedef struct _LpScreenLedger          LpScreenLedger;
typedef struct _LpScreenLedgerClass     LpScreenLedgerClass;

typedef struct _LpScreenMegaprojects    LpScreenMegaprojects;
typedef struct _LpScreenMegaprojectsClass LpScreenMegaprojectsClass;

typedef struct _LpDialogEvent           LpDialogEvent;
typedef struct _LpDialogEventClass      LpDialogEventClass;

typedef struct _LpWidgetExposureMeter   LpWidgetExposureMeter;
typedef struct _LpWidgetExposureMeterClass LpWidgetExposureMeterClass;

typedef struct _LpWidgetSynergyIndicator LpWidgetSynergyIndicator;
typedef struct _LpWidgetSynergyIndicatorClass LpWidgetSynergyIndicatorClass;

/* ==========================================================================
 * Feedback Types (src/feedback/)
 * ========================================================================== */

typedef struct _LpFloatingText          LpFloatingText;
typedef struct _LpFloatingTextClass     LpFloatingTextClass;

typedef struct _LpGrowthParticles       LpGrowthParticles;
typedef struct _LpGrowthParticlesClass  LpGrowthParticlesClass;

typedef struct _LpAchievementPopup      LpAchievementPopup;
typedef struct _LpAchievementPopupClass LpAchievementPopupClass;

/* ==========================================================================
 * State Types (src/states/)
 * ========================================================================== */

typedef struct _LpStateMainMenu         LpStateMainMenu;
typedef struct _LpStateMainMenuClass    LpStateMainMenuClass;

typedef struct _LpStateWake             LpStateWake;
typedef struct _LpStateWakeClass        LpStateWakeClass;

typedef struct _LpStateAnalyze          LpStateAnalyze;
typedef struct _LpStateAnalyzeClass     LpStateAnalyzeClass;

typedef struct _LpStateDecide           LpStateDecide;
typedef struct _LpStateDecideClass      LpStateDecideClass;

typedef struct _LpStateSlumber          LpStateSlumber;
typedef struct _LpStateSlumberClass     LpStateSlumberClass;

typedef struct _LpStateSimulating       LpStateSimulating;
typedef struct _LpStateSimulatingClass  LpStateSimulatingClass;

typedef struct _LpStatePause            LpStatePause;
typedef struct _LpStatePauseClass       LpStatePauseClass;

typedef struct _LpStateSettings         LpStateSettings;
typedef struct _LpStateSettingsClass    LpStateSettingsClass;

typedef struct _LpStateFirstAwakening   LpStateFirstAwakening;
typedef struct _LpStateFirstAwakeningClass LpStateFirstAwakeningClass;

/* ==========================================================================
 * Achievement Types (src/achievement/)
 * ========================================================================== */

typedef struct _LpAchievementManager    LpAchievementManager;
typedef struct _LpAchievementManagerClass LpAchievementManagerClass;

/* ==========================================================================
 * Steam Types (src/steam/)
 * ========================================================================== */

typedef struct _LpSteamBridge           LpSteamBridge;
typedef struct _LpSteamBridgeClass      LpSteamBridgeClass;

G_END_DECLS

#endif /* LP_TYPES_H */
