/* lp-ui-sounds.c - UI Sound Effect Manager
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-ui-sounds.h"

struct _LpUiSounds
{
    GObject           parent_instance;

    /* Sound storage */
    GHashTable       *sounds;       /* string -> GrlSound* */

    /* Configuration */
    gboolean          enabled;
    gfloat            volume;

    /* Data directory */
    gchar            *data_dir;
};

G_DEFINE_FINAL_TYPE (LpUiSounds, lp_ui_sounds, G_TYPE_OBJECT)

static LpUiSounds *default_ui_sounds = NULL;

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

static void
lp_ui_sounds_play_internal (LpUiSounds  *self,
                            const gchar *sound_id)
{
    GrlSound *sound;

    if (!self->enabled)
        return;

    sound = g_hash_table_lookup (self->sounds, sound_id);
    if (sound != NULL)
    {
        grl_sound_set_volume (sound, self->volume);
        grl_sound_play (sound);
    }
    else
    {
        lp_log_debug ("Sound not found: %s", sound_id);
    }
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_ui_sounds_finalize (GObject *object)
{
    LpUiSounds *self = LP_UI_SOUNDS (object);

    g_clear_pointer (&self->sounds, g_hash_table_unref);
    g_clear_pointer (&self->data_dir, g_free);

    G_OBJECT_CLASS (lp_ui_sounds_parent_class)->finalize (object);
}

static void
lp_ui_sounds_class_init (LpUiSoundsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_ui_sounds_finalize;
}

static void
lp_ui_sounds_init (LpUiSounds *self)
{
    self->sounds = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           g_free, g_object_unref);
    self->enabled = TRUE;
    self->volume = 0.7f;

    /* Determine data directory */
    if (g_file_test ("data/audio", G_FILE_TEST_IS_DIR))
    {
        self->data_dir = g_strdup ("data");
    }
    else
    {
        self->data_dir = g_build_filename (
            g_get_user_data_dir (), "lichs-portfolio", NULL);
    }
}

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_ui_sounds_get_default:
 *
 * Gets the default UI sounds manager.
 * Initializes on first call, loading sounds from the manifest.
 *
 * Returns: (transfer none): The default #LpUiSounds
 */
LpUiSounds *
lp_ui_sounds_get_default (void)
{
    if (default_ui_sounds == NULL)
    {
        g_autoptr(GError) error = NULL;
        g_autofree gchar *path = NULL;

        default_ui_sounds = g_object_new (LP_TYPE_UI_SOUNDS, NULL);

        /* Load default manifest */
        path = g_build_filename (default_ui_sounds->data_dir,
                                  "audio", "ui-sounds.yaml", NULL);

        if (!lp_ui_sounds_load_manifest (default_ui_sounds, path, &error))
        {
            lp_log_debug ("UI sounds manifest not found: %s", path);
            /* Not fatal - sounds just won't play */
        }
    }

    return default_ui_sounds;
}

/* ==========================================================================
 * Sound Playback
 * ========================================================================== */

/**
 * lp_ui_sounds_play:
 * @self: an #LpUiSounds
 * @sound_id: the sound ID to play
 *
 * Plays a UI sound effect by ID.
 * Does nothing if the sound ID is not found.
 */
void
lp_ui_sounds_play (LpUiSounds  *self,
                   const gchar *sound_id)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));
    g_return_if_fail (sound_id != NULL);

    lp_ui_sounds_play_internal (self, sound_id);
}

/**
 * lp_ui_sounds_play_click:
 * @self: an #LpUiSounds
 *
 * Plays the button click sound.
 */
void
lp_ui_sounds_play_click (LpUiSounds *self)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    lp_ui_sounds_play_internal (self, LP_UI_SOUND_CLICK);
}

/**
 * lp_ui_sounds_play_purchase:
 * @self: an #LpUiSounds
 *
 * Plays the purchase confirmation sound.
 */
void
lp_ui_sounds_play_purchase (LpUiSounds *self)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    lp_ui_sounds_play_internal (self, LP_UI_SOUND_PURCHASE);
}

/**
 * lp_ui_sounds_play_sell:
 * @self: an #LpUiSounds
 *
 * Plays the sell confirmation sound.
 */
void
lp_ui_sounds_play_sell (LpUiSounds *self)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    lp_ui_sounds_play_internal (self, LP_UI_SOUND_SELL);
}

/**
 * lp_ui_sounds_play_achievement:
 * @self: an #LpUiSounds
 *
 * Plays the achievement unlock fanfare.
 */
void
lp_ui_sounds_play_achievement (LpUiSounds *self)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    lp_ui_sounds_play_internal (self, LP_UI_SOUND_ACHIEVEMENT);
}

/**
 * lp_ui_sounds_play_event:
 * @self: an #LpUiSounds
 *
 * Plays the event notification chime.
 */
void
lp_ui_sounds_play_event (LpUiSounds *self)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    lp_ui_sounds_play_internal (self, LP_UI_SOUND_EVENT);
}

/**
 * lp_ui_sounds_play_error:
 * @self: an #LpUiSounds
 *
 * Plays the error/invalid action sound.
 */
void
lp_ui_sounds_play_error (LpUiSounds *self)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    lp_ui_sounds_play_internal (self, LP_UI_SOUND_ERROR);
}

/* ==========================================================================
 * Configuration
 * ========================================================================== */

/**
 * lp_ui_sounds_set_enabled:
 * @self: an #LpUiSounds
 * @enabled: whether UI sounds are enabled
 *
 * Enables or disables all UI sounds.
 */
void
lp_ui_sounds_set_enabled (LpUiSounds *self,
                           gboolean    enabled)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    self->enabled = enabled;
}

/**
 * lp_ui_sounds_get_enabled:
 * @self: an #LpUiSounds
 *
 * Gets whether UI sounds are enabled.
 *
 * Returns: %TRUE if sounds are enabled
 */
gboolean
lp_ui_sounds_get_enabled (LpUiSounds *self)
{
    g_return_val_if_fail (LP_IS_UI_SOUNDS (self), FALSE);

    return self->enabled;
}

/**
 * lp_ui_sounds_set_volume:
 * @self: an #LpUiSounds
 * @volume: volume level (0.0 to 1.0)
 *
 * Sets the UI sound volume.
 */
void
lp_ui_sounds_set_volume (LpUiSounds *self,
                          gfloat      volume)
{
    g_return_if_fail (LP_IS_UI_SOUNDS (self));

    self->volume = CLAMP (volume, 0.0f, 1.0f);
}

/**
 * lp_ui_sounds_get_volume:
 * @self: an #LpUiSounds
 *
 * Gets the UI sound volume.
 *
 * Returns: volume level (0.0 to 1.0)
 */
gfloat
lp_ui_sounds_get_volume (LpUiSounds *self)
{
    g_return_val_if_fail (LP_IS_UI_SOUNDS (self), 0.0f);

    return self->volume;
}

/**
 * lp_ui_sounds_load_manifest:
 * @self: an #LpUiSounds
 * @path: path to the sound manifest YAML file
 * @error: return location for error, or %NULL
 *
 * Loads sound definitions from a manifest file.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_ui_sounds_load_manifest (LpUiSounds   *self,
                             const gchar  *path,
                             GError      **error)
{
    g_autoptr(GKeyFile) keyfile = NULL;
    gchar **keys = NULL;
    gsize n_keys = 0;
    gsize i;
    g_autofree gchar *audio_dir = NULL;

    g_return_val_if_fail (LP_IS_UI_SOUNDS (self), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);

    if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_NOENT,
                     "Sound manifest not found: %s", path);
        return FALSE;
    }

    lp_log_debug ("Loading UI sound manifest: %s", path);

    /* Parse manifest as key file for simplicity */
    keyfile = g_key_file_new ();
    if (!g_key_file_load_from_file (keyfile, path,
                                     G_KEY_FILE_NONE, error))
    {
        return FALSE;
    }

    /* Get directory containing the manifest */
    audio_dir = g_path_get_dirname (path);

    /* Load each sound */
    keys = g_key_file_get_keys (keyfile, "sounds", &n_keys, NULL);
    if (keys != NULL)
    {
        for (i = 0; i < n_keys; i++)
        {
            g_autofree gchar *filename = NULL;
            g_autofree gchar *sound_path = NULL;
            GrlSound *sound;

            filename = g_key_file_get_string (keyfile, "sounds",
                                               keys[i], NULL);
            if (filename == NULL)
                continue;

            sound_path = g_build_filename (audio_dir, filename, NULL);

            if (!g_file_test (sound_path, G_FILE_TEST_EXISTS))
            {
                lp_log_debug ("Sound file not found: %s", sound_path);
                continue;
            }

            sound = grl_sound_new_from_file (sound_path, NULL);
            if (sound != NULL)
            {
                g_hash_table_insert (self->sounds,
                                     g_strdup (keys[i]),
                                     sound);
                lp_log_debug ("Loaded sound: %s -> %s", keys[i], filename);
            }
        }

        g_strfreev (keys);
    }

    return TRUE;
}
