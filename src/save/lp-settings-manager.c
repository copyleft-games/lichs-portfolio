/* lp-settings-manager.c - Game Settings Management
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include <sys/stat.h>
#include <errno.h>

#include "lp-settings-manager.h"

/* Settings file name */
#define SETTINGS_FILENAME "settings.yaml"

/* Default values */
#define DEFAULT_FULLSCREEN      FALSE
#define DEFAULT_VSYNC           TRUE
#define DEFAULT_WINDOW_WIDTH    1280
#define DEFAULT_WINDOW_HEIGHT   720
#define DEFAULT_MASTER_VOLUME   0.8f
#define DEFAULT_MUSIC_VOLUME    0.7f
#define DEFAULT_SFX_VOLUME      1.0f
#define DEFAULT_MUTED           FALSE
#define DEFAULT_AUTOSAVE        TRUE
#define DEFAULT_AUTOSAVE_MINS   5
#define DEFAULT_PAUSE_EVENTS    TRUE
#define DEFAULT_NOTIFICATIONS   TRUE
#define DEFAULT_UI_SCALE        1.0f

struct _LpSettingsManager
{
    GObject parent_instance;

    gchar *settings_path;

    /* Graphics settings */
    gboolean fullscreen;
    gboolean vsync;
    gint     window_width;
    gint     window_height;

    /* Audio settings */
    gfloat   master_volume;
    gfloat   music_volume;
    gfloat   sfx_volume;
    gboolean muted;

    /* Gameplay settings */
    gboolean autosave_enabled;
    guint    autosave_interval;
    gboolean pause_on_events;
    gboolean show_notifications;

    /* Accessibility settings */
    gfloat   ui_scale;
};

G_DEFINE_TYPE (LpSettingsManager, lp_settings_manager, G_TYPE_OBJECT)

static LpSettingsManager *default_manager = NULL;

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

/*
 * get_default_settings_path:
 *
 * Gets the default settings file path.
 */
static gchar *
get_default_settings_path (void)
{
    const gchar *config_dir;

    config_dir = g_get_user_config_dir ();
    return g_build_filename (config_dir, "lichs-portfolio", SETTINGS_FILENAME, NULL);
}

/*
 * ensure_config_directory:
 *
 * Ensures the config directory exists.
 */
static gboolean
ensure_config_directory (const gchar  *path,
                         GError      **error)
{
    g_autofree gchar *dir = NULL;

    dir = g_path_get_dirname (path);
    if (g_file_test (dir, G_FILE_TEST_IS_DIR))
        return TRUE;

    if (g_mkdir_with_parents (dir, 0755) != 0)
    {
        int saved_errno = errno;
        g_set_error (error, G_IO_ERROR, g_io_error_from_errno (saved_errno),
                     "Failed to create config directory: %s", g_strerror (saved_errno));
        return FALSE;
    }

    return TRUE;
}

/*
 * clamp_float:
 *
 * Clamps a float value to a range.
 */
static gfloat
clamp_float (gfloat value,
             gfloat min,
             gfloat max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_settings_manager_dispose (GObject *object)
{
    LpSettingsManager *self = LP_SETTINGS_MANAGER (object);

    g_clear_pointer (&self->settings_path, g_free);

    G_OBJECT_CLASS (lp_settings_manager_parent_class)->dispose (object);
}

static void
lp_settings_manager_class_init (LpSettingsManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = lp_settings_manager_dispose;
}

static void
lp_settings_manager_init (LpSettingsManager *self)
{
    self->settings_path = get_default_settings_path ();

    /* Initialize with defaults */
    self->fullscreen = DEFAULT_FULLSCREEN;
    self->vsync = DEFAULT_VSYNC;
    self->window_width = DEFAULT_WINDOW_WIDTH;
    self->window_height = DEFAULT_WINDOW_HEIGHT;

    self->master_volume = DEFAULT_MASTER_VOLUME;
    self->music_volume = DEFAULT_MUSIC_VOLUME;
    self->sfx_volume = DEFAULT_SFX_VOLUME;
    self->muted = DEFAULT_MUTED;

    self->autosave_enabled = DEFAULT_AUTOSAVE;
    self->autosave_interval = DEFAULT_AUTOSAVE_MINS;
    self->pause_on_events = DEFAULT_PAUSE_EVENTS;
    self->show_notifications = DEFAULT_NOTIFICATIONS;

    self->ui_scale = DEFAULT_UI_SCALE;
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_settings_manager_get_default:
 *
 * Gets the singleton settings manager instance.
 *
 * Returns: (transfer none): The default #LpSettingsManager
 */
LpSettingsManager *
lp_settings_manager_get_default (void)
{
    if (default_manager == NULL)
    {
        default_manager = g_object_new (LP_TYPE_SETTINGS_MANAGER, NULL);
        g_object_add_weak_pointer (G_OBJECT (default_manager),
                                   (gpointer *)&default_manager);
    }

    return default_manager;
}

/* ==========================================================================
 * Persistence
 * ========================================================================== */

/**
 * lp_settings_manager_load:
 * @self: an #LpSettingsManager
 * @error: (nullable): return location for error
 *
 * Loads settings from disk.
 *
 * Returns: %TRUE on success (or if no settings file exists)
 */
gboolean
lp_settings_manager_load (LpSettingsManager  *self,
                          GError            **error)
{
    g_autoptr(LrgSaveContext) context = NULL;

    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), FALSE);

    /* If file doesn't exist, use defaults - that's OK */
    if (!g_file_test (self->settings_path, G_FILE_TEST_EXISTS))
    {
        lp_log_debug ("No settings file found, using defaults");
        return TRUE;
    }

    lp_log_info ("Loading settings from: %s", self->settings_path);

    context = lrg_save_context_new_from_file (self->settings_path, error);
    if (context == NULL)
    {
        lp_log_warning ("Failed to load settings: %s",
                        error && *error ? (*error)->message : "unknown");
        return FALSE;
    }

    /* Graphics settings */
    if (lrg_save_context_enter_section (context, "graphics"))
    {
        self->fullscreen = lrg_save_context_read_boolean (context, "fullscreen",
                                                          DEFAULT_FULLSCREEN);
        self->vsync = lrg_save_context_read_boolean (context, "vsync",
                                                     DEFAULT_VSYNC);
        self->window_width = (gint)lrg_save_context_read_int (context, "window-width",
                                                               DEFAULT_WINDOW_WIDTH);
        self->window_height = (gint)lrg_save_context_read_int (context, "window-height",
                                                                DEFAULT_WINDOW_HEIGHT);
        lrg_save_context_leave_section (context);
    }

    /* Audio settings */
    if (lrg_save_context_enter_section (context, "audio"))
    {
        self->master_volume = (gfloat)lrg_save_context_read_double (context,
                                                                    "master-volume",
                                                                    DEFAULT_MASTER_VOLUME);
        self->music_volume = (gfloat)lrg_save_context_read_double (context,
                                                                   "music-volume",
                                                                   DEFAULT_MUSIC_VOLUME);
        self->sfx_volume = (gfloat)lrg_save_context_read_double (context,
                                                                 "sfx-volume",
                                                                 DEFAULT_SFX_VOLUME);
        self->muted = lrg_save_context_read_boolean (context, "muted", DEFAULT_MUTED);

        /* Clamp volumes to valid range */
        self->master_volume = clamp_float (self->master_volume, 0.0f, 1.0f);
        self->music_volume = clamp_float (self->music_volume, 0.0f, 1.0f);
        self->sfx_volume = clamp_float (self->sfx_volume, 0.0f, 1.0f);

        lrg_save_context_leave_section (context);
    }

    /* Gameplay settings */
    if (lrg_save_context_enter_section (context, "gameplay"))
    {
        self->autosave_enabled = lrg_save_context_read_boolean (context, "autosave",
                                                                DEFAULT_AUTOSAVE);
        self->autosave_interval = (guint)lrg_save_context_read_uint (context,
                                                                      "autosave-interval",
                                                                      DEFAULT_AUTOSAVE_MINS);
        self->pause_on_events = lrg_save_context_read_boolean (context, "pause-on-events",
                                                               DEFAULT_PAUSE_EVENTS);
        self->show_notifications = lrg_save_context_read_boolean (context,
                                                                  "show-notifications",
                                                                  DEFAULT_NOTIFICATIONS);
        lrg_save_context_leave_section (context);
    }

    /* Accessibility settings */
    if (lrg_save_context_enter_section (context, "accessibility"))
    {
        self->ui_scale = (gfloat)lrg_save_context_read_double (context, "ui-scale",
                                                               DEFAULT_UI_SCALE);
        self->ui_scale = clamp_float (self->ui_scale, 0.75f, 2.0f);
        lrg_save_context_leave_section (context);
    }

    lp_log_info ("Settings loaded successfully");
    return TRUE;
}

/**
 * lp_settings_manager_save:
 * @self: an #LpSettingsManager
 * @error: (nullable): return location for error
 *
 * Saves settings to disk.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_settings_manager_save (LpSettingsManager  *self,
                          GError            **error)
{
    g_autoptr(LrgSaveContext) context = NULL;

    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), FALSE);

    lp_log_info ("Saving settings to: %s", self->settings_path);

    /* Ensure directory exists */
    if (!ensure_config_directory (self->settings_path, error))
        return FALSE;

    context = lrg_save_context_new_for_save ();

    /* Graphics settings */
    lrg_save_context_begin_section (context, "graphics");
    lrg_save_context_write_boolean (context, "fullscreen", self->fullscreen);
    lrg_save_context_write_boolean (context, "vsync", self->vsync);
    lrg_save_context_write_int (context, "window-width", self->window_width);
    lrg_save_context_write_int (context, "window-height", self->window_height);
    lrg_save_context_end_section (context);

    /* Audio settings */
    lrg_save_context_begin_section (context, "audio");
    lrg_save_context_write_double (context, "master-volume", self->master_volume);
    lrg_save_context_write_double (context, "music-volume", self->music_volume);
    lrg_save_context_write_double (context, "sfx-volume", self->sfx_volume);
    lrg_save_context_write_boolean (context, "muted", self->muted);
    lrg_save_context_end_section (context);

    /* Gameplay settings */
    lrg_save_context_begin_section (context, "gameplay");
    lrg_save_context_write_boolean (context, "autosave", self->autosave_enabled);
    lrg_save_context_write_uint (context, "autosave-interval", self->autosave_interval);
    lrg_save_context_write_boolean (context, "pause-on-events", self->pause_on_events);
    lrg_save_context_write_boolean (context, "show-notifications", self->show_notifications);
    lrg_save_context_end_section (context);

    /* Accessibility settings */
    lrg_save_context_begin_section (context, "accessibility");
    lrg_save_context_write_double (context, "ui-scale", self->ui_scale);
    lrg_save_context_end_section (context);

    /* Write to file */
    if (!lrg_save_context_to_file (context, self->settings_path, error))
    {
        lp_log_warning ("Failed to write settings file: %s",
                        error && *error ? (*error)->message : "unknown");
        return FALSE;
    }

    lp_log_info ("Settings saved successfully");
    return TRUE;
}

/**
 * lp_settings_manager_reset_to_defaults:
 * @self: an #LpSettingsManager
 *
 * Resets all settings to their default values.
 */
void
lp_settings_manager_reset_to_defaults (LpSettingsManager *self)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));

    lp_log_info ("Resetting settings to defaults");

    self->fullscreen = DEFAULT_FULLSCREEN;
    self->vsync = DEFAULT_VSYNC;
    self->window_width = DEFAULT_WINDOW_WIDTH;
    self->window_height = DEFAULT_WINDOW_HEIGHT;

    self->master_volume = DEFAULT_MASTER_VOLUME;
    self->music_volume = DEFAULT_MUSIC_VOLUME;
    self->sfx_volume = DEFAULT_SFX_VOLUME;
    self->muted = DEFAULT_MUTED;

    self->autosave_enabled = DEFAULT_AUTOSAVE;
    self->autosave_interval = DEFAULT_AUTOSAVE_MINS;
    self->pause_on_events = DEFAULT_PAUSE_EVENTS;
    self->show_notifications = DEFAULT_NOTIFICATIONS;

    self->ui_scale = DEFAULT_UI_SCALE;
}

/* ==========================================================================
 * Graphics Settings
 * ========================================================================== */

gboolean
lp_settings_manager_get_fullscreen (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), FALSE);
    return self->fullscreen;
}

void
lp_settings_manager_set_fullscreen (LpSettingsManager *self,
                                    gboolean           fullscreen)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->fullscreen = fullscreen;
}

gboolean
lp_settings_manager_get_vsync (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), TRUE);
    return self->vsync;
}

void
lp_settings_manager_set_vsync (LpSettingsManager *self,
                               gboolean           vsync)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->vsync = vsync;
}

gint
lp_settings_manager_get_window_width (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), DEFAULT_WINDOW_WIDTH);
    return self->window_width;
}

gint
lp_settings_manager_get_window_height (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), DEFAULT_WINDOW_HEIGHT);
    return self->window_height;
}

void
lp_settings_manager_set_window_size (LpSettingsManager *self,
                                     gint               width,
                                     gint               height)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    g_return_if_fail (width > 0 && height > 0);

    self->window_width = width;
    self->window_height = height;
}

/* ==========================================================================
 * Audio Settings
 * ========================================================================== */

gfloat
lp_settings_manager_get_master_volume (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), DEFAULT_MASTER_VOLUME);
    return self->master_volume;
}

void
lp_settings_manager_set_master_volume (LpSettingsManager *self,
                                       gfloat             volume)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->master_volume = clamp_float (volume, 0.0f, 1.0f);
}

gfloat
lp_settings_manager_get_music_volume (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), DEFAULT_MUSIC_VOLUME);
    return self->music_volume;
}

void
lp_settings_manager_set_music_volume (LpSettingsManager *self,
                                      gfloat             volume)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->music_volume = clamp_float (volume, 0.0f, 1.0f);
}

gfloat
lp_settings_manager_get_sfx_volume (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), DEFAULT_SFX_VOLUME);
    return self->sfx_volume;
}

void
lp_settings_manager_set_sfx_volume (LpSettingsManager *self,
                                    gfloat             volume)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->sfx_volume = clamp_float (volume, 0.0f, 1.0f);
}

gboolean
lp_settings_manager_get_muted (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), FALSE);
    return self->muted;
}

void
lp_settings_manager_set_muted (LpSettingsManager *self,
                               gboolean           muted)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->muted = muted;
}

/* ==========================================================================
 * Gameplay Settings
 * ========================================================================== */

gboolean
lp_settings_manager_get_autosave_enabled (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), TRUE);
    return self->autosave_enabled;
}

void
lp_settings_manager_set_autosave_enabled (LpSettingsManager *self,
                                          gboolean           enabled)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->autosave_enabled = enabled;
}

guint
lp_settings_manager_get_autosave_interval (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), DEFAULT_AUTOSAVE_MINS);
    return self->autosave_interval;
}

void
lp_settings_manager_set_autosave_interval (LpSettingsManager *self,
                                           guint              minutes)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    g_return_if_fail (minutes > 0);
    self->autosave_interval = minutes;
}

gboolean
lp_settings_manager_get_pause_on_events (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), TRUE);
    return self->pause_on_events;
}

void
lp_settings_manager_set_pause_on_events (LpSettingsManager *self,
                                         gboolean           pause)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->pause_on_events = pause;
}

gboolean
lp_settings_manager_get_show_notifications (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), TRUE);
    return self->show_notifications;
}

void
lp_settings_manager_set_show_notifications (LpSettingsManager *self,
                                            gboolean           show)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->show_notifications = show;
}

/* ==========================================================================
 * Accessibility Settings
 * ========================================================================== */

gfloat
lp_settings_manager_get_ui_scale (LpSettingsManager *self)
{
    g_return_val_if_fail (LP_IS_SETTINGS_MANAGER (self), DEFAULT_UI_SCALE);
    return self->ui_scale;
}

void
lp_settings_manager_set_ui_scale (LpSettingsManager *self,
                                  gfloat             scale)
{
    g_return_if_fail (LP_IS_SETTINGS_MANAGER (self));
    self->ui_scale = clamp_float (scale, 0.75f, 2.0f);
}
