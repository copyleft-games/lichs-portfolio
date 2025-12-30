# Lich's Portfolio - Development Plan

## Project Overview

Build a 2D idle economic strategy game using the libregnum game engine. The player controls Malachar the Undying, a lich building a financial empire across centuries through compound interest and strategic investment.

**Target Platforms:** Linux, Windows (via Steam)
**Engine:** libregnum (GObject-based, deps/libregnum/)
**Build System:** GNU Make with cross-compilation support
**License:** AGPL-3.0-or-later

---

## Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Art Style | Minimalist/Abstract | Clean UI, icons, shapes. Faster development, timeless aesthetic |
| Audio | Procedural/Ambient | Use LrgProceduralAudio. Full music added post-MVP |
| MVP Focus | Core Loop First | Slumber/wake cycle, basic investments, time simulation |
| Steam App ID | Placeholder (480) | Use Spacewar test ID until Steamworks integration |
| Input | Mouse + Keyboard | Full keyboard navigation; gamepad support in later phase |
| Steam Dependency | Optional | Game fully playable without Steam; Steam adds cloud saves, achievements sync |
| Documentation | Comprehensive | `docs/` folder with markdown; well-commented source code |
| Testing | GTest | Comprehensive unit tests for all game systems |
| Achievements | Dual System | Local LrgAchievementManager + optional Steam sync |
| Minimum Slumber | 25 years | Reinforces "long view" identity; differentiates from quick-feedback idle games |
| Feedback Systems | Visual animations | Floating numbers, growth particles, synergy effects for dopamine hits |
| Progression Layers | Multi-system | Synergies + Exposure + Ledger + Echo Trees create strategic depth |
| Early Game Hook | First Awakening | 60-second tutorial montage showing compound growth to hook players immediately |

---

## Core Principles

### Steam Independence

The game MUST be fully playable without Steam:

- Local save system works without Steam Cloud
- Local achievement system (LrgAchievementManager) tracks all achievements
- When Steam is available (STEAM=1 build), achievements sync to Steam
- Steam provides optional enhancements: cloud saves, leaderboards, workshop
- Build without STEAM=1 for development/testing

### Input System

All menus and gameplay support both mouse and keyboard:

- Mouse: Click buttons, drag sliders, hover tooltips
- Keyboard: Tab/Arrow navigation, Enter to select, Escape to back
- Focus indicators for keyboard navigation
- Gamepad support deferred to later phase

### Documentation Requirements

Every system must have:

- `docs/` markdown file explaining architecture and usage
- Source code comments explaining logic (not excessive, but followable)
- GIR-compatible doc comments for all public API
- Examples in documentation where helpful

### Testing Requirements

GTest unit tests covering:

- Investment calculations and returns
- Agent aging, succession, death
- Kingdom simulation stability
- Save/load round-trips
- Prestige mechanics and carryover
- UI widget behavior (where testable)

---

## Directory Structure

```
lichs-portfolio/
├── Makefile                    # Root build orchestration
├── config.mk                   # Build configuration
├── rules.mk                    # Build helpers
├── README.md                   # Project documentation
├── CLAUDE.md                   # AI assistant context
├── COPYING                     # AGPL-3.0-or-later license
├── src/
│   ├── main.c                  # Entry point
│   ├── lp-types.h              # Forward declarations
│   ├── lp-enums.h/.c           # Game-specific enums
│   ├── lp-log.h                # Logging macros
│   ├── core/                   # Core game classes
│   │   ├── lp-application.h/.c
│   │   ├── lp-game-data.h/.c
│   │   ├── lp-phylactery.h/.c
│   │   ├── lp-prestige.h/.c
│   │   ├── lp-exposure-manager.h/.c  # Discovery/tension tracking
│   │   ├── lp-synergy-manager.h/.c   # Investment synergy detection
│   │   ├── lp-ledger.h/.c            # Hidden information discovery
│   │   └── lp-megaproject.h/.c       # Multi-century projects
│   ├── simulation/             # World simulation
│   │   ├── lp-world-simulation.h/.c
│   │   ├── lp-kingdom.h/.c
│   │   ├── lp-region.h/.c
│   │   ├── lp-event.h/.c
│   │   ├── lp-event-generator.h/.c
│   │   └── lp-competitor.h/.c
│   ├── investment/             # Investment system
│   │   ├── lp-investment.h/.c
│   │   ├── lp-investment-property.h/.c
│   │   ├── lp-investment-trade.h/.c
│   │   ├── lp-investment-financial.h/.c
│   │   ├── lp-investment-magical.h/.c
│   │   ├── lp-investment-political.h/.c
│   │   ├── lp-investment-dark.h/.c
│   │   └── lp-portfolio.h/.c
│   ├── agent/                  # Agent management
│   │   ├── lp-agent.h/.c
│   │   ├── lp-agent-individual.h/.c
│   │   ├── lp-agent-family.h/.c
│   │   ├── lp-agent-cult.h/.c
│   │   ├── lp-agent-bound.h/.c
│   │   ├── lp-agent-manager.h/.c
│   │   └── lp-trait.h/.c             # Bloodline traits system
│   ├── ui/                     # Custom UI screens
│   │   ├── lp-screen-portfolio.h/.c
│   │   ├── lp-screen-world-map.h/.c
│   │   ├── lp-screen-agents.h/.c
│   │   ├── lp-screen-intelligence.h/.c
│   │   ├── lp-screen-slumber.h/.c
│   │   ├── lp-screen-ledger.h/.c         # Discovery/knowledge screen
│   │   ├── lp-screen-megaprojects.h/.c   # Long-term projects screen
│   │   ├── lp-dialog-event.h/.c
│   │   ├── lp-widget-exposure-meter.h/.c # Exposure tracking widget
│   │   ├── lp-widget-synergy-indicator.h/.c # Synergy bonus display
│   │   └── lp-theme.c
│   ├── feedback/               # Visual feedback systems
│   │   ├── lp-floating-text.h/.c         # Gold change popups
│   │   ├── lp-growth-particles.h/.c      # Portfolio growth effects
│   │   └── lp-achievement-popup.h/.c     # Achievement notifications
│   ├── states/                 # Game states
│   │   ├── lp-state-main-menu.h/.c
│   │   ├── lp-state-wake.h/.c
│   │   ├── lp-state-analyze.h/.c
│   │   ├── lp-state-decide.h/.c
│   │   ├── lp-state-slumber.h/.c
│   │   ├── lp-state-simulating.h/.c
│   │   ├── lp-state-pause.h/.c
│   │   └── lp-state-settings.h/.c
│   ├── achievement/            # Achievement system
│   │   └── lp-achievement-manager.h/.c
│   └── steam/                  # Steam integration (optional)
│       └── lp-steam-bridge.h/.c
├── data/
│   ├── config.yaml             # Game configuration
│   ├── kingdoms/               # Kingdom definitions
│   ├── investments/            # Investment type definitions
│   ├── events/                 # Event templates
│   ├── agents/                 # Agent templates
│   ├── upgrades/               # Phylactery upgrades
│   ├── achievements/           # Achievement definitions
│   ├── synergies/              # Investment synergy definitions
│   ├── traits/                 # Bloodline trait definitions
│   ├── megaprojects/           # Multi-century project definitions
│   ├── ledger/                 # Discoverable information entries
│   └── eras/                   # Era transition definitions
├── assets/
│   ├── textures/               # Sprites, UI elements
│   ├── fonts/                  # Game fonts
│   ├── audio/                  # Music, SFX
│   └── shaders/                # Custom shaders (optional)
├── tests/
│   ├── Makefile
│   ├── test-big-number.c
│   ├── test-investment.c
│   ├── test-agent.c
│   ├── test-kingdom.c
│   ├── test-simulation.c
│   ├── test-save-load.c
│   ├── test-prestige.c
│   ├── test-portfolio.c
│   ├── test-achievement.c
│   ├── test-synergy.c            # Synergy detection tests
│   ├── test-exposure.c           # Exposure meter tests
│   ├── test-trait.c              # Bloodline inheritance tests
│   └── test-megaproject.c        # Long-term project tests
├── docs/
│   ├── architecture.md         # High-level system overview
│   ├── investment-system.md    # Investment mechanics documentation
│   ├── agent-system.md         # Agent management documentation
│   ├── simulation.md           # World simulation documentation
│   ├── save-system.md          # Save/load documentation
│   ├── ui-navigation.md        # Input and UI navigation guide
│   └── achievements.md         # Achievement system documentation
├── design/
│   ├── GAME.md                 # Game design document
│   └── PLAN.md                 # This file
└── deps/
    └── libregnum/              # Game engine (submodule)
```

---

## Phase 0: Project Foundation

**Goal:** Set up project infrastructure, documentation, and build system.

### 0.1 Documentation Files

#### README.md
- Project description and elevator pitch
- Build requirements (Fedora packages)
- Build instructions (make, make DEBUG=1, make WINDOWS=1)
- Cross-compilation setup (mingw64 toolchain)
- Steam SDK integration notes
- License information

#### CLAUDE.md
- Project architecture overview
- Directory structure guide
- GObject type hierarchy for game-specific classes (Lp* prefix)
- Key files to reference for patterns
- Build commands and debugging tips
- Naming conventions (following libregnum patterns)

### 0.2 Build System

#### config.mk
```makefile
VERSION_MAJOR := 0
VERSION_MINOR := 1
VERSION_MICRO := 0
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    TARGET_PLATFORM := linux
endif

# Cross-compilation
ifeq ($(WINDOWS),1)
    TARGET_PLATFORM := windows
    CC := x86_64-w64-mingw32-gcc
    PKG_CONFIG := x86_64-w64-mingw32-pkg-config
    EXE_EXT := .exe
endif

# Build directories
DEBUG ?= 0
BUILDDIR := build/$(if $(filter 1,$(DEBUG)),debug,release)

# Compiler flags
COMMON_CFLAGS := -std=gnu89
WARN_CFLAGS := -Wall -Wextra -Werror -Wstrict-prototypes
OPT_CFLAGS := $(if $(filter 1,$(DEBUG)),-g3 -O0,-O2)

# pkg-config
GLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags glib-2.0 gobject-2.0)
GLIB_LIBS := $(shell $(PKG_CONFIG) --libs glib-2.0 gobject-2.0)
```

#### rules.mk
- Helper functions: `print_status`, `print_compile`, `print_link`
- Common build rules
- Directory creation targets

#### Makefile (root)
```makefile
include config.mk
include rules.mk

.PHONY: all deps game test clean install

all: deps game

deps:
	$(MAKE) -C deps/libregnum

game: deps
	$(MAKE) -C src

test: game
	$(MAKE) -C tests run

clean:
	$(MAKE) -C deps/libregnum clean
	$(MAKE) -C src clean
	$(MAKE) -C tests clean

install:
	# Install to PREFIX
```

### 0.3 Initial Source Files

#### src/main.c
```c
/* main.c - Entry point for Lich's Portfolio */
#include "core/lp-application.h"

int
main (int    argc,
      char **argv)
{
    LpApplication *app;
    int status;

    app = lp_application_get_default ();
    status = lp_application_run (app, argc, argv);
    g_object_unref (app);

    return status;
}
```

#### src/lp-types.h
- Forward declarations for all Lp* types
- Avoid circular includes

#### src/lp-enums.h/.c
- LpAssetClass (property, trade, financial, magical, political, dark)
- LpAgentType (individual, family, cult, bound)
- LpRiskLevel (low, medium, high, extreme)
- LpEventType (economic, political, magical, personal)

### 0.4 Initial Documentation

#### docs/architecture.md
- High-level system overview
- Core loop diagram
- Module dependencies
- Key libregnum integrations

### 0.5 Test Infrastructure

#### tests/Makefile
- Build and run all tests
- Link against libregnum and game library

#### tests/test-stub.c
- Initial test file to verify infrastructure works

### 0.6 GAME.md Enhancements

Add to existing GAME.md:
- Technical architecture section
- Data format specifications (YAML schemas)
- Achievement definitions with unlock conditions
- Sound requirements (procedural ambient)
- Localization key structure

---

## Phase 1: Core Architecture

**Goal:** Implement the main game loop and state management.

### 1.1 LpApplication

**File:** `src/core/lp-application.h/.c`

```c
#define LP_TYPE_APPLICATION (lp_application_get_type ())
G_DECLARE_FINAL_TYPE (LpApplication, lp_application, LP, APPLICATION, GObject)

/* Singleton access */
LpApplication *lp_application_get_default (void);

/* Lifecycle */
gboolean lp_application_startup  (LpApplication  *self, GError **error);
void     lp_application_shutdown (LpApplication  *self);
gint     lp_application_run      (LpApplication  *self, int argc, char **argv);
```

**Owns:**
- LrgEngine (from libregnum)
- LrgGameStateManager
- LpGameData
- LpAchievementManager

### 1.2 LpGameData

**File:** `src/core/lp-game-data.h/.c`

```c
#define LP_TYPE_GAME_DATA (lp_game_data_get_type ())
G_DECLARE_FINAL_TYPE (LpGameData, lp_game_data, LP, GAME_DATA, GObject)

/* Implements LrgSaveable interface */
```

**Properties:**
- `current_year` (guint64)
- `total_years_played` (guint64)
- `portfolio` (LpPortfolio)
- `agent_manager` (LpAgentManager)
- `phylactery` (LpPhylactery)

### 1.3 Game States

All extend `LrgGameState` from libregnum.

| State | Purpose | Transparent | Blocking |
|-------|---------|-------------|----------|
| LpStateMainMenu | Title, new game, load, settings | No | Yes |
| LpStateWake | Wake report screen | No | Yes |
| LpStateAnalyze | World map, portfolio, intelligence | No | Yes |
| LpStateDecide | Investment/agent actions | No | Yes |
| LpStateSlumber | Slumber configuration | No | Yes |
| LpStateSimulating | Visual time passage | No | Yes |
| LpStatePause | Pause menu | Yes | Yes |
| LpStateSettings | Settings screen | Yes | Yes |

**Virtual method overrides:**
- `enter()` - Initialize state, load resources
- `exit()` - Cleanup
- `update(delta)` - Frame logic
- `draw()` - Render
- `handle_input(event)` - Process input, return TRUE if consumed

### 1.4 Basic Simulation Loop

**File:** `src/simulation/lp-world-simulation.h/.c`

```c
/* Simulation tick levels */
void lp_world_simulation_tick_year       (LpWorldSimulation *self);
void lp_world_simulation_tick_decade     (LpWorldSimulation *self);
void lp_world_simulation_tick_generation (LpWorldSimulation *self);
void lp_world_simulation_tick_era        (LpWorldSimulation *self);

/* High-level simulation */
void lp_world_simulation_simulate_years (LpWorldSimulation *self, guint years);
```

### 1.5 LpExposureManager

**File:** `src/core/lp-exposure-manager.h/.c`

**Purpose:** Track player visibility to mortal institutions. Creates strategic tension without harsh punishment.

```c
#define LP_TYPE_EXPOSURE_MANAGER (lp_exposure_manager_get_type ())
G_DECLARE_FINAL_TYPE (LpExposureManager, lp_exposure_manager, LP, EXPOSURE_MANAGER, GObject)

/* Singleton access */
LpExposureManager *lp_exposure_manager_get_default (void);

/* Exposure calculation */
gint     lp_exposure_manager_get_total          (LpExposureManager *self);
void     lp_exposure_manager_recalculate        (LpExposureManager *self);

/* Threshold checks */
gboolean lp_exposure_manager_is_hidden          (LpExposureManager *self);
gboolean lp_exposure_manager_triggers_scrutiny  (LpExposureManager *self);
gboolean lp_exposure_manager_triggers_crusade   (LpExposureManager *self);

/* Decay over time */
void     lp_exposure_manager_apply_decay        (LpExposureManager *self, guint decades);
```

**Exposure Thresholds:**
| Level | Range | Effect |
|-------|-------|--------|
| Hidden | 0-24% | No effects |
| Scrutiny | 25-49% | Inquisitor investigation events |
| Suspicion | 50-74% | Holy orders take notice |
| Hunt | 75-99% | Active search (-10% income) |
| Crusade | 100% | Direct attack on holdings |

**Signals:**
- `::threshold-crossed` - Emitted when entering new exposure level

### 1.6 LpSynergyManager

**File:** `src/core/lp-synergy-manager.h/.c`

**Purpose:** Detect and apply investment synergy bonuses. Loaded from YAML data files.

```c
#define LP_TYPE_SYNERGY_MANAGER (lp_synergy_manager_get_type ())
G_DECLARE_FINAL_TYPE (LpSynergyManager, lp_synergy_manager, LP, SYNERGY_MANAGER, GObject)

/* Singleton access */
LpSynergyManager *lp_synergy_manager_get_default (void);

/* Load synergies from data files */
gboolean  lp_synergy_manager_load_synergies (LpSynergyManager *self,
                                             const gchar      *data_dir,
                                             GError          **error);

/* Detection */
GPtrArray *lp_synergy_manager_detect_active  (LpSynergyManager *self,
                                              LpPortfolio      *portfolio);

/* Apply bonuses */
void       lp_synergy_manager_apply_bonuses  (LpSynergyManager *self,
                                              LpPortfolio      *portfolio);
```

**Synergy Data Structure:**
```yaml
# data/synergies/trade-monopoly.yaml
id: trade-monopoly
name: "Trade Monopoly"
description: "Control the entire supply chain"
requirements:
  - asset_class: trade
    subtype: route
  - asset_class: trade
    subtype: commodity
    constraint: same_commodity_as_route
  - asset_class: trade
    subtype: guild
    constraint: region_contains_route
bonus:
  type: multiplier
  target: all_requirements
  value: 1.5
```

### 1.7 LpLedger (Skeleton)

**File:** `src/core/lp-ledger.h/.c`

**Purpose:** Track discovered hidden information. Full implementation in Phase 5.

```c
#define LP_TYPE_LEDGER (lp_ledger_get_type ())
G_DECLARE_FINAL_TYPE (LpLedger, lp_ledger, LP, LEDGER, GObject)

/* Implements LrgSaveable interface */

/* Entry categories */
typedef enum
{
    LP_LEDGER_CATEGORY_ECONOMIC,    /* Market patterns, cycles */
    LP_LEDGER_CATEGORY_AGENT,       /* Bloodline secrets */
    LP_LEDGER_CATEGORY_COMPETITOR,  /* Immortal weaknesses */
    LP_LEDGER_CATEGORY_HIDDEN       /* Game mechanics */
} LpLedgerCategory;

/* Discovery */
gboolean lp_ledger_is_discovered    (LpLedger    *self, const gchar *entry_id);
void     lp_ledger_discover         (LpLedger    *self, const gchar *entry_id);
guint    lp_ledger_count_discovered (LpLedger    *self, LpLedgerCategory category);
```

### 1.8 First Awakening Tutorial

**File:** `src/states/lp-state-first-awakening.h/.c`

**Purpose:** Hook players in the first 60 seconds with visible compound growth.

```c
#define LP_TYPE_STATE_FIRST_AWAKENING (lp_state_first_awakening_get_type ())
G_DECLARE_FINAL_TYPE (LpStateFirstAwakening, lp_state_first_awakening, LP, STATE_FIRST_AWAKENING, LrgGameState)
```

**Tutorial Sequence:**
```
Year 0:    1,000 gp (starting gold)
Year 10:   1,200 gp (+20%) - "Your first investment pays dividends"
Year 25:   2,100 gp (+75%) - "The Crane family serves you well"
Year 50:   5,800 gp (+380%) - "Valdris falls. Your bonds pay out"
Year 100:  47,000 gp (+4,600%) - "You remember this pattern..."
```

**Technical Implementation:**
- Use `LrgTweenManager` for number animations
- Use `LrgTransitionManager` for screen fades
- Scripted events with narrative text
- Skip button for returning players

---

## Phase 2: Investment System

**Goal:** Implement all investment types and portfolio management.

### 2.1 LpInvestment (Base Class)

**File:** `src/investment/lp-investment.h/.c`

```c
#define LP_TYPE_INVESTMENT (lp_investment_get_type ())
G_DECLARE_DERIVABLE_TYPE (LpInvestment, lp_investment, LP, INVESTMENT, GObject)

struct _LpInvestmentClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    LrgBigNumber *(*calculate_returns) (LpInvestment *self, guint years);
    void          (*apply_event)       (LpInvestment *self, LpEvent *event);
    gboolean      (*can_sell)          (LpInvestment *self);

    gpointer _reserved[8];
};
```

**Properties:**
- `id` (gchar*)
- `name` (gchar*)
- `description` (gchar*)
- `purchase_price` (LrgBigNumber)
- `current_value` (LrgBigNumber)
- `purchase_year` (guint64)
- `asset_class` (LpAssetClass)
- `risk_level` (LpRiskLevel)
- `owning_region` (LpRegion)

### 2.2 Investment Subclasses

| Class | Risk | Return | Special Mechanics |
|-------|------|--------|-------------------|
| LpInvestmentProperty | Low | Steady | Survives political upheaval |
| LpInvestmentTrade | Medium | Variable | Affected by route disruption |
| LpInvestmentFinancial | Variable | Fixed/Loss | Kingdom default risk |
| LpInvestmentMagical | High | High | Adventurer interference |
| LpInvestmentPolitical | Special | Multiplier | Affects other investments |
| LpInvestmentDark | Extreme | Extreme | Must stay hidden, unlockable |

### 2.3 LpPortfolio

**File:** `src/investment/lp-portfolio.h/.c`

```c
/* Portfolio operations */
void          lp_portfolio_add_investment    (LpPortfolio *self, LpInvestment *inv);
void          lp_portfolio_remove_investment (LpPortfolio *self, LpInvestment *inv);
GPtrArray    *lp_portfolio_get_by_class      (LpPortfolio *self, LpAssetClass class);
LrgBigNumber *lp_portfolio_calculate_total   (LpPortfolio *self);
LrgBigNumber *lp_portfolio_calculate_income  (LpPortfolio *self, guint years);
```

### 2.4 Market Integration

Use `LrgMarket` from libregnum:
- Register resources: gold, silver, commodities
- Regional price variations
- Supply/demand simulation

### 2.5 Synergy Detection and Bonuses

**Integration with LpSynergyManager (from 1.6)**

Synergies are checked when:
- Portfolio changes (buy/sell)
- At wake time
- During income calculation

**Initial Synergies for MVP:**

| Synergy | Requirements | Bonus |
|---------|--------------|-------|
| Trade Monopoly | Route + Commodity + Guild (same region) | +50% income to all three |
| Crisis Profiteer | Kingdom bonds + Insurance pools | 200% payout when kingdom defaults |
| Eternal Landlord | Agricultural land + Mill rights + Agent family manager | +75% stability bonus |
| Shadow Bank | Financial + Political + Dark investments | +100% to financial income |
| Diversified Portfolio | 4+ asset classes | +10% to all income |

**Synergy UI Indicator:**
- Portfolio screen shows active synergies
- Hover reveals requirements and bonus
- Incomplete synergies show progress toward completion

### 2.6 Compound Interest Preview Panel

**File:** `src/ui/lp-compound-preview.h/.c`

**Purpose:** Visualize exponential growth before slumber commitment.

```c
#define LP_TYPE_COMPOUND_PREVIEW (lp_compound_preview_get_type ())
G_DECLARE_FINAL_TYPE (LpCompoundPreview, lp_compound_preview, LP, COMPOUND_PREVIEW, LrgWidget)

/* Create preview for portfolio */
LpCompoundPreview *lp_compound_preview_new (LpPortfolio *portfolio);

/* Recalculate projections */
void lp_compound_preview_update (LpCompoundPreview *self);
```

**Display Format:**
```
┌─────────────────────────────────────────────────────┐
│ PROJECTED GROWTH (current portfolio)                │
├─────────────────────────────────────────────────────┤
│ Slumber  25 years:    +67%     →   4.2M gp         │
│ Slumber  50 years:   +180%     →   7.0M gp         │
│ Slumber 100 years:   +520%     →  15.5M gp         │
│ Slumber 250 years: +3,400%     →  87.5M gp         │
│                                                     │
│         [Simple exponential curve chart]            │
└─────────────────────────────────────────────────────┘
```

**Technical:**
- Uses `LrgBigNumber` for all calculations
- Simple line rendering for growth curve (graylib primitives)
- Updates when slumber duration slider changes

---

## Phase 3: Agent System

**Goal:** Implement agent types with progressive unlocking and bloodline traits.

**Unlock Progression:**
| Agent Type | Unlock Condition | Phase |
|------------|------------------|-------|
| Individual | Start | MVP |
| Family | 3 generations of individual succession | MVP |
| Cult | Phylactery Level 5 | Post-MVP |
| Bound | Dark Arts tree unlocked | Late-game |

### 3.1 LpAgent (Base Class)

**File:** `src/agent/lp-agent.h/.c`

```c
#define LP_TYPE_AGENT (lp_agent_get_type ())
G_DECLARE_DERIVABLE_TYPE (LpAgent, lp_agent, LP, AGENT, GObject)

struct _LpAgentClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    void     (*on_year_passed) (LpAgent *self);
    void     (*on_death)       (LpAgent *self);
    void     (*on_betrayal)    (LpAgent *self);
    gboolean (*can_recruit)    (LpAgent *self);

    gpointer _reserved[8];
};
```

**Properties:**
- `id`, `name` (gchar*)
- `age`, `max_age` (guint)
- `loyalty` (gint, 0-100)
- `competence` (LpSkillFlags)
- `cover_status` (LpCoverStatus enum)
- `knowledge_level` (LpKnowledgeLevel enum)
- `assigned_investments` (GPtrArray)
- `traits` (GPtrArray of LpTrait) - **NEW**

### 3.2 LpTrait System

**File:** `src/agent/lp-trait.h/.c`

**Purpose:** Bloodline traits that agents can possess and pass to successors.

```c
#define LP_TYPE_TRAIT (lp_trait_get_type ())
G_DECLARE_FINAL_TYPE (LpTrait, lp_trait, LP, TRAIT, GObject)

/* Load traits from data files */
LpTrait *lp_trait_load_from_yaml (const gchar *file_path, GError **error);

/* Inheritance calculation */
gboolean lp_trait_roll_inheritance (LpTrait *self,
                                    LpAgent *parent1,
                                    LpAgent *parent2);

/* Effect application */
void lp_trait_apply_effects (LpTrait *self, LpAgent *agent);
```

**Trait Data Structure:**
```yaml
# data/traits/ruthless.yaml
id: ruthless
name: "Ruthless"
description: "This bloodline will do whatever it takes"
inheritance:
  base_chance: 30
  per_parent_with_trait: 15
effects:
  - type: income_multiplier
    value: 1.1
  - type: unlock_action
    action: eliminate_competitor_agent
  - type: loyalty_modifier
    value: -5
conflicts_with: [compassionate, cautious]
synergizes_with: [ambitious, cunning]
```

**Initial Traits for MVP:**

| Trait | Inheritance | Effects |
|-------|-------------|---------|
| Merchant | 35% base | +10% trade income |
| Ruthless | 30% base | +10% income, can eliminate, -5 loyalty |
| Paranoid | 25% base | -20% discovery chance, -10% efficiency |
| Ambitious | 40% base | +15% skill growth, +10% betrayal chance |
| Scholarly | 30% base | +25% Ledger discovery, slower action |
| Connected | 35% base | +15% political influence |

### 3.3 LpAgentIndividual

**File:** `src/agent/lp-agent-individual.h/.c`

**MVP Implementation.** Single mortal agent with highest skill potential.

**Properties:**
- `successor` (LpAgentIndividual*) - Designated successor
- `training_progress` (gfloat 0-1) - Successor readiness

**Succession Events:**
- Agent reaching max_age triggers succession
- If no trained successor: skills lost, loyalty reset
- If trained successor: 75% skill retention, base loyalty

### 3.4 LpAgentFamily (with Trait Inheritance)

**File:** `src/agent/lp-agent-family.h/.c`

**Unlocks after 3 generations of individual succession.**

```c
#define LP_TYPE_AGENT_FAMILY (lp_agent_family_get_type ())
G_DECLARE_FINAL_TYPE (LpAgentFamily, lp_agent_family, LP, AGENT_FAMILY, LpAgent)

/* Family-specific */
guint    lp_agent_family_get_generation      (LpAgentFamily *self);
GPtrArray *lp_agent_family_get_bloodline_traits (LpAgentFamily *self);
void     lp_agent_family_on_succession       (LpAgentFamily *self);
```

**Properties:**
- `generation` (guint) - Current generation number
- `bloodline_traits` (GPtrArray) - Traits accumulated in bloodline
- `founding_year` (guint64) - When family was established

**Trait Inheritance:**
- On succession, roll inheritance for each parent trait
- Children can develop new traits (rare, ~5% per generation)
- Traits can conflict and cancel each other
- Maximum 4 traits per agent

**Example Family Evolution:**
```
Gen 1: Marcus Crane (Merchant)
Gen 2: Helena Crane (Merchant + Ruthless)
Gen 3: Tobias Crane (Ruthless + Paranoid)  # Merchant lost to conflict
Gen 4: Victor Crane (Ruthless + Paranoid + Ambitious)  # New trait emerged
```

### 3.5 LpAgentCult (Post-MVP)

**File:** `src/agent/lp-agent-cult.h/.c`

**Unlocks at Phylactery Level 5.** Self-perpetuating religious organization.

**Deferred to Post-MVP Phase.**

### 3.6 LpAgentBound (Late-Game)

**File:** `src/agent/lp-agent-bound.h/.c`

**Unlocks with Dark Arts phylactery tree.** Undead servants.

**Deferred to Late-Game Phase.**

### 3.7 LpAgentManager

**File:** `src/agent/lp-agent-manager.h/.c`

```c
/* Agent lifecycle */
void lp_agent_manager_recruit_agent   (LpAgentManager *self, LpAgent *agent);
void lp_agent_manager_dismiss_agent   (LpAgentManager *self, LpAgent *agent);
void lp_agent_manager_eliminate_agent (LpAgentManager *self, LpAgent *agent);

/* Simulation */
void lp_agent_manager_process_year    (LpAgentManager *self);

/* Trait system */
void lp_agent_manager_process_succession (LpAgentManager *self,
                                          LpAgent        *dying_agent);
```

---

## Phase 4: World Simulation

**Goal:** Implement kingdoms, regions, events, and competitors.

### 4.1 LpKingdom

**File:** `src/simulation/lp-kingdom.h/.c`

**Attributes (0-100 each):**
- `stability` - Likelihood of collapse
- `prosperity` - Economic health
- `military` - War capability
- `culture` - Resistance to change
- `tolerance` - Acceptance of magic/undead

**Behaviors:**
- Low stability triggers collapse events
- High military triggers wars
- Low tolerance triggers crusades

### 4.2 LpRegion

**File:** `src/simulation/lp-region.h/.c`

**Properties:**
- Geography type (coastal, inland, mountain, forest)
- Owning kingdom reference
- Trade route connections
- Resource deposits

### 4.3 Event System

**File:** `src/simulation/lp-event.h/.c`

```c
#define LP_TYPE_EVENT (lp_event_get_type ())
G_DECLARE_DERIVABLE_TYPE (LpEvent, lp_event, LP, EVENT, GObject)

struct _LpEventClass
{
    GObjectClass parent_class;

    void       (*apply_effects) (LpEvent *self, LpWorldSimulation *sim);
    GPtrArray *(*get_choices)   (LpEvent *self);

    gpointer _reserved[8];
};
```

**Event types:**
- Economic: Market crashes, trade discoveries
- Political: Wars, successions, revolutions
- Magical: Artifact discoveries, divine interventions
- Personal: Agent deaths, discovery attempts

### 4.4 Event Generator

**File:** `src/simulation/lp-event-generator.h/.c`

- Weighted probability based on world state
- `generate_yearly_events()`
- `generate_decade_events()`
- `generate_era_events()`

### 4.5 Competitors

**File:** `src/simulation/lp-competitor.h/.c`

Types: Dragons, elder vampires, rival liches

AI behaviors:
- Expand territory
- React to player actions
- Propose alliances or conflicts

---

## Phase 5: Progression Systems

**Goal:** Implement phylactery upgrades and prestige.

### 5.1 LpPhylactery

**File:** `src/core/lp-phylactery.h/.c`

Uses `LrgUnlockTree` for upgrade structure.

**Upgrade Categories:**

| Category | Effects |
|----------|---------|
| Temporal Mastery | Longer slumber, time efficiency |
| Network Expansion | More agents, family/cult mechanics |
| Divination | Better predictions, early warnings |
| Resilience | Survive disasters, faster recovery |
| Dark Arts | Unlock dark investments (hidden) |

### 5.2 LpPrestigeManager

**File:** `src/core/lp-prestige.h/.c`

Extends `LrgPrestige` from libregnum.

**Override methods:**
- `calculate_reward()` - Echo calculation formula
- `on_prestige()` - Handle phylactery destruction
- `get_bonus_multiplier()` - Cumulative prestige bonuses

**Persistent data (survives prestige):**
- Echoes (memory fragments)
- Artifacts (unique items)
- Codex entries (unlocked lore)

### 5.3 Echo Specialization Trees

**Purpose:** Clear prestige rewards that change playstyle.

Four specialization trees, purchased with Echoes:

```
THE ECONOMIST                    THE MANIPULATOR
├─ Startup Capital (1)          ├─ Established Network (1)
│   └─ 2x starting gold         │   └─ Start with agent family
├─ Market Sense (3)             ├─ Whisper Network (3)
│   └─ +15% divination          │   └─ Double agent mechanics
├─ Compound Master (10)         ├─ Shadow Council (10)
│   └─ +2% base interest        │   └─ Political influence 2x
└─ Perfect Foresight (25)       └─ Puppetmaster (25)
    └─ See 50 years ahead           └─ Competitors start weaker

THE SCHOLAR                      THE ARCHITECT
├─ Memory Fragments (1)         ├─ Phylactery Preservation (1)
│   └─ Keep Ledger 25%          │   └─ Keep 1 upgrade
├─ Pattern Recognition (3)      ├─ Eternal Projects (3)
│   └─ +25% discovery speed     │   └─ Keep megaproject 25%
├─ Cosmic Insight (10)          ├─ Dimensional Vault (10)
│   └─ Hidden investments       │   └─ Keep 50% of gold
└─ Omniscience (25)             └─ Immortal Holdings (25)
    └─ Full Ledger persists         └─ Keep 1 investment
```

**Implementation:**
Each tree uses `LrgUnlockTree` from libregnum:

```c
/* Create specialization tree */
g_autoptr(LrgUnlockTree) economist_tree = lrg_unlock_tree_new ();

g_autoptr(LrgUnlockNode) startup = lrg_unlock_node_new ("startup-capital", "Startup Capital");
lrg_unlock_node_set_cost_simple (startup, 1.0);  /* 1 Echo */
lrg_unlock_tree_add_node (economist_tree, startup);

g_autoptr(LrgUnlockNode) market = lrg_unlock_node_new ("market-sense", "Market Sense");
lrg_unlock_node_set_cost_simple (market, 3.0);
lrg_unlock_node_add_requirement (market, "startup-capital");
lrg_unlock_tree_add_node (economist_tree, market);
```

### 5.4 Megaproject System

**File:** `src/core/lp-megaproject.h/.c`

**Purpose:** Multi-century investments for late-game goals.

```c
#define LP_TYPE_MEGAPROJECT (lp_megaproject_get_type ())
G_DECLARE_FINAL_TYPE (LpMegaproject, lp_megaproject, LP, MEGAPROJECT, GObject)

/* Implements LrgSaveable interface */

/* Load from YAML */
LpMegaproject *lp_megaproject_load_from_yaml (const gchar *file_path, GError **error);

/* Progress tracking */
guint    lp_megaproject_get_years_remaining (LpMegaproject *self);
gfloat   lp_megaproject_get_progress        (LpMegaproject *self);
gboolean lp_megaproject_advance_years       (LpMegaproject *self, guint years);
gboolean lp_megaproject_is_complete         (LpMegaproject *self);

/* Risk */
gboolean lp_megaproject_roll_discovery      (LpMegaproject *self);
```

**Megaproject Data Structure:**
```yaml
# data/megaprojects/lich-road.yaml
id: lich-road
name: "The Lich Road"
description: "A hidden tunnel network connecting your properties"
duration: 500
cost_per_year: 2000
unlock_requirement: phylactery_level_7
phases:
  - name: "Survey"
    years: 50
    effect: null
  - name: "Initial Construction"
    years: 200
    effect:
      type: property_income_bonus
      value: 0.05
  - name: "Main Tunnels"
    years: 200
    effect:
      type: property_income_bonus
      value: 0.15
      agent_travel: instant
  - name: "Completion"
    years: 50
    effect:
      type: property_immune_seizure
      hidden_transfer: true
risk:
  discovery_per_decade: 2
  on_discovery: project_destroyed
```

**Available Megaprojects:**

| Project | Duration | Unlock | Effect |
|---------|----------|--------|--------|
| The Lich Road | 500 years | Level 7 | Hidden tunnels; immune to seizure |
| Phylactery Network | 1000 years | Level 10 | 5 backups; prestige insurance |
| The Deep Vault | 300 years | Level 5 | Underground treasury; gold survives disasters |
| Mortal Mask | 750 years | Level 12 | Perfect disguise; direct mortal interaction |

### 5.5 Ledger Discovery Mechanics

**Full implementation of LpLedger from 1.7 skeleton.**

**Discovery Methods:**

| Method | Trigger | Discovery Type |
|--------|---------|----------------|
| Agent Reports | Random per cycle per agent | Economic, Agent |
| Event Survival | Specific events | Economic, Competitor |
| Competitor Interaction | Alliance/conflict | Competitor |
| Achievement Completion | Specific achievements | Hidden |
| Investment Milestones | Portfolio thresholds | Economic |

**Ledger Entry Data:**
```yaml
# data/ledger/valdris-defaults.yaml
id: valdris-defaults
category: economic
name: "Valdris Default Cycle"
description: "Valdris defaults on crown bonds approximately every 127 years (±30)."
effect:
  type: divination_hint
  target: valdris
  event_type: default
discovery:
  method: event_survival
  event_id: valdris-default
  occurrences_required: 2
hidden: false
```

**Ledger UI:**
- Organized by category tabs
- Discovered entries show full text
- Undiscovered entries show "???" with hint about discovery method
- Progress counter per category

### 5.6 Idle Integration

Use `LrgIdleCalculator`:
- Track passive income during slumber
- Offline progression with efficiency caps
- Uses `LP_MIN_SLUMBER_YEARS` (25) as minimum

Use `LrgAutomation`:
- Dormant orders (if X happens, do Y)
- Wake conditions (threshold triggers)
- Maximum 5 dormant orders initially (upgradeable)

---

## Phase 6: User Interface

**Goal:** Implement all game screens with keyboard/mouse navigation.

### 6.1 Screen Classes

All extend `LrgWidget` or `LrgContainer`.

| Screen | Purpose |
|--------|---------|
| LpScreenPortfolio | Portfolio breakdown, asset allocation |
| LpScreenWorldMap | Interactive map with kingdoms |
| LpScreenAgents | Agent list and management |
| LpScreenIntelligence | Reports, predictions, competitor info |
| LpScreenSlumber | Duration picker, wake conditions |

### 6.2 Event Dialog

**File:** `src/ui/lp-dialog-event.h/.c`

- Narrative text display
- Malachar's commentary
- Choice buttons
- Keyboard navigation (1-4 for choices)

### 6.3 Theme Configuration

**File:** `src/ui/lp-theme.c`

Dark fantasy aesthetic:
- Primary: Deep purple (#2d1b4e)
- Secondary: Bone white (#e8e0d5)
- Accent: Gold (#c9a227)
- Background: Near black (#0a0a0f)
- Text: Off-white (#d4d0c8)

### 6.4 Input Handling

All screens support:
- Mouse: Click, drag, hover
- Keyboard: Tab (next), Shift+Tab (prev), Enter (select), Escape (back)
- Arrow keys for list navigation
- Number keys for quick selection

---

## Phase 6.5: Feedback Systems

**Goal:** Make "numbers going up" visible and satisfying. Essential for idle game dopamine.

### 6.5.1 Floating Text System

**File:** `src/feedback/lp-floating-text.h/.c`

**Purpose:** Show gold changes as floating numbers that drift upward and fade.

```c
#define LP_TYPE_FLOATING_TEXT (lp_floating_text_get_type ())
G_DECLARE_FINAL_TYPE (LpFloatingText, lp_floating_text, LP, FLOATING_TEXT, LrgWidget)

/* Create floating text at position */
LpFloatingText *lp_floating_text_new (const gchar *text,
                                       gfloat       x,
                                       gfloat       y,
                                       GrlColor    *color);

/* Spawn gold change indicator */
void lp_floating_text_spawn_gold (LrgContainer    *parent,
                                  LrgBigNumber    *amount,
                                  gboolean         positive,
                                  gfloat           x,
                                  gfloat           y);
```

**Behavior:**
- Positive changes: Gold color, "+X gp", floats upward
- Negative changes: Red color, "-X gp", floats upward
- Large changes (>10% net worth): Larger font, particle burst
- Uses `LrgTweenManager` for animation

### 6.5.2 Growth Particles

**File:** `src/feedback/lp-growth-particles.h/.c`

**Purpose:** Visual celebration of portfolio growth.

```c
#define LP_TYPE_GROWTH_PARTICLES (lp_growth_particles_get_type ())
G_DECLARE_FINAL_TYPE (LpGrowthParticles, lp_growth_particles, LP, GROWTH_PARTICLES, LrgWidget)

/* Spawn particles at location */
void lp_growth_particles_spawn (LpGrowthParticles *self,
                                gfloat             x,
                                gfloat             y,
                                LpGrowthIntensity  intensity);
```

**Intensities:**
| Intensity | Trigger | Effect |
|-----------|---------|--------|
| MINOR | <10% growth | Few golden sparkles |
| MODERATE | 10-50% growth | Coin shower |
| MAJOR | 50-200% growth | Gold burst with trails |
| LEGENDARY | >200% growth | Full screen golden rain |

**Uses:** `LrgParticleSystem` from libregnum.

### 6.5.3 Synergy Activation Effects

**File:** `src/feedback/lp-synergy-effect.h/.c`

**Purpose:** Visual feedback when synergies activate or complete.

```c
void lp_synergy_effect_play_activation (LrgContainer *parent,
                                        LpSynergy    *synergy);

void lp_synergy_effect_play_completion (LrgContainer *parent,
                                        LpSynergy    *synergy);
```

**Behavior:**
- Lines connect synergy-linked investments on portfolio screen
- Pulse effect when synergy bonus applies
- Sound cue (procedural "ding")

### 6.5.4 Achievement Popup

**File:** `src/feedback/lp-achievement-popup.h/.c`

**Purpose:** Celebration when achievements unlock.

```c
#define LP_TYPE_ACHIEVEMENT_POPUP (lp_achievement_popup_get_type ())
G_DECLARE_FINAL_TYPE (LpAchievementPopup, lp_achievement_popup, LP, ACHIEVEMENT_POPUP, LrgWidget)

/* Show achievement */
void lp_achievement_popup_show (LpAchievementPopup *self,
                                LrgAchievement     *achievement);
```

**Display:**
- Slides in from top-right
- Shows achievement icon, name, description
- Gold border animation
- Auto-dismisses after 5 seconds
- Click to dismiss early

### 6.5.5 Slumber Time Visualization

**File:** `src/feedback/lp-slumber-visualization.h/.c`

**Purpose:** Make slumber phase visually interesting instead of just a loading screen.

**Implementation:**
- Event timeline scrolls by during simulation
- Key events highlighted with brief text
- "Dormant order triggered!" notifications
- Year counter ticks up with sound
- Can be accelerated with button hold

---

## Phase 7: Save/Load and Settings

**Goal:** Implement persistent storage.

### 7.1 Saveable Classes

All implement `LrgSaveable` interface:
- LpGameData
- LpPortfolio
- LpAgentManager
- LpKingdom (each instance)
- LpPhylactery
- LpPrestigeManager
- LpAchievementManager

### 7.2 Save Format

YAML-based via `LrgSaveContext`:

```yaml
save_version: 1
game_data:
  current_year: 1247
  total_years_played: 497
portfolio:
  investments:
    - type: property
      id: valdris-farmland-001
      purchase_price: "1000"
      current_value: "4520"
agents:
  # ...
```

### 7.3 Settings Groups

Use `LrgSettings`:
- Graphics (LrgGraphicsSettings)
- Audio (LrgAudioSettings)
- Gameplay (custom group):
  - Offline time ratio
  - Auto-pause on events
  - Notification preferences
  - UI scale

---

## Phase 8: Achievement System & Steam Integration

**Goal:** Implement local achievements with optional Steam sync.

### 8.1 Local Achievement System

**File:** `src/achievement/lp-achievement-manager.h/.c`

Wraps `LrgAchievementManager` from libregnum.

**Features:**
- Tracks all achievements locally
- Persists to save file
- In-game unlock notifications
- Progress tracking for incremental achievements

### 8.2 Achievement Definitions

| ID | Name | Condition | Hidden |
|----|------|-----------|--------|
| first_million | First Million | Reach 1,000,000 gp | No |
| centennial | Centennial | Complete 100-year slumber | No |
| dynasty | Dynasty | 5-generation agent family | No |
| hostile_takeover | Hostile Takeover | Own kingdom's entire debt | No |
| patient_investor | Patient Investor | 500-year investment hold | No |
| dark_awakening | Dark Awakening | Unlock dark investments | Yes |
| soul_trader | Soul Trader | Complete first soul trade | Yes |
| transcendence | Transcendence | Complete first prestige | No |

### 8.3 Steam Bridge (Optional)

**File:** `src/steam/lp-steam-bridge.h/.c`

Conditional compilation with `STEAM=1`:
- Syncs achievements to Steam
- Steam Cloud save integration
- Leaderboard uploads
- All methods are no-ops when Steam unavailable

---

## Phase 9: Polish and Content

**Goal:** Add narrative content, visual polish, and audio.

### 9.1 Narrative Content

Event templates with Malachar commentary:
- Kingdom collapse events
- Agent succession events
- Competitor interactions
- Discovery/crusade events

### 9.2 Visual Effects

Using libregnum systems:
- `LrgParticleSystem` for magical effects
- `LrgTweenManager` for UI animations
- `LrgTransitionManager` for screen transitions

### 9.3 Audio

Procedural ambient using `LrgProceduralAudio`:
- Dark fantasy drone sounds
- UI feedback sounds
- Event notification chimes

### 9.4 Tutorial System

Using `LrgTutorialManager`:
- First-time player guidance
- Highlight important UI elements
- Explain core mechanics progressively

### 9.5 Localization

Using `LrgLocalization`:
- English as base language
- String keys in data files
- Structure for future translations

---

## Phase 10: Testing and Release

**Goal:** Comprehensive testing and Steam release.

### 10.1 Unit Tests (GTest)

| Test File | Coverage |
|-----------|----------|
| test-big-number.c | LrgBigNumber operations |
| test-investment.c | Investment calculations |
| test-agent.c | Agent lifecycle |
| test-kingdom.c | Kingdom simulation |
| test-simulation.c | World simulation ticks |
| test-save-load.c | Save/load round-trips |
| test-prestige.c | Prestige mechanics |
| test-portfolio.c | Portfolio management |
| test-achievement.c | Achievement unlocking |

### 10.2 Integration Testing

- Full game loop (new game -> invest -> slumber -> wake)
- Save/load across versions
- Cross-platform testing (Linux, Windows)
- Steam integration testing (STEAM=1)
- Offline mode testing

### 10.3 Documentation Verification

- All docs/*.md files complete
- Source code comments reviewed
- CLAUDE.md accurate
- README.md build instructions tested

### 10.4 Release Preparation

- Steam store page assets
- Trading cards (6 cards)
- Achievement icons
- Press kit
- Beta testing period

---

## MVP Milestone Checklist

Phase 1-3 MVP Goal: **Playable core loop with engagement systems**

### Core Gameplay
- [ ] Player can start new game **with First Awakening tutorial**
- [ ] Player can view current portfolio **with synergy indicators**
- [ ] Player can buy/sell 3 basic investment types (property, trade, bonds)
- [ ] Player can set slumber duration (25, 50, 100 years minimum)
- [ ] Time simulation runs and calculates returns
- [ ] Wake report shows what happened **with visual growth animations**
- [ ] **Compound interest preview panel visible on slumber screen**

### Engagement Systems (NEW)
- [ ] First Awakening tutorial plays on new game (skippable)
- [ ] At least 2 synergies functional and detecting
- [ ] Exposure meter visible and tracking player visibility
- [ ] Floating gold numbers on income events
- [ ] Portfolio growth particles on significant gains
- [ ] Achievement popup shows on unlock
- [ ] Ledger UI accessible (empty structure, ready for content)

### Input & UI
- [ ] All menus navigable with mouse
- [ ] All menus navigable with keyboard (Tab/Arrows/Enter/Escape)
- [ ] Focus indicators visible for keyboard navigation
- [ ] Basic UI with minimalist styling
- [ ] Synergy indicator widget on portfolio screen
- [ ] Exposure meter widget on main HUD

### Infrastructure
- [ ] Game saves and loads correctly (local, no Steam required)
- [ ] Ambient procedural audio plays
- [ ] Local achievement system tracks progress
- [ ] Unit tests pass for core systems
- [ ] Synergy manager loads from YAML data files
- [ ] Trait system functional for agent families

### Documentation
- [ ] README.md complete with build instructions
- [ ] CLAUDE.md accurate for project
- [ ] docs/architecture.md explains system design
- [ ] Source code has clear comments

---

## Build Dependencies

### Fedora Packages

```bash
sudo dnf install gcc make pkg-config
sudo dnf install glib2-devel gobject-introspection-devel
sudo dnf install libdex-devel libyaml-devel json-glib-devel
```

### Windows Cross-Compilation

```bash
sudo dnf install mingw64-gcc mingw64-glib2 mingw64-pkg-config
```

### Steam SDK (Optional)

- Download Steamworks SDK from Valve
- Place in `deps/steamworks/`
- Build with `make STEAM=1`

---

## GObject Patterns Reference

### Type Declaration

```c
#define LP_TYPE_GAME_DATA (lp_game_data_get_type ())
G_DECLARE_FINAL_TYPE (LpGameData, lp_game_data, LP, GAME_DATA, GObject)
```

### Saveable Implementation

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

static gboolean
lp_game_data_load (LrgSaveable    *saveable,
                   LrgSaveContext *ctx,
                   GError        **error)
{
    LpGameData *self = LP_GAME_DATA (saveable);
    self->current_year = lrg_save_context_read_uint (ctx, "current-year", 847);
    return TRUE;
}
```

### Memory Management

```c
g_autoptr(LpInvestment) inv = lp_investment_property_new ();
g_autoptr(GError) error = NULL;
g_autofree gchar *name = g_strdup ("test");
g_clear_object (&self->portfolio);
return g_steal_pointer (&result);
```

---

## Estimated Scope

| Phase | Description | Estimated Files | Notes |
|-------|-------------|-----------------|-------|
| 0 | Foundation | 8 | Build system, docs |
| 1 | Core Architecture | 16 | +exposure, synergy, ledger, tutorial |
| 2 | Investment System | 16 | +compound preview, synergy integration |
| 3 | Agent System | 12 | +trait system |
| 4 | World Simulation | 12 | Kingdom, events, competitors |
| 5 | Progression | 12 | +megaprojects, echo trees, ledger mechanics |
| 6 | User Interface | 18 | +ledger screen, megaproject screen, widgets |
| 6.5 | Feedback Systems | 8 | Floating text, particles, popups |
| 7 | Save/Settings | 4 | Persistence |
| 8 | Achievements/Steam | 4 | Local + optional Steam sync |
| 9 | Polish/Content | 10+ | Narrative, VFX, audio |
| 10 | Testing | 16+ | +synergy, exposure, trait, megaproject tests |

**Total: ~130+ source files**

**Data Files (YAML):**
| Directory | Count | Purpose |
|-----------|-------|---------|
| data/synergies/ | 10+ | Investment synergy definitions |
| data/traits/ | 10+ | Bloodline trait definitions |
| data/megaprojects/ | 5+ | Multi-century project definitions |
| data/ledger/ | 20+ | Discoverable information entries |
| data/eras/ | 4 | Era transition definitions |
| data/kingdoms/ | 6+ | Kingdom definitions |
| data/investments/ | 20+ | Investment type definitions |
| data/events/ | 30+ | Event templates |
| data/achievements/ | 20+ | Achievement definitions |

---

## Data Schema Examples

### Synergy Schema

```yaml
# data/synergies/shadow-bank.yaml
id: shadow-bank
name: "Shadow Bank"
description: "The unholy trinity of finance, politics, and darkness"
tier: 3                           # Requires tier-2 synergies first
requirements:
  - asset_class: financial
    min_value: 100000             # Minimum gold value
  - asset_class: political
    min_count: 2                  # At least 2 investments
  - asset_class: dark
    min_count: 1
bonus:
  - type: income_multiplier
    target: financial
    value: 2.0                    # Double financial income
  - type: exposure_reduction
    value: -10                    # Reduce exposure by 10%
lore: "When the currency is backed by souls, inflation is... theological."
achievement: shadow-banker        # Unlocks this achievement when active
discovery: automatic              # Appears when conditions met
```

### Trait Schema

```yaml
# data/traits/ruthless.yaml
id: ruthless
name: "Ruthless"
description: "This bloodline will do whatever it takes"
icon: trait-ruthless              # Asset reference
inheritance:
  base_chance: 30                 # 30% base inheritance
  per_parent_with_trait: 15       # +15% per parent with trait
  can_emerge: true                # Can spontaneously appear (5% base)
effects:
  - type: income_multiplier
    value: 1.1                    # +10% income
  - type: unlock_action
    action: eliminate_competitor_agent
  - type: loyalty_modifier
    value: -5                     # -5 loyalty (more independent)
  - type: discovery_modifier
    value: 0.05                   # +5% chance of being discovered
conflicts_with:
  - compassionate
  - cautious
synergizes_with:
  - ambitious
  - cunning
lore: "The Crane bloodline learned early: mercy is a luxury for the living."
```

### Megaproject Schema

```yaml
# data/megaprojects/deep-vault.yaml
id: deep-vault
name: "The Deep Vault"
description: "An underground treasury hidden beneath the earth itself"
icon: megaproject-vault
duration: 300                     # Total years to complete
cost_per_year: 1500               # Gold cost per year
unlock_requirement:
  phylactery_level: 5
phases:
  - name: "Excavation"
    years: 100
    description: "Agents dig deep beneath your primary holding"
    effect: null
  - name: "Fortification"
    years: 100
    description: "Magical wards and physical reinforcements"
    effect:
      type: gold_protection
      value: 0.25                 # Protect 25% of gold from disasters
  - name: "Completion"
    years: 100
    description: "The vault becomes fully operational"
    effect:
      type: gold_protection
      value: 1.0                  # Full gold protection
      disaster_immunity: true     # Gold survives any event
risk:
  discovery_per_decade: 3         # 3% discovery chance per decade
  on_discovery: project_delayed   # Discovery delays but doesn't destroy
lore: "Even the gods would need to dig to find what lies beneath."
achievement: deep-buried
```

### Ledger Entry Schema

```yaml
# data/ledger/valdris-defaults.yaml
id: valdris-defaults
category: economic                # economic, agent, competitor, hidden
name: "Valdris Default Cycle"
description: "Valdris defaults on crown bonds approximately every 127 years (±30)."
detail: |
  Historical analysis reveals the Kingdom of Valdris operates on a
  predictable debt cycle. Aggressive military spending leads to
  over-leverage, followed by default and economic collapse.
  The cycle typically takes 100-150 years.
effect:
  type: divination_hint
  target: valdris
  event_type: default
  accuracy: 0.7                   # 70% prediction accuracy
discovery:
  method: event_survival          # How this entry is discovered
  event_id: valdris-default
  occurrences_required: 2         # Must survive 2 defaults
hidden: false                     # False = shows in Ledger when discovered
lore: "Those who remember history are condemned to profit from it."
```

### Era Schema

```yaml
# data/eras/industrial-dawn.yaml
id: industrial-dawn
name: "The Industrial Dawn"
description: "Steam and steel reshape the world of commerce"
trigger:
  year_range: [2500, 3000]        # Random trigger in this range
  probability_per_decade: 15      # 15% chance per decade once in range
effects:
  - type: new_asset_class
    class: manufacturing          # Unlock manufacturing investments
  - type: modify_asset_returns
    class: trade
    modifier: 1.5                 # Trade routes become more valuable
  - type: modify_asset_returns
    class: property
    subtype: agricultural
    modifier: 0.7                 # Agricultural land less valuable
  - type: exposure_decay_modifier
    value: 1.5                    # Exposure decays faster (more anonymity)
new_investments:
  - id: factory
    name: "Factory"
    description: "Industrial production facility"
    base_return: 0.08
    risk: medium
  - id: railroad
    name: "Railroad Investment"
    description: "Shares in the emerging rail networks"
    base_return: 0.12
    risk: high
narrative:
  announcement: |
    The age of muscle gives way to the age of machines.
    Smoke rises from new towers of industry.
    You sense... opportunity.
lore: "Progress is merely decay with better marketing."
```

---

## GObject Patterns Reference (Extended)

### Manager Singleton Pattern

```c
/* lp-synergy-manager.h */
#define LP_TYPE_SYNERGY_MANAGER (lp_synergy_manager_get_type ())
G_DECLARE_FINAL_TYPE (LpSynergyManager, lp_synergy_manager, LP, SYNERGY_MANAGER, GObject)

LpSynergyManager *lp_synergy_manager_get_default (void);
```

```c
/* lp-synergy-manager.c */
#include "lp-synergy-manager.h"

struct _LpSynergyManager
{
    GObject  parent_instance;

    GPtrArray *synergies;         /* Array of LpSynergy* */
    GHashTable *synergy_by_id;    /* id -> LpSynergy* */
};

G_DEFINE_TYPE (LpSynergyManager, lp_synergy_manager, G_TYPE_OBJECT)

static LpSynergyManager *default_manager = NULL;

LpSynergyManager *
lp_synergy_manager_get_default (void)
{
    if (default_manager == NULL)
    {
        default_manager = g_object_new (LP_TYPE_SYNERGY_MANAGER, NULL);
        g_object_add_weak_pointer (G_OBJECT (default_manager),
                                   (gpointer *)&default_manager);
    }

    return default_manager;
}

static void
lp_synergy_manager_finalize (GObject *object)
{
    LpSynergyManager *self = LP_SYNERGY_MANAGER (object);

    g_clear_pointer (&self->synergies, g_ptr_array_unref);
    g_clear_pointer (&self->synergy_by_id, g_hash_table_unref);

    G_OBJECT_CLASS (lp_synergy_manager_parent_class)->finalize (object);
}

static void
lp_synergy_manager_init (LpSynergyManager *self)
{
    self->synergies = g_ptr_array_new_with_free_func (g_object_unref);
    self->synergy_by_id = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, NULL);
}

static void
lp_synergy_manager_class_init (LpSynergyManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_synergy_manager_finalize;
}
```

### Data-Driven Loading Pattern

```c
/**
 * lp_synergy_manager_load_synergies:
 * @self: an #LpSynergyManager
 * @data_dir: path to synergies data directory
 * @error: return location for error
 *
 * Loads all synergy definitions from YAML files in @data_dir.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_synergy_manager_load_synergies (LpSynergyManager  *self,
                                   const gchar       *data_dir,
                                   GError           **error)
{
    g_autoptr(GDir) dir = NULL;
    const gchar *filename;

    g_return_val_if_fail (LP_IS_SYNERGY_MANAGER (self), FALSE);
    g_return_val_if_fail (data_dir != NULL, FALSE);

    dir = g_dir_open (data_dir, 0, error);
    if (dir == NULL)
        return FALSE;

    while ((filename = g_dir_read_name (dir)) != NULL)
    {
        g_autofree gchar *path = NULL;
        g_autoptr(LpSynergy) synergy = NULL;
        g_autoptr(GError) local_error = NULL;

        if (!g_str_has_suffix (filename, ".yaml"))
            continue;

        path = g_build_filename (data_dir, filename, NULL);
        synergy = lp_synergy_load_from_yaml (path, &local_error);

        if (synergy == NULL)
        {
            g_warning ("Failed to load synergy %s: %s",
                       filename, local_error->message);
            continue;
        }

        g_ptr_array_add (self->synergies, g_object_ref (synergy));
        g_hash_table_insert (self->synergy_by_id,
                             g_strdup (lp_synergy_get_id (synergy)),
                             synergy);
    }

    return TRUE;
}
```

### Signal Emission Pattern

```c
/* In lp-exposure-manager.h */
/* Signals */
enum
{
    SIGNAL_THRESHOLD_CROSSED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/* In class_init */
static void
lp_exposure_manager_class_init (LpExposureManagerClass *klass)
{
    /* ... */

    /**
     * LpExposureManager::threshold-crossed:
     * @self: the #LpExposureManager
     * @old_level: the previous exposure level
     * @new_level: the new exposure level
     *
     * Emitted when exposure crosses a threshold boundary.
     */
    signals[SIGNAL_THRESHOLD_CROSSED] =
        g_signal_new ("threshold-crossed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      2,
                      G_TYPE_INT,   /* old_level */
                      G_TYPE_INT);  /* new_level */
}

/* Emitting the signal */
static void
lp_exposure_manager_check_thresholds (LpExposureManager *self)
{
    LpExposureLevel old_level;
    LpExposureLevel new_level;

    old_level = self->current_level;
    new_level = calculate_level (self->total_exposure);

    if (old_level != new_level)
    {
        self->current_level = new_level;
        g_signal_emit (self, signals[SIGNAL_THRESHOLD_CROSSED], 0,
                       old_level, new_level);
    }
}
```

### Derivable Type with Virtual Methods

```c
/* lp-investment.h */
#define LP_TYPE_INVESTMENT (lp_investment_get_type ())
G_DECLARE_DERIVABLE_TYPE (LpInvestment, lp_investment, LP, INVESTMENT, GObject)

struct _LpInvestmentClass
{
    GObjectClass parent_class;

    /* Virtual methods - subclasses override these */
    LrgBigNumber *(*calculate_returns) (LpInvestment *self,
                                        guint         years);
    void          (*apply_event)       (LpInvestment *self,
                                        LpEvent      *event);
    gboolean      (*can_sell)          (LpInvestment *self);
    gdouble       (*get_risk_modifier) (LpInvestment *self);

    /* Padding for ABI stability */
    gpointer _reserved[8];
};

/* Public API - calls virtual methods */
LrgBigNumber *lp_investment_calculate_returns (LpInvestment *self,
                                                guint         years);
```

```c
/* lp-investment.c */
LrgBigNumber *
lp_investment_calculate_returns (LpInvestment *self,
                                  guint         years)
{
    LpInvestmentClass *klass;

    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    klass = LP_INVESTMENT_GET_CLASS (self);
    g_return_val_if_fail (klass->calculate_returns != NULL, NULL);

    return klass->calculate_returns (self, years);
}
```

```c
/* lp-investment-property.c - Subclass implementation */
#include "lp-investment-property.h"

struct _LpInvestmentProperty
{
    LpInvestment parent_instance;

    gchar   *region_id;
    gdouble  stability_bonus;
};

G_DEFINE_TYPE (LpInvestmentProperty, lp_investment_property, LP_TYPE_INVESTMENT)

static LrgBigNumber *
lp_investment_property_calculate_returns (LpInvestment *investment,
                                           guint         years)
{
    LpInvestmentProperty *self = LP_INVESTMENT_PROPERTY (investment);
    g_autoptr(LrgBigNumber) base = NULL;
    g_autoptr(LrgBigNumber) rate = NULL;
    g_autoptr(LrgBigNumber) result = NULL;

    base = lp_investment_get_current_value (investment);

    /* Property has stable 3-5% annual returns */
    rate = lrg_big_number_new_double (1.0 + (0.03 * self->stability_bonus));

    result = lrg_big_number_pow_uint (rate, years);
    result = lrg_big_number_multiply (base, result);

    return g_steal_pointer (&result);
}

static void
lp_investment_property_class_init (LpInvestmentPropertyClass *klass)
{
    LpInvestmentClass *investment_class = LP_INVESTMENT_CLASS (klass);

    /* Override virtual methods */
    investment_class->calculate_returns = lp_investment_property_calculate_returns;
}
```

---

*This document serves as the technical implementation plan for Lich's Portfolio. Refer to design/GAME.md for the full game design document.*
