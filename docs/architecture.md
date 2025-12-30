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
├── LpRegion (implements LrgSaveable) [Phase 4]
├── LpKingdom (implements LrgSaveable) [Phase 4]
├── LpEvent (derivable, implements LrgSaveable) [Phase 4]
│   ├── LpEventEconomic (final)
│   ├── LpEventPolitical (final)
│   ├── LpEventMagical (final)
│   └── LpEventPersonal (final)
├── LpEventGenerator (singleton) [Phase 4]
└── LpCompetitor (implements LrgSaveable) [Phase 4]
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

---

## Phase 4 Implementation Status

Phase 4 implements the world simulation with kingdoms, regions, events, and immortal competitors.

### Region System

```
LpRegion (final, implements LrgSaveable)
├── Properties:
│   ├── id (gchar*) - Unique identifier
│   ├── name (gchar*) - Display name
│   ├── geography-type (LpGeographyType) - Terrain type
│   ├── owning-kingdom-id (gchar*) - Current owner
│   ├── population (guint) - Population size
│   ├── resource-modifier (gdouble) - Resource production bonus
│   └── trade-connected (gboolean) - Connected to trade network
│
├── Geography Types:
│   ├── COASTAL (trade bonus)
│   ├── INLAND (agriculture bonus)
│   ├── MOUNTAIN (mining bonus)
│   ├── FOREST (lumber bonus)
│   ├── DESERT (magical bonus)
│   └── SWAMP (dark arts bonus)
│
└── Signals:
    ├── ownership-changed (old_id, new_id)
    └── devastated
```

### Kingdom System

```
LpKingdom (final, implements LrgSaveable)
├── Core Attributes (0-100):
│   ├── stability - Government stability (low = collapse risk)
│   ├── prosperity - Economic health
│   ├── military - War capability
│   ├── culture - Resistance to change
│   └── tolerance - Magic/undead acceptance
│
├── State Properties:
│   ├── ruler-name (gchar*) - Current ruler
│   ├── dynasty-years (guint) - Years of current dynasty
│   ├── is-collapsed (gboolean) - Kingdom has collapsed
│   ├── at-war-with-id (gchar*) - Current enemy
│   └── regions (GPtrArray) - Controlled region IDs
│
├── Diplomatic Relations (GHashTable):
│   ├── ALLIANCE - Military allies
│   ├── NEUTRAL - Default state
│   ├── RIVALRY - Economic competition
│   ├── WAR - Active conflict
│   └── VASSALAGE - Subordinate relationship
│
├── Methods:
│   ├── lp_kingdom_tick_year() - Annual attribute changes
│   ├── lp_kingdom_roll_collapse() - Check for collapse
│   ├── lp_kingdom_roll_war() - Check for war initiation
│   └── lp_kingdom_roll_crusade() - Check for anti-undead crusade
│
└── Signals:
    ├── attribute-changed (attr, old, new)
    ├── collapsed
    ├── war-declared (enemy_id)
    ├── war-ended (enemy_id)
    └── crusade-launched
```

### Event System

```
LpEvent (derivable base class, implements LrgSaveable)
├── Virtual Methods:
│   ├── apply_effects() - Apply event to world state
│   ├── get_choices() - Get player choices (nullable)
│   ├── get_investment_modifier() - Modify investment returns
│   ├── get_narrative_text() - Generate description text
│   └── can_occur() - Check if event can trigger
│
├── Properties:
│   ├── id, name, description (gchar*)
│   ├── event-type (LpEventType)
│   ├── severity (LpEventSeverity)
│   ├── year-occurred (guint64)
│   ├── affects-region-id, affects-kingdom-id (gchar*)
│   ├── duration-years (guint, 0 = instant)
│   └── is-active (gboolean)
│
├── Severity Levels:
│   ├── MINOR - Local impact
│   ├── MODERATE - Regional impact
│   ├── MAJOR - Kingdom-wide impact
│   └── CATASTROPHIC - World-changing
│
├── LpEventEconomic (final)
│   ├── market-modifier (gdouble) - Market adjustment factor
│   └── affected-asset-class (LpAssetClass)
│
├── LpEventPolitical (final)
│   ├── stability-impact (gint) - Kingdom stability change
│   └── causes-war (gboolean)
│
├── LpEventMagical (final)
│   ├── exposure-impact (gint) - Lich exposure change
│   └── affects-dark-investments (gboolean)
│
└── LpEventPersonal (final)
    ├── target-agent-id (gchar*) - Affected agent
    ├── is-betrayal (gboolean) - Agent betrayal event
    └── is-death (gboolean) - Agent death event
```

### LpEventChoice (GBoxed)

```c
struct _LpEventChoice {
    gchar    *id;              /* Choice identifier */
    gchar    *text;            /* Display text */
    gchar    *consequence;     /* Result description */
    gboolean  requires_gold;   /* Needs gold to select */
    guint64   gold_cost;       /* Gold cost if required */
    gboolean  requires_agent;  /* Needs available agent */
};
```

### Event Generator

```
LpEventGenerator (singleton)
├── Properties:
│   ├── base-yearly-event-chance (gdouble, default 0.3)
│   ├── base-decade-event-chance (gdouble, default 0.7)
│   └── base-era-event-chance (gdouble, default 0.9)
│
├── Methods:
│   ├── lp_event_generator_get_default() - Singleton accessor
│   ├── lp_event_generator_generate_yearly_events() -> GList
│   ├── lp_event_generator_generate_decade_events() -> GList
│   └── lp_event_generator_generate_era_events() -> GList
│
└── Event Creation:
    ├── lp_event_generator_create_economic_event()
    ├── lp_event_generator_create_political_event()
    ├── lp_event_generator_create_magical_event()
    └── lp_event_generator_create_personal_event()
```

### Competitor System

```
LpCompetitor (final, implements LrgSaveable)
├── Properties:
│   ├── id, name (gchar*)
│   ├── competitor-type (LpCompetitorType)
│   ├── stance (LpCompetitorStance)
│   ├── power-level, aggression, greed, cunning (gint 0-100)
│   ├── territory-region-ids (GPtrArray)
│   ├── wealth (LrgBigNumber*)
│   ├── is-active (gboolean)
│   └── is-known (gboolean) - Discovered by player
│
├── Competitor Types:
│   ├── DRAGON - Hoards wealth, territorial
│   ├── VAMPIRE - Political influence, blood bonds
│   ├── LICH - Fellow undead, magical competition
│   ├── FAE - Trickster, long-term schemes
│   └── DEMON - Corruption, soul contracts
│
├── Stance Toward Player:
│   ├── UNKNOWN - Not yet encountered
│   ├── WARY - Cautious observation
│   ├── NEUTRAL - Neither friend nor foe
│   ├── FRIENDLY - Potential ally
│   ├── HOSTILE - Active opposition
│   └── ALLIED - Formal alliance
│
├── AI Components:
│   ├── behavior_tree (LrgBehaviorTree*) - Decision making
│   └── blackboard (LrgBlackboard*) - State storage
│
├── Methods:
│   ├── lp_competitor_tick_year() - Annual AI decisions
│   ├── lp_competitor_react_to_event() - Respond to world events
│   ├── lp_competitor_expand_territory() - Territorial expansion
│   └── lp_competitor_get_player_threat_level() - Calculate threat
│
└── Signals:
    ├── discovered - Player learns of competitor
    ├── stance-changed (old, new)
    ├── territory-expanded (region_id)
    ├── territory-lost (region_id)
    ├── destroyed
    ├── alliance-proposed
    └── conflict-declared
```

### World Simulation (Enhanced)

```
LpWorldSimulation (final, implements LrgSaveable)
├── New Collections:
│   ├── kingdoms (GPtrArray of LpKingdom*)
│   ├── regions (GPtrArray of LpRegion*)
│   ├── competitors (GPtrArray of LpCompetitor*)
│   └── active_events (GPtrArray of LpEvent*)
│
├── Components:
│   └── event_generator (LpEventGenerator*) - Event generation
│
├── New Methods:
│   ├── lp_world_simulation_add_kingdom()
│   ├── lp_world_simulation_get_kingdom_by_id()
│   ├── lp_world_simulation_remove_kingdom()
│   ├── lp_world_simulation_add_region()
│   ├── lp_world_simulation_get_region_by_id()
│   ├── lp_world_simulation_add_competitor()
│   ├── lp_world_simulation_get_competitor_by_id()
│   └── lp_world_simulation_get_known_competitors()
│
├── Updated advance_year Flow:
│   1. Increment year
│   2. Update economic cycle
│   3. Tick all kingdoms (attribute drift, war checks)
│   4. Tick all competitors (AI decisions)
│   5. Generate yearly events
│   6. Generate decade events (year % 10 == 0)
│   7. Generate era events (year % 100 == 0)
│   8. Apply events to world state
│   9. Emit signals for major changes
│   10. Return events list
│
└── New Signals:
    ├── kingdom-collapsed (kingdom_id)
    ├── war-started (kingdom1_id, kingdom2_id)
    ├── war-ended (kingdom1_id, kingdom2_id)
    └── competitor-discovered (competitor_id)
```

### Updated Type Hierarchy

```
GObject
├── ... (Phase 1-3 types unchanged)
│
├── LpRegion (final, implements LrgSaveable) [Phase 4]
├── LpKingdom (final, implements LrgSaveable) [Phase 4]
│
├── LpEvent (derivable, implements LrgSaveable) [Phase 4]
│   ├── LpEventEconomic (final)
│   ├── LpEventPolitical (final)
│   ├── LpEventMagical (final)
│   └── LpEventPersonal (final)
│
├── LpEventGenerator (singleton) [Phase 4]
│
└── LpCompetitor (final, implements LrgSaveable) [Phase 4]
    └── Uses: LrgBehaviorTree, LrgBlackboard
```

### Implemented Components

| Component | File(s) | Status |
|-----------|---------|--------|
| LpRegion | simulation/lp-region.h/.c | Geography, trade routes |
| LpKingdom | simulation/lp-kingdom.h/.c | 5 core attributes, diplomacy |
| LpEvent | simulation/lp-event.h/.c | Derivable base, LpEventChoice |
| LpEventEconomic | simulation/lp-event-economic.h/.c | Market events |
| LpEventPolitical | simulation/lp-event-political.h/.c | Political events |
| LpEventMagical | simulation/lp-event-magical.h/.c | Magical events |
| LpEventPersonal | simulation/lp-event-personal.h/.c | Agent events |
| LpEventGenerator | simulation/lp-event-generator.h/.c | Weighted generation |
| LpCompetitor | simulation/lp-competitor.h/.c | AI with behavior tree |
| LpWorldSimulation | simulation/lp-world-simulation.h/.c | Full integration |

### New Enumerations

```c
/* Region geography types */
typedef enum {
    LP_GEOGRAPHY_TYPE_COASTAL,
    LP_GEOGRAPHY_TYPE_INLAND,
    LP_GEOGRAPHY_TYPE_MOUNTAIN,
    LP_GEOGRAPHY_TYPE_FOREST,
    LP_GEOGRAPHY_TYPE_DESERT,
    LP_GEOGRAPHY_TYPE_SWAMP
} LpGeographyType;

/* Kingdom diplomatic relations */
typedef enum {
    LP_KINGDOM_RELATION_ALLIANCE,
    LP_KINGDOM_RELATION_NEUTRAL,
    LP_KINGDOM_RELATION_RIVALRY,
    LP_KINGDOM_RELATION_WAR,
    LP_KINGDOM_RELATION_VASSALAGE
} LpKingdomRelation;

/* Immortal competitor types */
typedef enum {
    LP_COMPETITOR_TYPE_DRAGON,
    LP_COMPETITOR_TYPE_VAMPIRE,
    LP_COMPETITOR_TYPE_LICH,
    LP_COMPETITOR_TYPE_FAE,
    LP_COMPETITOR_TYPE_DEMON
} LpCompetitorType;

/* Competitor stance toward player */
typedef enum {
    LP_COMPETITOR_STANCE_UNKNOWN,
    LP_COMPETITOR_STANCE_WARY,
    LP_COMPETITOR_STANCE_NEUTRAL,
    LP_COMPETITOR_STANCE_FRIENDLY,
    LP_COMPETITOR_STANCE_HOSTILE,
    LP_COMPETITOR_STANCE_ALLIED
} LpCompetitorStance;

/* Event severity levels */
typedef enum {
    LP_EVENT_SEVERITY_MINOR,
    LP_EVENT_SEVERITY_MODERATE,
    LP_EVENT_SEVERITY_MAJOR,
    LP_EVENT_SEVERITY_CATASTROPHIC
} LpEventSeverity;
```

### Tests

| Test File | Coverage |
|-----------|----------|
| test-simulation.c | Regions, kingdoms, events, generator, competitors, world simulation |

### Key Design Decisions

1. **Behavior Tree AI**: Competitors use libregnum's `LrgBehaviorTree` and `LrgBlackboard` for flexible decision-making that can be data-driven.

2. **Event Polymorphism**: Base `LpEvent` class with virtual methods allows different event types to implement custom logic while sharing common infrastructure.

3. **Kingdom Attributes**: The 5 core attributes (stability, prosperity, military, culture, tolerance) drive all kingdom behavior and event probabilities.

4. **Discovery Mechanics**: Competitors start unknown (`is-known = FALSE`) and are revealed through events or player investigation.

5. **Weighted Event Generation**: Events use weighted probabilities influenced by world state, ensuring dynamic and contextual event selection.

6. **Singleton Event Generator**: Single generator instance ensures consistent event creation and probability management.

### Deferred to Later Phases

- Region conquest mechanics (Phase 4+)
- Detailed diplomatic actions (Phase 4+)
- Competitor alliance negotiation UI (Phase 6)
- Event choice interface (Phase 6)
- Map visualization (Phase 6)

---

## Phase 5 Implementation Status

Phase 5 implements the progression systems including prestige mechanics, upgrade trees, megaprojects, and enhanced discovery tracking.

### Prestige Manager

```
LpPrestigeManager (derivable, implements LrgSaveable)
├── Virtual Methods:
│   ├── calculate_echo_reward() - Calculate Echoes gained from prestige
│   ├── can_prestige() - Check if prestige requirements are met
│   ├── on_prestige() - Called when prestige is performed
│   └── get_bonus_multiplier() - Calculate prestige bonus multiplier
│
├── Echo Management:
│   ├── echoes (LrgBigNumber*) - Current Echo count
│   ├── total_echoes_earned (LrgBigNumber*) - Lifetime Echoes
│   └── times_prestiged (guint64) - Prestige count
│
├── Echo Specialization Trees (4 trees using LrgUnlockTree):
│   ├── LP_ECHO_TREE_ECONOMIST - Starting gold, compound interest
│   ├── LP_ECHO_TREE_MANIPULATOR - Agent and political bonuses
│   ├── LP_ECHO_TREE_SCHOLAR - Ledger retention, knowledge bonuses
│   └── LP_ECHO_TREE_ARCHITECT - Gold retention, structure bonuses
│
├── Bonus Queries:
│   ├── lp_prestige_manager_get_starting_gold_multiplier()
│   ├── lp_prestige_manager_get_compound_interest_bonus()
│   ├── lp_prestige_manager_get_ledger_retention()
│   └── lp_prestige_manager_get_gold_retention()
│
└── Signals:
    └── prestige-performed (echoes_gained)
```

### Phylactery (Enhanced)

```
LpPhylactery (final, implements LrgSaveable)
├── Points Management:
│   ├── points (guint64) - Available phylactery points
│   ├── total_points_earned (guint64) - Lifetime points
│   └── level (guint) - Derived from total upgrades
│
├── Upgrade Categories (5 trees using LrgUnlockTree):
│   ├── LP_UPGRADE_CATEGORY_TEMPORAL - Slumber duration, time efficiency
│   │   ├── extended-slumber-1/2/3 - +50/100/150 years max slumber
│   │   ├── time-mastery-1/2 - +10/20% time efficiency
│   │   └── temporal-anchor - Reduce event impact during slumber
│   │
│   ├── LP_UPGRADE_CATEGORY_NETWORK - Agent capacity and types
│   │   ├── expanded-network-1/2/3 - +1/2/3 max agents
│   │   ├── family-lineages - Unlock family agents
│   │   └── cult-followers - Unlock cult agents
│   │
│   ├── LP_UPGRADE_CATEGORY_DIVINATION - Predictions and warnings
│   │   ├── foresight-1/2/3 - +10/20/30% prediction accuracy
│   │   ├── early-warning-1/2 - +10/20 years event warning
│   │   └── market-sense - See market trends
│   │
│   ├── LP_UPGRADE_CATEGORY_RESILIENCE - Survival and recovery
│   │   ├── hardened-assets-1/2 - +10/20% disaster survival
│   │   ├── rapid-recovery-1/2 - +25/50% recovery speed
│   │   └── shadow-ward - Reduce exposure decay
│   │
│   └── LP_UPGRADE_CATEGORY_DARK_ARTS - Dark investments (hidden)
│       ├── dark-initiation - Unlock dark investments
│       ├── soul-binding - Unlock bound agents
│       └── dark-mastery - +50% dark investment income
│
├── Bonus Calculations:
│   ├── lp_phylactery_get_max_slumber_years() - Base 100 + upgrades
│   ├── lp_phylactery_get_time_efficiency_bonus() - 1.0 + upgrades
│   ├── lp_phylactery_get_max_agents() - Base 3 + upgrades
│   ├── lp_phylactery_get_prediction_bonus() - 0-100%
│   ├── lp_phylactery_get_warning_years() - Event warning period
│   ├── lp_phylactery_get_disaster_survival_bonus() - 0-100%
│   ├── lp_phylactery_get_recovery_bonus() - Multiplier
│   └── lp_phylactery_get_dark_income_bonus() - Multiplier
│
├── Unlock Queries:
│   ├── lp_phylactery_has_family_agents()
│   ├── lp_phylactery_has_cult_agents()
│   ├── lp_phylactery_has_dark_investments()
│   └── lp_phylactery_has_bound_agents()
│
└── Methods:
    ├── lp_phylactery_add_points() - Add points from prestige
    ├── lp_phylactery_purchase_upgrade() - Buy upgrade from tree
    ├── lp_phylactery_has_upgrade() - Check if owned
    └── lp_phylactery_can_purchase_upgrade() - Check if available
```

### Megaproject System

```
LpMegaproject (final, implements LrgSaveable)
├── Properties:
│   ├── id, name, description (gchar*)
│   ├── cost_per_year (LrgBigNumber*) - Ongoing maintenance cost
│   ├── unlock_level (guint) - Required phylactery level
│   └── discovery_risk (guint 0-100) - Risk per decade
│
├── States (LpMegaprojectState):
│   ├── LOCKED - Not yet available
│   ├── AVAILABLE - Can be started
│   ├── ACTIVE - In progress
│   ├── PAUSED - Progress preserved, no cost
│   ├── DISCOVERED - Found by enemies
│   ├── COMPLETE - All phases finished
│   └── DESTROYED - Lost to enemy action
│
├── Phases (GPtrArray of LpMegaprojectPhase):
│   ├── name (gchar*) - Phase name
│   ├── years (guint) - Years to complete
│   ├── effect_type (gchar*) - Effect when complete
│   └── effect_value (gdouble) - Effect magnitude
│
├── Progress Tracking:
│   ├── years_invested (guint) - Total years worked
│   ├── current_phase_index (guint) - Current phase
│   ├── years_in_current_phase (guint) - Progress in phase
│   └── total_duration (guint) - Sum of all phase years
│
├── Effect Types:
│   ├── property_income_bonus - Increase property returns
│   ├── agent_travel - Enable instant agent travel
│   └── property_immune_seizure - Protect from seizure
│
├── Methods:
│   ├── lp_megaproject_can_start() - Check level requirement
│   ├── lp_megaproject_start() - Begin project
│   ├── lp_megaproject_pause() - Suspend work
│   ├── lp_megaproject_resume() - Continue work
│   ├── lp_megaproject_advance_years() - Progress phases
│   ├── lp_megaproject_roll_discovery() - Risk check
│   ├── lp_megaproject_destroy() - Enemy destruction
│   └── lp_megaproject_hide() - Re-conceal discovered
│
└── Signals:
    ├── state-changed (old, new)
    ├── phase-completed (index, phase)
    ├── discovered
    ├── destroyed
    └── completed
```

### LpMegaprojectPhase (GBoxed)

```c
struct _LpMegaprojectPhase {
    gchar   *name;          /* Phase name */
    guint    years;         /* Years to complete */
    gchar   *effect_type;   /* Effect type string */
    gdouble  effect_value;  /* Effect magnitude */
};
```

### Ledger (Enhanced)

```
LpLedger (final, implements LrgSaveable)
├── Entry Structure:
│   ├── entry_id (gchar*) - Unique identifier
│   ├── category (LpLedgerCategory) - Entry category
│   ├── occurrences_required (guint) - Needed for discovery
│   ├── occurrences_current (guint) - Current progress
│   └── is_discovered (gboolean) - Fully discovered flag
│
├── Discovery Methods (LpDiscoveryMethod):
│   ├── LP_DISCOVERY_METHOD_MANUAL - Debug/testing
│   ├── LP_DISCOVERY_METHOD_AGENT_REPORT - Random from agents
│   ├── LP_DISCOVERY_METHOD_EVENT_SURVIVAL - Surviving events
│   ├── LP_DISCOVERY_METHOD_COMPETITOR - Immortal interaction
│   ├── LP_DISCOVERY_METHOD_ACHIEVEMENT - Achievement reward
│   └── LP_DISCOVERY_METHOD_MILESTONE - Investment milestone
│
├── Registration:
│   ├── lp_ledger_register_entry() - Pre-register multi-occurrence
│   └── lp_ledger_is_registered() - Check if registered
│
├── Progress Tracking:
│   ├── lp_ledger_progress_entry() - Advance by one occurrence
│   ├── lp_ledger_get_progress() - Get current occurrences
│   ├── lp_ledger_get_required_occurrences() - Get required
│   └── lp_ledger_get_progress_fraction() - Progress 0.0-1.0
│
├── Query Methods:
│   ├── lp_ledger_has_discovered() - Full discovery check
│   ├── lp_ledger_has_started() - Progress > 0 check
│   ├── lp_ledger_get_discovered_count() - Total discoveries
│   ├── lp_ledger_get_in_progress_count() - Partial progress
│   ├── lp_ledger_get_all_discoveries() - List all discovered
│   └── lp_ledger_get_all_in_progress() - List in-progress
│
├── Prestige:
│   └── lp_ledger_apply_retention() - Keep fraction on prestige
│
└── Signals:
    ├── entry-discovered (entry_id, category)
    └── entry-progressed (entry_id, current, required)
```

### New Enumerations

```c
/* Phylactery upgrade categories */
typedef enum {
    LP_UPGRADE_CATEGORY_TEMPORAL,    /* Slumber duration, time efficiency */
    LP_UPGRADE_CATEGORY_NETWORK,     /* Agent capacity and types */
    LP_UPGRADE_CATEGORY_DIVINATION,  /* Predictions and warnings */
    LP_UPGRADE_CATEGORY_RESILIENCE,  /* Survival and recovery */
    LP_UPGRADE_CATEGORY_DARK_ARTS    /* Dark investments (hidden) */
} LpUpgradeCategory;

/* Megaproject states */
typedef enum {
    LP_MEGAPROJECT_STATE_LOCKED,     /* Not yet available */
    LP_MEGAPROJECT_STATE_AVAILABLE,  /* Can be started */
    LP_MEGAPROJECT_STATE_ACTIVE,     /* In progress */
    LP_MEGAPROJECT_STATE_PAUSED,     /* Progress preserved */
    LP_MEGAPROJECT_STATE_DISCOVERED, /* Found by enemies */
    LP_MEGAPROJECT_STATE_COMPLETE,   /* All phases done */
    LP_MEGAPROJECT_STATE_DESTROYED   /* Lost to enemy action */
} LpMegaprojectState;

/* Echo specialization trees */
typedef enum {
    LP_ECHO_TREE_ECONOMIST,   /* Starting gold, compound interest */
    LP_ECHO_TREE_MANIPULATOR, /* Agent and political bonuses */
    LP_ECHO_TREE_SCHOLAR,     /* Ledger retention, knowledge */
    LP_ECHO_TREE_ARCHITECT    /* Gold retention, structures */
} LpEchoTree;
```

### Updated Type Hierarchy

```
GObject
├── ... (Phase 1-4 types)
│
├── LpPrestigeManager (derivable, implements LrgSaveable) [Phase 5]
│   └── Uses: LrgUnlockTree (4 echo trees)
│
├── LpMegaproject (final, implements LrgSaveable) [Phase 5]
│   └── Contains: GPtrArray of LpMegaprojectPhase
│
├── LpPhylactery (enhanced) [Phase 5]
│   └── Uses: LrgUnlockTree (5 upgrade trees)
│
└── LpLedger (enhanced) [Phase 5]
    └── Progress tracking with occurrences
```

### Implemented Components

| Component | File(s) | Status |
|-----------|---------|--------|
| LpPrestigeManager | core/lp-prestige-manager.h/.c | Derivable, echo trees |
| LpMegaproject | core/lp-megaproject.h/.c | Multi-century projects |
| LpPhylactery (enhanced) | core/lp-phylactery.h/.c | 5 upgrade trees |
| LpLedger (enhanced) | core/lp-ledger.h/.c | Progress tracking |

### Tests

| Test File | Coverage |
|-----------|----------|
| test-progression.c | Prestige, megaprojects, phylactery, ledger progress |

### Key Design Decisions

1. **Derivable Prestige Manager**: Allows subclassing for testing and future variations in prestige mechanics.

2. **Echo Trees**: Four specialization trees using `LrgUnlockTree` provide permanent bonuses that persist across prestige resets.

3. **Upgrade Categories**: Five phylactery categories organize upgrades by theme, with Dark Arts hidden until specific conditions are met.

4. **Multi-Occurrence Discoveries**: Some ledger entries require multiple occurrences to fully discover, adding depth to the discovery system.

5. **Megaproject Phases**: Breaking large projects into phases provides incremental benefits and progress milestones.

6. **Discovery Risk**: Megaprojects can be discovered by enemies, adding tension and risk management to long-term investments.

7. **Prestige Retention**: Scholar tree upgrades allow retaining ledger discoveries across prestige, rewarding meta-progression investment.

### Deferred to Later Phases

- Echo tree unlock UI (Phase 6)
- Megaproject management screens (Phase 6)
- Phylactery upgrade visualization (Phase 6)
- Discovery journal UI (Phase 6)
- Idle calculations integration (Phase 5+)

---

## Phase 6 Implementation Status

Phase 6 implements the user interface system including theme configuration, custom widgets, screens, and dialogs.

### Theme Configuration

```
lp-theme.h/.c
├── Purpose: Configure dark fantasy aesthetic for LrgTheme singleton
├── Dark Fantasy Color Palette:
│   ├── Primary: #2d1b4e (Deep purple)
│   ├── Secondary: #e8e0d5 (Bone white)
│   ├── Accent: #c9a227 (Gold)
│   ├── Background: #0a0a0f (Near black)
│   ├── Surface: #1a1025 (Dark purple)
│   ├── Text: #d4d0c8 (Off-white)
│   ├── Text Secondary: #8a8580 (Muted gray)
│   ├── Border: #3d2b5e (Dark purple border)
│   ├── Error: #9e2a2a (Blood red)
│   └── Success: #2a9e4a (Emerald)
│
├── Game-Specific Colors:
│   ├── Gold (#c9a227) - Wealth display
│   ├── Danger (#9e2a2a) - Blood red warnings
│   ├── Hidden (#1a3a5c) - Dark blue stealth
│   ├── Scrutiny (#c9b327) - Yellow exposure
│   ├── Suspicion (#c97327) - Orange exposure
│   ├── Hunt (#c94a27) - Red-orange exposure
│   ├── Crusade (#c92727) - Bright red exposure
│   ├── Synergy (#27c9c9) - Cyan synergy
│   └── Inactive (#3a3a3a) - Dark gray disabled
│
└── API:
    ├── lp_theme_configure_default() - Configure LrgTheme
    ├── lp_theme_get_gold_color()
    ├── lp_theme_get_danger_color()
    ├── lp_theme_get_hidden_color()
    ├── lp_theme_get_scrutiny_color()
    ├── lp_theme_get_suspicion_color()
    ├── lp_theme_get_hunt_color()
    ├── lp_theme_get_crusade_color()
    ├── lp_theme_get_synergy_color()
    └── lp_theme_get_inactive_color()
```

### Custom Widgets

#### Exposure Meter Widget

```
LpWidgetExposureMeter (final, extends LrgWidget)
├── Purpose: Display player's current exposure level with color-coded fill
├── Properties:
│   ├── value (guint 0-100) - Current exposure percentage
│   ├── level (LpExposureLevel) - Derived from value thresholds
│   ├── show-label (gboolean) - Display level label
│   ├── show-percentage (gboolean) - Display percentage text
│   └── orientation (LrgOrientation) - Horizontal or vertical
│
├── Level Thresholds:
│   ├── 0-24: LP_EXPOSURE_LEVEL_HIDDEN (blue)
│   ├── 25-49: LP_EXPOSURE_LEVEL_SCRUTINY (yellow)
│   ├── 50-74: LP_EXPOSURE_LEVEL_SUSPICION (orange)
│   ├── 75-99: LP_EXPOSURE_LEVEL_HUNT (red-orange)
│   └── 100: LP_EXPOSURE_LEVEL_CRUSADE (bright red)
│
├── Visual Features:
│   ├── Background bar with surface color
│   ├── Fill bar with level-appropriate color
│   ├── Threshold markers at 25%, 50%, 75%
│   ├── Border color matches current level
│   └── Optional label and percentage display
│
└── Signals:
    └── level-changed (old_level, new_level) - Threshold crossed
```

#### Synergy Indicator Widget

```
LpWidgetSynergyIndicator (final, extends LrgWidget)
├── Purpose: Display active synergies and total bonus
├── Properties:
│   ├── synergy-count (guint) - Number of active synergies (read-only)
│   ├── total-bonus (gdouble) - Bonus multiplier (read-only)
│   ├── show-details (gboolean) - Show synergy list
│   └── compact (gboolean) - Compact display mode
│
├── Display Modes:
│   ├── Normal: "◇ Synergies: N (xM.M)"
│   └── Compact: "◇ xM.M"
│
├── Visual Features:
│   ├── Diamond icon (filled when active, outline when none)
│   ├── Synergy color (cyan) when active
│   ├── Inactive color when no synergies
│   └── Optional detailed list of individual synergies
│
├── Manager Integration:
│   ├── Connects to LpSynergyManager singleton
│   ├── Listens to synergies-changed signal
│   └── Auto-updates on manager changes
│
└── API:
    ├── lp_widget_synergy_indicator_new()
    ├── lp_widget_synergy_indicator_get_synergy_count()
    ├── lp_widget_synergy_indicator_get_total_bonus()
    └── lp_widget_synergy_indicator_refresh()
```

### Screen Classes

All screens extend `LrgContainer` and implement keyboard navigation.

#### Portfolio Screen

```
LpScreenPortfolio (final, extends LrgContainer)
├── Purpose: Main portfolio management and investment display
├── Properties:
│   ├── portfolio (LpPortfolio*) - Portfolio to display
│   ├── view-mode (LpPortfolioViewMode) - Current view mode
│   └── selected-investment (LpInvestment*) - Selected item
│
├── View Modes:
│   ├── LP_PORTFOLIO_VIEW_LIST - Investment list view
│   ├── LP_PORTFOLIO_VIEW_ALLOCATION - Asset allocation chart
│   └── LP_PORTFOLIO_VIEW_PERFORMANCE - Performance history
│
├── Child Widgets:
│   ├── LpWidgetExposureMeter - Exposure display
│   └── LpWidgetSynergyIndicator - Synergy display (compact)
│
├── Keyboard Navigation:
│   ├── Up/Down - Select investment
│   ├── Tab - Cycle view mode
│   ├── B - Buy investment dialog
│   └── S - Sell selected investment
│
└── Signals:
    ├── investment-selected (LpInvestment*)
    ├── buy-requested
    └── sell-requested (LpInvestment*)
```

#### World Map Screen

```
LpScreenWorldMap (final, extends LrgContainer)
├── Purpose: Display kingdoms, regions, and world state
├── Properties:
│   ├── simulation (LpWorldSimulation*) - World to display
│   └── selected-kingdom (LpKingdom*) - Selected kingdom
│
├── Features:
│   ├── Kingdom map visualization (placeholder)
│   ├── Current year display
│   └── Region selection support
│
└── Signals:
    ├── kingdom-selected (LpKingdom*)
    └── region-clicked (LpRegion*)
```

#### Agents Screen

```
LpScreenAgents (final, extends LrgContainer)
├── Purpose: Agent list and management
├── Properties:
│   ├── manager (LpAgentManager*) - Agent manager
│   └── selected-agent (LpAgent*) - Selected agent
│
├── Features:
│   ├── Agent count display
│   └── Agent list (placeholder)
│
└── Signals:
    ├── agent-selected (LpAgent*)
    └── recruit-requested
```

#### Intelligence Screen

```
LpScreenIntelligence (final, extends LrgContainer)
├── Purpose: Reports, predictions, and competitor info
└── Features: Placeholder for intelligence reports
```

#### Slumber Screen

```
LpScreenSlumber (final, extends LrgContainer)
├── Purpose: Slumber duration and configuration
├── Properties:
│   └── duration (guint) - Selected slumber duration (min 25 years)
│
├── Duration Presets: 25, 50, 100, 250, 500 years
│
├── Keyboard Navigation:
│   ├── 1-5 - Select preset
│   ├── Left/Right - Cycle presets
│   └── Enter - Confirm slumber
│
└── Signals:
    └── slumber-confirmed (guint duration)
```

#### Ledger Screen

```
LpScreenLedger (final, extends LrgContainer)
├── Purpose: View discovered secrets
├── Properties:
│   └── ledger (LpLedger*) - Ledger to display
│
├── Features:
│   ├── Discovery count display
│   └── Secret list (placeholder)
│
└── API:
    └── lp_screen_ledger_refresh()
```

#### Megaprojects Screen

```
LpScreenMegaprojects (final, extends LrgContainer)
├── Purpose: View and manage multi-century projects
└── Features: Placeholder for megaproject management
```

### Event Dialog

```
LpDialogEvent (final, extends LrgContainer)
├── Purpose: Display events with narrative and choices
├── Properties:
│   ├── event (LpEvent*) - Event to display
│   └── selected-choice (gint) - Selected choice index
│
├── Visual Features:
│   ├── Dimmed background overlay
│   ├── Centered dialog box
│   ├── Event title and description
│   ├── Numbered choice buttons
│   └── Selection highlight
│
├── Keyboard Navigation:
│   ├── 1-4 - Select choice directly
│   ├── Up/Down - Navigate choices
│   ├── Enter - Confirm selection
│   └── Escape - Dismiss dialog
│
└── Signals:
    ├── choice-confirmed (gint index)
    └── dismissed
```

### Updated Type Hierarchy

```
GObject
├── ... (Phase 1-5 types)
│
├── LrgWidget (from libregnum)
│   ├── LpWidgetExposureMeter (final) [Phase 6]
│   └── LpWidgetSynergyIndicator (final) [Phase 6]
│
└── LrgContainer (from libregnum, extends LrgWidget)
    ├── LpScreenPortfolio (final) [Phase 6]
    ├── LpScreenWorldMap (final) [Phase 6]
    ├── LpScreenAgents (final) [Phase 6]
    ├── LpScreenIntelligence (final) [Phase 6]
    ├── LpScreenSlumber (final) [Phase 6]
    ├── LpScreenLedger (final) [Phase 6]
    ├── LpScreenMegaprojects (final) [Phase 6]
    └── LpDialogEvent (final) [Phase 6]
```

### Implemented Components

| Component | File(s) | Status |
|-----------|---------|--------|
| Theme | ui/lp-theme.h/.c | Dark fantasy colors |
| Exposure Meter | ui/lp-widget-exposure-meter.h/.c | Level display widget |
| Synergy Indicator | ui/lp-widget-synergy-indicator.h/.c | Bonus display widget |
| Portfolio Screen | ui/lp-screen-portfolio.h/.c | Investment management |
| World Map Screen | ui/lp-screen-world-map.h/.c | Kingdom display |
| Agents Screen | ui/lp-screen-agents.h/.c | Agent management |
| Intelligence Screen | ui/lp-screen-intelligence.h/.c | Reports (skeleton) |
| Slumber Screen | ui/lp-screen-slumber.h/.c | Duration selection |
| Ledger Screen | ui/lp-screen-ledger.h/.c | Discovery viewing |
| Megaprojects Screen | ui/lp-screen-megaprojects.h/.c | Projects (skeleton) |
| Event Dialog | ui/lp-dialog-event.h/.c | Event choices |

### Tests

| Test File | Coverage |
|-----------|----------|
| test-ui.c | Theme, widgets, screens, dialogs |

### Key Design Decisions

1. **LrgWidget Extension**: Custom widgets extend `LrgWidget` to integrate with libregnum's rendering and event systems.

2. **LrgContainer for Screens**: Screens extend `LrgContainer` to support child widget management and layout.

3. **Keyboard-First Navigation**: All screens support keyboard navigation for accessibility and game controller compatibility.

4. **Theme Colors**: Exposure levels map to specific colors that convey urgency (blue→yellow→orange→red).

5. **Manager Integration**: Widgets connect to singleton managers (Exposure, Synergy) for automatic updates.

6. **Modular Views**: Portfolio screen supports multiple view modes for different perspectives on investments.

7. **Dialog Overlay**: Event dialog renders over other content with a dimmed background for modal presentation.

### Deferred to Later Phases

- Detailed allocation chart rendering (Phase 6+)
- Performance graph visualization (Phase 6+)
- Map visualization with regions (Phase 6+)
- Agent list with details (Phase 6+)
- Megaproject phase visualization (Phase 6+)

---

## Phase 6.5 Implementation Status

Phase 6.5 implements visual feedback systems to make "numbers going up" visible and satisfying - essential for idle game dopamine.

### Floating Text Widget

```
LpFloatingText (final, extends LrgWidget)
├── Purpose: Display gold change popups that drift upward and fade out
├── Properties:
│   ├── text (gchar*) - Text to display
│   ├── lifetime (gfloat) - Duration in seconds (default: 2.0)
│   ├── velocity-y (gfloat) - Upward speed (default: -50.0)
│   └── alpha (gfloat read-only) - Current opacity 0.0-1.0
│
├── Animation:
│   ├── Text floats upward (y += velocity_y * delta)
│   ├── Alpha stays 1.0 for first 50% of lifetime
│   └── Alpha fades to 0.0 over remaining 50%
│
├── API:
│   ├── lp_floating_text_new() - Create with text, position, color
│   ├── lp_floating_text_spawn_gold() - Convenience for gold changes
│   ├── lp_floating_text_is_finished() - Check if animation complete
│   └── lp_floating_text_update() - Advance animation
│
└── Colors:
    ├── Positive amounts: Gold (#c9a227)
    └── Negative amounts: Danger red (#9e2a2a)
```

### Growth Particles Widget

```
LpGrowthParticles (final, extends LrgWidget)
├── Purpose: Visual celebration of portfolio growth using particles
├── Properties:
│   └── intensity (LpGrowthIntensity read-only) - Current intensity level
│
├── Intensity Levels:
│   ├── LP_GROWTH_INTENSITY_MINOR - 10 particles, small (2-4 size)
│   ├── LP_GROWTH_INTENSITY_MODERATE - 30 particles, medium (3-6 size)
│   ├── LP_GROWTH_INTENSITY_MAJOR - 75 particles, large (4-8 size)
│   └── LP_GROWTH_INTENSITY_LEGENDARY - 200 particles, full screen (5-10 size)
│
├── Particle Configuration:
│   ├── Color: Gold (#c9a227) with fade to transparent
│   ├── Direction: Upward burst
│   ├── Uses libregnum LrgParticleSystem and LrgParticleEmitter
│   └── One-shot bursts (no looping)
│
└── API:
    ├── lp_growth_particles_new() - Create particle system
    ├── lp_growth_particles_spawn() - Emit burst at position
    ├── lp_growth_particles_is_alive() - Check for active particles
    ├── lp_growth_particles_update() - Advance simulation
    └── lp_growth_particles_clear() - Kill all particles
```

### Synergy Effect Widget

```
LpSynergyEffect (final, extends LrgWidget)
├── Purpose: Visual feedback when synergies activate - lines connect investments
├── Properties:
│   ├── progress (gfloat read-only) - Animation progress 0.0-1.0
│   └── is-complete (gboolean read-only) - Animation finished
│
├── Modes:
│   ├── ACTIVATION - Cyan line draws from source to target with pulse
│   └── COMPLETION - Expanding ring flash at center
│
├── Visual Features:
│   ├── Line grows from source toward target (ease_out_quad)
│   ├── Bright white pulse dot travels along line
│   ├── Line fades during last 30% of animation
│   └── Color: Synergy cyan (#27c9c9)
│
├── Durations:
│   ├── Activation: 1.0 second
│   └── Completion: 0.5 second
│
└── API:
    ├── lp_synergy_effect_new() - Create effect widget
    ├── lp_synergy_effect_play_activation() - Line from source to target
    ├── lp_synergy_effect_play_completion() - Flash at center
    ├── lp_synergy_effect_set_endpoints() - Configure line positions
    └── lp_synergy_effect_update() - Advance animation
```

### Achievement Popup Widget

```
LpAchievementPopup (final, extends LrgContainer)
├── Purpose: Celebration popup when achievements unlock
├── Properties:
│   ├── name (gchar* read-only) - Achievement name
│   ├── description (gchar* read-only) - Achievement description
│   ├── is-visible (gboolean read-only) - Currently showing
│   └── auto-dismiss-time (gfloat) - Seconds before auto-hide (default: 5.0)
│
├── Animation States:
│   ├── HIDDEN - Not visible
│   ├── SLIDING_IN - Moving from right (0.3s, ease_out_cubic)
│   ├── VISIBLE - Showing with pulsing gold border
│   └── SLIDING_OUT - Moving to right (0.3s, ease_in_cubic)
│
├── Visual Layout:
│   ├── "★ Achievement Unlocked!" header in gold
│   ├── Separator line
│   ├── Achievement name in primary text color
│   ├── Achievement description in secondary color
│   └── Pulsing gold border (alpha oscillates with sine wave)
│
├── Interaction:
│   ├── Click anywhere to dismiss early
│   └── Auto-dismiss after timeout
│
└── API:
    ├── lp_achievement_popup_new() - Create popup
    ├── lp_achievement_popup_show() - Display with name/description
    ├── lp_achievement_popup_dismiss() - Start slide-out
    └── lp_achievement_popup_update() - Advance animation
```

### Slumber Visualization Widget

```
LpSlumberVisualization (final, extends LrgContainer)
├── Purpose: Make slumber phase visually interesting with timeline
├── Properties:
│   ├── current-year (guint64 read-only) - Year being displayed
│   ├── target-year (guint64 read-only) - Final wake year
│   ├── simulation-speed (gfloat read-only) - Years per second
│   └── is-accelerating (gboolean read-only) - Fast-forward active
│
├── Visual Elements:
│   ├── Year counter (48pt font, pulsing gold)
│   ├── Progress bar (gold fill on surface background)
│   ├── Event timeline (last 8 events with age-based fading)
│   └── Acceleration prompt "Hold SPACE to accelerate"
│
├── Event Display:
│   ├── Key events highlighted in gold
│   ├── Dormant orders shown in cyan
│   ├── Normal events in secondary color
│   └── Older events fade toward 30% opacity
│
├── Acceleration:
│   ├── Normal speed: 1.0x
│   ├── Accelerated speed: 5.0x
│   └── Hold SPACE or Enter to enable
│
└── API:
    ├── lp_slumber_visualization_new() - Create visualization
    ├── lp_slumber_visualization_start() - Begin with year range
    ├── lp_slumber_visualization_stop() - End visualization
    ├── lp_slumber_visualization_set_year() - Update year counter
    ├── lp_slumber_visualization_add_event() - Add to timeline
    ├── lp_slumber_visualization_accelerate() - Toggle fast-forward
    └── lp_slumber_visualization_update() - Advance animation
```

### New Enumerations

```c
/* Growth particle intensity levels */
typedef enum {
    LP_GROWTH_INTENSITY_MINOR,      /* <10% growth - few sparkles */
    LP_GROWTH_INTENSITY_MODERATE,   /* 10-50% growth - coin shower */
    LP_GROWTH_INTENSITY_MAJOR,      /* 50-200% growth - gold burst */
    LP_GROWTH_INTENSITY_LEGENDARY   /* >200% growth - golden rain */
} LpGrowthIntensity;
```

### Updated Type Hierarchy

```
GObject
├── ... (Phase 1-6 types)
│
├── LrgWidget (from libregnum)
│   ├── LpFloatingText (final) [Phase 6.5]
│   ├── LpGrowthParticles (final) [Phase 6.5]
│   └── LpSynergyEffect (final) [Phase 6.5]
│
└── LrgContainer (from libregnum)
    ├── LpAchievementPopup (final) [Phase 6.5]
    └── LpSlumberVisualization (final) [Phase 6.5]
```

### Implemented Components

| Component | File(s) | Status |
|-----------|---------|--------|
| Floating Text | feedback/lp-floating-text.h/.c | Gold popup animation |
| Growth Particles | feedback/lp-growth-particles.h/.c | Particle celebration |
| Synergy Effect | feedback/lp-synergy-effect.h/.c | Line/pulse animation |
| Achievement Popup | feedback/lp-achievement-popup.h/.c | Slide-in notification |
| Slumber Visualization | feedback/lp-slumber-visualization.h/.c | Year/timeline display |

### Tests

| Test File | Coverage |
|-----------|----------|
| test-feedback.c | All 5 feedback widgets with 19 test cases |

### Key Design Decisions

1. **libregnum Particle System**: Growth particles use `LrgParticleSystem` and `LrgParticleEmitter` from libregnum for efficient particle management.

2. **Vector-Based Drawing**: Synergy effects use `GrlVector2` for float-precision line and circle drawing via `grl_draw_line_ex` and `grl_draw_circle_v`.

3. **Easing Functions**: Animations use cubic and quadratic easing for smooth, professional-feeling motion.

4. **Auto-Cleanup**: Floating text and particles are designed to be added to containers and auto-removed when finished.

5. **Theme Integration**: All colors use the existing theme system (`lp_theme_get_gold_color`, `lp_theme_get_synergy_color`, etc.).

6. **Acceleration UX**: Slumber visualization supports hold-to-accelerate for player control over time passage.

7. **Event Timeline**: Slumber visualization shows recent events with age-based fading for context during time passage.

---

## Related Documents

- [Game Design Document](../design/GAME.md) - Full game design
- [Development Plan](../design/PLAN.md) - Technical implementation phases
- [Project Guide](../CLAUDE.md) - Code conventions and patterns
