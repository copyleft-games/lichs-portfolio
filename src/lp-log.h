/* lp-log.h - Logging Macros for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Logging macros for each game module. Use G_MESSAGES_DEBUG to filter output.
 *
 * Example:
 *   G_MESSAGES_DEBUG=LichsPortfolio-Core ./lichs-portfolio
 *   G_MESSAGES_DEBUG=all ./lichs-portfolio
 */

#ifndef LP_LOG_H
#define LP_LOG_H

#include <glib.h>

G_BEGIN_DECLS

/* ==========================================================================
 * Log Domain Definitions
 * ========================================================================== */

/* Main application domain */
#define LP_LOG_DOMAIN_APP           "LichsPortfolio"

/* Module-specific domains */
#define LP_LOG_DOMAIN_CORE          "LichsPortfolio-Core"
#define LP_LOG_DOMAIN_SIMULATION    "LichsPortfolio-Simulation"
#define LP_LOG_DOMAIN_INVESTMENT    "LichsPortfolio-Investment"
#define LP_LOG_DOMAIN_AGENT         "LichsPortfolio-Agent"
#define LP_LOG_DOMAIN_UI            "LichsPortfolio-UI"
#define LP_LOG_DOMAIN_STATE         "LichsPortfolio-State"
#define LP_LOG_DOMAIN_GAMESTATE     "LichsPortfolio-GameState"
#define LP_LOG_DOMAIN_FEEDBACK      "LichsPortfolio-Feedback"
#define LP_LOG_DOMAIN_ACHIEVEMENT   "LichsPortfolio-Achievement"
#define LP_LOG_DOMAIN_STEAM         "LichsPortfolio-Steam"
#define LP_LOG_DOMAIN_MCP           "LichsPortfolio-MCP"

/* ==========================================================================
 * Logging Macros
 * ========================================================================== */

/**
 * lp_debug:
 * @domain: Log domain (e.g., LP_LOG_DOMAIN_CORE)
 * @...: Format string and arguments
 *
 * Log a debug message. Only visible when G_MESSAGES_DEBUG includes the domain.
 */
#define lp_debug(domain, ...) \
    g_log (domain, G_LOG_LEVEL_DEBUG, __VA_ARGS__)

/**
 * lp_info:
 * @domain: Log domain (e.g., LP_LOG_DOMAIN_CORE)
 * @...: Format string and arguments
 *
 * Log an informational message.
 */
#define lp_info(domain, ...) \
    g_log (domain, G_LOG_LEVEL_INFO, __VA_ARGS__)

/**
 * lp_message:
 * @domain: Log domain (e.g., LP_LOG_DOMAIN_CORE)
 * @...: Format string and arguments
 *
 * Log a general message.
 */
#define lp_message(domain, ...) \
    g_log (domain, G_LOG_LEVEL_MESSAGE, __VA_ARGS__)

/**
 * lp_warning:
 * @domain: Log domain (e.g., LP_LOG_DOMAIN_CORE)
 * @...: Format string and arguments
 *
 * Log a warning message.
 */
#define lp_warning(domain, ...) \
    g_log (domain, G_LOG_LEVEL_WARNING, __VA_ARGS__)

/**
 * lp_error:
 * @domain: Log domain (e.g., LP_LOG_DOMAIN_CORE)
 * @...: Format string and arguments
 *
 * Log an error message (non-fatal).
 */
#define lp_error(domain, ...) \
    g_log (domain, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

/**
 * lp_critical:
 * @domain: Log domain (e.g., LP_LOG_DOMAIN_CORE)
 * @...: Format string and arguments
 *
 * Log a critical error message.
 */
#define lp_critical(domain, ...) \
    g_log (domain, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

/* ==========================================================================
 * Convenience Macros (use inside specific modules)
 * ========================================================================== */

/*
 * These macros are meant to be used when LP_LOG_DOMAIN is defined
 * at the top of a source file. Example:
 *
 *   #define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
 *   #include "lp-log.h"
 *
 *   lp_log_debug ("Initializing engine...");
 */

#ifdef LP_LOG_DOMAIN

#define lp_log_debug(...)    lp_debug (LP_LOG_DOMAIN, __VA_ARGS__)
#define lp_log_info(...)     lp_info (LP_LOG_DOMAIN, __VA_ARGS__)
#define lp_log_message(...)  lp_message (LP_LOG_DOMAIN, __VA_ARGS__)
#define lp_log_warning(...)  lp_warning (LP_LOG_DOMAIN, __VA_ARGS__)
#define lp_log_error(...)    lp_error (LP_LOG_DOMAIN, __VA_ARGS__)
#define lp_log_critical(...) lp_critical (LP_LOG_DOMAIN, __VA_ARGS__)

#endif /* LP_LOG_DOMAIN */

G_END_DECLS

#endif /* LP_LOG_H */
