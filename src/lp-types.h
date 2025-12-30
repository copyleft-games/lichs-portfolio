/* lp-types.h - Forward Declarations for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file contains forward declarations for all Lp* types.
 * Include this to avoid circular dependencies between headers.
 *
 * NOTE: For final types using G_DECLARE_FINAL_TYPE, we only forward
 * declare the instance struct, NOT the class struct (as GLib creates
 * an anonymous struct for final type classes).
 */

#ifndef LP_TYPES_H
#define LP_TYPES_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/* ==========================================================================
 * Core Types (src/core/)
 *
 * Final types: only instance struct forward declared
 * ========================================================================== */

typedef struct _LpApplication           LpApplication;
typedef struct _LpGameData              LpGameData;
typedef struct _LpPhylactery            LpPhylactery;
typedef struct _LpExposureManager       LpExposureManager;
typedef struct _LpSynergyManager        LpSynergyManager;
typedef struct _LpSynergy               LpSynergy;
typedef struct _LpLedger                LpLedger;
typedef struct _LpMegaproject           LpMegaproject;

/* Derivable types: both instance and class */
typedef struct _LpPrestigeManager       LpPrestigeManager;
typedef struct _LpPrestigeManagerClass  LpPrestigeManagerClass;

/* ==========================================================================
 * Simulation Types (src/simulation/)
 * ========================================================================== */

typedef struct _LpWorldSimulation       LpWorldSimulation;

/* Final types */
typedef struct _LpKingdom               LpKingdom;
typedef struct _LpRegion                LpRegion;
typedef struct _LpEventGenerator        LpEventGenerator;
typedef struct _LpCompetitor            LpCompetitor;

/* Derivable base class for events */
typedef struct _LpEvent                 LpEvent;
typedef struct _LpEventClass            LpEventClass;

/* Final event subtypes */
typedef struct _LpEventEconomic         LpEventEconomic;
typedef struct _LpEventPolitical        LpEventPolitical;
typedef struct _LpEventMagical          LpEventMagical;
typedef struct _LpEventPersonal         LpEventPersonal;

/* Boxed types */
typedef struct _LpEventChoice           LpEventChoice;

/* ==========================================================================
 * Investment Types (src/investment/)
 * ========================================================================== */

typedef struct _LpPortfolio             LpPortfolio;

/* Derivable base class */
typedef struct _LpInvestment            LpInvestment;
typedef struct _LpInvestmentClass       LpInvestmentClass;

/* Final investment subtypes */
typedef struct _LpInvestmentProperty    LpInvestmentProperty;
typedef struct _LpInvestmentTrade       LpInvestmentTrade;
typedef struct _LpInvestmentFinancial   LpInvestmentFinancial;
typedef struct _LpInvestmentMagical     LpInvestmentMagical;
typedef struct _LpInvestmentPolitical   LpInvestmentPolitical;
typedef struct _LpInvestmentDark        LpInvestmentDark;

/* ==========================================================================
 * Agent Types (src/agent/)
 * ========================================================================== */

typedef struct _LpAgentManager          LpAgentManager;

/* Derivable base class */
typedef struct _LpAgent                 LpAgent;
typedef struct _LpAgentClass            LpAgentClass;

/* Final agent subtypes */
typedef struct _LpAgentIndividual       LpAgentIndividual;
typedef struct _LpAgentFamily           LpAgentFamily;
typedef struct _LpAgentCult             LpAgentCult;
typedef struct _LpAgentBound            LpAgentBound;

typedef struct _LpTrait                 LpTrait;
typedef struct _LpTraitClass            LpTraitClass;

/* ==========================================================================
 * UI Types (src/ui/)
 * ========================================================================== */

typedef struct _LpScreenPortfolio       LpScreenPortfolio;
typedef struct _LpScreenWorldMap        LpScreenWorldMap;
typedef struct _LpScreenAgents          LpScreenAgents;
typedef struct _LpScreenIntelligence    LpScreenIntelligence;
typedef struct _LpScreenSlumber         LpScreenSlumber;
typedef struct _LpScreenLedger          LpScreenLedger;
typedef struct _LpScreenMegaprojects    LpScreenMegaprojects;
typedef struct _LpDialogEvent           LpDialogEvent;
typedef struct _LpWidgetExposureMeter   LpWidgetExposureMeter;
typedef struct _LpWidgetSynergyIndicator LpWidgetSynergyIndicator;

/* ==========================================================================
 * Feedback Types (src/feedback/)
 * ========================================================================== */

typedef struct _LpFloatingText          LpFloatingText;
typedef struct _LpGrowthParticles       LpGrowthParticles;
typedef struct _LpSynergyEffect         LpSynergyEffect;
typedef struct _LpAchievementPopup      LpAchievementPopup;
typedef struct _LpSlumberVisualization  LpSlumberVisualization;

/* ==========================================================================
 * State Types (src/states/)
 *
 * All game states are final types extending LrgGameState
 * ========================================================================== */

typedef struct _LpStateMainMenu         LpStateMainMenu;
typedef struct _LpStateWake             LpStateWake;
typedef struct _LpStateAnalyze          LpStateAnalyze;
typedef struct _LpStateDecide           LpStateDecide;
typedef struct _LpStateSlumber          LpStateSlumber;
typedef struct _LpStateSimulating       LpStateSimulating;
typedef struct _LpStatePause            LpStatePause;
typedef struct _LpStateSettings         LpStateSettings;
typedef struct _LpStateFirstAwakening   LpStateFirstAwakening;

/* ==========================================================================
 * Achievement Types (src/achievement/)
 * ========================================================================== */

typedef struct _LpAchievementManager    LpAchievementManager;

/* ==========================================================================
 * Save Types (src/save/)
 * ========================================================================== */

typedef struct _LpSaveManager           LpSaveManager;
typedef struct _LpSettingsManager       LpSettingsManager;

/* ==========================================================================
 * Steam Types (src/steam/)
 * ========================================================================== */

typedef struct _LpSteamBridge           LpSteamBridge;

G_END_DECLS

#endif /* LP_TYPES_H */
