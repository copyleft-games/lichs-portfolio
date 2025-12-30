/* lp-strings.h - Localization Helper for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpStrings wraps LrgLocalization to provide convenient string access.
 * It loads locale files from data/locales/ and provides helper functions
 * for string lookup, formatting, and pluralization.
 */

#ifndef LP_STRINGS_H
#define LP_STRINGS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STRINGS (lp_strings_get_type ())

G_DECLARE_FINAL_TYPE (LpStrings, lp_strings, LP, STRINGS, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_strings_get_default:
 *
 * Gets the default strings manager instance.
 * Initializes on first call, loading the English locale.
 *
 * Returns: (transfer none): The default #LpStrings
 */
LpStrings *
lp_strings_get_default (void);

/* ==========================================================================
 * Locale Management
 * ========================================================================== */

/**
 * lp_strings_load_locale:
 * @self: an #LpStrings
 * @code: locale code (e.g., "en", "de", "fr")
 * @error: return location for error, or %NULL
 *
 * Loads a locale from data/locales/{code}.yaml.
 * Does nothing if the locale is already loaded.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_strings_load_locale (LpStrings    *self,
                        const gchar  *code,
                        GError      **error);

/**
 * lp_strings_set_locale:
 * @self: an #LpStrings
 * @code: locale code to set as current
 *
 * Sets the current locale. Loads it if not already loaded.
 *
 * Returns: %TRUE if the locale was set successfully
 */
gboolean
lp_strings_set_locale (LpStrings   *self,
                       const gchar *code);

/**
 * lp_strings_get_locale:
 * @self: an #LpStrings
 *
 * Gets the current locale code.
 *
 * Returns: (transfer none): The current locale code
 */
const gchar *
lp_strings_get_locale (LpStrings *self);

/* ==========================================================================
 * String Lookup
 * ========================================================================== */

/**
 * lp_str:
 * @key: the string key to look up
 *
 * Convenience function to get a localized string from the default manager.
 * Returns the key itself if the string is not found.
 *
 * Returns: (transfer none): The localized string
 */
const gchar *
lp_str (const gchar *key);

/**
 * lp_strings_get:
 * @self: an #LpStrings
 * @key: the string key to look up
 *
 * Gets a localized string by key.
 * Returns the key itself if the string is not found.
 *
 * Returns: (transfer none): The localized string
 */
const gchar *
lp_strings_get (LpStrings   *self,
                const gchar *key);

/**
 * lp_str_format:
 * @key: the string key to look up
 * @...: printf-style format arguments
 *
 * Gets a localized string and formats it with arguments.
 * The string should contain printf-style format specifiers.
 *
 * Returns: (transfer full): A newly allocated formatted string
 */
gchar *
lp_str_format (const gchar *key,
               ...);

/**
 * lp_strings_format:
 * @self: an #LpStrings
 * @key: the string key to look up
 * @...: printf-style format arguments
 *
 * Gets a localized string and formats it with arguments.
 *
 * Returns: (transfer full): A newly allocated formatted string
 */
gchar *
lp_strings_format (LpStrings   *self,
                   const gchar *key,
                   ...);

/**
 * lp_str_plural:
 * @key: the base string key
 * @count: the count for pluralization
 *
 * Gets a pluralized string from the default manager.
 *
 * Returns: (transfer none): The pluralized string
 */
const gchar *
lp_str_plural (const gchar *key,
               gint         count);

/**
 * lp_strings_get_plural:
 * @self: an #LpStrings
 * @key: the base string key
 * @count: the count for pluralization
 *
 * Gets a pluralized string.
 * Falls back to "other" form if specific form not found.
 *
 * Returns: (transfer none): The pluralized string
 */
const gchar *
lp_strings_get_plural (LpStrings   *self,
                       const gchar *key,
                       gint         count);

/**
 * lp_strings_format_plural:
 * @self: an #LpStrings
 * @key: the base string key
 * @count: the count for pluralization
 * @...: printf-style format arguments
 *
 * Gets a pluralized string and formats it with arguments.
 *
 * Returns: (transfer full): A newly allocated formatted string
 */
gchar *
lp_strings_format_plural (LpStrings   *self,
                          const gchar *key,
                          gint         count,
                          ...);

/* ==========================================================================
 * String Key Constants
 * ========================================================================== */

/* UI Strings */
#define LP_STR_UI_PORTFOLIO         "ui.portfolio"
#define LP_STR_UI_AGENTS            "ui.agents"
#define LP_STR_UI_WORLD_MAP         "ui.world_map"
#define LP_STR_UI_INTELLIGENCE      "ui.intelligence"
#define LP_STR_UI_LEDGER            "ui.ledger"
#define LP_STR_UI_MEGAPROJECTS      "ui.megaprojects"
#define LP_STR_UI_SLUMBER           "ui.slumber"
#define LP_STR_UI_GOLD              "ui.gold"
#define LP_STR_UI_YEAR              "ui.year"
#define LP_STR_UI_NEW_GAME          "ui.new_game"
#define LP_STR_UI_CONTINUE          "ui.continue"
#define LP_STR_UI_SETTINGS          "ui.settings"
#define LP_STR_UI_QUIT              "ui.quit"
#define LP_STR_UI_BUY               "ui.buy"
#define LP_STR_UI_SELL              "ui.sell"
#define LP_STR_UI_CONFIRM           "ui.confirm"
#define LP_STR_UI_CANCEL            "ui.cancel"

/* Event Strings */
#define LP_STR_EVENT_ECONOMIC       "event.type.economic"
#define LP_STR_EVENT_POLITICAL      "event.type.political"
#define LP_STR_EVENT_MAGICAL        "event.type.magical"
#define LP_STR_EVENT_PERSONAL       "event.type.personal"

/* Malachar Strings */
#define LP_STR_MALACHAR_GREETING    "malachar.greeting"
#define LP_STR_MALACHAR_SLUMBER     "malachar.slumber"
#define LP_STR_MALACHAR_WAKE        "malachar.wake"

/* Plurals */
#define LP_STR_YEAR_PLURAL          "year"
#define LP_STR_GOLD_PLURAL          "gold"
#define LP_STR_AGENT_PLURAL         "agent"
#define LP_STR_INVESTMENT_PLURAL    "investment"

G_END_DECLS

#endif /* LP_STRINGS_H */
