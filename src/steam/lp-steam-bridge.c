/* lp-steam-bridge.c - Steam Integration Bridge
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Provides optional Steam integration via libregnum's Steam module.
 * All functionality gracefully degrades when Steam is unavailable.
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_STEAM
#include "../lp-log.h"

#include "lp-steam-bridge.h"
#include "../achievement/lp-achievement-manager.h"

/*
 * Steam integration is only available when built with STEAM=1.
 * This conditionally compiles the Steam SDK calls.
 */
#ifdef STEAM_BUILD
#include <libregnum.h>  /* Includes lrg-steam-client.h, lrg-steam-achievements.h */
#define HAS_STEAM 1
#else
#define HAS_STEAM 0
#endif

struct _LpSteamBridge
{
    GObject parent_instance;

#if HAS_STEAM
    LrgSteamClient       *client;       /* Steam SDK client wrapper */
    LrgSteamAchievements *achievements; /* Steam achievements API */
#endif

    gboolean initialized;
    gboolean available;
};

static LpSteamBridge *default_bridge = NULL;

G_DEFINE_TYPE (LpSteamBridge, lp_steam_bridge, G_TYPE_OBJECT)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_steam_bridge_get_default:
 *
 * Gets the default Steam bridge instance.
 *
 * Returns: (transfer none): The default #LpSteamBridge
 */
LpSteamBridge *
lp_steam_bridge_get_default (void)
{
    if (default_bridge == NULL)
        default_bridge = g_object_new (LP_TYPE_STEAM_BRIDGE, NULL);

    return default_bridge;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_steam_bridge_dispose (GObject *object)
{
#if HAS_STEAM
    LpSteamBridge *self = LP_STEAM_BRIDGE (object);

    g_clear_object (&self->achievements);
    g_clear_object (&self->client);
#endif

    G_OBJECT_CLASS (lp_steam_bridge_parent_class)->dispose (object);
}

static void
lp_steam_bridge_finalize (GObject *object)
{
    LpSteamBridge *self = LP_STEAM_BRIDGE (object);

    if (default_bridge == self)
        default_bridge = NULL;

    lp_log_debug ("Finalizing Steam bridge");

    G_OBJECT_CLASS (lp_steam_bridge_parent_class)->finalize (object);
}

static void
lp_steam_bridge_class_init (LpSteamBridgeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = lp_steam_bridge_dispose;
    object_class->finalize = lp_steam_bridge_finalize;
}

static void
lp_steam_bridge_init (LpSteamBridge *self)
{
#if HAS_STEAM
    self->client = NULL;
    self->achievements = NULL;
#endif
    self->initialized = FALSE;
    self->available = FALSE;
}

/* ==========================================================================
 * Initialization
 * ========================================================================== */

/**
 * lp_steam_bridge_is_available:
 * @self: an #LpSteamBridge
 *
 * Checks if Steam integration is available.
 *
 * Returns: %TRUE if Steam is available
 */
gboolean
lp_steam_bridge_is_available (LpSteamBridge *self)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), FALSE);

    return self->available;
}

/**
 * lp_steam_bridge_initialize:
 * @self: an #LpSteamBridge
 * @app_id: the Steam App ID
 * @error: (nullable): return location for error
 *
 * Initializes the Steam SDK.
 *
 * Returns: %TRUE on success or graceful fallback
 */
gboolean
lp_steam_bridge_initialize (LpSteamBridge  *self,
                            guint32         app_id,
                            GError        **error)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), FALSE);

    if (self->initialized)
    {
        lp_log_debug ("Steam bridge already initialized");
        return TRUE;
    }

#if HAS_STEAM
    {
        g_autoptr(GError) local_error = NULL;

        lp_log_info ("Initializing Steam SDK with App ID %u", app_id);

        /* Create Steam client */
        self->client = lrg_steam_client_new (app_id, &local_error);
        if (self->client == NULL)
        {
            lp_log_warning ("Steam client unavailable: %s",
                           local_error ? local_error->message : "unknown error");
            self->initialized = TRUE;
            self->available = FALSE;
            return TRUE;  /* Graceful fallback */
        }

        /* Check if Steam is running */
        if (!lrg_steam_client_is_available (self->client))
        {
            lp_log_info ("Steam client not running, features disabled");
            g_clear_object (&self->client);
            self->initialized = TRUE;
            self->available = FALSE;
            return TRUE;  /* Graceful fallback */
        }

        /* Create achievements wrapper */
        self->achievements = lrg_steam_achievements_new (self->client);
        if (self->achievements == NULL)
        {
            lp_log_warning ("Steam achievements unavailable");
            /* Continue without achievements */
        }

        self->initialized = TRUE;
        self->available = TRUE;

        lp_log_info ("Steam SDK initialized successfully");
        return TRUE;
    }
#else
    (void)app_id;
    (void)error;

    lp_log_debug ("Steam SDK not compiled in (STEAM=1 required)");
    self->initialized = TRUE;
    self->available = FALSE;
    return TRUE;  /* Graceful fallback */
#endif
}

/**
 * lp_steam_bridge_shutdown:
 * @self: an #LpSteamBridge
 *
 * Shuts down the Steam SDK.
 */
void
lp_steam_bridge_shutdown (LpSteamBridge *self)
{
    g_return_if_fail (LP_IS_STEAM_BRIDGE (self));

    if (!self->initialized)
        return;

#if HAS_STEAM
    if (self->available)
    {
        lp_log_info ("Shutting down Steam SDK");
        g_clear_object (&self->achievements);
        if (self->client != NULL)
        {
            lrg_steam_client_shutdown (self->client);
            g_clear_object (&self->client);
        }
    }
#endif

    self->initialized = FALSE;
    self->available = FALSE;
}

/**
 * lp_steam_bridge_run_callbacks:
 * @self: an #LpSteamBridge
 *
 * Runs Steam callbacks.
 */
void
lp_steam_bridge_run_callbacks (LpSteamBridge *self)
{
    g_return_if_fail (LP_IS_STEAM_BRIDGE (self));

#if HAS_STEAM
    if (self->available && self->client != NULL)
        lrg_steam_client_run_callbacks (self->client);
#endif
}

/* ==========================================================================
 * Achievement Sync
 * ========================================================================== */

/**
 * lp_steam_bridge_sync_achievement:
 * @self: an #LpSteamBridge
 * @achievement_id: the achievement ID
 *
 * Syncs an achievement to Steam.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_sync_achievement (LpSteamBridge *self,
                                  const gchar   *achievement_id)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), TRUE);
    g_return_val_if_fail (achievement_id != NULL, TRUE);

    if (!self->available)
        return TRUE;  /* No-op when unavailable */

#if HAS_STEAM
    if (self->achievements != NULL)
    {
        g_autoptr(GError) error = NULL;

        if (!lrg_steam_achievements_unlock (self->achievements, achievement_id, &error))
        {
            lp_log_warning ("Failed to sync achievement '%s' to Steam: %s",
                           achievement_id,
                           error ? error->message : "unknown error");
            return FALSE;
        }

        lp_log_debug ("Synced achievement '%s' to Steam", achievement_id);
    }
#else
    (void)achievement_id;
#endif

    return TRUE;
}

/**
 * lp_steam_bridge_sync_all_achievements:
 * @self: an #LpSteamBridge
 * @manager: the achievement manager
 *
 * Syncs all unlocked achievements to Steam.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_sync_all_achievements (LpSteamBridge        *self,
                                       LpAchievementManager *manager)
{
    GList *all;
    GList *l;
    gboolean success = TRUE;

    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), TRUE);
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (manager), TRUE);

    if (!self->available)
        return TRUE;  /* No-op when unavailable */

    all = lp_achievement_manager_get_all (manager);

    for (l = all; l != NULL; l = l->next)
    {
        LrgAchievement *achievement = LRG_ACHIEVEMENT (l->data);
        const gchar *id;

        if (!lrg_achievement_is_unlocked (achievement))
            continue;

        id = lrg_achievement_get_id (achievement);
        if (!lp_steam_bridge_sync_achievement (self, id))
            success = FALSE;
    }

    g_list_free (all);

    /* Store changes to Steam servers */
    if (success)
        success = lp_steam_bridge_store_stats (self);

    return success;
}

/**
 * lp_steam_bridge_clear_achievement:
 * @self: an #LpSteamBridge
 * @achievement_id: the achievement ID
 *
 * Clears an achievement on Steam (for testing).
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_clear_achievement (LpSteamBridge *self,
                                   const gchar   *achievement_id)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), TRUE);
    g_return_val_if_fail (achievement_id != NULL, TRUE);

    if (!self->available)
        return TRUE;

#if HAS_STEAM
    if (self->achievements != NULL)
    {
        g_autoptr(GError) error = NULL;

        if (!lrg_steam_achievements_clear (self->achievements, achievement_id, &error))
        {
            lp_log_warning ("Failed to clear achievement '%s' on Steam: %s",
                           achievement_id,
                           error ? error->message : "unknown error");
            return FALSE;
        }

        lp_log_debug ("Cleared achievement '%s' on Steam", achievement_id);
    }
#else
    (void)achievement_id;
#endif

    return TRUE;
}

/**
 * lp_steam_bridge_store_stats:
 * @self: an #LpSteamBridge
 *
 * Stores achievement changes to Steam servers.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_store_stats (LpSteamBridge *self)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), TRUE);

    if (!self->available)
        return TRUE;

#if HAS_STEAM
    if (self->achievements != NULL)
    {
        g_autoptr(GError) error = NULL;

        if (!lrg_steam_achievements_store (self->achievements, &error))
        {
            lp_log_warning ("Failed to store stats to Steam: %s",
                           error ? error->message : "unknown error");
            return FALSE;
        }

        lp_log_debug ("Stored stats to Steam");
    }
#endif

    return TRUE;
}

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
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_cloud_write (LpSteamBridge  *self,
                             const gchar    *filename,
                             const gchar    *data,
                             gsize           length,
                             GError        **error)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (data != NULL || length == 0, FALSE);

    if (!self->available)
    {
        /* Steam unavailable - caller should use local save instead */
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_NOT_SUPPORTED,
                     "Steam Cloud not available");
        return FALSE;
    }

#if HAS_STEAM
    if (self->client != NULL)
    {
        LrgSteamCloud *cloud = lrg_steam_client_get_cloud (self->client);
        if (cloud != NULL)
        {
            if (!lrg_steam_cloud_write_file (cloud, filename, data, length, error))
                return FALSE;

            lp_log_debug ("Wrote %zu bytes to Steam Cloud: %s", length, filename);
            return TRUE;
        }
    }
#endif

    (void)data;
    (void)length;

    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_SUPPORTED,
                 "Steam Cloud not available");
    return FALSE;
}

/**
 * lp_steam_bridge_cloud_read:
 * @self: an #LpSteamBridge
 * @filename: the cloud filename
 * @data: (out) (transfer full): location for the data
 * @length: (out): location for the data length
 * @error: (nullable): return location for error
 *
 * Reads data from Steam Cloud.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_steam_bridge_cloud_read (LpSteamBridge  *self,
                            const gchar    *filename,
                            gchar         **data,
                            gsize          *length,
                            GError        **error)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);
    g_return_val_if_fail (length != NULL, FALSE);

    *data = NULL;
    *length = 0;

    if (!self->available)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_NOT_SUPPORTED,
                     "Steam Cloud not available");
        return FALSE;
    }

#if HAS_STEAM
    if (self->client != NULL)
    {
        LrgSteamCloud *cloud = lrg_steam_client_get_cloud (self->client);
        if (cloud != NULL)
        {
            if (!lrg_steam_cloud_read_file (cloud, filename, data, length, error))
                return FALSE;

            lp_log_debug ("Read %zu bytes from Steam Cloud: %s", *length, filename);
            return TRUE;
        }
    }
#endif

    g_set_error (error,
                 G_IO_ERROR,
                 G_IO_ERROR_NOT_SUPPORTED,
                 "Steam Cloud not available");
    return FALSE;
}

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
                              const gchar   *filename)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    if (!self->available)
        return FALSE;

#if HAS_STEAM
    if (self->client != NULL)
    {
        LrgSteamCloud *cloud = lrg_steam_client_get_cloud (self->client);
        if (cloud != NULL)
            return lrg_steam_cloud_file_exists (cloud, filename);
    }
#else
    (void)filename;
#endif

    return FALSE;
}

/* ==========================================================================
 * User Info
 * ========================================================================== */

/**
 * lp_steam_bridge_get_user_name:
 * @self: an #LpSteamBridge
 *
 * Gets the Steam user's display name.
 *
 * Returns: (transfer none) (nullable): the user name
 */
const gchar *
lp_steam_bridge_get_user_name (LpSteamBridge *self)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), NULL);

    if (!self->available)
        return NULL;

#if HAS_STEAM
    if (self->client != NULL)
        return lrg_steam_client_get_user_name (self->client);
#endif

    return NULL;
}

/**
 * lp_steam_bridge_get_user_id:
 * @self: an #LpSteamBridge
 *
 * Gets the Steam user's ID.
 *
 * Returns: the user ID, or 0
 */
guint64
lp_steam_bridge_get_user_id (LpSteamBridge *self)
{
    g_return_val_if_fail (LP_IS_STEAM_BRIDGE (self), 0);

    if (!self->available)
        return 0;

#if HAS_STEAM
    if (self->client != NULL)
        return lrg_steam_client_get_user_id (self->client);
#endif

    return 0;
}
