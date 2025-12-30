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
├── LpInvestment (derivable, implements LrgSaveable) [Phase 2]
│   ├── LpInvestmentProperty (final) [Phase 2]
│   ├── LpInvestmentTrade (final) [Phase 2]
│   ├── LpInvestmentFinancial (final) [Phase 2]
│   ├── LpInvestmentMagical [Phase 2+]
│   ├── LpInvestmentPolitical [Phase 2+]
│   └── LpInvestmentDark [Phase 2+]
│
├── LpAgent (derivable, implements LrgSaveable) [Phase 3]
│   ├── LpAgentIndividual (final) [Phase 3]
│   ├── LpAgentFamily (final) [Phase 3]
│   ├── LpAgentCult [Phase 3+]
│   └── LpAgentBound [Phase 3+]
│
├── LpTrait (derivable, implements LrgSaveable) [Phase 3]
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

---

## Phase 2 Implementation Status

Phase 2 implements the investment system, providing the core financial mechanics.

### Investment Type Hierarchy

```
LpInvestment (derivable base class)
├── Virtual Methods:
│   ├── calculate_returns() - Calculate value growth over years
│   ├── apply_event() - Handle world events
│   ├── can_sell() - Check if sellable
│   ├── get_risk_modifier() - Risk calculation factor
│   └── get_base_return_rate() - Base annual return
│
├── LpInvestmentProperty (3-5% returns, low risk)
│   ├── Types: Agricultural, Urban, Mining, Timber, Coastal
│   ├── Features: Stability bonus, Improvements system
│   └── Max improvements: 5 per property
│
├── LpInvestmentTrade (5-8% returns, medium risk)
│   ├── Types: Route, Commodity, Guild, Shipping, Caravan
│   ├── Features: Route status, Market modifier
│   └── Signals: route-status-changed
│
└── LpInvestmentFinancial (4-12% returns, variable risk)
    ├── Types: Crown Bond, Noble Debt, Merchant Note, Insurance, Usury
    ├── Features: Interest rate, Face value, Debt status, Default handling
    └── Signals: debt-status-changed
```

### Implemented Components

| Component | File(s) | Status |
|-----------|---------|--------|
| LpInvestment | investment/lp-investment.h/.c | Base class, LrgSaveable |
| LpInvestmentProperty | investment/lp-investment-property.h/.c | Property investments |
| LpInvestmentTrade | investment/lp-investment-trade.h/.c | Trade investments |
| LpInvestmentFinancial | investment/lp-investment-financial.h/.c | Financial instruments |
| LpPortfolio (enhanced) | investment/lp-portfolio.h/.c | Full investment management |

### Portfolio Methods Added

```c
/* Investment management */
void          lp_portfolio_add_investment         (LpPortfolio *self, LpInvestment *inv);
gboolean      lp_portfolio_remove_investment      (LpPortfolio *self, LpInvestment *inv);
LpInvestment *lp_portfolio_get_investment_by_id   (LpPortfolio *self, const gchar *id);
GPtrArray    *lp_portfolio_get_investments_by_class (LpPortfolio *self, LpAssetClass class);
GPtrArray    *lp_portfolio_get_investments_by_risk (LpPortfolio *self, LpRiskLevel level);

/* Value calculations */
LrgBigNumber *lp_portfolio_get_total_value        (LpPortfolio *self);
LrgBigNumber *lp_portfolio_get_investment_value   (LpPortfolio *self);
LrgBigNumber *lp_portfolio_calculate_income       (LpPortfolio *self, guint years);

/* Slumber mechanics */
LrgBigNumber *lp_portfolio_apply_slumber          (LpPortfolio *self, guint years);
void          lp_portfolio_apply_event            (LpPortfolio *self, LpEvent *event);
```

### Tests

| Test File | Coverage |
|-----------|----------|
| test-exposure.c | Exposure thresholds, levels, decay |
| test-portfolio.c | Gold operations, LrgSaveable |
| test-ledger.c | Discovery tracking |
| test-game-data.c | GameData creation, child objects |
| test-investment.c | All investment types, portfolio management |

### Deferred to Later Phases

- Magical investments (Phase 2+)
- Political investments (Phase 2+)
- Dark investments (Phase 2+)
- World simulation logic (Phase 4)
- Event system (Phase 4)
- Graphics/UI (Phase 6)
- Steam integration (Phase 7)

---

## Phase 3 Implementation Status

Phase 3 implements the agent system with bloodline traits and inheritance mechanics.

### Agent Type Hierarchy

```
LpAgent (derivable base class, implements LrgSaveable)
├── Virtual Methods:
│   ├── on_year_passed() - Handle aging, loyalty decay
│   ├── on_death() - Handle agent death and succession
│   ├── on_betrayal() - Handle betrayal (exposure increase)
│   ├── can_recruit() - Check if can recruit successors
│   └── get_agent_type() - Return agent type enum
│
├── Properties:
│   ├── id (gchar*) - Unique identifier
│   ├── name (gchar*) - Display name
│   ├── age (guint) - Current age in years
│   ├── max_age (guint) - Maximum lifespan
│   ├── loyalty (gint 0-100) - Loyalty to the lich
│   ├── competence (gint 0-100) - Skill level
│   ├── cover_status (LpCoverStatus) - Cover identity status
│   ├── knowledge_level (LpKnowledgeLevel) - Knowledge of true master
│   ├── traits (GPtrArray of LpTrait)
│   └── assigned_investments (GPtrArray)
│
├── Signals:
│   ├── died - Emitted when agent dies
│   ├── betrayed - Emitted when agent betrays
│   └── loyalty-changed - Emitted when loyalty changes
│
├── LpAgentIndividual (final)
│   ├── Single mortal agent with explicit succession
│   ├── Properties: successor, training_progress
│   ├── Skill retention: 25% untrained, 75% trained successor
│   └── Training based on mentor's competence
│
└── LpAgentFamily (final)
    ├── Bloodline dynasty that spans generations
    ├── Properties: family_name, generation, founding_year, bloodline_traits
    ├── Generation advancement on head's death
    ├── Trait inheritance with generation bonus (+2% per gen, max 95%)
    ├── 5% base chance for new trait emergence per generation
    └── Maximum 4 traits per agent
```

### Trait System

```
LpTrait (derivable, implements LrgSaveable)
├── Virtual Methods:
│   ├── apply_effects() - Apply trait effects to agent
│   └── roll_inheritance() - Roll for inheritance success
│
├── Properties:
│   ├── id (gchar*) - Unique identifier
│   ├── name (gchar*) - Display name
│   ├── description (gchar*) - Detailed description
│   ├── inheritance_chance (gfloat 0-1) - Base inheritance probability
│   ├── income_modifier (gfloat) - Income multiplier (1.0 = no change)
│   ├── loyalty_modifier (gint) - Loyalty bonus/penalty
│   ├── discovery_modifier (gfloat) - Discovery chance multiplier
│   └── conflicts_with (GPtrArray) - Conflicting trait IDs
│
└── Key Methods:
    ├── lp_trait_copy() - Create trait copy
    ├── lp_trait_conflicts_with() - Check trait conflicts
    └── lp_trait_roll_inheritance() - Roll with generation bonus
```

### Agent Manager (Enhanced)

```c
/* Agent lifecycle management */
void          lp_agent_manager_add_agent           (LpAgentManager *self, LpAgent *agent);
gboolean      lp_agent_manager_remove_agent        (LpAgentManager *self, LpAgent *agent);
LpAgent      *lp_agent_manager_get_agent_by_id     (LpAgentManager *self, const gchar *id);

/* Filtering */
GList        *lp_agent_manager_get_available_agents (LpAgentManager *self);
GList        *lp_agent_manager_get_agents_by_type   (LpAgentManager *self, LpAgentType type);

/* Simulation */
void          lp_agent_manager_advance_years       (LpAgentManager *self, guint years);
void          lp_agent_manager_process_year        (LpAgentManager *self);
LpAgent      *lp_agent_manager_process_succession  (LpAgentManager *self, LpAgent *dying);

/* Statistics */
guint         lp_agent_manager_get_total_exposure  (LpAgentManager *self);
gint          lp_agent_manager_get_average_loyalty (LpAgentManager *self);
gint          lp_agent_manager_get_average_competence (LpAgentManager *self);
```

### Implemented Components

| Component | File(s) | Status |
|-----------|---------|--------|
| LpAgent | agent/lp-agent.h/.c | Base class, LrgSaveable |
| LpTrait | agent/lp-trait.h/.c | Trait system, LrgSaveable |
| LpAgentIndividual | agent/lp-agent-individual.h/.c | Individual agents |
| LpAgentFamily | agent/lp-agent-family.h/.c | Family bloodlines |
| LpAgentManager (enhanced) | agent/lp-agent-manager.h/.c | Full lifecycle management |

### Agent Enumerations

```c
/* Agent types */
typedef enum {
    LP_AGENT_TYPE_INDIVIDUAL,  /* Single mortal */
    LP_AGENT_TYPE_FAMILY,      /* Bloodline dynasty */
    LP_AGENT_TYPE_CULT,        /* Secret society (future) */
    LP_AGENT_TYPE_BOUND        /* Magically bound (future) */
} LpAgentType;

/* Cover identity status */
typedef enum {
    LP_COVER_STATUS_SECURE,    /* No suspicion */
    LP_COVER_STATUS_SUSPECTED, /* Under investigation */
    LP_COVER_STATUS_EXPOSED    /* Identity known */
} LpCoverStatus;

/* Knowledge of true master */
typedef enum {
    LP_KNOWLEDGE_LEVEL_NONE,    /* Doesn't know */
    LP_KNOWLEDGE_LEVEL_PARTIAL, /* Suspects something */
    LP_KNOWLEDGE_LEVEL_FULL     /* Knows everything */
} LpKnowledgeLevel;
```

### Tests

| Test File | Coverage |
|-----------|----------|
| test-agent.c | Base agent, traits, individual, family, manager |

### Key Design Decisions

1. **Derivable Base Classes**: Both `LpAgent` and `LpTrait` are derivable to allow future subtypes (Cult, Bound, specialized traits).

2. **Virtual Methods**: Agent behavior varies by type through virtual methods, enabling polymorphic processing.

3. **Trait Inheritance**: Family agents accumulate bloodline traits that pass down through generations with increasing probability.

4. **Generation Bonus**: Each generation increases inheritance chance by 2% (capped at 95%), rewarding long-serving families.

5. **Skill Retention**: Individual agents preserve 25-75% of skills through succession based on training.

6. **Exposure Contribution**: Agents contribute to overall exposure based on cover status and knowledge level.

### Deferred to Later Phases

- Cult agents (Phase 3+)
- Bound agents (Phase 3+)
- Agent recruitment UI (Phase 6)
- Agent management screens (Phase 6)

## Related Documents

- [Game Design Document](../design/GAME.md) - Full game design
- [Development Plan](../design/PLAN.md) - Technical implementation phases
- [Project Guide](../CLAUDE.md) - Code conventions and patterns
