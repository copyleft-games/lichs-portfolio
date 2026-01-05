/* lp-save-manager.h - Save/Load Management
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Coordinates save/load operations for the game.
 *
 * This manager handles:
 * - Save file location and naming
 * - Creating save contexts and orchestrating saves
 * - Loading saves and restoring game state
 * - Autosave functionality
 * - Save slot management
 */

#ifndef LP_SAVE_MANAGER_H
#define LP_SAVE_MANAGER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SAVE_MANAGER (lp_save_manager_get_type ())

G_DECLARE_FINAL_TYPE (LpSaveManager, lp_save_manager, LP, SAVE_MANAGER, GObject)

/* Save format version - increment when save format changes */
#define LP_SAVE_VERSION (1)

/* Maximum number of save slots */
#define LP_MAX_SAVE_SLOTS (10)

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
LpSaveManager * lp_save_manager_get_default (void);

/* ==========================================================================
 * Save Operations
 * ========================================================================== */

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
gboolean lp_save_manager_save_game (LpSaveManager  *self,
                                    LpGameData     *game_data,
                                    guint           slot,
                                    GError        **error);

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
gboolean lp_save_manager_save_to_file (LpSaveManager  *self,
                                       LpGameData     *game_data,
                                       const gchar    *path,
                                       GError        **error);

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
gboolean lp_save_manager_quicksave (LpSaveManager  *self,
                                    LpGameData     *game_data,
                                    GError        **error);

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
gboolean lp_save_manager_autosave (LpSaveManager  *self,
                                   LpGameData     *game_data,
                                   GError        **error);

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
gboolean lp_save_manager_load_autosave (LpSaveManager  *self,
                                        LpGameData     *game_data,
                                        GError        **error);

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
gboolean lp_save_manager_load_game (LpSaveManager  *self,
                                    LpGameData     *game_data,
                                    guint           slot,
                                    GError        **error);

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
gboolean lp_save_manager_load_from_file (LpSaveManager  *self,
                                         LpGameData     *game_data,
                                         const gchar    *path,
                                         GError        **error);

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
gboolean lp_save_manager_quickload (LpSaveManager  *self,
                                    LpGameData     *game_data,
                                    GError        **error);

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
gboolean lp_save_manager_slot_exists (LpSaveManager *self,
                                      guint          slot);

/**
 * lp_save_manager_autosave_exists:
 * @self: an #LpSaveManager
 *
 * Checks if an autosave file exists.
 *
 * Returns: %TRUE if autosave exists
 */
gboolean lp_save_manager_autosave_exists (LpSaveManager *self);

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
gboolean lp_save_manager_get_slot_info (LpSaveManager *self,
                                        guint          slot,
                                        guint64       *year,
                                        guint64       *total_years,
                                        gint64        *timestamp);

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
gboolean lp_save_manager_delete_slot (LpSaveManager  *self,
                                      guint           slot,
                                      GError        **error);

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
const gchar * lp_save_manager_get_save_directory (LpSaveManager *self);

/**
 * lp_save_manager_get_slot_path:
 * @self: an #LpSaveManager
 * @slot: save slot number
 *
 * Gets the file path for a save slot.
 *
 * Returns: (transfer full): The save file path (caller owns)
 */
gchar * lp_save_manager_get_slot_path (LpSaveManager *self,
                                       guint          slot);

/**
 * lp_save_manager_ensure_directory:
 * @self: an #LpSaveManager
 * @error: (nullable): return location for error
 *
 * Ensures the save directory exists.
 *
 * Returns: %TRUE on success
 */
gboolean lp_save_manager_ensure_directory (LpSaveManager  *self,
                                           GError        **error);

G_END_DECLS

#endif /* LP_SAVE_MANAGER_H */
