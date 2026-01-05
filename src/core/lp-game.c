/* lp-game.c - Main Game (LrgIdle2DTemplate Subclass)
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_APP
#include "../lp-log.h"

#include "lp-game.h"
#include "lp-game-data.h"
#include "lp-gameplay-settings.h"
#include "lp-phylactery.h"
#include "lp-prestige.h"
#include "../achievement/lp-achievement-manager.h"
#include "../investment/lp-portfolio.h"
#include "../investment/lp-investment.h"
#include "../save/lp-save-manager.h"
#include "../states/lp-state-main-menu.h"
#include "../states/lp-state-pause.h"
#include "../states/lp-state-settings.h"
#include "../states/lp-state-welcome-back.h"
#include "../tutorial/lp-tutorial-sequences.h"

#include <raylib.h>
#include <math.h>

/* Window configuration */
#define WINDOW_WIDTH   1280
#define WINDOW_HEIGHT  720
#define WINDOW_TITLE   "Lich's Portfolio"

/* Offline progress configuration */
#define OFFLINE_EFFICIENCY  0.10   /* 10% of normal production */
#define MAX_OFFLINE_HOURS   168.0  /* 1 week cap */

/* Seconds per game year for conversion */
#define SECONDS_PER_YEAR (365.25 * 24.0 * 3600.0)

struct _LpGame
{
    LrgIdle2DTemplate parent_instance;

    LpGameData           *game_data;
    LpAchievementManager *achievement_manager;
};

/*
 * Static instance tracking.
 * Since only one game runs at a time, this provides an easy way for
 * states to access the game without needing singleton machinery.
 */
static LpGame *current_game_instance = NULL;

G_DEFINE_TYPE (LpGame, lp_game, LRG_TYPE_IDLE_2D_TEMPLATE)

/* ==========================================================================
 * Forward Declarations
 * ========================================================================== */

static void lp_game_sync_generators_internal (LpGame *self);

/* ==========================================================================
 * LrgGameTemplate Virtual Method Overrides
 * ========================================================================== */

/*
 * configure:
 *
 * Called before window creation. Configure window properties.
 */
static void
lp_game_real_configure (LrgGameTemplate *template)
{
    lp_log_info ("Configuring Lich's Portfolio...");

    /* Set window properties via parent class */
    lrg_game_template_set_title (template, WINDOW_TITLE);

    /* Set virtual resolution to match window for 1:1 pixel mapping.
     * This game uses fixed pixel positions for UI, not scalable layout. */
    lrg_game_2d_template_set_virtual_resolution (LRG_GAME_2D_TEMPLATE (template),
                                                  WINDOW_WIDTH, WINDOW_HEIGHT);

    /* Configure idle template settings */
    lrg_idle_2d_template_set_offline_efficiency (LRG_IDLE_2D_TEMPLATE (template),
                                                  OFFLINE_EFFICIENCY);
    lrg_idle_2d_template_set_max_offline_hours (LRG_IDLE_2D_TEMPLATE (template),
                                                 MAX_OFFLINE_HOURS);
    lrg_idle_2d_template_set_show_offline_popup (LRG_IDLE_2D_TEMPLATE (template), FALSE);
}

/*
 * pre_startup:
 *
 * Called before initial state is pushed. Initialize subsystems.
 */
static void
lp_game_real_pre_startup (LrgGameTemplate *template)
{
    LpGame *self = LP_GAME (template);
    LrgSettings *settings;
    LpGameplaySettings *gameplay;

    lp_log_info ("Pre-startup: Initializing subsystems...");

    /* Disable raylib's ESC-to-close behavior */
    SetExitKey (KEY_NULL);

    /*
     * Initialize settings system.
     * Register our custom gameplay settings group and load from disk.
     */
    settings = lrg_settings_get_default ();
    gameplay = lp_gameplay_settings_new ();
    lrg_settings_add_group (settings, LRG_SETTINGS_GROUP (gameplay));
    lrg_settings_load_default_path (settings, "lichs-portfolio", NULL);

    lp_log_info ("Settings loaded");

    /* Apply saved graphics settings to window */
    {
        LrgGraphicsSettings *gfx;
        gint width = 0;
        gint height = 0;

        gfx = lrg_settings_get_graphics (settings);
        lrg_graphics_settings_get_resolution (gfx, &width, &height);

        if (width > 0 && height > 0)
        {
            lp_log_info ("Applying saved resolution: %dx%d", width, height);
            lrg_game_template_set_window_size (template, width, height);
            lrg_game_2d_template_set_virtual_resolution (LRG_GAME_2D_TEMPLATE (template),
                                                          width, height);
        }

        /* Apply saved fullscreen setting */
        {
            gboolean saved_fullscreen;
            gboolean current_fullscreen;

            saved_fullscreen = lrg_graphics_settings_get_fullscreen_mode (gfx);
            current_fullscreen = lrg_game_template_is_fullscreen (template);

            if (saved_fullscreen != current_fullscreen)
            {
                lp_log_info ("Applying saved fullscreen: %s",
                             saved_fullscreen ? "On" : "Off");
                lrg_game_template_toggle_fullscreen (template);
            }
        }
    }

    /* Create achievement manager singleton */
    self->achievement_manager = lp_achievement_manager_get_default ();

    /* Game data is created when starting/loading a game, not at startup */
    self->game_data = NULL;

    /* Set as current instance for state access */
    current_game_instance = self;

    /* Set game reference on tutorial sequences */
    lp_tutorial_sequences_set_game (lp_tutorial_sequences_get_default (), self);

    lp_log_info ("Pre-startup complete");
}

/*
 * post_startup:
 *
 * Called after initial state is pushed.
 * Offline progress is handled directly in on_offline_progress_calculated().
 */
static void
lp_game_real_post_startup (LrgGameTemplate *template)
{
    (void)template;

    lp_log_info ("Post-startup complete");
}

/*
 * shutdown:
 *
 * Called during shutdown. Save and cleanup.
 */
static void
lp_game_real_shutdown (LrgGameTemplate *template)
{
    LpGame *self = LP_GAME (template);
    LrgSettings *settings;

    lp_log_info ("Shutting down...");

    /* Autosave game data before shutdown */
    if (self->game_data != NULL)
    {
        LpSaveManager *save_mgr;
        g_autoptr(GError) error = NULL;

        save_mgr = lp_save_manager_get_default ();
        if (!lp_save_manager_autosave (save_mgr, self->game_data, &error))
        {
            lp_log_warning ("Autosave on shutdown failed: %s",
                            error ? error->message : "unknown error");
        }
        else
        {
            lp_log_info ("Autosaved game on shutdown");
        }
    }

    /* Save settings if modified */
    settings = lrg_settings_get_default ();
    if (lrg_settings_is_dirty (settings))
    {
        lp_log_info ("Saving settings...");
        lrg_settings_save_default_path (settings, "lichs-portfolio", NULL);
    }

    /* Clear current instance */
    if (current_game_instance == self)
    {
        current_game_instance = NULL;
    }

    /* Clear game data */
    g_clear_object (&self->game_data);

    /*
     * Achievement manager is a singleton; don't clear it here.
     */
    self->achievement_manager = NULL;

    lp_log_info ("Shutdown complete");
}

/*
 * create_initial_state:
 *
 * Creates the first game state (main menu).
 */
static LrgGameState *
lp_game_real_create_initial_state (LrgGameTemplate *template)
{
    (void)template;

    return LRG_GAME_STATE (lp_state_main_menu_new ());
}

/*
 * create_pause_state:
 *
 * Creates the pause menu state.
 */
static LrgGameState *
lp_game_real_create_pause_state (LrgGameTemplate *template)
{
    (void)template;

    return LRG_GAME_STATE (lp_state_pause_new ());
}

/*
 * create_settings_state:
 *
 * Creates the settings menu state.
 */
static LrgGameState *
lp_game_real_create_settings_state (LrgGameTemplate *template)
{
    (void)template;

    return LRG_GAME_STATE (lp_state_settings_new ());
}

/* ==========================================================================
 * LrgIdleTemplate Virtual Method Overrides
 * ========================================================================== */

/*
 * create_idle_calculator:
 *
 * Creates the idle calculator with investment generators.
 */
static LrgIdleCalculator *
lp_game_real_create_idle_calculator (LrgIdle2DTemplate *template)
{
    LrgIdleCalculator *calc;

    calc = lrg_idle_calculator_new ();

    /*
     * Generators will be added when game data is created/loaded
     * via lp_game_sync_generators().
     */

    return calc;
}

/*
 * create_prestige:
 *
 * Creates the custom prestige layer.
 */
static LrgPrestige *
lp_game_real_create_prestige (LrgIdle2DTemplate *template)
{
    LpPrestige *prestige;

    prestige = lp_prestige_new ();

    /*
     * Phylactery and portfolio will be set when game data is created/loaded.
     */

    return LRG_PRESTIGE (prestige);
}

/*
 * on_offline_progress_calculated:
 *
 * Called when offline progress is calculated. Apply gold and show welcome back.
 */
static void
lp_game_real_on_offline_progress_calculated (LrgIdle2DTemplate  *template,
                                             const LrgBigNumber *progress,
                                             gdouble             seconds_offline)
{
    LpGame *self = LP_GAME (template);
    LpPortfolio *portfolio;
    LrgGameStateManager *manager;
    LpStateWelcomeBack *welcome_back;

    /* Only process if we have game data */
    if (self->game_data == NULL)
    {
        return;
    }

    /* Only process if there's actual progress */
    if (progress == NULL || lrg_big_number_to_double (progress) <= 0.0)
    {
        return;
    }

    lp_log_info ("Offline progress calculated: %s gold over %.0f seconds",
                 lrg_big_number_format_short (progress),
                 seconds_offline);

    /* Add gold to portfolio */
    portfolio = lp_game_data_get_portfolio (self->game_data);
    lp_portfolio_add_gold (portfolio, progress);

    /* Push welcome-back state directly - no storage needed */
    welcome_back = lp_state_welcome_back_new ();
    lp_state_welcome_back_set_offline_data (welcome_back, seconds_offline, progress);

    manager = lrg_game_template_get_state_manager (LRG_GAME_TEMPLATE (template));
    lrg_game_state_manager_push (manager, LRG_GAME_STATE (welcome_back));
}

/*
 * format_big_number:
 *
 * Formats numbers with gold notation.
 */
static gchar *
lp_game_real_format_big_number (LrgIdle2DTemplate  *template,
                                const LrgBigNumber *number)
{
    (void)template;

    /* Use short format with 2 decimal places */
    return lrg_big_number_format_short (number);
}

/*
 * get_offline_efficiency:
 *
 * Returns offline efficiency, potentially modified by phylactery.
 */
static gdouble
lp_game_real_get_offline_efficiency (LrgIdle2DTemplate *template)
{
    LpGame *self = LP_GAME (template);
    gdouble base_efficiency = OFFLINE_EFFICIENCY;

    if (self->game_data != NULL)
    {
        LpPhylactery *phylactery = lp_game_data_get_phylactery (self->game_data);

        /* Time efficiency bonus from phylactery applies to offline too */
        gdouble bonus = lp_phylactery_get_time_efficiency_bonus (phylactery);
        base_efficiency *= bonus;
    }

    return base_efficiency;
}

/*
 * get_max_offline_hours:
 *
 * Returns max offline hours, potentially modified by phylactery.
 */
static gdouble
lp_game_real_get_max_offline_hours (LrgIdle2DTemplate *template)
{
    LpGame *self = LP_GAME (template);
    gdouble base_hours = MAX_OFFLINE_HOURS;

    if (self->game_data != NULL)
    {
        LpPhylactery *phylactery = lp_game_data_get_phylactery (self->game_data);

        /* Temporal mastery might extend max offline time */
        guint max_slumber = lp_phylactery_get_max_slumber_years (phylactery);

        /* Scale: 100 years base = 168 hours, 200 years = 336 hours */
        if (max_slumber > 100)
        {
            base_hours = MAX_OFFLINE_HOURS * ((gdouble)max_slumber / 100.0);
        }
    }

    return base_hours;
}

/* ==========================================================================
 * Generator Sync
 * ========================================================================== */

/*
 * lp_game_sync_generators_internal:
 *
 * Synchronizes investments to idle calculator generators.
 */
static void
lp_game_sync_generators_internal (LpGame *self)
{
    LrgIdleCalculator *calc;
    LpPortfolio       *portfolio;
    GPtrArray         *investments;
    guint              i;

    if (self->game_data == NULL)
    {
        return;
    }

    calc = lrg_idle_2d_template_get_idle_calculator (LRG_IDLE_2D_TEMPLATE (self));
    if (calc == NULL)
    {
        return;
    }

    portfolio = lp_game_data_get_portfolio (self->game_data);
    investments = lp_portfolio_get_investments (portfolio);

    for (i = 0; i < investments->len; i++)
    {
        LpInvestment  *inv;
        const gchar   *id;
        gdouble        base_rate;
        gdouble        current_value;
        gdouble        annual_income;
        gdouble        per_second;

        inv = g_ptr_array_index (investments, i);
        id = lp_investment_get_id (inv);

        /*
         * Calculate annual income for this investment.
         * annual_income = current_value * base_return_rate
         */
        base_rate = lp_investment_get_base_return_rate (inv);
        current_value = lrg_big_number_to_double (lp_investment_get_current_value (inv));
        annual_income = current_value * base_rate;

        /* Convert to per-second rate */
        per_second = annual_income / SECONDS_PER_YEAR;

        /* Add or update generator */
        lrg_idle_2d_template_add_generator (LRG_IDLE_2D_TEMPLATE (self), id, per_second);
        lrg_idle_2d_template_set_generator_count (LRG_IDLE_2D_TEMPLATE (self), id, 1);
    }

    lp_log_debug ("Synced %u investments to idle generators", investments->len);
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_game_dispose (GObject *object)
{
    LpGame *self = LP_GAME (object);

    g_clear_object (&self->game_data);

    if (current_game_instance == self)
    {
        current_game_instance = NULL;
    }

    G_OBJECT_CLASS (lp_game_parent_class)->dispose (object);
}

static void
lp_game_class_init (LpGameClass *klass)
{
    GObjectClass         *object_class = G_OBJECT_CLASS (klass);
    LrgGameTemplateClass *template_class = LRG_GAME_TEMPLATE_CLASS (klass);
    LrgIdle2DTemplateClass *idle_class = LRG_IDLE_2D_TEMPLATE_CLASS (klass);

    object_class->dispose = lp_game_dispose;

    /* Override LrgGameTemplate virtual methods */
    template_class->configure = lp_game_real_configure;
    template_class->pre_startup = lp_game_real_pre_startup;
    template_class->post_startup = lp_game_real_post_startup;
    template_class->shutdown = lp_game_real_shutdown;
    template_class->create_initial_state = lp_game_real_create_initial_state;
    template_class->create_pause_state = lp_game_real_create_pause_state;
    template_class->create_settings_state = lp_game_real_create_settings_state;

    /* Override LrgIdle2DTemplate virtual methods */
    idle_class->create_idle_calculator = lp_game_real_create_idle_calculator;
    idle_class->create_prestige = lp_game_real_create_prestige;
    idle_class->on_offline_progress_calculated = lp_game_real_on_offline_progress_calculated;
    idle_class->format_big_number = lp_game_real_format_big_number;
    idle_class->get_offline_efficiency = lp_game_real_get_offline_efficiency;
    idle_class->get_max_offline_hours = lp_game_real_get_max_offline_hours;
}

static void
lp_game_init (LpGame *self)
{
    self->game_data = NULL;
    self->achievement_manager = NULL;
}

/* ==========================================================================
 * Public API - Construction
 * ========================================================================== */

/**
 * lp_game_new:
 *
 * Creates a new game instance.
 *
 * Returns: (transfer full): A new #LpGame
 */
LpGame *
lp_game_new (void)
{
    return g_object_new (LP_TYPE_GAME,
                         "window-width", WINDOW_WIDTH,
                         "window-height", WINDOW_HEIGHT,
                         NULL);
}

/* ==========================================================================
 * Public API - State Access Helper
 * ========================================================================== */

/**
 * lp_game_get_from_state:
 * @state: an #LrgGameState
 *
 * Gets the game instance from a game state.
 *
 * Returns: (transfer none) (nullable): The #LpGame, or %NULL if not found
 */
LpGame *
lp_game_get_from_state (LrgGameState *state)
{
    (void)state;

    /*
     * Since only one game runs at a time, we use the static instance.
     * This avoids complex plumbing while maintaining a clean API.
     */
    return current_game_instance;
}

/* ==========================================================================
 * Public API - Subsystem Access
 * ========================================================================== */

/**
 * lp_game_get_game_data:
 * @self: an #LpGame
 *
 * Gets the current game data.
 *
 * Returns: (transfer none) (nullable): The #LpGameData, or %NULL
 */
LpGameData *
lp_game_get_game_data (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    return self->game_data;
}

/**
 * lp_game_get_phylactery:
 * @self: an #LpGame
 *
 * Gets the phylactery from the current game data.
 *
 * Returns: (transfer none) (nullable): The #LpPhylactery, or %NULL
 */
LpPhylactery *
lp_game_get_phylactery (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    if (self->game_data == NULL)
    {
        return NULL;
    }

    return lp_game_data_get_phylactery (self->game_data);
}

/**
 * lp_game_get_prestige_layer:
 * @self: an #LpGame
 *
 * Gets the prestige layer from the template.
 *
 * Returns: (transfer none) (nullable): The #LpPrestige, or %NULL
 */
LpPrestige *
lp_game_get_prestige_layer (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    return LP_PRESTIGE (lrg_idle_2d_template_get_prestige (LRG_IDLE_2D_TEMPLATE (self)));
}

/**
 * lp_game_get_achievement_manager:
 * @self: an #LpGame
 *
 * Gets the achievement manager.
 *
 * Returns: (transfer none): The #LpAchievementManager
 */
LpAchievementManager *
lp_game_get_achievement_manager (LpGame *self)
{
    g_return_val_if_fail (LP_IS_GAME (self), NULL);

    return self->achievement_manager;
}

/* ==========================================================================
 * Public API - Game Management
 * ========================================================================== */

/**
 * lp_game_new_game:
 * @self: an #LpGame
 *
 * Starts a new game, creating fresh game data.
 */
void
lp_game_new_game (LpGame *self)
{
    LpPrestige   *prestige;
    LpPortfolio  *portfolio;
    LpPhylactery *phylactery;

    g_return_if_fail (LP_IS_GAME (self));

    lp_log_info ("Starting new game");

    /* Clear any existing game data */
    g_clear_object (&self->game_data);

    /* Create fresh game data */
    self->game_data = lp_game_data_new ();

    /* Configure prestige layer with new game's phylactery and portfolio */
    prestige = lp_game_get_prestige_layer (self);
    if (prestige != NULL)
    {
        portfolio = lp_game_data_get_portfolio (self->game_data);
        phylactery = lp_game_data_get_phylactery (self->game_data);

        lp_prestige_set_portfolio (prestige, portfolio);
        lp_prestige_set_phylactery (prestige, phylactery);
    }

    /* Sync investments to idle calculator */
    lp_game_sync_generators_internal (self);

    lp_log_info ("New game created, starting year: %lu",
                 (gulong)lp_game_data_get_current_year (self->game_data));
}

/**
 * lp_game_load_game:
 * @self: an #LpGame
 * @slot: the save slot to load from
 * @error: (nullable): return location for error
 *
 * Loads a saved game from the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_game_load_game (LpGame   *self,
                   guint     slot,
                   GError  **error)
{
    LpSaveManager *save_mgr;
    LpPrestige    *prestige;
    LpPortfolio   *portfolio;
    LpPhylactery  *phylactery;

    g_return_val_if_fail (LP_IS_GAME (self), FALSE);

    lp_log_info ("Loading game from slot %u", slot);

    save_mgr = lp_save_manager_get_default ();

    /* Clear existing game data and create fresh container for loading */
    g_clear_object (&self->game_data);
    self->game_data = lp_game_data_new ();

    if (!lp_save_manager_load_game (save_mgr, self->game_data, slot, error))
    {
        lp_log_warning ("Failed to load game from slot %u", slot);
        g_clear_object (&self->game_data);
        return FALSE;
    }

    /* Configure prestige layer with loaded game's phylactery and portfolio */
    prestige = lp_game_get_prestige_layer (self);
    if (prestige != NULL)
    {
        portfolio = lp_game_data_get_portfolio (self->game_data);
        phylactery = lp_game_data_get_phylactery (self->game_data);

        lp_prestige_set_portfolio (prestige, portfolio);
        lp_prestige_set_phylactery (prestige, phylactery);
    }

    /* Sync investments to idle calculator */
    lp_game_sync_generators_internal (self);

    lp_log_info ("Game loaded successfully from slot %u", slot);
    return TRUE;
}

/**
 * lp_game_save_game:
 * @self: an #LpGame
 * @slot: the save slot to save to
 * @error: (nullable): return location for error
 *
 * Saves the current game to the specified slot.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
lp_game_save_game (LpGame   *self,
                   guint     slot,
                   GError  **error)
{
    LpSaveManager *save_mgr;

    g_return_val_if_fail (LP_IS_GAME (self), FALSE);

    if (self->game_data == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "No game data to save");
        return FALSE;
    }

    lp_log_info ("Saving game to slot %u", slot);

    save_mgr = lp_save_manager_get_default ();

    if (!lp_save_manager_save_game (save_mgr, self->game_data, slot, error))
    {
        lp_log_warning ("Failed to save game to slot %u", slot);
        return FALSE;
    }

    lp_log_info ("Game saved successfully to slot %u", slot);
    return TRUE;
}

/* ==========================================================================
 * Public API - Idle System Integration
 * ========================================================================== */

/**
 * lp_game_sync_generators:
 * @self: an #LpGame
 *
 * Synchronizes investments to idle calculator generators.
 */
void
lp_game_sync_generators (LpGame *self)
{
    g_return_if_fail (LP_IS_GAME (self));

    lp_game_sync_generators_internal (self);
}

