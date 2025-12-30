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

# Core (Phase 1+)
# SOURCES += core/lp-application.c
# SOURCES += core/lp-game-data.c

# Simulation (Phase 4+)
# SOURCES += simulation/lp-world-simulation.c
# SOURCES += simulation/lp-kingdom.c

# Investment (Phase 2+)
# SOURCES += investment/lp-investment.c
# SOURCES += investment/lp-portfolio.c

# Agent (Phase 3+)
# SOURCES += agent/lp-agent.c
# SOURCES += agent/lp-agent-manager.c

# UI (Phase 6+)
# SOURCES += ui/lp-theme.c

# States (Phase 1+)
# SOURCES += states/lp-state-main-menu.c

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

$(OBJDIR)/src/main.o: main.c lp-log.h lp-enums.h
$(OBJDIR)/src/lp-enums.o: lp-enums.c lp-enums.h

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
