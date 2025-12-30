# Lich's Portfolio - Project Guide

## Project Overview

Lich's Portfolio is a 2D idle/incremental strategy game built on the libregnum game engine. The player controls Malachar the Undying, a lich building a financial empire across centuries.

**Key Technologies:**
- Language: C (gnu89 standard)
- Object System: GLib/GObject
- Game Engine: libregnum (deps/libregnum/)
- Build System: GNU Make
- Data Format: YAML (via yaml-glib)

## Directory Structure

```
lichs-portfolio/
├── Makefile                    # Root build orchestration
├── config.mk                   # Build configuration
├── rules.mk                    # Build helper functions
├── src/
│   ├── main.c                  # Entry point
│   ├── lp-types.h              # Forward declarations for all Lp* types
│   ├── lp-enums.h/.c           # Game enumerations with GType
│   ├── lp-log.h                # Logging macros
│   ├── core/                   # Core game classes
│   │   ├── lp-application.h/.c # Main application singleton
│   │   ├── lp-game-data.h/.c   # Game state container
│   │   ├── lp-phylactery.h/.c  # Upgrade tree
│   │   └── lp-prestige.h/.c    # Prestige/meta-progression
│   ├── simulation/             # World simulation
│   │   ├── lp-world-simulation.h/.c
│   │   ├── lp-kingdom.h/.c
│   │   ├── lp-region.h/.c
│   │   ├── lp-event.h/.c
│   │   └── lp-competitor.h/.c
│   ├── investment/             # Investment system
│   │   ├── lp-investment.h/.c  # Base class
│   │   ├── lp-investment-*.h/.c # Concrete types
│   │   └── lp-portfolio.h/.c
│   ├── agent/                  # Agent system
│   │   ├── lp-agent.h/.c       # Base class
│   │   ├── lp-agent-*.h/.c     # Concrete types
│   │   └── lp-agent-manager.h/.c
│   ├── ui/                     # UI screens
│   └── states/                 # Game states (extend LrgGameState)
├── data/                       # YAML data files
├── assets/                     # Textures, fonts, audio
├── tests/                      # GTest unit tests
├── docs/                       # Documentation
└── design/
    ├── GAME.md                 # Game design document
    └── PLAN.md                 # Technical implementation plan
```

## Build Commands

```bash
make                  # Build deps + game (release)
make DEBUG=1          # Debug build with -g3 -O0
make WINDOWS=1        # Cross-compile for Windows
make STEAM=1          # Build with Steam SDK
make test             # Run unit tests
make clean            # Remove build artifacts
make help             # Show all targets
```

## Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Macros/Defines | `UPPERCASE_SNAKE_CASE` | `LP_TYPE_INVESTMENT` |
| Types/Classes | `PascalCase` with `Lp` prefix | `LpInvestment`, `LpAgentManager` |
| Functions | `lowercase_snake_case` with `lp_` prefix | `lp_investment_calculate_returns` |
| Variables | `lowercase_snake_case` | `current_year`, `agent_count` |
| Properties | `kebab-case` | `"purchase-price"`, `"current-value"` |
| Signals | `kebab-case` | `"investment-sold"`, `"agent-died"` |
| Enums | `PascalCase` type, `UPPERCASE` values | `LpAssetClass`, `LP_ASSET_CLASS_PROPERTY` |

## GObject Patterns

### Type Declaration

```c
/* In header file */
#define LP_TYPE_GAME_DATA (lp_game_data_get_type ())
G_DECLARE_FINAL_TYPE (LpGameData, lp_game_data, LP, GAME_DATA, GObject)

/* For derivable types (can be subclassed) */
G_DECLARE_DERIVABLE_TYPE (LpInvestment, lp_investment, LP, INVESTMENT, GObject)

struct _LpInvestmentClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    LrgBigNumber *(*calculate_returns) (LpInvestment *self, guint years);
    void          (*apply_event)       (LpInvestment *self, LpEvent *event);

    gpointer _reserved[8];  /* ABI stability padding */
};
```

### Function Signatures

Return type on separate line, parameters aligned:

```c
LrgBigNumber *
lp_investment_calculate_returns (LpInvestment *self,
                                 guint         years)
{
    g_return_val_if_fail (LP_IS_INVESTMENT (self), NULL);

    /* Implementation */
}
```

### Memory Management

```c
/* Use g_autoptr for automatic cleanup */
g_autoptr(LpInvestment) inv = lp_investment_property_new ();
g_autoptr(GError) error = NULL;
g_autofree gchar *name = g_strdup ("test");

/* Clear object references */
g_clear_object (&self->portfolio);
g_clear_pointer (&self->name, g_free);

/* Transfer ownership */
return g_steal_pointer (&result);
```

### Interface Implementation (LrgSaveable)

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
    lrg_save_context_write_uint (ctx, "total-years", self->total_years_played);

    /* Save nested objects */
    lrg_save_context_begin_section (ctx, "portfolio");
    lrg_saveable_save (LRG_SAVEABLE (self->portfolio), ctx, error);
    lrg_save_context_end_section (ctx);

    return TRUE;
}

static gboolean
lp_game_data_load (LrgSaveable    *saveable,
                   LrgSaveContext *ctx,
                   GError        **error)
{
    LpGameData *self = LP_GAME_DATA (saveable);

    self->current_year = lrg_save_context_read_uint (ctx, "current-year", 847);
    self->total_years_played = lrg_save_context_read_uint (ctx, "total-years", 0);

    if (lrg_save_context_enter_section (ctx, "portfolio"))
    {
        lrg_saveable_load (LRG_SAVEABLE (self->portfolio), ctx, error);
        lrg_save_context_leave_section (ctx);
    }

    return TRUE;
}

/* In interface init */
static void
lp_game_data_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_game_data_get_save_id;
    iface->save = lp_game_data_save;
    iface->load = lp_game_data_load;
}
```

## Key libregnum Types

### Idle Module (src/idle/)

| Type | Purpose |
|------|---------|
| `LrgBigNumber` | Arbitrary precision numbers (mantissa * 10^exp) |
| `LrgIdleCalculator` | Passive income calculation, offline progress |
| `LrgPrestige` | Prestige/reset mechanics (derivable) |
| `LrgUnlockTree` | Unlock/upgrade dependency graph |
| `LrgAutomation` | Conditional actions, threshold triggers |

### Economy Module (src/economy/)

| Type | Purpose |
|------|---------|
| `LrgResource` | Resource type definition (derivable) |
| `LrgResourcePool` | Container for resource quantities |
| `LrgMarket` | Supply/demand pricing simulation |
| `LrgProducer` | Produces resources over time (component) |

### Game State (src/gamestate/)

| Type | Purpose |
|------|---------|
| `LrgGameState` | Base class for game states (derivable) |
| `LrgGameStateManager` | Stack-based state management |

### Save System (src/save/)

| Type | Purpose |
|------|---------|
| `LrgSaveable` | Interface for persistent objects |
| `LrgSaveContext` | YAML serialization context |
| `LrgSaveManager` | Coordinates save/load operations |

### UI (src/ui/)

| Type | Purpose |
|------|---------|
| `LrgWidget` | Base UI widget (derivable) |
| `LrgContainer` | Widget that holds children (derivable) |
| `LrgButton`, `LrgLabel`, etc. | Concrete widgets |
| `LrgTheme` | UI theming singleton |

## Game-Specific Types

### Core Types

| Type | File | Purpose |
|------|------|---------|
| `LpApplication` | core/lp-application.h | Main application singleton |
| `LpGameData` | core/lp-game-data.h | Game state container |
| `LpPhylactery` | core/lp-phylactery.h | Upgrade tree |
| `LpPrestigeManager` | core/lp-prestige.h | Prestige system |

### Investment Types

| Type | File | Purpose |
|------|------|---------|
| `LpInvestment` | investment/lp-investment.h | Base investment class |
| `LpInvestmentProperty` | investment/lp-investment-property.h | Real estate |
| `LpInvestmentTrade` | investment/lp-investment-trade.h | Trade routes |
| `LpInvestmentFinancial` | investment/lp-investment-financial.h | Bonds, notes |
| `LpPortfolio` | investment/lp-portfolio.h | Investment container |

### Agent Types

| Type | File | Purpose |
|------|------|---------|
| `LpAgent` | agent/lp-agent.h | Base agent class |
| `LpAgentIndividual` | agent/lp-agent-individual.h | Single mortal |
| `LpAgentFamily` | agent/lp-agent-family.h | Bloodline |
| `LpAgentManager` | agent/lp-agent-manager.h | Agent lifecycle |

### Simulation Types

| Type | File | Purpose |
|------|------|---------|
| `LpWorldSimulation` | simulation/lp-world-simulation.h | Main simulation |
| `LpKingdom` | simulation/lp-kingdom.h | Kingdom state |
| `LpRegion` | simulation/lp-region.h | Geographic region |
| `LpEvent` | simulation/lp-event.h | Base event class |

### Game States

| Type | File | Purpose |
|------|------|---------|
| `LpStateMainMenu` | states/lp-state-main-menu.h | Title screen |
| `LpStateWake` | states/lp-state-wake.h | Wake report |
| `LpStateAnalyze` | states/lp-state-analyze.h | World view |
| `LpStateSlumber` | states/lp-state-slumber.h | Slumber config |

## Enumerations

```c
/* Asset classes for investments */
typedef enum
{
    LP_ASSET_CLASS_PROPERTY,
    LP_ASSET_CLASS_TRADE,
    LP_ASSET_CLASS_FINANCIAL,
    LP_ASSET_CLASS_MAGICAL,
    LP_ASSET_CLASS_POLITICAL,
    LP_ASSET_CLASS_DARK
} LpAssetClass;

/* Agent types */
typedef enum
{
    LP_AGENT_TYPE_INDIVIDUAL,
    LP_AGENT_TYPE_FAMILY,
    LP_AGENT_TYPE_CULT,
    LP_AGENT_TYPE_BOUND
} LpAgentType;

/* Risk levels */
typedef enum
{
    LP_RISK_LEVEL_LOW,
    LP_RISK_LEVEL_MEDIUM,
    LP_RISK_LEVEL_HIGH,
    LP_RISK_LEVEL_EXTREME
} LpRiskLevel;

/* Event types */
typedef enum
{
    LP_EVENT_TYPE_ECONOMIC,
    LP_EVENT_TYPE_POLITICAL,
    LP_EVENT_TYPE_MAGICAL,
    LP_EVENT_TYPE_PERSONAL
} LpEventType;
```

## Comment Style

Always use `/* */` style, never `//`:

```c
/* Single line comment */

/*
 * Multi-line comment explaining
 * complex logic or algorithms.
 */

/**
 * lp_investment_calculate_returns:
 * @self: an #LpInvestment
 * @years: number of years to calculate
 *
 * Calculates the returns for this investment over the given period.
 *
 * Returns: (transfer full): The calculated returns as #LrgBigNumber
 */
LrgBigNumber *
lp_investment_calculate_returns (LpInvestment *self,
                                 guint         years)
{
    /* Implementation */
}
```

## Testing Patterns

```c
#include <glib.h>
#include "investment/lp-investment.h"

typedef struct
{
    LpInvestmentProperty *property;
} InvestmentFixture;

static void
fixture_set_up (InvestmentFixture *fixture,
                gconstpointer      user_data)
{
    fixture->property = lp_investment_property_new ();
}

static void
fixture_tear_down (InvestmentFixture *fixture,
                   gconstpointer      user_data)
{
    g_clear_object (&fixture->property);
}

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

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/investment/returns",
                InvestmentFixture, NULL,
                fixture_set_up,
                test_investment_returns,
                fixture_tear_down);

    return g_test_run ();
}
```

## Key Files Reference

### For GObject Patterns
- `deps/libregnum/src/core/lrg-engine.h/.c` - Derivable singleton
- `deps/libregnum/src/idle/lrg-prestige.h/.c` - Derivable with virtual methods
- `deps/libregnum/src/save/lrg-saveable.h/.c` - Interface definition

### For Idle Game Patterns
- `deps/libregnum/examples/game-chocolate-chip-clicker.c` - Full idle game example
- `deps/libregnum/src/idle/lrg-big-number.h/.c` - Arbitrary precision numbers
- `deps/libregnum/src/idle/lrg-idle-calculator.h/.c` - Passive income

### For Build System
- `deps/libregnum/config.mk` - Build configuration
- `deps/libregnum/rules.mk` - Build helpers
- `deps/libregnum/examples/Makefile` - Example game build

## Common Mistakes to Avoid

### GBoxed vs GObject (from graylib)

GBoxed types use `*_free()`, NOT `g_object_unref()`:

```c
/* CORRECT */
g_autoptr(GrlColor) color = grl_color_new (255, 100, 100, 255);
grl_color_free (color);  /* Or let g_autoptr handle it */

/* WRONG - will crash! */
g_object_unref (color);  /* GrlColor is GBoxed, not GObject */
```

### Transfer Semantics

Functions with `(transfer full)` parameters take ownership:

```c
/* Manager takes ownership - do NOT unref after */
LpInvestment *inv = lp_investment_property_new ();
lp_portfolio_add_investment (portfolio, inv);
/* Don't call g_object_unref (inv) here! */
```

### BigNumber Cleanup

`LrgBigNumber` is a GBoxed type:

```c
/* CORRECT */
g_autoptr(LrgBigNumber) num = lrg_big_number_new_double (1000.0);

/* Or manual cleanup */
LrgBigNumber *num = lrg_big_number_new_double (1000.0);
/* ... use num ... */
lrg_big_number_free (num);
```

## Debugging

```bash
# Run with GLib debug output
G_MESSAGES_DEBUG=all ./build/debug/lichs-portfolio

# Run with specific domain
G_MESSAGES_DEBUG=LichsPortfolio ./build/debug/lichs-portfolio

# Memory debugging with AddressSanitizer
make DEBUG=1 ASAN=1
./build/debug/lichs-portfolio
```

## Design Documents

- [design/GAME.md](design/GAME.md) - Full game design document
- [design/PLAN.md](design/PLAN.md) - Technical implementation plan
- [docs/architecture.md](docs/architecture.md) - System architecture

## Dependencies

For libregnum patterns and API, see:
- [deps/libregnum/CLAUDE.md](deps/libregnum/CLAUDE.md) - Engine guide
