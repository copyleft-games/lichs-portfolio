# Makefile - Lich's Portfolio Root Build Orchestration
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Root Makefile for building Lich's Portfolio.
# This coordinates building dependencies (libregnum) and the game itself.

include config.mk
include rules.mk

# =============================================================================
# Default Target
# =============================================================================

.PHONY: all
all: deps game
	$(call print_status,"Build complete: $(BINDIR_OUT)/$(PROGRAM_NAME)$(EXE_EXT)")

# =============================================================================
# Dependencies
# =============================================================================

.PHONY: deps
deps:
	$(call print_status,"Building libregnum...")
	@$(MAKE) --no-print-directory -C $(LIBREGNUM_DIR) DEBUG=$(DEBUG) MCP=$(MCP)

# =============================================================================
# Game
# =============================================================================

.PHONY: game
game: | $(BUILDDIR)
	$(call print_status,"Building $(PROGRAM_DISPLAY_NAME)...")
	@$(MAKE) --no-print-directory -C src MCP=$(MCP)

# =============================================================================
# Testing
# =============================================================================

.PHONY: test
test: game
	$(call print_status,"Running tests...")
	@$(MAKE) --no-print-directory -C tests run

.PHONY: test-build
test-build: game
	@$(MAKE) --no-print-directory -C tests

# =============================================================================
# Clean
# =============================================================================

.PHONY: clean
clean:
	$(call print_status,"Cleaning build artifacts...")
	@$(RMDIR) $(PROJECT_ROOT)/build
	@$(MAKE) --no-print-directory -C tests clean 2>/dev/null || true

.PHONY: distclean
distclean: clean
	$(call print_status,"Cleaning dependencies...")
	@$(MAKE) --no-print-directory -C $(LIBREGNUM_DIR) distclean

# =============================================================================
# Installation
# =============================================================================

.PHONY: install
install: all
	$(call print_status,"Installing to $(PREFIX)...")
	@$(MKDIR_P) $(DESTDIR)$(BINDIR)
	@$(INSTALL_PROGRAM) $(BINDIR_OUT)/$(PROGRAM_NAME)$(EXE_EXT) $(DESTDIR)$(BINDIR)/
	@$(MKDIR_P) $(DESTDIR)$(DATADIR)/$(PROGRAM_NAME)
	@cp -r data/* $(DESTDIR)$(DATADIR)/$(PROGRAM_NAME)/ 2>/dev/null || true
	@cp -r assets/* $(DESTDIR)$(DATADIR)/$(PROGRAM_NAME)/ 2>/dev/null || true
	$(call print_status,"Installation complete")

.PHONY: uninstall
uninstall:
	$(call print_status,"Uninstalling from $(PREFIX)...")
	@$(RM) $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME)$(EXE_EXT)
	@$(RMDIR) $(DESTDIR)$(DATADIR)/$(PROGRAM_NAME)
	$(call print_status,"Uninstallation complete")

# =============================================================================
# Run Target (convenience)
# =============================================================================

.PHONY: run
run: all
	@$(BINDIR_OUT)/$(PROGRAM_NAME)$(EXE_EXT)

.PHONY: debug-run
debug-run: all
	@G_MESSAGES_DEBUG=all $(BINDIR_OUT)/$(PROGRAM_NAME)$(EXE_EXT)

# =============================================================================
# Directory Creation
# =============================================================================

$(BUILDDIR):
	@$(MKDIR_P) $@
