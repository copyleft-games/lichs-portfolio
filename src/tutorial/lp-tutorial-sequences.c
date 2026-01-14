/* lp-tutorial-sequences.c - Tutorial Sequence Definitions
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-tutorial-sequences.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../investment/lp-portfolio.h"

struct _LpTutorialSequences
{
    GObject             parent_instance;

    /* Tutorial manager reference */
    LrgTutorialManager *manager;

    /* Game reference for accessing game data */
    LpGame             *game;

    /* Data directory */
    gchar              *data_dir;

    /* Initialization state */
    gboolean            initialized;
};

G_DEFINE_FINAL_TYPE (LpTutorialSequences, lp_tutorial_sequences, G_TYPE_OBJECT)

static LpTutorialSequences *default_tutorial_sequences = NULL;

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

static gchar *
get_tutorials_directory (LpTutorialSequences *self)
{
    g_autofree gchar *path = NULL;

    /* Try local data directory first (development) */
    path = g_build_filename (self->data_dir, "tutorials", NULL);
    if (g_file_test (path, G_FILE_TEST_IS_DIR))
    {
        return g_steal_pointer (&path);
    }

    /* Fall back to user data directory */
    return g_build_filename (g_get_user_data_dir (),
                              "lichs-portfolio", "tutorials", NULL);
}

static LrgTutorial *
create_intro_tutorial (void)
{
    LrgTutorial *tutorial;
    LrgTutorialStep *step;

    tutorial = lrg_tutorial_new (LP_TUTORIAL_INTRO, "Introduction");
    lrg_tutorial_set_description (tutorial,
        "Welcome to the eternal game of wealth and power.");
    lrg_tutorial_set_skippable (tutorial, TRUE);
    lrg_tutorial_set_repeatable (tutorial, FALSE);

    /*
     * Step 1: Narrative introduction
     * Malachar welcomes the player to their awakening.
     */
    step = lrg_tutorial_step_new_text (
        "Ah, you've awakened. I am Malachar, your guide through the "
        "centuries. As an undead lich, you possess a unique advantage: "
        "immortality. Use it wisely.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "welcome");
    lrg_tutorial_step_set_blocks_input (step, TRUE);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 2: Explain gold display
     * Highlight the resource counter.
     */
    step = lrg_tutorial_step_new_text (
        "Your gold reserves are displayed here. Gold is the foundation "
        "of all power in the mortal realm. Guard it jealously.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "gold-intro");
    lrg_tutorial_step_set_target_id (step, "gold-display");
    lrg_tutorial_step_set_highlight_style (step, LRG_HIGHLIGHT_STYLE_GLOW);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 3: Point to portfolio
     * Guide player to their first investment screen.
     */
    step = lrg_tutorial_step_new_text (
        "Your portfolio awaits. Here you shall make investments that "
        "will grow while you slumber. Click to proceed.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "portfolio-pointer");
    lrg_tutorial_step_set_target_id (step, "portfolio-button");
    lrg_tutorial_step_set_highlight_style (step, LRG_HIGHLIGHT_STYLE_SPOTLIGHT);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    return tutorial;
}

static LrgTutorial *
create_investment_tutorial (void)
{
    LrgTutorial *tutorial;
    LrgTutorialStep *step;

    tutorial = lrg_tutorial_new (LP_TUTORIAL_INVESTMENT, "Investment Basics");
    lrg_tutorial_set_description (tutorial,
        "Learn to build wealth through careful investment.");
    lrg_tutorial_set_skippable (tutorial, TRUE);
    lrg_tutorial_set_repeatable (tutorial, FALSE);

    /*
     * Step 1: Explain investments
     * Overview of the investment system.
     */
    step = lrg_tutorial_step_new_text (
        "Investments are the instruments of our eternal wealth. "
        "Each asset class carries different risks and rewards.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "investment-intro");
    lrg_tutorial_step_set_blocks_input (step, TRUE);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 2: Highlight available investments
     * Show the investment list.
     */
    step = lrg_tutorial_step_new_text (
        "Here you see available investments. Properties provide "
        "steady returns. Trade routes are riskier but more profitable. "
        "Financial instruments offer leverage.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "investment-list");
    lrg_tutorial_step_set_target_id (step, "investment-list");
    lrg_tutorial_step_set_highlight_style (step, LRG_HIGHLIGHT_STYLE_OUTLINE);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 3: Guide first purchase
     * Encourage making an investment.
     */
    step = lrg_tutorial_step_new_text (
        "Select an investment to begin. I recommend starting with "
        "something modest - a small property, perhaps. We have "
        "eternity to grow our empire.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "first-purchase-prompt");
    lrg_tutorial_step_set_target_id (step, "buy-button");
    lrg_tutorial_step_set_highlight_style (step, LRG_HIGHLIGHT_STYLE_SPOTLIGHT);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 4: Wait for investment
     * Condition step - advances when player has an investment.
     */
    step = lrg_tutorial_step_new_condition (LP_CONDITION_HAS_INVESTMENT);
    lrg_tutorial_step_set_id (step, "wait-for-investment");
    lrg_tutorial_step_set_auto_advance (step, TRUE);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 5: Congratulations
     * Celebrate first investment.
     */
    step = lrg_tutorial_step_new_text (
        "Excellent. Your first investment. The foundation of an "
        "eternal empire. Now, let us discuss the art of slumber...",
        "Malachar");
    lrg_tutorial_step_set_id (step, "investment-success");
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    return tutorial;
}

static LrgTutorial *
create_slumber_tutorial (void)
{
    LrgTutorial *tutorial;
    LrgTutorialStep *step;

    tutorial = lrg_tutorial_new (LP_TUTORIAL_SLUMBER, "The Art of Slumber");
    lrg_tutorial_set_description (tutorial,
        "Master time itself through the slumber mechanic.");
    lrg_tutorial_set_skippable (tutorial, TRUE);
    lrg_tutorial_set_repeatable (tutorial, FALSE);

    /*
     * Step 1: Explain slumber concept
     * Introduction to time passage.
     */
    step = lrg_tutorial_step_new_text (
        "As an immortal, you possess a power mortals can only dream of: "
        "the ability to skip through time. During slumber, your "
        "investments grow while the world ages.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "slumber-intro");
    lrg_tutorial_step_set_blocks_input (step, TRUE);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 2: Duration selector
     * Explain slumber duration options.
     */
    step = lrg_tutorial_step_new_text (
        "Choose how long to slumber. Decades pass in an instant. "
        "But beware - events may occur while you sleep. Kingdoms "
        "rise and fall. Agents age and die.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "duration-intro");
    lrg_tutorial_step_set_target_id (step, "duration-selector");
    lrg_tutorial_step_set_highlight_style (step, LRG_HIGHLIGHT_STYLE_OUTLINE);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 3: Explain dormant orders
     * Standing orders during slumber.
     */
    step = lrg_tutorial_step_new_text (
        "Before slumbering, you may set dormant orders. These are "
        "instructions your agents will follow while you sleep. "
        "Reinvest profits, sell failing assets, respond to events.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "dormant-orders");
    lrg_tutorial_step_set_target_id (step, "dormant-orders-panel");
    lrg_tutorial_step_set_highlight_style (step, LRG_HIGHLIGHT_STYLE_OUTLINE);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    /*
     * Step 4: Encourage first slumber
     * Final prompt.
     */
    step = lrg_tutorial_step_new_text (
        "When ready, enter the slumber. Time will pass, your "
        "investments will grow, and the world will change. "
        "Such is the privilege of immortality.",
        "Malachar");
    lrg_tutorial_step_set_id (step, "slumber-prompt");
    lrg_tutorial_step_set_target_id (step, "slumber-button");
    lrg_tutorial_step_set_highlight_style (step, LRG_HIGHLIGHT_STYLE_SPOTLIGHT);
    lrg_tutorial_add_step (tutorial, step);
    lrg_tutorial_step_free (step);

    return tutorial;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_tutorial_sequences_finalize (GObject *object)
{
    LpTutorialSequences *self = LP_TUTORIAL_SEQUENCES (object);

    g_clear_pointer (&self->data_dir, g_free);

    G_OBJECT_CLASS (lp_tutorial_sequences_parent_class)->finalize (object);
}

static void
lp_tutorial_sequences_class_init (LpTutorialSequencesClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_tutorial_sequences_finalize;
}

static void
lp_tutorial_sequences_init (LpTutorialSequences *self)
{
    self->initialized = FALSE;
    self->game = NULL;

    /* Determine data directory */
    if (g_file_test ("data/tutorials", G_FILE_TEST_IS_DIR))
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
 * lp_tutorial_sequences_get_default:
 *
 * Gets the default tutorial sequences instance.
 * Initializes and registers all tutorials on first call.
 *
 * Returns: (transfer none): The default #LpTutorialSequences
 */
LpTutorialSequences *
lp_tutorial_sequences_get_default (void)
{
    if (default_tutorial_sequences == NULL)
    {
        default_tutorial_sequences = g_object_new (LP_TYPE_TUTORIAL_SEQUENCES, NULL);
    }

    return default_tutorial_sequences;
}

/**
 * lp_tutorial_sequences_set_game:
 * @self: an #LpTutorialSequences
 * @game: the game instance to use for accessing game data
 *
 * Sets the game reference used for condition checking.
 * Must be called before using tutorials that need game data.
 */
void
lp_tutorial_sequences_set_game (LpTutorialSequences *self,
                                 LpGame              *game)
{
    g_return_if_fail (LP_IS_TUTORIAL_SEQUENCES (self));

    self->game = game;
}

/* ==========================================================================
 * Tutorial Control
 * ========================================================================== */

/**
 * lp_tutorial_sequences_init_tutorials:
 * @self: an #LpTutorialSequences
 * @manager: the #LrgTutorialManager to register with
 * @error: (nullable): return location for error
 *
 * Loads and registers all game tutorials with the manager.
 * Should be called during application initialization.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_tutorial_sequences_init_tutorials (LpTutorialSequences  *self,
                                       LrgTutorialManager   *manager,
                                       GError              **error)
{
    g_autoptr(LrgTutorial) intro = NULL;
    g_autoptr(LrgTutorial) investment = NULL;
    g_autoptr(LrgTutorial) slumber = NULL;
    g_autofree gchar *tutorials_dir = NULL;

    g_return_val_if_fail (LP_IS_TUTORIAL_SEQUENCES (self), FALSE);
    g_return_val_if_fail (LRG_IS_TUTORIAL_MANAGER (manager), FALSE);

    if (self->initialized)
        return TRUE;

    self->manager = manager;

    /* Try to load from YAML files first */
    tutorials_dir = get_tutorials_directory (self);
    if (g_file_test (tutorials_dir, G_FILE_TEST_IS_DIR))
    {
        guint count = lrg_tutorial_manager_load_from_directory (manager,
                                                                  tutorials_dir,
                                                                  NULL);
        if (count > 0)
        {
            lp_log_debug ("Loaded %u tutorials from %s", count, tutorials_dir);
            self->initialized = TRUE;
            return TRUE;
        }
    }

    /*
     * Create tutorials programmatically as fallback.
     * This ensures tutorials work even without data files.
     */
    lp_log_debug ("Creating tutorials programmatically");

    intro = create_intro_tutorial ();
    if (!lrg_tutorial_manager_register (manager, intro))
    {
        lp_log_debug ("Failed to register intro tutorial");
    }

    investment = create_investment_tutorial ();
    if (!lrg_tutorial_manager_register (manager, investment))
    {
        lp_log_debug ("Failed to register investment tutorial");
    }

    slumber = create_slumber_tutorial ();
    if (!lrg_tutorial_manager_register (manager, slumber))
    {
        lp_log_debug ("Failed to register slumber tutorial");
    }

    /* Set up condition callback */
    lrg_tutorial_manager_set_condition_callback (
        manager,
        lp_tutorial_sequences_check_condition,
        self,
        NULL);

    self->initialized = TRUE;
    return TRUE;
}

/**
 * lp_tutorial_sequences_maybe_start_intro:
 * @self: an #LpTutorialSequences
 *
 * Starts the intro tutorial if this is a new game.
 * Safe to call multiple times - checks completion status.
 */
void
lp_tutorial_sequences_maybe_start_intro (LpTutorialSequences *self)
{
    LpGameData *data;

    g_return_if_fail (LP_IS_TUTORIAL_SEQUENCES (self));
    g_return_if_fail (self->manager != NULL);
    g_return_if_fail (self->game != NULL);

    /* Check if already completed */
    if (lrg_tutorial_manager_is_completed (self->manager, LP_TUTORIAL_INTRO))
        return;

    /* Check if this is a new game (no years played) */
    data = lp_game_get_game_data (self->game);
    if (data == NULL || lp_game_data_get_total_years_played (data) > 0)
        return;

    lrg_tutorial_manager_start_tutorial (self->manager, LP_TUTORIAL_INTRO);
}

/**
 * lp_tutorial_sequences_maybe_start_investment:
 * @self: an #LpTutorialSequences
 *
 * Starts the investment tutorial on first portfolio visit.
 * Safe to call multiple times - checks completion status.
 */
void
lp_tutorial_sequences_maybe_start_investment (LpTutorialSequences *self)
{
    g_return_if_fail (LP_IS_TUTORIAL_SEQUENCES (self));
    g_return_if_fail (self->manager != NULL);

    /* Check if already completed */
    if (lrg_tutorial_manager_is_completed (self->manager, LP_TUTORIAL_INVESTMENT))
        return;

    /* Only show after intro is complete */
    if (!lrg_tutorial_manager_is_completed (self->manager, LP_TUTORIAL_INTRO))
        return;

    lrg_tutorial_manager_start_tutorial (self->manager, LP_TUTORIAL_INVESTMENT);
}

/**
 * lp_tutorial_sequences_maybe_start_slumber:
 * @self: an #LpTutorialSequences
 *
 * Starts the slumber tutorial on first slumber attempt.
 * Safe to call multiple times - checks completion status.
 */
void
lp_tutorial_sequences_maybe_start_slumber (LpTutorialSequences *self)
{
    g_return_if_fail (LP_IS_TUTORIAL_SEQUENCES (self));
    g_return_if_fail (self->manager != NULL);

    /* Check if already completed */
    if (lrg_tutorial_manager_is_completed (self->manager, LP_TUTORIAL_SLUMBER))
        return;

    /* Only show after investment tutorial is complete */
    if (!lrg_tutorial_manager_is_completed (self->manager, LP_TUTORIAL_INVESTMENT))
        return;

    lrg_tutorial_manager_start_tutorial (self->manager, LP_TUTORIAL_SLUMBER);
}

/**
 * lp_tutorial_sequences_check_condition:
 * @condition_id: the condition ID to check
 * @user_data: user data (unused)
 *
 * Callback for tutorial condition checking.
 * Used by LrgTutorialManager to evaluate step conditions.
 *
 * Returns: %TRUE if the condition is met
 */
gboolean
lp_tutorial_sequences_check_condition (const gchar *condition_id,
                                        gpointer     user_data)
{
    LpTutorialSequences *self = LP_TUTORIAL_SEQUENCES (user_data);
    LpGameData *data;
    LpPortfolio *portfolio;

    g_return_val_if_fail (condition_id != NULL, FALSE);

    /* Gracefully handle NULL context - can happen if called without proper setup */
    if (self == NULL || self->game == NULL)
        return FALSE;

    data = lp_game_get_game_data (self->game);
    if (data == NULL)
        return FALSE;

    portfolio = lp_game_data_get_portfolio (data);
    if (portfolio == NULL)
        return FALSE;

    if (g_strcmp0 (condition_id, LP_CONDITION_HAS_GOLD) == 0)
    {
        /* Check if player has any gold */
        g_autoptr(LrgBigNumber) gold = lp_portfolio_get_gold (portfolio);
        return gold != NULL && !lrg_big_number_is_zero (gold);
    }
    else if (g_strcmp0 (condition_id, LP_CONDITION_HAS_INVESTMENT) == 0)
    {
        /* Check if portfolio has any investments */
        return lp_portfolio_get_investment_count (portfolio) > 0;
    }
    else if (g_strcmp0 (condition_id, LP_CONDITION_PORTFOLIO_OPEN) == 0)
    {
        /* This would be set by the UI when portfolio screen is visible */
        /* For now, return FALSE - would need UI integration */
        return FALSE;
    }
    else if (g_strcmp0 (condition_id, LP_CONDITION_SLUMBER_SELECTED) == 0)
    {
        /* This would be set by the UI when slumber duration is selected */
        /* For now, return FALSE - would need UI integration */
        return FALSE;
    }

    return FALSE;
}
