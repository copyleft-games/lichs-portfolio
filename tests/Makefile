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

# Test linker flags
TEST_LDFLAGS := $(OPT_LDFLAGS)

# Test libraries
TEST_LIBS := $(GLIB_LIBS)

# =============================================================================
# Test Sources
# =============================================================================

# Phase 0: Infrastructure test
TEST_SOURCES := test-stub.c

# Phase 1+: Add more tests as features are implemented
# TEST_SOURCES += test-enums.c
# TEST_SOURCES += test-investment.c
# TEST_SOURCES += test-agent.c
# TEST_SOURCES += test-kingdom.c
# TEST_SOURCES += test-simulation.c
# TEST_SOURCES += test-save-load.c
# TEST_SOURCES += test-prestige.c
# TEST_SOURCES += test-portfolio.c
# TEST_SOURCES += test-achievement.c
# TEST_SOURCES += test-synergy.c
# TEST_SOURCES += test-exposure.c
# TEST_SOURCES += test-trait.c
# TEST_SOURCES += test-megaproject.c

# =============================================================================
# Test Binaries
# =============================================================================

TEST_BINARIES := $(TEST_SOURCES:%.c=$(TEST_BUILDDIR)/%)

# =============================================================================
# Build Rules
# =============================================================================

.PHONY: all
all: $(TEST_BINARIES)
	$(call print_status,"Tests built: $(words $(TEST_BINARIES)) test(s)")

# Pattern rule for test binaries
$(TEST_BUILDDIR)/test-%: test-%.c | $(TEST_BUILDDIR)
	$(call print_compile,$<)
	@$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS) $(TEST_LIBS)

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
		if $$test --tap; then \
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
		$$test -v || exit 1; \
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
