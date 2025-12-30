/* lp-strings.c - Localization Helper for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-strings.h"

struct _LpStrings
{
    GObject           parent_instance;

    LrgLocalization  *localization;
    gchar            *data_dir;
};

G_DEFINE_FINAL_TYPE (LpStrings, lp_strings, G_TYPE_OBJECT)

static LpStrings *default_strings = NULL;

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_strings_finalize (GObject *object)
{
    LpStrings *self = LP_STRINGS (object);

    g_clear_pointer (&self->data_dir, g_free);
    /* LrgLocalization is a singleton, don't unref */

    G_OBJECT_CLASS (lp_strings_parent_class)->finalize (object);
}

static void
lp_strings_class_init (LpStringsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_strings_finalize;
}

static void
lp_strings_init (LpStrings *self)
{
    self->localization = lrg_localization_get_default ();

    /* Determine data directory - check for local first, then installed */
    if (g_file_test ("data/locales", G_FILE_TEST_IS_DIR))
    {
        self->data_dir = g_strdup ("data");
    }
    else
    {
        /* Fallback to installed location */
        self->data_dir = g_build_filename (
            g_get_user_data_dir (), "lichs-portfolio", NULL);
    }
}

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
lp_strings_get_default (void)
{
    if (default_strings == NULL)
    {
        g_autoptr(GError) error = NULL;

        default_strings = g_object_new (LP_TYPE_STRINGS, NULL);

        /* Load English as default locale (optional - not fatal if missing) */
        if (!lp_strings_load_locale (default_strings, "en", &error))
        {
            lp_log_debug ("Locale file not found, strings will return keys: %s",
                          error->message);
        }
        else
        {
            lp_strings_set_locale (default_strings, "en");
        }
    }

    return default_strings;
}

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
                        GError      **error)
{
    g_autofree gchar *path = NULL;

    g_return_val_if_fail (LP_IS_STRINGS (self), FALSE);
    g_return_val_if_fail (code != NULL, FALSE);

    /* Check if already loaded */
    if (lrg_localization_has_locale (self->localization, code))
    {
        return TRUE;
    }

    /* Build path and load */
    path = g_build_filename (self->data_dir, "locales",
                             g_strdup_printf ("%s.yaml", code), NULL);

    lp_log_debug ("Loading locale from: %s", path);

    return lrg_localization_add_locale_from_file (self->localization,
                                                   path, error);
}

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
                       const gchar *code)
{
    g_autoptr(GError) error = NULL;

    g_return_val_if_fail (LP_IS_STRINGS (self), FALSE);
    g_return_val_if_fail (code != NULL, FALSE);

    /* Load if not already loaded */
    if (!lrg_localization_has_locale (self->localization, code))
    {
        if (!lp_strings_load_locale (self, code, &error))
        {
            lp_log_warning ("Failed to load locale '%s': %s",
                            code, error->message);
            return FALSE;
        }
    }

    /* Set as current */
    if (!lrg_localization_set_current (self->localization, code))
    {
        lp_log_warning ("Failed to set locale '%s' as current", code);
        return FALSE;
    }

    /* Also set English as fallback if setting non-English locale */
    if (g_strcmp0 (code, "en") != 0)
    {
        lrg_localization_set_fallback (self->localization, "en");
    }

    lp_log_info ("Locale set to: %s", code);
    return TRUE;
}

/**
 * lp_strings_get_locale:
 * @self: an #LpStrings
 *
 * Gets the current locale code.
 *
 * Returns: (transfer none): The current locale code
 */
const gchar *
lp_strings_get_locale (LpStrings *self)
{
    g_return_val_if_fail (LP_IS_STRINGS (self), "en");

    return lrg_localization_get_current_code (self->localization);
}

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
lp_str (const gchar *key)
{
    return lp_strings_get (lp_strings_get_default (), key);
}

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
                const gchar *key)
{
    const gchar *result;

    g_return_val_if_fail (LP_IS_STRINGS (self), key);
    g_return_val_if_fail (key != NULL, NULL);

    result = lrg_localization_get (self->localization, key);

    /* Return NULL if not found */
    return result;
}

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
               ...)
{
    LpStrings *strings;
    const gchar *format;
    gchar *result;
    va_list args;

    strings = lp_strings_get_default ();
    format = lp_strings_get (strings, key);

    va_start (args, key);
    result = g_strdup_vprintf (format, args);
    va_end (args);

    return result;
}

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
                   ...)
{
    const gchar *format;
    gchar *result;
    va_list args;

    g_return_val_if_fail (LP_IS_STRINGS (self), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    format = lp_strings_get (self, key);

    va_start (args, key);
    result = g_strdup_vprintf (format, args);
    va_end (args);

    return result;
}

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
               gint         count)
{
    return lp_strings_get_plural (lp_strings_get_default (), key, count);
}

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
                       gint         count)
{
    const gchar *result;

    g_return_val_if_fail (LP_IS_STRINGS (self), key);
    g_return_val_if_fail (key != NULL, NULL);

    result = lrg_localization_get_plural (self->localization, key, count);

    return result != NULL ? result : key;
}

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
                          ...)
{
    const gchar *format;
    gchar *result;
    va_list args;

    g_return_val_if_fail (LP_IS_STRINGS (self), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    format = lp_strings_get_plural (self, key, count);

    va_start (args, count);
    result = g_strdup_vprintf (format, args);
    va_end (args);

    return result;
}
