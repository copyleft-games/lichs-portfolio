# src/Makefile - Lich's Portfolio Source Build
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds the game executable from source files.

include ../config.mk
include ../rules.mk

# =============================================================================
# Source Files
# =============================================================================

# Main entry point
SOURCES := main.c

# Enumerations
SOURCES += lp-enums.c

# Core (Phase 1)
SOURCES += core/lp-application.c
SOURCES += core/lp-game-data.c
SOURCES += core/lp-phylactery.c
SOURCES += core/lp-exposure-manager.c
SOURCES += core/lp-synergy-manager.c
SOURCES += core/lp-ledger.c

# Simulation (Phase 1 skeleton)
SOURCES += simulation/lp-world-simulation.c

# Investment (Phase 1 skeleton)
SOURCES += investment/lp-portfolio.c

# Agent (Phase 1 skeleton)
SOURCES += agent/lp-agent-manager.c

# Achievement (Phase 1 skeleton)
SOURCES += achievement/lp-achievement-manager.c

# States (Phase 1)
SOURCES += states/lp-state-main-menu.c
SOURCES += states/lp-state-wake.c
SOURCES += states/lp-state-analyze.c
SOURCES += states/lp-state-decide.c
SOURCES += states/lp-state-slumber.c
SOURCES += states/lp-state-simulating.c
SOURCES += states/lp-state-pause.c
SOURCES += states/lp-state-settings.c

# UI (Phase 6+)
# SOURCES += ui/lp-theme.c

# =============================================================================
# Object Files
# =============================================================================

OBJECTS := $(SOURCES:%.c=$(OBJDIR)/src/%.o)

# =============================================================================
# Output
# =============================================================================

TARGET := $(BINDIR_OUT)/$(PROGRAM_NAME)$(EXE_EXT)

# =============================================================================
# Build Rules
# =============================================================================

.PHONY: all
all: $(TARGET)
	$(call print_status,"Game built: $(TARGET)")

$(TARGET): $(OBJECTS) | $(BINDIR_OUT)
	$(call print_link,"$(PROGRAM_NAME)$(EXE_EXT)")
	@$(CC) $(GAME_LDFLAGS) -o $@ $(OBJECTS) $(GAME_LIBS)

# Pattern rule for object files
$(OBJDIR)/src/%.o: %.c | $(OBJDIR)
	@$(MKDIR_P) $(dir $@)
	$(call print_compile,$<)
	@$(CC) $(GAME_CFLAGS) -c -o $@ $<

# =============================================================================
# Dependencies (explicit rules for header tracking)
# =============================================================================

# Main entry point
$(OBJDIR)/src/main.o: main.c core/lp-application.h

# Enumerations
$(OBJDIR)/src/lp-enums.o: lp-enums.c lp-enums.h

# Core
$(OBJDIR)/src/core/lp-application.o: core/lp-application.c core/lp-application.h \
	core/lp-game-data.h achievement/lp-achievement-manager.h states/lp-state-main-menu.h \
	lp-log.h lp-types.h
$(OBJDIR)/src/core/lp-game-data.o: core/lp-game-data.c core/lp-game-data.h \
	investment/lp-portfolio.h agent/lp-agent-manager.h core/lp-phylactery.h \
	core/lp-ledger.h simulation/lp-world-simulation.h lp-log.h lp-types.h
$(OBJDIR)/src/core/lp-phylactery.o: core/lp-phylactery.c core/lp-phylactery.h \
	lp-log.h lp-types.h
$(OBJDIR)/src/core/lp-exposure-manager.o: core/lp-exposure-manager.c \
	core/lp-exposure-manager.h lp-log.h lp-enums.h lp-types.h
$(OBJDIR)/src/core/lp-synergy-manager.o: core/lp-synergy-manager.c \
	core/lp-synergy-manager.h lp-log.h lp-types.h
$(OBJDIR)/src/core/lp-ledger.o: core/lp-ledger.c core/lp-ledger.h \
	lp-log.h lp-enums.h lp-types.h

# Simulation
$(OBJDIR)/src/simulation/lp-world-simulation.o: simulation/lp-world-simulation.c \
	simulation/lp-world-simulation.h lp-log.h lp-types.h

# Investment
$(OBJDIR)/src/investment/lp-portfolio.o: investment/lp-portfolio.c \
	investment/lp-portfolio.h lp-log.h lp-types.h

# Agent
$(OBJDIR)/src/agent/lp-agent-manager.o: agent/lp-agent-manager.c \
	agent/lp-agent-manager.h lp-log.h lp-types.h

# Achievement
$(OBJDIR)/src/achievement/lp-achievement-manager.o: achievement/lp-achievement-manager.c \
	achievement/lp-achievement-manager.h lp-log.h lp-types.h

# States
$(OBJDIR)/src/states/lp-state-main-menu.o: states/lp-state-main-menu.c \
	states/lp-state-main-menu.h lp-log.h lp-types.h
$(OBJDIR)/src/states/lp-state-wake.o: states/lp-state-wake.c \
	states/lp-state-wake.h lp-log.h lp-types.h
$(OBJDIR)/src/states/lp-state-analyze.o: states/lp-state-analyze.c \
	states/lp-state-analyze.h lp-log.h lp-types.h
$(OBJDIR)/src/states/lp-state-decide.o: states/lp-state-decide.c \
	states/lp-state-decide.h lp-log.h lp-types.h
$(OBJDIR)/src/states/lp-state-slumber.o: states/lp-state-slumber.c \
	states/lp-state-slumber.h lp-log.h lp-types.h
$(OBJDIR)/src/states/lp-state-simulating.o: states/lp-state-simulating.c \
	states/lp-state-simulating.h lp-log.h lp-types.h
$(OBJDIR)/src/states/lp-state-pause.o: states/lp-state-pause.c \
	states/lp-state-pause.h lp-log.h lp-types.h
$(OBJDIR)/src/states/lp-state-settings.o: states/lp-state-settings.c \
	states/lp-state-settings.h lp-log.h lp-types.h

# =============================================================================
# Directory Creation
# =============================================================================

$(OBJDIR):
	@$(MKDIR_P) $@
	@$(MKDIR_P) $(OBJDIR)/src/core
	@$(MKDIR_P) $(OBJDIR)/src/simulation
	@$(MKDIR_P) $(OBJDIR)/src/investment
	@$(MKDIR_P) $(OBJDIR)/src/agent
	@$(MKDIR_P) $(OBJDIR)/src/ui
	@$(MKDIR_P) $(OBJDIR)/src/states
	@$(MKDIR_P) $(OBJDIR)/src/feedback
	@$(MKDIR_P) $(OBJDIR)/src/achievement
	@$(MKDIR_P) $(OBJDIR)/src/steam

$(BINDIR_OUT):
	@$(MKDIR_P) $@

# =============================================================================
# Clean
# =============================================================================

.PHONY: clean
clean:
	@$(RM) $(OBJECTS) $(TARGET)
