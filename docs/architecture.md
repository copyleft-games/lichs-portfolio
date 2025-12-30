# Lich's Portfolio - System Architecture

## Overview

Lich's Portfolio is built on the libregnum game engine, using GLib/GObject for object-oriented C programming. The game follows an idle/incremental gameplay loop with a focus on long-term strategic investment.

## Core Loop

```
┌─────────────────────────────────────────────────────────────┐
│                    THE ETERNAL CYCLE                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│    ┌──────────┐                          ┌──────────┐      │
│    │  WAKE    │                          │  SLUMBER │      │
│    │          │                          │          │      │
│    │ • Review │                          │ • Time   │      │
│    │   results│                          │   passes │      │
│    │ • Handle │                          │ • Events │      │
│    │   crises │                          │   occur  │      │
│    │ • Collect│                          │ • Markets│      │
│    │   returns│                          │   shift  │      │
│    └────┬─────┘                          └────▲─────┘      │
│         │                                     │            │
│         ▼                                     │            │
│    ┌──────────┐                          ┌────┴─────┐      │
│    │  ANALYZE │                          │  COMMIT  │      │
│    │          ├─────────────────────────►│          │      │
│    │ • World  │                          │ • Set    │      │
│    │   state  │      ┌──────────┐        │   slumber│      │
│    │ • Trends │      │  DECIDE  │        │   length │      │
│    │ • Risks  │─────►│          │───────►│ • Lock   │      │
│    │ • Agents │      │ • Invest │        │   orders │      │
│    │          │      │ • Agents │        │ • Set    │      │
│    └──────────┘      │ • Hedge  │        │   wakers │      │
│                      │ • Divest │        └──────────┘      │
│                      └──────────┘                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## Module Dependencies

```
                          ┌─────────────┐
                          │LpApplication│
                          └──────┬──────┘
                                 │
         ┌───────────────────────┼───────────────────────┐
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐   ┌─────────────────┐   ┌─────────────────┐
│   LpGameData    │   │LrgGameStateManager│ │LpAchievementManager│
└────────┬────────┘   └────────┬────────┘   └─────────────────┘
         │                     │
    ┌────┴────┐               │
    │         │               │
    ▼         ▼               ▼
┌───────┐ ┌───────┐   ┌─────────────────┐
│LpPort-│ │LpAgent│   │   LpState*      │
│folio  │ │Manager│   │  (GameStates)   │
└───┬───┘ └───┬───┘   └────────┬────────┘
    │         │                │
    ▼         ▼                │
┌───────┐ ┌───────┐           │
│LpInve-│ │LpAgent│           │
│stment*│ │  *    │           │
└───────┘ └───────┘           │
                              │
                              ▼
                    ┌─────────────────┐
                    │LpWorldSimulation│
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
              ▼              ▼              ▼
         ┌────────┐    ┌────────┐    ┌──────────┐
         │LpKingdom│   │LpEvent*│    │LpCompet- │
         └────────┘    └────────┘    │itor     │
                                     └──────────┘
```

## Key libregnum Integrations

### LrgEngine

The central engine singleton provides access to all subsystems:

```c
LrgEngine *engine = lrg_engine_get_default ();
lrg_engine_startup (engine, &error);

/* Access subsystems */
LrgRegistry *registry = lrg_engine_get_registry (engine);
LrgDataLoader *loader = lrg_engine_get_data_loader (engine);
LrgAudioManager *audio = lrg_audio_manager_get_default ();
```

### LrgGameState

Game states manage different screens/modes. All extend `LrgGameState`:

| State | Purpose | Transparent | Blocking |
|-------|---------|-------------|----------|
| LpStateMainMenu | Title screen | No | Yes |
| LpStateWake | Wake report | No | Yes |
| LpStateAnalyze | World view | No | Yes |
| LpStateDecide | Investment actions | No | Yes |
| LpStateSlumber | Slumber config | No | Yes |
| LpStateSimulating | Time passage | No | Yes |
| LpStatePause | Pause menu | Yes | Yes |
| LpStateSettings | Settings | Yes | Yes |

### LrgSaveable

Interface for persistent objects. Implement for save/load support:

```c
static const gchar *
lp_game_data_get_save_id (LrgSaveable *saveable)
{
    return "game-data";
}

static gboolean
lp_game_data_save (LrgSaveable    *saveable,
                   LrgSaveContext *ctx,
                   GError        **error)
{
    LpGameData *self = LP_GAME_DATA (saveable);
    lrg_save_context_write_uint (ctx, "current-year", self->current_year);
    return TRUE;
}
```

### LrgBigNumber

Arbitrary precision numbers for large gold values:

```c
g_autoptr(LrgBigNumber) gold = lrg_big_number_new_double (1000.0);
g_autoptr(LrgBigNumber) rate = lrg_big_number_new_double (1.05);
g_autoptr(LrgBigNumber) years = lrg_big_number_new_uint (100);

/* Compound interest: gold * (rate ^ years) */
g_autoptr(LrgBigNumber) result = lrg_big_number_pow (rate, years);
result = lrg_big_number_multiply (gold, result);
```

### LrgIdleCalculator

Offline progression during slumber:

```c
LrgIdleCalculator *calc = lrg_idle_calculator_new ();
lrg_idle_calculator_set_base_income (calc, portfolio_income);
lrg_idle_calculator_set_time_ratio (calc, 10.0);  /* 1 real hour = 10 game years */

/* Calculate offline earnings */
LrgBigNumber *earnings = lrg_idle_calculator_calculate_offline (calc, seconds_elapsed);
```

## Data Flow

### Investment System

```
data/investments/*.yaml
         │
         ▼
┌─────────────────────┐
│   LrgDataLoader     │
└─────────┬───────────┘
          │
          ▼
┌─────────────────────┐
│   LpInvestment*     │ (Property, Trade, Financial, etc.)
└─────────┬───────────┘
          │
          ▼
┌─────────────────────┐
│    LpPortfolio      │
└─────────┬───────────┘
          │
          ▼
┌─────────────────────┐
│  LpSynergyManager   │ (detects bonuses)
└─────────────────────┘
```

### Event System

```
data/events/*.yaml
         │
         ▼
┌─────────────────────┐
│ LpEventGenerator    │
└─────────┬───────────┘
          │
          ▼
┌─────────────────────┐
│     LpEvent*        │ (Economic, Political, Magical, Personal)
└─────────┬───────────┘
          │
          ▼
┌─────────────────────┐
│ LpWorldSimulation   │
└─────────┬───────────┘
          │
    ┌─────┴─────┐
    │           │
    ▼           ▼
┌────────┐ ┌────────┐
│LpPort- │ │LpAgent │
│folio   │ │Manager │
└────────┘ └────────┘
```

## File Organization

### Source Modules

| Directory | Purpose |
|-----------|---------|
| src/core/ | Application, GameData, Phylactery, Prestige |
| src/simulation/ | WorldSimulation, Kingdom, Region, Event |
| src/investment/ | Investment types, Portfolio |
| src/agent/ | Agent types, AgentManager, Traits |
| src/ui/ | Screens, Widgets, Theme |
| src/states/ | Game states |
| src/feedback/ | Visual feedback (particles, popups) |
| src/achievement/ | Achievement tracking |
| src/steam/ | Steam integration (optional) |

### Data Files (YAML)

| Directory | Purpose |
|-----------|---------|
| data/kingdoms/ | Kingdom definitions |
| data/investments/ | Investment type definitions |
| data/events/ | Event templates |
| data/agents/ | Agent templates |
| data/upgrades/ | Phylactery upgrades |
| data/achievements/ | Achievement definitions |
| data/synergies/ | Investment synergy definitions |
| data/traits/ | Bloodline trait definitions |
| data/megaprojects/ | Multi-century project definitions |
| data/ledger/ | Discoverable information |
| data/eras/ | Era transition definitions |

## GObject Type Hierarchy

```
GObject
├── LpApplication (singleton) [Phase 1]
├── LpGameData (implements LrgSaveable) [Phase 1]
│   ├── owns LpPortfolio
│   ├── owns LpAgentManager
│   ├── owns LpPhylactery
│   ├── owns LpLedger
│   └── owns LpWorldSimulation
│
├── LpPhylactery (implements LrgSaveable) [Phase 1]
├── LpPortfolio (implements LrgSaveable) [Phase 1]
├── LpAgentManager (implements LrgSaveable) [Phase 1 skeleton]
├── LpLedger (implements LrgSaveable) [Phase 1]
├── LpWorldSimulation (implements LrgSaveable) [Phase 1 skeleton]
│
├── LpExposureManager (singleton) [Phase 1]
├── LpSynergyManager (singleton) [Phase 1 skeleton]
├── LpAchievementManager (singleton, implements LrgSaveable) [Phase 1 skeleton]
│
├── LpInvestment (derivable) [Phase 2+]
│   ├── LpInvestmentProperty
│   ├── LpInvestmentTrade
│   ├── LpInvestmentFinancial
│   ├── LpInvestmentMagical
│   ├── LpInvestmentPolitical
│   └── LpInvestmentDark
│
├── LpAgent (derivable) [Phase 3+]
│   ├── LpAgentIndividual
│   ├── LpAgentFamily
│   ├── LpAgentCult
│   └── LpAgentBound
│
├── LrgGameState (from libregnum, derivable)
│   ├── LpStateMainMenu [Phase 1]
│   ├── LpStateWake [Phase 1 skeleton]
│   ├── LpStateAnalyze [Phase 1 skeleton]
│   ├── LpStateDecide [Phase 1 skeleton]
│   ├── LpStateSlumber [Phase 1 skeleton]
│   ├── LpStateSimulating [Phase 1]
│   ├── LpStatePause [Phase 1 skeleton]
│   └── LpStateSettings [Phase 1 skeleton]
│
├── LpKingdom [Phase 4+]
├── LpRegion [Phase 4+]
├── LpEvent (derivable) [Phase 4+]
└── LpCompetitor [Phase 5+]
```

## Build System

The build system uses GNU Make with the following structure:

```
config.mk    - Build configuration (version, flags, paths)
rules.mk     - Helper functions (color output, directory creation)
Makefile     - Root orchestration
src/Makefile - Game source compilation
tests/Makefile - Test compilation and execution
```

### Key Make Targets

| Target | Purpose |
|--------|---------|
| `make` | Build deps + game |
| `make DEBUG=1` | Debug build |
| `make WINDOWS=1` | Cross-compile for Windows |
| `make STEAM=1` | Build with Steam SDK |
| `make test` | Run unit tests |
| `make clean` | Remove build artifacts |

## Testing

Tests use GLib's GTest framework:

```c
static void
test_investment_returns (InvestmentFixture *fixture,
                         gconstpointer      user_data)
{
    g_autoptr(LrgBigNumber) price = lrg_big_number_new_double (1000.0);
    lp_investment_set_purchase_price (LP_INVESTMENT (fixture->property), price);

    g_autoptr(LrgBigNumber) returns = lp_investment_calculate_returns (
        LP_INVESTMENT (fixture->property), 10);

    g_assert_nonnull (returns);
    g_assert_cmpfloat (lrg_big_number_to_double (returns), >, 1000.0);
}
```

Run with: `make test`

## Phase 1 Implementation Status

Phase 1 establishes the core architecture with skeleton implementations. The game can run but has minimal gameplay.

### Implemented

| Component | File(s) | Status |
|-----------|---------|--------|
| LpApplication | core/lp-application.h/.c | Singleton, main loop |
| LpGameData | core/lp-game-data.h/.c | Central game state, LrgSaveable |
| LpPortfolio | investment/lp-portfolio.h/.c | Gold tracking, LrgSaveable |
| LpAgentManager | agent/lp-agent-manager.h/.c | Skeleton, LrgSaveable |
| LpPhylactery | core/lp-phylactery.h/.c | Upgrade points, LrgSaveable |
| LpLedger | core/lp-ledger.h/.c | Discovery tracking, LrgSaveable |
| LpWorldSimulation | simulation/lp-world-simulation.h/.c | Skeleton, LrgSaveable |
| LpExposureManager | core/lp-exposure-manager.h/.c | Exposure tracking singleton |
| LpSynergyManager | core/lp-synergy-manager.h/.c | Skeleton singleton |
| LpAchievementManager | achievement/lp-achievement-manager.h/.c | Skeleton singleton |
| LpStateMainMenu | states/lp-state-main-menu.h/.c | Menu state |
| LpStateWake | states/lp-state-wake.h/.c | Skeleton |
| LpStateAnalyze | states/lp-state-analyze.h/.c | Skeleton |
| LpStateDecide | states/lp-state-decide.h/.c | Skeleton |
| LpStateSlumber | states/lp-state-slumber.h/.c | Skeleton |
| LpStateSimulating | states/lp-state-simulating.h/.c | Year counter |
| LpStatePause | states/lp-state-pause.h/.c | Overlay skeleton |
| LpStateSettings | states/lp-state-settings.h/.c | Overlay skeleton |

### Tests

| Test File | Coverage |
|-----------|----------|
| test-exposure.c | Exposure thresholds, levels, decay |
| test-portfolio.c | Gold operations, LrgSaveable |
| test-ledger.c | Discovery tracking |
| test-game-data.c | GameData creation, child objects |

### Deferred to Later Phases

- Investment types (Phase 2)
- Agent types (Phase 3)
- World simulation logic (Phase 4)
- Event system (Phase 4)
- Graphics/UI (Phase 6)
- Steam integration (Phase 7)

## Related Documents

- [Game Design Document](../design/GAME.md) - Full game design
- [Development Plan](../design/PLAN.md) - Technical implementation phases
- [Project Guide](../CLAUDE.md) - Code conventions and patterns
