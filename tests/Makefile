# tests/Makefile - Lich's Portfolio Test Build
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds and runs unit tests using GTest.

include ../config.mk
include ../rules.mk

# =============================================================================
# Test Configuration
# =============================================================================

TEST_BUILDDIR := $(BUILDDIR)/tests

# Test compiler flags
TEST_CFLAGS := $(BASE_CFLAGS) $(OPT_CFLAGS)
TEST_CFLAGS += -I$(PROJECT_ROOT)/src
TEST_CFLAGS += $(GLIB_CFLAGS)
TEST_CFLAGS += $(LIBREGNUM_CFLAGS)

# Test linker flags (use same libregnum paths as game)
TEST_LDFLAGS := $(OPT_LDFLAGS) $(LIBREGNUM_LDFLAGS)

# Test libraries - link against libregnum and glib (use same libs as game)
TEST_LIBS := $(LIBREGNUM_LIBS)
TEST_LIBS += $(GLIB_LIBS) $(JSON_LIBS) $(YAML_LIBS)
TEST_LIBS += $(PLATFORM_LIBS)

# Source objects to link against (for tests)
GAME_OBJECTS := $(OBJDIR)/src/lp-enums.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-game.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-prestige.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-exposure-manager.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-synergy-manager.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-ledger.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-phylactery.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-game-data.o
GAME_OBJECTS += $(OBJDIR)/src/investment/lp-investment.o
GAME_OBJECTS += $(OBJDIR)/src/investment/lp-investment-property.o
GAME_OBJECTS += $(OBJDIR)/src/investment/lp-investment-trade.o
GAME_OBJECTS += $(OBJDIR)/src/investment/lp-investment-financial.o
GAME_OBJECTS += $(OBJDIR)/src/investment/lp-portfolio.o
GAME_OBJECTS += $(OBJDIR)/src/agent/lp-agent.o
GAME_OBJECTS += $(OBJDIR)/src/agent/lp-trait.o
GAME_OBJECTS += $(OBJDIR)/src/agent/lp-agent-individual.o
GAME_OBJECTS += $(OBJDIR)/src/agent/lp-agent-family.o
GAME_OBJECTS += $(OBJDIR)/src/agent/lp-agent-manager.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-world-simulation.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-region.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-kingdom.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-event.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-event-economic.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-event-political.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-event-magical.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-event-personal.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-event-generator.o
GAME_OBJECTS += $(OBJDIR)/src/simulation/lp-competitor.o
GAME_OBJECTS += $(OBJDIR)/src/achievement/lp-achievement-manager.o

# Phase 5: Progression
GAME_OBJECTS += $(OBJDIR)/src/core/lp-prestige-manager.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-megaproject.o

# Phase 6: UI
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-theme.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-widget-exposure-meter.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-widget-synergy-indicator.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-screen-portfolio.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-screen-world-map.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-screen-agents.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-screen-intelligence.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-screen-slumber.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-screen-ledger.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-screen-megaprojects.o
GAME_OBJECTS += $(OBJDIR)/src/ui/lp-dialog-event.o

# Phase 6.5: Feedback Systems
GAME_OBJECTS += $(OBJDIR)/src/feedback/lp-floating-text.o
GAME_OBJECTS += $(OBJDIR)/src/feedback/lp-growth-particles.o
GAME_OBJECTS += $(OBJDIR)/src/feedback/lp-synergy-effect.o
GAME_OBJECTS += $(OBJDIR)/src/feedback/lp-achievement-popup.o
GAME_OBJECTS += $(OBJDIR)/src/feedback/lp-slumber-visualization.o

# Phase 7: Save
GAME_OBJECTS += $(OBJDIR)/src/save/lp-save-manager.o
GAME_OBJECTS += $(OBJDIR)/src/core/lp-gameplay-settings.o

# Phase 8: Steam Integration
GAME_OBJECTS += $(OBJDIR)/src/steam/lp-steam-bridge.o

# States (required by lp-game)
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-main-menu.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-wake.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-analyze.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-decide.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-slumber.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-simulating.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-pause.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-settings.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-welcome-back.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-investments.o
GAME_OBJECTS += $(OBJDIR)/src/states/lp-state-agents.o

# Phase 9: Polish and Content
GAME_OBJECTS += $(OBJDIR)/src/core/lp-strings.o
GAME_OBJECTS += $(OBJDIR)/src/narrative/lp-malachar-voice.o
GAME_OBJECTS += $(OBJDIR)/src/audio/lp-ambient-audio.o
GAME_OBJECTS += $(OBJDIR)/src/audio/lp-ui-sounds.o
GAME_OBJECTS += $(OBJDIR)/src/tutorial/lp-tutorial-sequences.o

# =============================================================================
# Test Sources
# =============================================================================

# Phase 0: Infrastructure test (standalone, no game objects)
STANDALONE_TESTS := test-stub.c

# Phase 1: Core type tests (require game objects)
LINKED_TESTS := test-exposure.c
LINKED_TESTS += test-portfolio.c
LINKED_TESTS += test-ledger.c
LINKED_TESTS += test-game-data.c

# Phase 2: Investment tests
LINKED_TESTS += test-investment.c

# Phase 3: Agent tests
LINKED_TESTS += test-agent.c

# Phase 4: Simulation tests
LINKED_TESTS += test-simulation.c

# Phase 5: Progression tests
LINKED_TESTS += test-progression.c

# Phase 6: UI tests
LINKED_TESTS += test-ui.c

# Phase 6.5: Feedback tests
LINKED_TESTS += test-feedback.c

# Phase 7: Save/Settings tests
LINKED_TESTS += test-save-load.c

# Phase 8: Achievement tests
LINKED_TESTS += test-achievement.c

# Phase 9: Polish tests
LINKED_TESTS += test-polish.c

# Phase 10: Additional coverage tests
LINKED_TESTS += test-synergy.c
LINKED_TESTS += test-trait.c
LINKED_TESTS += test-megaproject.c
LINKED_TESTS += test-integration.c

# =============================================================================
# Test Binaries
# =============================================================================

STANDALONE_BINARIES := $(STANDALONE_TESTS:%.c=$(TEST_BUILDDIR)/%)
LINKED_BINARIES := $(LINKED_TESTS:%.c=$(TEST_BUILDDIR)/%)
TEST_BINARIES := $(STANDALONE_BINARIES) $(LINKED_BINARIES)

# =============================================================================
# Build Rules
# =============================================================================

.PHONY: all
all: $(TEST_BINARIES)
	$(call print_status,"Tests built: $(words $(TEST_BINARIES)) test(s)")

# Standalone tests (no game objects needed)
$(TEST_BUILDDIR)/test-stub: test-stub.c | $(TEST_BUILDDIR)
	$(call print_compile,$<)
	@$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS) $(TEST_LIBS)

# Linked tests (require game objects and libregnum)
$(TEST_BUILDDIR)/test-%: test-%.c $(GAME_OBJECTS) | $(TEST_BUILDDIR)
	$(call print_compile,$<)
	@$(CC) $(TEST_CFLAGS) -o $@ $< $(GAME_OBJECTS) $(TEST_LDFLAGS) $(TEST_LIBS)

# =============================================================================
# Run Tests
# =============================================================================

.PHONY: run
run: all
	$(call print_status,"Running tests...")
	@failed=0; \
	for test in $(TEST_BINARIES); do \
		echo ""; \
		echo "Running: $$(basename $$test)"; \
		echo "----------------------------------------"; \
		if LD_LIBRARY_PATH="$(LIBREGNUM_LIBDIR):$(LIBREGNUM_DIR)/deps/graylib/build/lib:$(LIBREGNUM_DIR)/deps/yaml-glib/build:$$LD_LIBRARY_PATH" $$test --tap; then \
			echo "PASSED"; \
		else \
			echo "FAILED"; \
			failed=1; \
		fi; \
	done; \
	echo ""; \
	if [ $$failed -eq 0 ]; then \
		printf "$(COLOR_GREEN)$(COLOR_BOLD)==>$(COLOR_RESET) %s\n" "All tests passed"; \
	else \
		printf "$(COLOR_RED)$(COLOR_BOLD)Error:$(COLOR_RESET) %s\n" "Some tests failed"; \
		exit 1; \
	fi

# Verbose test run (shows all output)
.PHONY: run-verbose
run-verbose: all
	$(call print_status,"Running tests (verbose)...")
	@for test in $(TEST_BINARIES); do \
		echo ""; \
		echo "Running: $$test"; \
		LD_LIBRARY_PATH="$(LIBREGNUM_LIBDIR):$(LIBREGNUM_DIR)/deps/graylib/build/lib:$(LIBREGNUM_DIR)/deps/yaml-glib/build:$$LD_LIBRARY_PATH" $$test -v || exit 1; \
	done
	$(call print_status,"All tests passed")

# =============================================================================
# Directory Creation
# =============================================================================

$(TEST_BUILDDIR):
	@$(MKDIR_P) $@

# =============================================================================
# Clean
# =============================================================================

.PHONY: clean
clean:
	@$(RM) $(TEST_BINARIES)
	@$(RMDIR) $(TEST_BUILDDIR) 2>/dev/null || true
