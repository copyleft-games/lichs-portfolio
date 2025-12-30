/* lp-steam-bridge.h - Steam Integration Bridge
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Optional Steam integration for achievement sync and cloud saves.
 * All methods are no-ops when Steam is unavailable (non-STEAM builds
 * or when Steam client is not running).
 *
 * Build with STEAM=1 to enable Steam SDK integration.
 */

#ifndef LP_STEAM_BRIDGE_H
#define LP_STEAM_BRIDGE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STEAM_BRIDGE (lp_steam_bridge_get_type ())

G_DECLARE_FINAL_TYPE (LpSteamBridge, lp_steam_bridge, LP, STEAM_BRIDGE, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_steam_bridge_get_default:
 *
 * Gets the default Steam bridge instance.
 * Creates it if it doesn't exist.
 *
 * Returns: (transfer none): The default #LpSteamBridge instance
 */
LpSteamBridge *
lp_steam_bridge_get_default (void);

/* ==========================================================================
 * Initialization
 * ========================================================================== */

/**
 * lp_steam_bridge_is_available:
 * @self: an #LpSteamBridge
 *
 * Checks if Steam integration is available.
 *
 * Returns %TRUE if:
 * - Built with STEAM=1
 * - Steam client is running
 * - Steam SDK initialized successfully
 *
 * Returns: %TRUE if Steam is available
 */
gboolean
lp_steam_bridge_is_available (LpSteamBridge *self);

/**
 * lp_steam_bridge_initialize:
 * @self: an #LpSteamBridge
 * @app_id: the Steam App ID (use 480 for testing)
 * @error: (nullable): return location for error
 *
 * Initializes the Steam SDK connection.
 * This is a no-op if not built with STEAM=1.
 *
 * Returns: %TRUE on success or if Steam unavailable (graceful fallback)
 */
gboolean
lp_steam_bridge_initialize (LpSteamBridge  *self,
                            guint32         app_id,
                            GError        **error);

/**
 * lp_steam_bridge_shutdown:
 * @self: an #LpSteamBridge
 *
 * Shuts down the Steam SDK connection.
 */
void
lp_steam_bridge_shutdown (LpSteamBridge *self);

/**
 * lp_steam_bridge_run_callbacks:
 * @self: an #LpSteamBridge
 *
 * Runs Steam callbacks. Should be called each frame.
 */
void
lp_steam_bridge_run_callbacks (LpSteamBridge *self);

/* ==========================================================================
 * Achievement Sync
 * ========================================================================== */

/**
 * lp_steam_bridge_sync_achievement:
 * @self: an #LpSteamBridge
 * @achievement_id: the achievement ID to sync
 *
 * Syncs an achievement unlock to Steam.
 * This is a no-op if Steam is unavailable.
 *
 * Returns: %TRUE if synced successfully or Steam unavailable
 */
gboolean
lp_steam_bridge_sync_achievement (LpSteamBridge *self,
                                  const gchar   *achievement_id);

/**
 * lp_steam_bridge_sync_all_achievements:
 * @self: an #LpSteamBridge
 * @manager: the #LpAchievementManager to sync from
 *
 * Syncs all unlocked achievements to Steam.
 * This is a no-op if Steam is unavailable.
 *
 * Returns: %TRUE if synced successfully or Steam unavailable
 */
gboolean
lp_steam_bridge_sync_all_achievements (LpSteamBridge        *self,
                                       LpAchievementManager *manager);

/**
 * lp_steam_bridge_clear_achievement:
 * @self: an #LpSteamBridge
 * @achievement_id: the achievement ID to clear
 *
 * Clears an achievement on Steam (for testing only).
 * This is a no-op if Steam is unavailable.
 *
 * Returns: %TRUE if cleared successfully or Steam unavailable
 */
gboolean
lp_steam_bridge_clear_achievement (LpSteamBridge *self,
                                   const gchar   *achievement_id);

/**
 * lp_steam_bridge_store_stats:
 * @self: an #LpSteamBridge
 *
 * Stores achievement changes to Steam servers.
 * Must be called after sync operations for changes to persist.
 *
 * Returns: %TRUE if stored successfully or Steam unavailable
 */
gboolean
lp_steam_bridge_store_stats (LpSteamBridge *self);

/* ==========================================================================
 * Cloud Save
 * ========================================================================== */

/**
 * lp_steam_bridge_cloud_write:
 * @self: an #LpSteamBridge
 * @filename: the cloud filename
 * @data: the data to write
 * @length: the data length
 * @error: (nullable): return location for error
 *
 * Writes data to Steam Cloud.
 * Falls back to local file if Steam unavailable.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_cloud_write (LpSteamBridge  *self,
                             const gchar    *filename,
                             const gchar    *data,
                             gsize           length,
                             GError        **error);

/**
 * lp_steam_bridge_cloud_read:
 * @self: an #LpSteamBridge
 * @filename: the cloud filename
 * @data: (out) (transfer full): location for the data
 * @length: (out): location for the data length
 * @error: (nullable): return location for error
 *
 * Reads data from Steam Cloud.
 * Falls back to local file if Steam unavailable.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_cloud_read (LpSteamBridge  *self,
                            const gchar    *filename,
                            gchar         **data,
                            gsize          *length,
                            GError        **error);

/**
 * lp_steam_bridge_cloud_exists:
 * @self: an #LpSteamBridge
 * @filename: the cloud filename
 *
 * Checks if a file exists in Steam Cloud.
 *
 * Returns: %TRUE if the file exists
 */
gboolean
lp_steam_bridge_cloud_exists (LpSteamBridge *self,
                              const gchar   *filename);

/* ==========================================================================
 * User Info
 * ========================================================================== */

/**
 * lp_steam_bridge_get_user_name:
 * @self: an #LpSteamBridge
 *
 * Gets the Steam user's display name.
 *
 * Returns: (transfer none) (nullable): the user name, or %NULL if unavailable
 */
const gchar *
lp_steam_bridge_get_user_name (LpSteamBridge *self);

/**
 * lp_steam_bridge_get_user_id:
 * @self: an #LpSteamBridge
 *
 * Gets the Steam user's ID (Steam64 format).
 *
 * Returns: the user ID, or 0 if unavailable
 */
guint64
lp_steam_bridge_get_user_id (LpSteamBridge *self);

G_END_DECLS

#endif /* LP_STEAM_BRIDGE_H */
