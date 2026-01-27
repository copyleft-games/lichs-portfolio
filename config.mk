# config.mk - Lich's Portfolio Build Configuration
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# This file contains all configurable build options for Lich's Portfolio.
# Override any option on the command line: make DEBUG=1

# =============================================================================
# Default Target
# =============================================================================

# Ensure 'all' is always the default target regardless of include order
.DEFAULT_GOAL := all

# =============================================================================
# Project Root Path
# =============================================================================

# Determine the project root based on where config.mk is located
# This ensures paths work correctly when make is run from subdirectories
CONFIG_MK_PATH := $(dir $(lastword $(MAKEFILE_LIST)))
PROJECT_ROOT := $(realpath $(CONFIG_MK_PATH))

# =============================================================================
# Version Information
# =============================================================================

VERSION_MAJOR := 0
VERSION_MINOR := 1
VERSION_MICRO := 0
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

# =============================================================================
# Program Names
# =============================================================================

PROGRAM_NAME := lichs-portfolio
PROGRAM_DISPLAY_NAME := Lich's Portfolio

# =============================================================================
# Installation Directories
# =============================================================================

PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)
BINDIR ?= $(EXEC_PREFIX)/bin
DATADIR ?= $(PREFIX)/share
DOCDIR ?= $(DATADIR)/doc/$(PROGRAM_NAME)

# =============================================================================
# Build Options
# =============================================================================

# Build unit tests
BUILD_TESTS ?= 1

# =============================================================================
# Dependency Paths
# =============================================================================

# Path to libregnum (submodule)
LIBREGNUM_DIR ?= $(PROJECT_ROOT)/deps/libregnum

# =============================================================================
# Debug Configuration
# =============================================================================

# Debug build mode:
#   0 = Release build (-O2, no debug symbols)
#   1 = Debug build (-g3 -O0, full debug info for gdb)
DEBUG ?= 0

# Enable AddressSanitizer (requires DEBUG=1)
ASAN ?= 0

# Enable UndefinedBehaviorSanitizer (requires DEBUG=1)
UBSAN ?= 0

# =============================================================================
# Optional Features
# =============================================================================

# Enable Steam SDK integration (requires deps/steamworks/)
STEAM ?= 0

# Steam App ID (use Spacewar test ID until real ID assigned)
STEAM_APPID ?= 480

# Enable MCP server for AI debugging (requires libregnum MCP support)
# Usage: make MCP=1
# Enables HTTP transport for Claude Code / Clawdbot / AI agent integration
# Default port: 5005 (configurable via MCP_HTTP_PORT env var)
MCP ?= 0

# MCP HTTP port (only used when MCP=1)
MCP_HTTP_PORT ?= 5005

# =============================================================================
# Compiler and Tools
# =============================================================================

CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
PKG_CONFIG ?= pkg-config
INSTALL ?= install
INSTALL_DATA ?= $(INSTALL) -m 644
INSTALL_PROGRAM ?= $(INSTALL) -m 755
SED ?= sed
MKDIR_P ?= mkdir -p
RM ?= rm -f
RMDIR ?= rm -rf

# =============================================================================
# Compiler Flags
# =============================================================================

# C standard (gnu89, NO -pedantic)
CSTD := gnu89

# Warning flags (ZERO tolerance - warnings are errors)
WARN_CFLAGS := -Wall -Wextra -Werror
WARN_CFLAGS += -Wformat=2 -Wformat-security
WARN_CFLAGS += -Wnull-dereference
WARN_CFLAGS += -Wstack-protector
WARN_CFLAGS += -Wstrict-prototypes
WARN_CFLAGS += -Wmissing-prototypes
WARN_CFLAGS += -Wold-style-definition
WARN_CFLAGS += -Wdeclaration-after-statement
WARN_CFLAGS += -Wno-unused-parameter
# Disable warning from GLib's g_once_init_enter macro (GLib internal issue)
WARN_CFLAGS += -Wno-discarded-qualifiers

# Feature test macros
FEATURE_CFLAGS := -D_GNU_SOURCE

# =============================================================================
# Platform Detection
# =============================================================================

# Detect host platform
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
endif
ifeq ($(UNAME_S),Darwin)
    PLATFORM := macos
endif

# Default target platform matches host
TARGET_PLATFORM ?= $(PLATFORM)

# =============================================================================
# Cross-Compilation Support
# =============================================================================

# Windows cross-compilation (mingw64)
ifeq ($(WINDOWS),1)
    TARGET_PLATFORM := windows
    CROSS := x86_64-w64-mingw32-
    CC := $(CROSS)gcc
    AR := $(CROSS)ar
    RANLIB := $(CROSS)ranlib
    PKG_CONFIG := $(CROSS)pkg-config
    EXE_EXT := .exe
else
    EXE_EXT :=
endif

# =============================================================================
# Build Directories
# =============================================================================

ifeq ($(DEBUG),1)
    BUILDDIR := $(PROJECT_ROOT)/build/debug
else
    BUILDDIR := $(PROJECT_ROOT)/build/release
endif

OBJDIR := $(BUILDDIR)/obj
BINDIR_OUT := $(BUILDDIR)

# =============================================================================
# Optimization Flags
# =============================================================================

ifeq ($(DEBUG),1)
    OPT_CFLAGS := -g3 -O0
    OPT_CFLAGS += -DLP_DEBUG=1
else
    OPT_CFLAGS := -O2
    OPT_CFLAGS += -DNDEBUG
endif

# Sanitizers (debug only)
ifeq ($(DEBUG),1)
    ifeq ($(ASAN),1)
        OPT_CFLAGS += -fsanitize=address -fno-omit-frame-pointer
        OPT_LDFLAGS += -fsanitize=address
    endif
    ifeq ($(UBSAN),1)
        OPT_CFLAGS += -fsanitize=undefined
        OPT_LDFLAGS += -fsanitize=undefined
    endif
endif

# =============================================================================
# Steam SDK Configuration
# =============================================================================

ifeq ($(STEAM),1)
    STEAM_SDK_DIR ?= $(PROJECT_ROOT)/deps/steamworks/sdk
    STEAM_CFLAGS := -DLP_STEAM=1 -I$(STEAM_SDK_DIR)/public
    ifeq ($(TARGET_PLATFORM),linux)
        STEAM_LIBS := -L$(STEAM_SDK_DIR)/redistributable_bin/linux64 -lsteam_api
    endif
    ifeq ($(TARGET_PLATFORM),windows)
        STEAM_LIBS := -L$(STEAM_SDK_DIR)/redistributable_bin/win64 -lsteam_api64
    endif
else
    STEAM_CFLAGS :=
    STEAM_LIBS :=
endif

# =============================================================================
# MCP Server Configuration (for AI debugging)
# =============================================================================

ifeq ($(MCP),1)
    # MCP requires libregnum to be built with MCP=1
    MCP_GLIB_DIR := $(LIBREGNUM_DIR)/deps/mcp-glib
    MCP_CFLAGS := -DLP_ENABLE_MCP=1 -DMCP_HTTP_PORT=$(MCP_HTTP_PORT)
    MCP_CFLAGS += -I$(MCP_GLIB_DIR)/src
    MCP_LIBS := -L$(MCP_GLIB_DIR)/build -lmcp-glib-1.0
    # Also need libsoup for HTTP transport
    MCP_SOUP_CFLAGS := $(shell $(PKG_CONFIG) --cflags libsoup-3.0 2>/dev/null)
    MCP_SOUP_LIBS := $(shell $(PKG_CONFIG) --libs libsoup-3.0 2>/dev/null)
    MCP_CFLAGS += $(MCP_SOUP_CFLAGS)
    MCP_LIBS += $(MCP_SOUP_LIBS)
else
    MCP_CFLAGS :=
    MCP_LIBS :=
endif

# =============================================================================
# pkg-config Dependencies
# =============================================================================

# Required dependencies
GLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags glib-2.0 gobject-2.0 gio-2.0)
GLIB_LIBS := $(shell $(PKG_CONFIG) --libs glib-2.0 gobject-2.0 gio-2.0)

JSON_CFLAGS := $(shell $(PKG_CONFIG) --cflags json-glib-1.0)
JSON_LIBS := $(shell $(PKG_CONFIG) --libs json-glib-1.0)

YAML_CFLAGS := $(shell $(PKG_CONFIG) --cflags yaml-0.1)
YAML_LIBS := $(shell $(PKG_CONFIG) --libs yaml-0.1)

# libdex (async/futures) - optional on Windows
ifneq ($(TARGET_PLATFORM),windows)
    DEX_CFLAGS := $(shell $(PKG_CONFIG) --cflags libdex-1 2>/dev/null)
    DEX_LIBS := $(shell $(PKG_CONFIG) --libs libdex-1 2>/dev/null)
    ifneq ($(DEX_CFLAGS),)
        HAS_LIBDEX := 1
    endif
endif

# =============================================================================
# libregnum Integration
# =============================================================================

# Include paths for libregnum
LIBREGNUM_CFLAGS := -I$(LIBREGNUM_DIR)/src
LIBREGNUM_CFLAGS += -I$(LIBREGNUM_DIR)/deps/graylib/src
LIBREGNUM_CFLAGS += -I$(LIBREGNUM_DIR)/deps/yaml-glib/src

# Library path (use same debug/release as game)
ifeq ($(DEBUG),1)
    LIBREGNUM_LIBDIR := $(LIBREGNUM_DIR)/build/debug/lib
else
    LIBREGNUM_LIBDIR := $(LIBREGNUM_DIR)/build/release/lib
endif

LIBREGNUM_LDFLAGS := -L$(LIBREGNUM_LIBDIR)
LIBREGNUM_LDFLAGS += -L$(LIBREGNUM_DIR)/deps/graylib/build/lib
LIBREGNUM_LDFLAGS += -L$(LIBREGNUM_DIR)/deps/yaml-glib/build
LIBREGNUM_LIBS := -llibregnum -lgraylib -lyaml-glib

# Platform-specific libraries
ifeq ($(TARGET_PLATFORM),linux)
    PLATFORM_LIBS := -lm -lpthread -lGL -lX11
endif
ifeq ($(TARGET_PLATFORM),windows)
    PLATFORM_LIBS := -lm -lopengl32 -lgdi32 -lwinmm -lws2_32
endif

# =============================================================================
# Combined Flags
# =============================================================================

# Base compiler flags
BASE_CFLAGS := -std=$(CSTD) $(WARN_CFLAGS) $(FEATURE_CFLAGS)

# All compiler flags for game
GAME_CFLAGS := $(BASE_CFLAGS) $(OPT_CFLAGS)
GAME_CFLAGS += -I$(PROJECT_ROOT)/src
GAME_CFLAGS += $(LIBREGNUM_CFLAGS)
GAME_CFLAGS += $(GLIB_CFLAGS) $(JSON_CFLAGS) $(YAML_CFLAGS)
GAME_CFLAGS += $(DEX_CFLAGS)
GAME_CFLAGS += $(STEAM_CFLAGS)
GAME_CFLAGS += $(MCP_CFLAGS)

# All linker flags for game
GAME_LDFLAGS := $(OPT_LDFLAGS) $(LIBREGNUM_LDFLAGS)

# All libraries for game
GAME_LIBS := $(LIBREGNUM_LIBS)
GAME_LIBS += $(GLIB_LIBS) $(JSON_LIBS) $(YAML_LIBS)
GAME_LIBS += $(DEX_LIBS)
GAME_LIBS += $(STEAM_LIBS)
GAME_LIBS += $(MCP_LIBS)
GAME_LIBS += $(PLATFORM_LIBS)

# RPATH for development (find libregnum at runtime)
ifeq ($(TARGET_PLATFORM),linux)
    GAME_LDFLAGS += -Wl,-rpath,$(LIBREGNUM_LIBDIR)
    GAME_LDFLAGS += -Wl,-rpath,$(LIBREGNUM_DIR)/deps/graylib/build/lib
    GAME_LDFLAGS += -Wl,-rpath,$(LIBREGNUM_DIR)/deps/yaml-glib/build
    ifeq ($(MCP),1)
        GAME_LDFLAGS += -Wl,-rpath,$(MCP_GLIB_DIR)/build
    endif
endif
