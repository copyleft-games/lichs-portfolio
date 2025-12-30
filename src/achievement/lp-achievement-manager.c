/* lp-achievement-manager.c - Achievement Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_ACHIEVEMENT
#include "../lp-log.h"

#include "lp-achievement-manager.h"

/* Achievement state */
typedef struct
{
    gchar   *achievement_id;
    guint    progress;     /* 0-100 */
    gboolean unlocked;
} AchievementState;

static void
achievement_state_free (AchievementState *state)
{
    if (state != NULL)
    {
        g_free (state->achievement_id);
        g_free (state);
    }
}

struct _LpAchievementManager
{
    GObject parent_instance;

    GHashTable *achievements;   /* achievement_id -> AchievementState */
    guint       total_count;    /* Total defined achievements (from data) */
};

enum
{
    SIGNAL_ACHIEVEMENT_UNLOCKED,
    SIGNAL_PROGRESS_UPDATED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

static LpAchievementManager *default_manager = NULL;

/* Forward declarations for LrgSaveable interface */
static void lp_achievement_manager_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpAchievementManager, lp_achievement_manager, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_achievement_manager_saveable_init))

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_achievement_manager_get_default:
 *
 * Gets the default achievement manager instance.
 *
 * Returns: (transfer none): The default #LpAchievementManager
 */
LpAchievementManager *
lp_achievement_manager_get_default (void)
{
    if (default_manager == NULL)
        default_manager = g_object_new (LP_TYPE_ACHIEVEMENT_MANAGER, NULL);

    return default_manager;
}

/* ==========================================================================
 * LrgSaveable Interface Implementation
 * ========================================================================== */

static const gchar *
lp_achievement_manager_get_save_id (LrgSaveable *saveable)
{
    return "achievement-manager";
}

static gboolean
lp_achievement_manager_save (LrgSaveable    *saveable,
                             LrgSaveContext *context,
                             GError        **error)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (saveable);
    GHashTableIter iter;
    gpointer key, value;
    guint count;

    g_hash_table_iter_init (&iter, self->achievements);
    count = 0;

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        AchievementState *state = (AchievementState *)value;
        g_autofree gchar *section_name = g_strdup_printf ("achievement-%u", count);

        lrg_save_context_begin_section (context, section_name);
        lrg_save_context_write_string (context, "id", state->achievement_id);
        lrg_save_context_write_uint (context, "progress", state->progress);
        lrg_save_context_write_boolean (context, "unlocked", state->unlocked);
        lrg_save_context_end_section (context);

        count++;
    }

    lrg_save_context_write_uint (context, "achievement-count", count);

    return TRUE;
}

static gboolean
lp_achievement_manager_load (LrgSaveable    *saveable,
                             LrgSaveContext *context,
                             GError        **error)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (saveable);
    guint count;
    guint i;

    g_hash_table_remove_all (self->achievements);

    count = (guint)lrg_save_context_read_uint (context, "achievement-count", 0);

    for (i = 0; i < count; i++)
    {
        g_autofree gchar *section_name = g_strdup_printf ("achievement-%u", i);
        const gchar *achievement_id;
        AchievementState *state;

        if (!lrg_save_context_enter_section (context, section_name))
            continue;

        achievement_id = lrg_save_context_read_string (context, "id", NULL);

        if (achievement_id != NULL)
        {
            state = g_new0 (AchievementState, 1);
            state->achievement_id = g_strdup (achievement_id);
            state->progress = (guint)lrg_save_context_read_uint (context, "progress", 0);
            state->unlocked = lrg_save_context_read_boolean (context, "unlocked", FALSE);

            g_hash_table_insert (self->achievements,
                                 g_strdup (achievement_id),
                                 state);
        }

        lrg_save_context_leave_section (context);
    }

    lp_log_debug ("Loaded %u achievement states", count);

    return TRUE;
}

static void
lp_achievement_manager_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_achievement_manager_get_save_id;
    iface->save = lp_achievement_manager_save;
    iface->load = lp_achievement_manager_load;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_achievement_manager_finalize (GObject *object)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (object);

    lp_log_debug ("Finalizing achievement manager");

    g_clear_pointer (&self->achievements, g_hash_table_unref);

    if (default_manager == self)
        default_manager = NULL;

    G_OBJECT_CLASS (lp_achievement_manager_parent_class)->finalize (object);
}

static void
lp_achievement_manager_class_init (LpAchievementManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_achievement_manager_finalize;

    /**
     * LpAchievementManager::achievement-unlocked:
     * @self: the #LpAchievementManager
     * @achievement_id: the unlocked achievement ID
     *
     * Emitted when an achievement is unlocked.
     */
    signals[SIGNAL_ACHIEVEMENT_UNLOCKED] =
        g_signal_new ("achievement-unlocked",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_STRING);

    /**
     * LpAchievementManager::progress-updated:
     * @self: the #LpAchievementManager
     * @achievement_id: the achievement ID
     * @progress: the new progress value
     *
     * Emitted when achievement progress is updated.
     */
    signals[SIGNAL_PROGRESS_UPDATED] =
        g_signal_new ("progress-updated",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_STRING,
                      G_TYPE_UINT);
}

static void
lp_achievement_manager_init (LpAchievementManager *self)
{
    self->achievements = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                 g_free,
                                                 (GDestroyNotify)achievement_state_free);
    self->total_count = 0;  /* Phase 5+: Load from data */
}

/* ==========================================================================
 * Achievement Tracking (Skeleton)
 * ========================================================================== */

static AchievementState *
get_or_create_state (LpAchievementManager *self,
                     const gchar          *achievement_id)
{
    AchievementState *state;

    state = g_hash_table_lookup (self->achievements, achievement_id);

    if (state == NULL)
    {
        state = g_new0 (AchievementState, 1);
        state->achievement_id = g_strdup (achievement_id);
        state->progress = 0;
        state->unlocked = FALSE;

        g_hash_table_insert (self->achievements,
                             g_strdup (achievement_id),
                             state);
    }

    return state;
}

/**
 * lp_achievement_manager_unlock:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Unlocks an achievement.
 *
 * Returns: %TRUE if newly unlocked
 */
gboolean
lp_achievement_manager_unlock (LpAchievementManager *self,
                               const gchar          *achievement_id)
{
    AchievementState *state;

    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), FALSE);
    g_return_val_if_fail (achievement_id != NULL, FALSE);

    state = get_or_create_state (self, achievement_id);

    if (state->unlocked)
        return FALSE;

    state->unlocked = TRUE;
    state->progress = 100;

    lp_log_info ("Achievement unlocked: %s", achievement_id);

    g_signal_emit (self, signals[SIGNAL_ACHIEVEMENT_UNLOCKED], 0, achievement_id);

    return TRUE;
}

/**
 * lp_achievement_manager_is_unlocked:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Checks if an achievement is unlocked.
 *
 * Returns: %TRUE if unlocked
 */
gboolean
lp_achievement_manager_is_unlocked (LpAchievementManager *self,
                                    const gchar          *achievement_id)
{
    AchievementState *state;

    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), FALSE);
    g_return_val_if_fail (achievement_id != NULL, FALSE);

    state = g_hash_table_lookup (self->achievements, achievement_id);

    return state != NULL && state->unlocked;
}

/**
 * lp_achievement_manager_get_unlocked_count:
 * @self: an #LpAchievementManager
 *
 * Gets the number of unlocked achievements.
 *
 * Returns: Number unlocked
 */
guint
lp_achievement_manager_get_unlocked_count (LpAchievementManager *self)
{
    GHashTableIter iter;
    gpointer value;
    guint count = 0;

    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0);

    g_hash_table_iter_init (&iter, self->achievements);

    while (g_hash_table_iter_next (&iter, NULL, &value))
    {
        AchievementState *state = (AchievementState *)value;
        if (state->unlocked)
            count++;
    }

    return count;
}

/**
 * lp_achievement_manager_get_total_count:
 * @self: an #LpAchievementManager
 *
 * Gets the total number of achievements.
 *
 * Returns: Total count
 */
guint
lp_achievement_manager_get_total_count (LpAchievementManager *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0);

    /* Phase 1 skeleton: Return tracked count */
    return self->total_count > 0 ? self->total_count :
           g_hash_table_size (self->achievements);
}

/**
 * lp_achievement_manager_get_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets progress towards an achievement.
 *
 * Returns: Progress percentage (0-100)
 */
guint
lp_achievement_manager_get_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id)
{
    AchievementState *state;

    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0);
    g_return_val_if_fail (achievement_id != NULL, 0);

    state = g_hash_table_lookup (self->achievements, achievement_id);

    return state != NULL ? state->progress : 0;
}

/**
 * lp_achievement_manager_set_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 * @progress: the progress value (0-100)
 *
 * Sets progress towards an achievement.
 */
void
lp_achievement_manager_set_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id,
                                     guint                 progress)
{
    AchievementState *state;

    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));
    g_return_if_fail (achievement_id != NULL);

    state = get_or_create_state (self, achievement_id);

    if (state->unlocked)
        return;

    progress = MIN (progress, 100);

    if (state->progress == progress)
        return;

    state->progress = progress;

    g_signal_emit (self, signals[SIGNAL_PROGRESS_UPDATED], 0,
                   achievement_id, progress);

    /* Auto-unlock at 100% */
    if (progress >= 100)
        lp_achievement_manager_unlock (self, achievement_id);
}

/* ==========================================================================
 * Game Event Hooks (Skeleton)
 * ========================================================================== */

/**
 * lp_achievement_manager_on_gold_earned:
 * @self: an #LpAchievementManager
 * @amount: amount of gold earned
 *
 * Called when gold is earned.
 */
void
lp_achievement_manager_on_gold_earned (LpAchievementManager *self,
                                       gdouble               amount)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    /*
     * Phase 1 skeleton: No-op.
     * Phase 5+: Update gold-related achievements.
     */
}

/**
 * lp_achievement_manager_on_year_passed:
 * @self: an #LpAchievementManager
 * @year: the new year
 *
 * Called when a year passes.
 */
void
lp_achievement_manager_on_year_passed (LpAchievementManager *self,
                                       guint64               year)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    /*
     * Phase 1 skeleton: No-op.
     * Phase 5+: Update time-related achievements.
     */
}

/**
 * lp_achievement_manager_on_prestige:
 * @self: an #LpAchievementManager
 * @points_earned: phylactery points earned
 *
 * Called when player prestiges.
 */
void
lp_achievement_manager_on_prestige (LpAchievementManager *self,
                                    guint64               points_earned)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    /*
     * Phase 1 skeleton: No-op.
     * Phase 5+: Update prestige-related achievements.
     */
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_achievement_manager_reset:
 * @self: an #LpAchievementManager
 *
 * Resets all achievement progress.
 */
void
lp_achievement_manager_reset (LpAchievementManager *self)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    lp_log_warning ("Resetting all achievements (this is destructive!)");

    g_hash_table_remove_all (self->achievements);
}
