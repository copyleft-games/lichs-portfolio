/* lp-achievement-manager.c - Achievement Tracking System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Wraps LrgAchievementManager from libregnum and adds game-specific
 * event hooks for achievement tracking. Integrates with LpAchievementPopup
 * for unlock notifications and LpSteamBridge for optional Steam sync.
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_ACHIEVEMENT
#include "../lp-log.h"

#include "lp-achievement-manager.h"
#include "../feedback/lp-achievement-popup.h"

/* Achievement IDs - must match YAML files */
#define ACHIEVEMENT_FIRST_MILLION     "first_million"
#define ACHIEVEMENT_CENTENNIAL        "centennial"
#define ACHIEVEMENT_DYNASTY           "dynasty"
#define ACHIEVEMENT_HOSTILE_TAKEOVER  "hostile_takeover"
#define ACHIEVEMENT_PATIENT_INVESTOR  "patient_investor"
#define ACHIEVEMENT_DARK_AWAKENING    "dark_awakening"
#define ACHIEVEMENT_SOUL_TRADER       "soul_trader"
#define ACHIEVEMENT_TRANSCENDENCE     "transcendence"

/* Statistic names for tracking */
#define STAT_TOTAL_GOLD_EARNED       "total_gold_earned"
#define STAT_TOTAL_YEARS_SLUMBERED   "total_years_slumbered"
#define STAT_MAX_FAMILY_GENERATION   "max_family_generation"
#define STAT_MAX_INVESTMENT_YEARS    "max_investment_years"
#define STAT_PRESTIGE_COUNT          "prestige_count"

struct _LpAchievementManager
{
    GObject parent_instance;

    LrgAchievementManager *backend;        /* libregnum achievement manager */
    LpAchievementPopup    *popup;          /* Notification widget (weak ref) */
    gboolean               definitions_loaded;
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
 * Internal Helpers
 * ========================================================================== */

/*
 * show_unlock_notification:
 * @self: the achievement manager
 * @achievement: the achievement that was unlocked
 *
 * Shows the achievement unlock popup notification.
 */
static void
show_unlock_notification (LpAchievementManager *self,
                          LrgAchievement       *achievement)
{
    const gchar *name;
    const gchar *description;

    if (self->popup == NULL)
        return;

    name = lrg_achievement_get_name (achievement);
    description = lrg_achievement_get_description (achievement);

    lp_achievement_popup_show (self->popup, name, description);
}

/*
 * on_backend_unlocked:
 * @backend: the libregnum achievement manager
 * @id: the achievement ID that was unlocked
 * @user_data: our LpAchievementManager
 *
 * Handler for "achievement-unlocked" signal from libregnum backend.
 */
static void
on_backend_unlocked (LrgAchievementManager *backend,
                     const gchar           *id,
                     gpointer               user_data)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (user_data);
    LrgAchievement *achievement;

    achievement = lrg_achievement_manager_get (backend, id);
    if (achievement != NULL)
        show_unlock_notification (self, achievement);

    /* Re-emit our own signal */
    g_signal_emit (self, signals[SIGNAL_ACHIEVEMENT_UNLOCKED], 0, id);
}

/*
 * on_backend_progress:
 * @backend: the libregnum achievement manager
 * @achievement: the achievement that had progress change
 * @current: the current progress value
 * @target: the target progress value
 * @user_data: our LpAchievementManager
 *
 * Handler for "achievement-progress" signal from libregnum backend.
 */
static void
on_backend_progress (LrgAchievementManager *backend,
                     LrgAchievement        *achievement,
                     gint64                 current,
                     gint64                 target,
                     gpointer               user_data)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (user_data);
    const gchar *id;
    guint percentage = 0;

    (void)backend;

    if (achievement == NULL)
        return;

    id = lrg_achievement_get_id (achievement);
    if (target > 0)
        percentage = (guint)((current * 100) / target);

    g_signal_emit (self, signals[SIGNAL_PROGRESS_UPDATED], 0, id, percentage);
}

/*
 * register_achievement:
 * @self: the achievement manager
 * @id: achievement ID
 * @name: display name
 * @description: achievement description
 * @target: progress target (0 for instant achievements)
 * @hidden: whether achievement is hidden until unlocked
 * @points: point value
 *
 * Registers a single achievement definition.
 */
static void
register_achievement (LpAchievementManager *self,
                      const gchar          *id,
                      const gchar          *name,
                      const gchar          *description,
                      gint64                target,
                      gboolean              hidden,
                      guint                 points)
{
    LrgAchievement *achievement;

    if (target > 0)
        achievement = lrg_achievement_new_with_progress (id, name, description, target);
    else
        achievement = lrg_achievement_new (id, name, description);

    lrg_achievement_set_hidden (achievement, hidden);
    lrg_achievement_set_points (achievement, points);

    lrg_achievement_manager_register (self->backend, achievement);

    lp_log_debug ("Registered achievement: %s (%s)", id, name);
}

/*
 * register_builtin_achievements:
 * @self: the achievement manager
 *
 * Registers all built-in achievement definitions.
 * This is used when YAML files are not available or as fallback.
 */
static void
register_builtin_achievements (LpAchievementManager *self)
{
    /* Wealth achievements */
    register_achievement (self,
                          ACHIEVEMENT_FIRST_MILLION,
                          "First Million",
                          "Reach 1,000,000 gold pieces",
                          1000000,  /* target */
                          FALSE,    /* hidden */
                          10);      /* points */

    /* Time achievements */
    register_achievement (self,
                          ACHIEVEMENT_CENTENNIAL,
                          "Centennial",
                          "Complete a 100-year slumber",
                          100,     /* target */
                          FALSE,
                          20);

    register_achievement (self,
                          ACHIEVEMENT_PATIENT_INVESTOR,
                          "Patient Investor",
                          "Hold a single investment for 500 years",
                          500,     /* target */
                          FALSE,
                          50);

    /* Agent achievements */
    register_achievement (self,
                          ACHIEVEMENT_DYNASTY,
                          "Dynasty",
                          "Have an agent family reach the 5th generation",
                          5,       /* target */
                          FALSE,
                          30);

    /* Financial achievements */
    register_achievement (self,
                          ACHIEVEMENT_HOSTILE_TAKEOVER,
                          "Hostile Takeover",
                          "Own 100% of a kingdom's debt",
                          0,       /* instant */
                          FALSE,
                          40);

    /* Dark achievements (hidden) */
    register_achievement (self,
                          ACHIEVEMENT_DARK_AWAKENING,
                          "Dark Awakening",
                          "Unlock dark investments",
                          0,       /* instant */
                          TRUE,    /* hidden */
                          25);

    register_achievement (self,
                          ACHIEVEMENT_SOUL_TRADER,
                          "Soul Trader",
                          "Complete your first soul trade",
                          0,       /* instant */
                          TRUE,    /* hidden */
                          35);

    /* Prestige achievements */
    register_achievement (self,
                          ACHIEVEMENT_TRANSCENDENCE,
                          "Transcendence",
                          "Complete your first prestige cycle",
                          0,       /* instant */
                          FALSE,
                          100);

    lp_log_info ("Registered %u built-in achievements",
                 lrg_achievement_manager_get_count (self->backend));
}

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

    /*
     * Delegate to libregnum's achievement manager which also implements
     * LrgSaveable. This saves all achievement states and statistics.
     */
    lrg_save_context_begin_section (context, "libregnum-backend");
    if (!lrg_saveable_save (LRG_SAVEABLE (self->backend), context, error))
    {
        lrg_save_context_end_section (context);
        return FALSE;
    }
    lrg_save_context_end_section (context);

    return TRUE;
}

static gboolean
lp_achievement_manager_load (LrgSaveable    *saveable,
                             LrgSaveContext *context,
                             GError        **error)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (saveable);

    /*
     * Delegate to libregnum's achievement manager.
     */
    if (lrg_save_context_enter_section (context, "libregnum-backend"))
    {
        if (!lrg_saveable_load (LRG_SAVEABLE (self->backend), context, error))
        {
            lrg_save_context_leave_section (context);
            return FALSE;
        }
        lrg_save_context_leave_section (context);
    }

    lp_log_debug ("Loaded achievement states");

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
lp_achievement_manager_dispose (GObject *object)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (object);

    g_clear_object (&self->backend);

    /* popup is a weak reference, just clear the pointer */
    self->popup = NULL;

    G_OBJECT_CLASS (lp_achievement_manager_parent_class)->dispose (object);
}

static void
lp_achievement_manager_finalize (GObject *object)
{
    LpAchievementManager *self = LP_ACHIEVEMENT_MANAGER (object);

    lp_log_debug ("Finalizing achievement manager");

    if (default_manager == self)
        default_manager = NULL;

    G_OBJECT_CLASS (lp_achievement_manager_parent_class)->finalize (object);
}

static void
lp_achievement_manager_class_init (LpAchievementManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = lp_achievement_manager_dispose;
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
     * @progress: the new progress percentage (0-100)
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
    /* Create the libregnum backend */
    self->backend = lrg_achievement_manager_get_default ();
    g_object_ref (self->backend);

    /* Connect to backend signals */
    g_signal_connect (self->backend, "achievement-unlocked",
                      G_CALLBACK (on_backend_unlocked), self);
    g_signal_connect (self->backend, "achievement-progress",
                      G_CALLBACK (on_backend_progress), self);

    self->popup = NULL;
    self->definitions_loaded = FALSE;
}

/* ==========================================================================
 * Initialization
 * ========================================================================== */

/**
 * lp_achievement_manager_load_definitions:
 * @self: an #LpAchievementManager
 * @data_dir: path to the data/achievements/ directory
 * @error: (nullable): return location for error
 *
 * Loads achievement definitions from YAML files.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_achievement_manager_load_definitions (LpAchievementManager  *self,
                                         const gchar           *data_dir,
                                         GError               **error)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), FALSE);

    if (self->definitions_loaded)
    {
        lp_log_debug ("Achievement definitions already loaded");
        return TRUE;
    }

    /*
     * For now, use built-in achievement definitions.
     * TODO: Load from YAML files in data_dir when data loader is ready.
     */
    (void)data_dir;
    (void)error;

    register_builtin_achievements (self);
    self->definitions_loaded = TRUE;

    return TRUE;
}

/**
 * lp_achievement_manager_set_popup:
 * @self: an #LpAchievementManager
 * @popup: (nullable): the popup widget
 *
 * Sets the popup widget for notifications.
 */
void
lp_achievement_manager_set_popup (LpAchievementManager *self,
                                  LpAchievementPopup   *popup)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));
    g_return_if_fail (popup == NULL || LP_IS_ACHIEVEMENT_POPUP (popup));

    self->popup = popup;  /* Weak reference */
}

/**
 * lp_achievement_manager_get_popup:
 * @self: an #LpAchievementManager
 *
 * Gets the current popup widget.
 *
 * Returns: (transfer none) (nullable): the popup
 */
LpAchievementPopup *
lp_achievement_manager_get_popup (LpAchievementManager *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), NULL);

    return self->popup;
}

/* ==========================================================================
 * Achievement Access
 * ========================================================================== */

/**
 * lp_achievement_manager_get_achievement:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets an achievement by ID.
 *
 * Returns: (transfer none) (nullable): the achievement
 */
LrgAchievement *
lp_achievement_manager_get_achievement (LpAchievementManager *self,
                                        const gchar          *achievement_id)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), NULL);
    g_return_val_if_fail (achievement_id != NULL, NULL);

    return lrg_achievement_manager_get (self->backend, achievement_id);
}

/**
 * lp_achievement_manager_get_all:
 * @self: an #LpAchievementManager
 *
 * Gets all registered achievements.
 *
 * Returns: (transfer container) (element-type LrgAchievement): list of achievements
 */
GList *
lp_achievement_manager_get_all (LpAchievementManager *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), NULL);

    return lrg_achievement_manager_get_all (self->backend);
}

/* ==========================================================================
 * Achievement Tracking
 * ========================================================================== */

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
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), FALSE);
    g_return_val_if_fail (achievement_id != NULL, FALSE);

    return lrg_achievement_manager_unlock (self->backend, achievement_id);
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
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), FALSE);
    g_return_val_if_fail (achievement_id != NULL, FALSE);

    return lrg_achievement_manager_is_unlocked (self->backend, achievement_id);
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
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0);

    return lrg_achievement_manager_get_unlocked_count (self->backend);
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

    return lrg_achievement_manager_get_count (self->backend);
}

/**
 * lp_achievement_manager_get_completion_percentage:
 * @self: an #LpAchievementManager
 *
 * Gets the completion percentage.
 *
 * Returns: Completion percentage (0.0 to 1.0)
 */
gdouble
lp_achievement_manager_get_completion_percentage (LpAchievementManager *self)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0.0);

    return lrg_achievement_manager_get_completion_percentage (self->backend);
}

/* ==========================================================================
 * Progress Tracking
 * ========================================================================== */

/**
 * lp_achievement_manager_get_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets the raw progress value.
 *
 * Returns: Progress value
 */
gint64
lp_achievement_manager_get_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id)
{
    LrgAchievement *achievement;
    LrgAchievementProgress *progress;

    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0);
    g_return_val_if_fail (achievement_id != NULL, 0);

    achievement = lrg_achievement_manager_get (self->backend, achievement_id);
    if (achievement == NULL)
        return 0;

    progress = lrg_achievement_get_progress (achievement);
    if (progress == NULL)
        return 0;

    return lrg_achievement_progress_get_current (progress);
}

/**
 * lp_achievement_manager_get_progress_percentage:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 *
 * Gets progress percentage (0-100).
 *
 * Returns: Progress percentage
 */
guint
lp_achievement_manager_get_progress_percentage (LpAchievementManager *self,
                                                const gchar          *achievement_id)
{
    LrgAchievement *achievement;
    LrgAchievementProgress *progress;

    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0);
    g_return_val_if_fail (achievement_id != NULL, 0);

    achievement = lrg_achievement_manager_get (self->backend, achievement_id);
    if (achievement == NULL)
        return 0;

    if (lrg_achievement_is_unlocked (achievement))
        return 100;

    progress = lrg_achievement_get_progress (achievement);
    if (progress == NULL)
        return 0;

    return (guint)(lrg_achievement_progress_get_percentage (progress) * 100.0);
}

/**
 * lp_achievement_manager_set_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 * @value: the progress value
 *
 * Sets progress towards an achievement.
 */
void
lp_achievement_manager_set_progress (LpAchievementManager *self,
                                     const gchar          *achievement_id,
                                     gint64                value)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));
    g_return_if_fail (achievement_id != NULL);

    lrg_achievement_manager_set_progress (self->backend, achievement_id, value);
}

/**
 * lp_achievement_manager_increment_progress:
 * @self: an #LpAchievementManager
 * @achievement_id: the achievement ID
 * @amount: the amount to add
 *
 * Increments progress towards an achievement.
 */
void
lp_achievement_manager_increment_progress (LpAchievementManager *self,
                                           const gchar          *achievement_id,
                                           gint64                amount)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));
    g_return_if_fail (achievement_id != NULL);

    lrg_achievement_manager_increment_progress (self->backend, achievement_id, amount);
}

/* ==========================================================================
 * Statistics
 * ========================================================================== */

/**
 * lp_achievement_manager_set_stat:
 * @self: an #LpAchievementManager
 * @name: statistic name
 * @value: the value
 *
 * Sets a tracked statistic.
 */
void
lp_achievement_manager_set_stat (LpAchievementManager *self,
                                 const gchar          *name,
                                 gint64                value)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));
    g_return_if_fail (name != NULL);

    lrg_achievement_manager_set_stat_int (self->backend, name, value);
}

/**
 * lp_achievement_manager_get_stat:
 * @self: an #LpAchievementManager
 * @name: statistic name
 *
 * Gets a tracked statistic.
 *
 * Returns: The value
 */
gint64
lp_achievement_manager_get_stat (LpAchievementManager *self,
                                 const gchar          *name)
{
    g_return_val_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self), 0);
    g_return_val_if_fail (name != NULL, 0);

    return lrg_achievement_manager_get_stat_int (self->backend, name);
}

/**
 * lp_achievement_manager_increment_stat:
 * @self: an #LpAchievementManager
 * @name: statistic name
 * @amount: the amount to add
 *
 * Increments a tracked statistic.
 */
void
lp_achievement_manager_increment_stat (LpAchievementManager *self,
                                       const gchar          *name,
                                       gint64                amount)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));
    g_return_if_fail (name != NULL);

    lrg_achievement_manager_increment_stat (self->backend, name, amount);
}

/* ==========================================================================
 * Game Event Hooks
 * ========================================================================== */

/**
 * lp_achievement_manager_on_gold_changed:
 * @self: an #LpAchievementManager
 * @total_gold: current total gold amount
 *
 * Called when gold balance changes.
 */
void
lp_achievement_manager_on_gold_changed (LpAchievementManager *self,
                                        gdouble               total_gold)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    /* Update first_million achievement progress */
    if (total_gold >= 1000000.0)
    {
        lp_achievement_manager_set_progress (self, ACHIEVEMENT_FIRST_MILLION,
                                             (gint64)total_gold);
    }
    else
    {
        lp_achievement_manager_set_progress (self, ACHIEVEMENT_FIRST_MILLION,
                                             (gint64)total_gold);
    }
}

/**
 * lp_achievement_manager_on_slumber_complete:
 * @self: an #LpAchievementManager
 * @years_slumbered: number of years slumbered
 *
 * Called when a slumber cycle completes.
 */
void
lp_achievement_manager_on_slumber_complete (LpAchievementManager *self,
                                            guint                 years_slumbered)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    /* Track total years slumbered */
    lp_achievement_manager_increment_stat (self, STAT_TOTAL_YEARS_SLUMBERED,
                                           years_slumbered);

    /* Check centennial achievement */
    if (years_slumbered >= 100)
    {
        lp_achievement_manager_set_progress (self, ACHIEVEMENT_CENTENNIAL,
                                             years_slumbered);
    }
}

/**
 * lp_achievement_manager_on_family_succession:
 * @self: an #LpAchievementManager
 * @generation: the new generation number
 *
 * Called when an agent family has a succession.
 */
void
lp_achievement_manager_on_family_succession (LpAchievementManager *self,
                                             guint                 generation)
{
    gint64 max_gen;

    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    /* Track max generation reached */
    max_gen = lp_achievement_manager_get_stat (self, STAT_MAX_FAMILY_GENERATION);
    if (generation > (guint)max_gen)
    {
        lp_achievement_manager_set_stat (self, STAT_MAX_FAMILY_GENERATION,
                                         generation);
    }

    /* Update dynasty achievement */
    lp_achievement_manager_set_progress (self, ACHIEVEMENT_DYNASTY, generation);
}

/**
 * lp_achievement_manager_on_investment_held:
 * @self: an #LpAchievementManager
 * @investment_id: the investment ID
 * @years_held: number of years held
 *
 * Called to track long-term investment holdings.
 */
void
lp_achievement_manager_on_investment_held (LpAchievementManager *self,
                                           const gchar          *investment_id,
                                           guint                 years_held)
{
    gint64 max_years;

    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    (void)investment_id;  /* May be used for per-investment tracking later */

    /* Track max years any investment has been held */
    max_years = lp_achievement_manager_get_stat (self, STAT_MAX_INVESTMENT_YEARS);
    if (years_held > (guint)max_years)
    {
        lp_achievement_manager_set_stat (self, STAT_MAX_INVESTMENT_YEARS,
                                         years_held);
    }

    /* Update patient investor achievement */
    lp_achievement_manager_set_progress (self, ACHIEVEMENT_PATIENT_INVESTOR,
                                         years_held);
}

/**
 * lp_achievement_manager_on_dark_unlock:
 * @self: an #LpAchievementManager
 *
 * Called when dark investments are unlocked.
 */
void
lp_achievement_manager_on_dark_unlock (LpAchievementManager *self)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    lp_achievement_manager_unlock (self, ACHIEVEMENT_DARK_AWAKENING);
}

/**
 * lp_achievement_manager_on_soul_trade:
 * @self: an #LpAchievementManager
 *
 * Called when a soul trade is completed.
 */
void
lp_achievement_manager_on_soul_trade (LpAchievementManager *self)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    lp_achievement_manager_unlock (self, ACHIEVEMENT_SOUL_TRADER);
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

    (void)points_earned;  /* May be used for progress-based prestige achievements */

    lp_achievement_manager_increment_stat (self, STAT_PRESTIGE_COUNT, 1);
    lp_achievement_manager_unlock (self, ACHIEVEMENT_TRANSCENDENCE);
}

/**
 * lp_achievement_manager_on_kingdom_debt_owned:
 * @self: an #LpAchievementManager
 * @kingdom_id: the kingdom ID
 * @debt_percentage: percentage of kingdom's debt owned (0.0-1.0)
 *
 * Called when player's ownership of a kingdom's debt changes.
 */
void
lp_achievement_manager_on_kingdom_debt_owned (LpAchievementManager *self,
                                              const gchar          *kingdom_id,
                                              gdouble               debt_percentage)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    (void)kingdom_id;  /* May be used for per-kingdom tracking */

    /* Check for 100% ownership */
    if (debt_percentage >= 1.0)
    {
        lp_achievement_manager_unlock (self, ACHIEVEMENT_HOSTILE_TAKEOVER);
    }
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_achievement_manager_reset:
 * @self: an #LpAchievementManager
 *
 * Resets all achievement progress and unlocks.
 */
void
lp_achievement_manager_reset (LpAchievementManager *self)
{
    g_return_if_fail (LP_IS_ACHIEVEMENT_MANAGER (self));

    lp_log_info ("Resetting all achievements");

    lrg_achievement_manager_reset_all (self->backend);
    lrg_achievement_manager_reset_stats (self->backend);
}
