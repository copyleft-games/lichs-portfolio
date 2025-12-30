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

# Progression (Phase 5)
SOURCES += core/lp-prestige-manager.c
SOURCES += core/lp-megaproject.c

# Simulation (Phase 4)
SOURCES += simulation/lp-world-simulation.c
SOURCES += simulation/lp-region.c
SOURCES += simulation/lp-kingdom.c
SOURCES += simulation/lp-event.c
SOURCES += simulation/lp-event-economic.c
SOURCES += simulation/lp-event-political.c
SOURCES += simulation/lp-event-magical.c
SOURCES += simulation/lp-event-personal.c
SOURCES += simulation/lp-event-generator.c
SOURCES += simulation/lp-competitor.c

# Investment (Phase 2)
SOURCES += investment/lp-investment.c
SOURCES += investment/lp-investment-property.c
SOURCES += investment/lp-investment-trade.c
SOURCES += investment/lp-investment-financial.c
SOURCES += investment/lp-portfolio.c

# Agent (Phase 3)
SOURCES += agent/lp-agent.c
SOURCES += agent/lp-trait.c
SOURCES += agent/lp-agent-individual.c
SOURCES += agent/lp-agent-family.c
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

# UI (Phase 6)
SOURCES += ui/lp-theme.c
SOURCES += ui/lp-widget-exposure-meter.c
SOURCES += ui/lp-widget-synergy-indicator.c
SOURCES += ui/lp-screen-portfolio.c
SOURCES += ui/lp-screen-world-map.c
SOURCES += ui/lp-screen-agents.c
SOURCES += ui/lp-screen-intelligence.c
SOURCES += ui/lp-screen-slumber.c
SOURCES += ui/lp-screen-ledger.c
SOURCES += ui/lp-screen-megaprojects.c
SOURCES += ui/lp-dialog-event.c

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

# Progression (Phase 5)
$(OBJDIR)/src/core/lp-prestige-manager.o: core/lp-prestige-manager.c \
	core/lp-prestige-manager.h lp-log.h lp-enums.h lp-types.h
$(OBJDIR)/src/core/lp-megaproject.o: core/lp-megaproject.c \
	core/lp-megaproject.h lp-log.h lp-enums.h lp-types.h

# Simulation (Phase 4)
$(OBJDIR)/src/simulation/lp-world-simulation.o: simulation/lp-world-simulation.c \
	simulation/lp-world-simulation.h simulation/lp-kingdom.h simulation/lp-region.h \
	simulation/lp-event.h simulation/lp-event-generator.h simulation/lp-competitor.h \
	lp-log.h lp-types.h
$(OBJDIR)/src/simulation/lp-region.o: simulation/lp-region.c simulation/lp-region.h \
	lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-kingdom.o: simulation/lp-kingdom.c simulation/lp-kingdom.h \
	lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-event.o: simulation/lp-event.c simulation/lp-event.h \
	lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-event-economic.o: simulation/lp-event-economic.c \
	simulation/lp-event-economic.h simulation/lp-event.h lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-event-political.o: simulation/lp-event-political.c \
	simulation/lp-event-political.h simulation/lp-event.h simulation/lp-kingdom.h \
	lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-event-magical.o: simulation/lp-event-magical.c \
	simulation/lp-event-magical.h simulation/lp-event.h lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-event-personal.o: simulation/lp-event-personal.c \
	simulation/lp-event-personal.h simulation/lp-event.h lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-event-generator.o: simulation/lp-event-generator.c \
	simulation/lp-event-generator.h simulation/lp-event.h simulation/lp-event-economic.h \
	simulation/lp-event-political.h simulation/lp-event-magical.h simulation/lp-event-personal.h \
	simulation/lp-world-simulation.h lp-types.h lp-enums.h
$(OBJDIR)/src/simulation/lp-competitor.o: simulation/lp-competitor.c \
	simulation/lp-competitor.h simulation/lp-event.h simulation/lp-world-simulation.h \
	lp-log.h lp-types.h lp-enums.h

# Investment
$(OBJDIR)/src/investment/lp-investment.o: investment/lp-investment.c \
	investment/lp-investment.h lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/investment/lp-investment-property.o: investment/lp-investment-property.c \
	investment/lp-investment-property.h investment/lp-investment.h lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/investment/lp-investment-trade.o: investment/lp-investment-trade.c \
	investment/lp-investment-trade.h investment/lp-investment.h lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/investment/lp-investment-financial.o: investment/lp-investment-financial.c \
	investment/lp-investment-financial.h investment/lp-investment.h lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/investment/lp-portfolio.o: investment/lp-portfolio.c \
	investment/lp-portfolio.h investment/lp-investment.h investment/lp-investment-property.h \
	investment/lp-investment-trade.h investment/lp-investment-financial.h lp-log.h lp-types.h lp-enums.h

# Agent
$(OBJDIR)/src/agent/lp-agent.o: agent/lp-agent.c agent/lp-agent.h \
	agent/lp-trait.h lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/agent/lp-trait.o: agent/lp-trait.c agent/lp-trait.h \
	lp-log.h lp-types.h
$(OBJDIR)/src/agent/lp-agent-individual.o: agent/lp-agent-individual.c \
	agent/lp-agent-individual.h agent/lp-agent.h agent/lp-trait.h \
	lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/agent/lp-agent-family.o: agent/lp-agent-family.c \
	agent/lp-agent-family.h agent/lp-agent.h agent/lp-trait.h \
	lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/agent/lp-agent-manager.o: agent/lp-agent-manager.c \
	agent/lp-agent-manager.h agent/lp-agent.h agent/lp-agent-individual.h \
	agent/lp-agent-family.h agent/lp-trait.h lp-log.h lp-types.h lp-enums.h

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

# UI (Phase 6)
$(OBJDIR)/src/ui/lp-theme.o: ui/lp-theme.c ui/lp-theme.h lp-log.h
$(OBJDIR)/src/ui/lp-widget-exposure-meter.o: ui/lp-widget-exposure-meter.c \
	ui/lp-widget-exposure-meter.h ui/lp-theme.h core/lp-exposure-manager.h \
	lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/ui/lp-widget-synergy-indicator.o: ui/lp-widget-synergy-indicator.c \
	ui/lp-widget-synergy-indicator.h ui/lp-theme.h core/lp-synergy-manager.h \
	lp-log.h lp-types.h
$(OBJDIR)/src/ui/lp-screen-portfolio.o: ui/lp-screen-portfolio.c \
	ui/lp-screen-portfolio.h ui/lp-theme.h ui/lp-widget-exposure-meter.h \
	ui/lp-widget-synergy-indicator.h investment/lp-portfolio.h investment/lp-investment.h \
	lp-log.h lp-types.h lp-enums.h
$(OBJDIR)/src/ui/lp-screen-world-map.o: ui/lp-screen-world-map.c \
	ui/lp-screen-world-map.h ui/lp-theme.h simulation/lp-world-simulation.h \
	simulation/lp-kingdom.h simulation/lp-region.h lp-log.h lp-types.h
$(OBJDIR)/src/ui/lp-screen-agents.o: ui/lp-screen-agents.c \
	ui/lp-screen-agents.h ui/lp-theme.h agent/lp-agent-manager.h agent/lp-agent.h \
	lp-log.h lp-types.h
$(OBJDIR)/src/ui/lp-screen-intelligence.o: ui/lp-screen-intelligence.c \
	ui/lp-screen-intelligence.h ui/lp-theme.h lp-log.h lp-types.h
$(OBJDIR)/src/ui/lp-screen-slumber.o: ui/lp-screen-slumber.c \
	ui/lp-screen-slumber.h ui/lp-theme.h lp-log.h lp-types.h
$(OBJDIR)/src/ui/lp-screen-ledger.o: ui/lp-screen-ledger.c \
	ui/lp-screen-ledger.h ui/lp-theme.h core/lp-ledger.h lp-log.h lp-types.h
$(OBJDIR)/src/ui/lp-screen-megaprojects.o: ui/lp-screen-megaprojects.c \
	ui/lp-screen-megaprojects.h ui/lp-theme.h lp-log.h lp-types.h
$(OBJDIR)/src/ui/lp-dialog-event.o: ui/lp-dialog-event.c \
	ui/lp-dialog-event.h ui/lp-theme.h simulation/lp-event.h lp-log.h lp-types.h

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
