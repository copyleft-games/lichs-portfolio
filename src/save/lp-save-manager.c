/* lp-save-manager.c - Save/Load Management
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include <sys/stat.h>
#include <errno.h>

#include "lp-save-manager.h"
#include "../core/lp-game-data.h"

/* Save file naming */
#define SAVE_FILE_PREFIX    "save"
#define SAVE_FILE_EXTENSION ".yaml"
#define AUTOSAVE_FILENAME   "autosave.yaml"
#define QUICKSAVE_SLOT      (0)

struct _LpSaveManager
{
    GObject parent_instance;

    gchar *save_directory;
};

G_DEFINE_TYPE (LpSaveManager, lp_save_manager, G_TYPE_OBJECT)

static LpSaveManager *default_manager = NULL;

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

/*
 * get_default_save_directory:
 *
 * Gets the default save directory path based on platform conventions.
 */
static gchar *
get_default_save_directory (void)
{
    const gchar *data_dir;

    data_dir = g_get_user_data_dir ();
    return g_build_filename (data_dir, "lichs-portfolio", "saves", NULL);
}

/*
 * build_slot_filename:
 * @slot: save slot number
 *
 * Builds the filename for a save slot.
 *
 * Returns: (transfer full): The filename string
 */
static gchar *
build_slot_filename (guint slot)
{
    return g_strdup_printf ("%s%u%s", SAVE_FILE_PREFIX, slot, SAVE_FILE_EXTENSION);
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_save_manager_dispose (GObject *object)
{
    LpSaveManager *self = LP_SAVE_MANAGER (object);

    g_clear_pointer (&self->save_directory, g_free);

    G_OBJECT_CLASS (lp_save_manager_parent_class)->dispose (object);
}

static void
lp_save_manager_class_init (LpSaveManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = lp_save_manager_dispose;
}

static void
lp_save_manager_init (LpSaveManager *self)
{
    self->save_directory = get_default_save_directory ();
}

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_save_manager_get_default:
 *
 * Gets the singleton save manager instance.
 *
 * Returns: (transfer none): The default #LpSaveManager
 */
LpSaveManager *
lp_save_manager_get_default (void)
{
    if (default_manager == NULL)
    {
        default_manager = g_object_new (LP_TYPE_SAVE_MANAGER, NULL);
        g_object_add_weak_pointer (G_OBJECT (default_manager),
                                   (gpointer *)&default_manager);
    }

    return default_manager;
}

/* ==========================================================================
 * Save Operations
 * ========================================================================== */

/**
 * lp_save_manager_save_to_file:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to save
 * @path: file path to save to
 * @error: (nullable): return location for error
 *
 * Saves the game to a specific file path.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_save_to_file (LpSaveManager  *self,
                              LpGameData     *game_data,
                              const gchar    *path,
                              GError        **error)
{
    g_autoptr(LrgSaveContext) context = NULL;
    gint64 timestamp;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);

    lp_log_info ("Saving game to: %s", path);

    /* Ensure save directory exists */
    if (!lp_save_manager_ensure_directory (self, error))
        return FALSE;

    /* Create save context */
    context = lrg_save_context_new_for_save ();
    lrg_save_context_set_version (context, LP_SAVE_VERSION);

    /* Write header metadata */
    timestamp = g_get_real_time () / G_USEC_PER_SEC;
    lrg_save_context_write_int (context, "save-timestamp", timestamp);
    lrg_save_context_write_uint (context, "save-version", LP_SAVE_VERSION);

    /* Save game data in its own section */
    lrg_save_context_begin_section (context,
                                    lrg_saveable_get_save_id (LRG_SAVEABLE (game_data)));
    if (!lrg_saveable_save (LRG_SAVEABLE (game_data), context, error))
    {
        lp_log_warning ("Failed to save game data: %s",
                        error && *error ? (*error)->message : "unknown error");
        return FALSE;
    }
    lrg_save_context_end_section (context);

    /* Write to file */
    if (!lrg_save_context_to_file (context, path, error))
    {
        lp_log_warning ("Failed to write save file: %s",
                        error && *error ? (*error)->message : "unknown error");
        return FALSE;
    }

    lp_log_info ("Game saved successfully");
    return TRUE;
}

/**
 * lp_save_manager_save_game:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to save
 * @slot: save slot number (0-9)
 * @error: (nullable): return location for error
 *
 * Saves the game to the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_save_game (LpSaveManager  *self,
                           LpGameData     *game_data,
                           guint           slot,
                           GError        **error)
{
    g_autofree gchar *path = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);
    g_return_val_if_fail (slot < LP_MAX_SAVE_SLOTS, FALSE);

    path = lp_save_manager_get_slot_path (self, slot);
    return lp_save_manager_save_to_file (self, game_data, path, error);
}

/**
 * lp_save_manager_quicksave:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to save
 * @error: (nullable): return location for error
 *
 * Performs a quicksave (slot 0).
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_quicksave (LpSaveManager  *self,
                           LpGameData     *game_data,
                           GError        **error)
{
    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);

    lp_log_info ("Quicksave");
    return lp_save_manager_save_game (self, game_data, QUICKSAVE_SLOT, error);
}

/**
 * lp_save_manager_autosave:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to save
 * @error: (nullable): return location for error
 *
 * Performs an autosave.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_autosave (LpSaveManager  *self,
                          LpGameData     *game_data,
                          GError        **error)
{
    g_autofree gchar *path = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);

    lp_log_info ("Autosave");
    path = g_build_filename (self->save_directory, AUTOSAVE_FILENAME, NULL);
    return lp_save_manager_save_to_file (self, game_data, path, error);
}

/* ==========================================================================
 * Load Operations
 * ========================================================================== */

/**
 * lp_save_manager_load_autosave:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to load into
 * @error: (nullable): return location for error
 *
 * Loads the autosave file.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_load_autosave (LpSaveManager  *self,
                               LpGameData     *game_data,
                               GError        **error)
{
    g_autofree gchar *path = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);

    lp_log_info ("Loading autosave");
    path = g_build_filename (self->save_directory, AUTOSAVE_FILENAME, NULL);
    return lp_save_manager_load_from_file (self, game_data, path, error);
}

/**
 * lp_save_manager_load_from_file:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to load into
 * @path: file path to load from
 * @error: (nullable): return location for error
 *
 * Loads a game from a specific file path.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_load_from_file (LpSaveManager  *self,
                                LpGameData     *game_data,
                                const gchar    *path,
                                GError        **error)
{
    g_autoptr(LrgSaveContext) context = NULL;
    guint save_version;
    const gchar *save_id;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);

    lp_log_info ("Loading game from: %s", path);

    /* Load save file */
    context = lrg_save_context_new_from_file (path, error);
    if (context == NULL)
    {
        lp_log_warning ("Failed to load save file: %s",
                        error && *error ? (*error)->message : "unknown error");
        return FALSE;
    }

    /* Check save version */
    save_version = (guint)lrg_save_context_read_uint (context, "save-version", 0);
    if (save_version > LP_SAVE_VERSION)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                     "Save file version %u is newer than supported version %u",
                     save_version, LP_SAVE_VERSION);
        lp_log_warning ("Unsupported save version: %u (max: %u)",
                        save_version, LP_SAVE_VERSION);
        return FALSE;
    }

    if (save_version < LP_SAVE_VERSION)
    {
        lp_log_info ("Loading older save format (version %u, current %u)",
                     save_version, LP_SAVE_VERSION);
        /* Future: migration logic would go here */
    }

    /* Load game data from its section */
    save_id = lrg_saveable_get_save_id (LRG_SAVEABLE (game_data));
    if (!lrg_save_context_enter_section (context, save_id))
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                     "Save file missing '%s' section", save_id);
        lp_log_warning ("Save file missing game-data section");
        return FALSE;
    }

    if (!lrg_saveable_load (LRG_SAVEABLE (game_data), context, error))
    {
        lp_log_warning ("Failed to load game data: %s",
                        error && *error ? (*error)->message : "unknown error");
        lrg_save_context_leave_section (context);
        return FALSE;
    }

    lrg_save_context_leave_section (context);

    lp_log_info ("Game loaded successfully");
    return TRUE;
}

/**
 * lp_save_manager_load_game:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to load into
 * @slot: save slot number (0-9)
 * @error: (nullable): return location for error
 *
 * Loads a game from the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_load_game (LpSaveManager  *self,
                           LpGameData     *game_data,
                           guint           slot,
                           GError        **error)
{
    g_autofree gchar *path = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);
    g_return_val_if_fail (slot < LP_MAX_SAVE_SLOTS, FALSE);

    path = lp_save_manager_get_slot_path (self, slot);

    if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                     "Save slot %u does not exist", slot);
        return FALSE;
    }

    return lp_save_manager_load_from_file (self, game_data, path, error);
}

/**
 * lp_save_manager_quickload:
 * @self: an #LpSaveManager
 * @game_data: the #LpGameData to load into
 * @error: (nullable): return location for error
 *
 * Performs a quickload (slot 0).
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_save_manager_quickload (LpSaveManager  *self,
                           LpGameData     *game_data,
                           GError        **error)
{
    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (LP_IS_GAME_DATA (game_data), FALSE);

    lp_log_info ("Quickload");
    return lp_save_manager_load_game (self, game_data, QUICKSAVE_SLOT, error);
}

/* ==========================================================================
 * Save Slot Information
 * ========================================================================== */

/**
 * lp_save_manager_slot_exists:
 * @self: an #LpSaveManager
 * @slot: save slot number (0-9)
 *
 * Checks if a save exists in the given slot.
 *
 * Returns: %TRUE if save exists
 */
gboolean
lp_save_manager_slot_exists (LpSaveManager *self,
                             guint          slot)
{
    g_autofree gchar *path = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (slot < LP_MAX_SAVE_SLOTS, FALSE);

    path = lp_save_manager_get_slot_path (self, slot);
    return g_file_test (path, G_FILE_TEST_EXISTS);
}

/**
 * lp_save_manager_autosave_exists:
 * @self: an #LpSaveManager
 *
 * Checks if an autosave file exists.
 *
 * Returns: %TRUE if autosave exists
 */
gboolean
lp_save_manager_autosave_exists (LpSaveManager *self)
{
    g_autofree gchar *path = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);

    path = g_build_filename (self->save_directory, AUTOSAVE_FILENAME, NULL);
    return g_file_test (path, G_FILE_TEST_EXISTS);
}

/**
 * lp_save_manager_get_slot_info:
 * @self: an #LpSaveManager
 * @slot: save slot number (0-9)
 * @year: (out) (optional): return location for current year
 * @total_years: (out) (optional): return location for total years played
 * @timestamp: (out) (optional): return location for save timestamp
 *
 * Gets information about a save slot.
 *
 * Returns: %TRUE if slot exists and info was retrieved
 */
gboolean
lp_save_manager_get_slot_info (LpSaveManager *self,
                               guint          slot,
                               guint64       *year,
                               guint64       *total_years,
                               gint64        *timestamp)
{
    g_autofree gchar *path = NULL;
    g_autoptr(LrgSaveContext) context = NULL;
    g_autoptr(GError) error = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (slot < LP_MAX_SAVE_SLOTS, FALSE);

    path = lp_save_manager_get_slot_path (self, slot);

    if (!g_file_test (path, G_FILE_TEST_EXISTS))
        return FALSE;

    context = lrg_save_context_new_from_file (path, &error);
    if (context == NULL)
    {
        lp_log_debug ("Failed to read slot %u info: %s", slot, error->message);
        return FALSE;
    }

    /* Read header metadata */
    if (timestamp != NULL)
        *timestamp = lrg_save_context_read_int (context, "save-timestamp", 0);

    /* Read game data section for year info */
    if (lrg_save_context_enter_section (context, "game-data"))
    {
        if (total_years != NULL)
            *total_years = lrg_save_context_read_uint (context, "total-years-played", 0);

        /* Year is in world-simulation subsection */
        if (year != NULL)
        {
            *year = 0;
            if (lrg_save_context_enter_section (context, "world-simulation"))
            {
                *year = lrg_save_context_read_uint (context, "current-year", 0);
                lrg_save_context_leave_section (context);
            }
        }

        lrg_save_context_leave_section (context);
    }

    return TRUE;
}

/**
 * lp_save_manager_delete_slot:
 * @self: an #LpSaveManager
 * @slot: save slot number (0-9)
 * @error: (nullable): return location for error
 *
 * Deletes a save slot.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_save_manager_delete_slot (LpSaveManager  *self,
                             guint           slot,
                             GError        **error)
{
    g_autofree gchar *path = NULL;
    g_autoptr(GFile) file = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);
    g_return_val_if_fail (slot < LP_MAX_SAVE_SLOTS, FALSE);

    path = lp_save_manager_get_slot_path (self, slot);

    if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
        /* Already doesn't exist, that's fine */
        return TRUE;
    }

    file = g_file_new_for_path (path);
    if (!g_file_delete (file, NULL, error))
    {
        lp_log_warning ("Failed to delete slot %u: %s", slot,
                        error && *error ? (*error)->message : "unknown");
        return FALSE;
    }

    lp_log_info ("Deleted save slot %u", slot);
    return TRUE;
}

/* ==========================================================================
 * Path Management
 * ========================================================================== */

/**
 * lp_save_manager_get_save_directory:
 * @self: an #LpSaveManager
 *
 * Gets the save directory path.
 *
 * Returns: (transfer none): The save directory path
 */
const gchar *
lp_save_manager_get_save_directory (LpSaveManager *self)
{
    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), NULL);

    return self->save_directory;
}

/**
 * lp_save_manager_get_slot_path:
 * @self: an #LpSaveManager
 * @slot: save slot number
 *
 * Gets the file path for a save slot.
 *
 * Returns: (transfer full): The save file path (caller owns)
 */
gchar *
lp_save_manager_get_slot_path (LpSaveManager *self,
                               guint          slot)
{
    g_autofree gchar *filename = NULL;

    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), NULL);

    filename = build_slot_filename (slot);
    return g_build_filename (self->save_directory, filename, NULL);
}

/**
 * lp_save_manager_ensure_directory:
 * @self: an #LpSaveManager
 * @error: (nullable): return location for error
 *
 * Ensures the save directory exists.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_save_manager_ensure_directory (LpSaveManager  *self,
                                  GError        **error)
{
    g_return_val_if_fail (LP_IS_SAVE_MANAGER (self), FALSE);

    if (g_file_test (self->save_directory, G_FILE_TEST_IS_DIR))
        return TRUE;

    if (g_mkdir_with_parents (self->save_directory, 0755) != 0)
    {
        int saved_errno = errno;
        g_set_error (error, G_IO_ERROR, g_io_error_from_errno (saved_errno),
                     "Failed to create save directory '%s': %s",
                     self->save_directory, g_strerror (saved_errno));
        return FALSE;
    }

    lp_log_debug ("Created save directory: %s", self->save_directory);
    return TRUE;
}
