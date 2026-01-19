/* lp-state-settings.c - Settings Menu Overlay State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-settings.h"
#include "../core/lp-game.h"
#include "../core/lp-gameplay-settings.h"
#include "../lp-input-helpers.h"
#include <graylib.h>
#include <libregnum.h>

/* Resolution options */
static const struct {
    gint width;
    gint height;
} resolutions[] = {
    { 1280, 720 },
    { 1920, 1080 },
    { 2560, 1440 }
};

/* Settings tabs */
typedef enum
{
    SETTINGS_TAB_GRAPHICS,
    SETTINGS_TAB_AUDIO,
    SETTINGS_TAB_GAMEPLAY,
    SETTINGS_TAB_CONTROLS,
    SETTINGS_TAB_COUNT
} SettingsTab;

/* Number of options per tab */
#define GRAPHICS_OPTIONS 3
#define AUDIO_OPTIONS    3
#define GAMEPLAY_OPTIONS 4
#define CONTROLS_OPTIONS 0  /* Read-only */

struct _LpStateSettings
{
    LrgGameState parent_instance;

    SettingsTab current_tab;
    gint        selected_option;

    /* Graphics settings */
    gint  resolution_idx;      /* 0=1280x720, 1=1920x1080, 2=2560x1440 */
    gboolean fullscreen;
    gboolean vsync;

    /* Audio settings */
    gint  master_volume;       /* 0-100 */
    gint  music_volume;        /* 0-100 */
    gint  sfx_volume;          /* 0-100 */

    /* Gameplay settings */
    gboolean auto_save;
    gboolean tutorials;
    gint  difficulty;          /* 0=Easy, 1=Normal, 2=Hard */
    gint  game_speed;          /* 0=1x, 1=2x, 2=4x, 3=10x */

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpStateSettings, lp_state_settings, LRG_TYPE_GAME_STATE)

/* ==========================================================================
 * Label Helpers
 * ========================================================================== */

static void
draw_label (LrgLabel       *label,
            const gchar    *text,
            gfloat          x,
            gfloat          y,
            gfloat          font_size,
            const GrlColor *color)
{
    lrg_label_set_text (label, text);
    lrg_widget_set_position (LRG_WIDGET (label), x, y);
    lrg_label_set_font_size (label, font_size);
    lrg_label_set_color (label, color);
    lrg_widget_draw (LRG_WIDGET (label));
}

static LrgLabel *
get_pool_label (LpStateSettings *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateSettings *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_settings_dispose (GObject *object)
{
    LpStateSettings *self = LP_STATE_SETTINGS (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_settings_parent_class)->dispose (object);
}

/* ==========================================================================
 * Settings Access Helpers
 * ========================================================================== */

static LrgGraphicsSettings *
get_graphics_settings (void)
{
    LrgSettings *settings = lrg_settings_get_default ();
    return LRG_GRAPHICS_SETTINGS (lrg_settings_get_group (settings, "graphics"));
}

static LrgAudioSettings *
get_audio_settings (void)
{
    LrgSettings *settings = lrg_settings_get_default ();
    return LRG_AUDIO_SETTINGS (lrg_settings_get_group (settings, "audio"));
}

static LpGameplaySettings *
get_gameplay_settings (void)
{
    LrgSettings *settings = lrg_settings_get_default ();
    return LP_GAMEPLAY_SETTINGS (lrg_settings_get_group (settings, "gameplay"));
}

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

static gint
resolution_to_index (gint width, gint height)
{
    gint i;

    for (i = 0; i < 3; i++)
    {
        if (resolutions[i].width == width && resolutions[i].height == height)
            return i;
    }
    return 0;  /* Default to 1280x720 */
}

static gint
get_option_count (SettingsTab tab)
{
    switch (tab)
    {
    case SETTINGS_TAB_GRAPHICS:
        return GRAPHICS_OPTIONS;
    case SETTINGS_TAB_AUDIO:
        return AUDIO_OPTIONS;
    case SETTINGS_TAB_GAMEPLAY:
        return GAMEPLAY_OPTIONS;
    case SETTINGS_TAB_CONTROLS:
        return CONTROLS_OPTIONS;
    default:
        return 0;
    }
}

static void
apply_resolution (LpStateSettings *self)
{
    LpGame *game;
    gint width, height;

    width = resolutions[self->resolution_idx].width;
    height = resolutions[self->resolution_idx].height;

    game = lp_game_get_from_state (LRG_GAME_STATE (self));

    /* Update both window size AND virtual resolution for 1:1 pixel mapping */
    lrg_game_template_set_window_size (LRG_GAME_TEMPLATE (game), width, height);
    lrg_game_2d_template_set_virtual_resolution (LRG_GAME_2D_TEMPLATE (game),
                                                  width, height);

    lp_log_info ("Resolution changed to: %dx%d", width, height);
}

static void
apply_fullscreen (LpStateSettings *self)
{
    LpGame *game;
    gboolean is_full;

    game = lp_game_get_from_state (LRG_GAME_STATE (self));
    is_full = lrg_game_template_is_fullscreen (LRG_GAME_TEMPLATE (game));

    /* Only toggle if current state doesn't match desired state */
    if (is_full != self->fullscreen)
    {
        lrg_game_template_toggle_fullscreen (LRG_GAME_TEMPLATE (game));
        lp_log_info ("Fullscreen toggled: %s", self->fullscreen ? "On" : "Off");
    }
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_settings_enter (LrgGameState *state)
{
    LpStateSettings *self = LP_STATE_SETTINGS (state);
    LrgGraphicsSettings *graphics;
    LrgAudioSettings *audio;
    LpGameplaySettings *gameplay;

    lp_log_info ("Entering settings");

    self->current_tab = SETTINGS_TAB_GRAPHICS;
    self->selected_option = 0;

    /* Load current values from settings groups */
    graphics = get_graphics_settings ();
    audio = get_audio_settings ();
    gameplay = get_gameplay_settings ();

    /* Graphics settings */
    if (graphics != NULL)
    {
        gint width, height;

        lrg_graphics_settings_get_resolution (graphics, &width, &height);
        self->resolution_idx = resolution_to_index (width, height);
        self->fullscreen = lrg_graphics_settings_get_fullscreen_mode (graphics);
        self->vsync = lrg_graphics_settings_get_vsync (graphics);
    }

    /* Audio settings */
    if (audio != NULL)
    {
        /* Convert 0.0-1.0 to 0-100 percentage */
        self->master_volume = (gint)(lrg_audio_settings_get_master_volume (audio) * 100.0);
        self->music_volume = (gint)(lrg_audio_settings_get_music_volume (audio) * 100.0);
        self->sfx_volume = (gint)(lrg_audio_settings_get_sfx_volume (audio) * 100.0);
    }

    /* Gameplay settings */
    if (gameplay != NULL)
    {
        self->auto_save = lp_gameplay_settings_get_autosave_enabled (gameplay);
        self->tutorials = lp_gameplay_settings_get_show_notifications (gameplay);
        self->difficulty = (gint)lp_gameplay_settings_get_difficulty (gameplay);
        self->game_speed = (gint)lp_gameplay_settings_get_game_speed (gameplay);
    }
}

static void
lp_state_settings_exit (LrgGameState *state)
{
    lp_log_info ("Exiting settings");
}

static void
lp_state_settings_update (LrgGameState *state,
                          gdouble       delta)
{
    LpStateSettings *self = LP_STATE_SETTINGS (state);
    gint option_count;

    (void)delta;

    option_count = get_option_count (self->current_tab);

    /* Navigate tabs with TAB/RB (next) or LB (previous) */
    if (LP_INPUT_TAB_NEXT_PRESSED () ||
        grl_input_is_key_pressed (GRL_KEY_H) ||
        grl_input_is_key_pressed (GRL_KEY_L))
    {
        self->current_tab++;
        if (self->current_tab >= SETTINGS_TAB_COUNT)
        {
            self->current_tab = 0;
        }
        self->selected_option = 0;  /* Reset selection when changing tabs */
    }

    if (LP_INPUT_TAB_PREV_PRESSED ())
    {
        self->current_tab--;
        if (self->current_tab < 0)
        {
            self->current_tab = SETTINGS_TAB_COUNT - 1;
        }
        self->selected_option = 0;
    }

    /* Navigate options with UP/DOWN (including vim keys and gamepad D-pad) */
    if (option_count > 0)
    {
        if (LP_INPUT_NAV_UP_PRESSED ())
        {
            self->selected_option--;
            if (self->selected_option < 0)
            {
                self->selected_option = option_count - 1;
            }
        }

        if (LP_INPUT_NAV_DOWN_PRESSED ())
        {
            self->selected_option++;
            if (self->selected_option >= option_count)
            {
                self->selected_option = 0;
            }
        }
    }

    /* Change values with LEFT/RIGHT (keyboard A/D or gamepad D-pad) */
    if (LP_INPUT_VALUE_DEC_PRESSED ())
    {
        switch (self->current_tab)
        {
        case SETTINGS_TAB_GRAPHICS:
            if (self->selected_option == 0)  /* Resolution */
            {
                if (self->resolution_idx > 0)
                {
                    LrgGraphicsSettings *gfx = get_graphics_settings ();
                    self->resolution_idx--;
                    if (gfx != NULL)
                        lrg_graphics_settings_set_resolution (gfx,
                            resolutions[self->resolution_idx].width,
                            resolutions[self->resolution_idx].height);
                    apply_resolution (self);
                }
            }
            else if (self->selected_option == 1)  /* Fullscreen */
            {
                LrgGraphicsSettings *gfx = get_graphics_settings ();
                self->fullscreen = !self->fullscreen;
                if (gfx != NULL)
                    lrg_graphics_settings_set_fullscreen_mode (gfx, self->fullscreen);
                apply_fullscreen (self);
            }
            else if (self->selected_option == 2)  /* VSync */
            {
                LrgGraphicsSettings *gfx = get_graphics_settings ();
                self->vsync = !self->vsync;
                if (gfx != NULL)
                    lrg_graphics_settings_set_vsync (gfx, self->vsync);
            }
            break;

        case SETTINGS_TAB_AUDIO:
            if (self->selected_option == 0)  /* Master */
            {
                LrgAudioSettings *aud = get_audio_settings ();
                self->master_volume = MAX (0, self->master_volume - 10);
                if (aud != NULL)
                    lrg_audio_settings_set_master_volume (aud, self->master_volume / 100.0);
            }
            else if (self->selected_option == 1)  /* Music */
            {
                LrgAudioSettings *aud = get_audio_settings ();
                self->music_volume = MAX (0, self->music_volume - 10);
                if (aud != NULL)
                    lrg_audio_settings_set_music_volume (aud, self->music_volume / 100.0);
            }
            else if (self->selected_option == 2)  /* SFX */
            {
                LrgAudioSettings *aud = get_audio_settings ();
                self->sfx_volume = MAX (0, self->sfx_volume - 10);
                if (aud != NULL)
                    lrg_audio_settings_set_sfx_volume (aud, self->sfx_volume / 100.0);
            }
            break;

        case SETTINGS_TAB_GAMEPLAY:
            if (self->selected_option == 0)  /* Auto-Save */
            {
                LpGameplaySettings *gp = get_gameplay_settings ();
                self->auto_save = !self->auto_save;
                if (gp != NULL)
                    lp_gameplay_settings_set_autosave_enabled (gp, self->auto_save);
            }
            else if (self->selected_option == 1)  /* Tutorials */
            {
                LpGameplaySettings *gp = get_gameplay_settings ();
                self->tutorials = !self->tutorials;
                if (gp != NULL)
                    lp_gameplay_settings_set_show_notifications (gp, self->tutorials);
            }
            else if (self->selected_option == 2)  /* Difficulty */
            {
                if (self->difficulty > 0)
                {
                    LpGameplaySettings *gp = get_gameplay_settings ();
                    self->difficulty--;
                    if (gp != NULL)
                        lp_gameplay_settings_set_difficulty (gp, (LpDifficulty)self->difficulty);
                }
            }
            else if (self->selected_option == 3)  /* Game Speed */
            {
                if (self->game_speed > 0)
                {
                    LpGameplaySettings *gp = get_gameplay_settings ();
                    self->game_speed--;
                    if (gp != NULL)
                        lp_gameplay_settings_set_game_speed (gp, (LpGameSpeed)self->game_speed);
                }
            }
            break;

        default:
            break;
        }
    }

    if (LP_INPUT_VALUE_INC_PRESSED ())
    {
        switch (self->current_tab)
        {
        case SETTINGS_TAB_GRAPHICS:
            if (self->selected_option == 0)  /* Resolution */
            {
                if (self->resolution_idx < 2)
                {
                    LrgGraphicsSettings *gfx = get_graphics_settings ();
                    self->resolution_idx++;
                    if (gfx != NULL)
                        lrg_graphics_settings_set_resolution (gfx,
                            resolutions[self->resolution_idx].width,
                            resolutions[self->resolution_idx].height);
                    apply_resolution (self);
                }
            }
            else if (self->selected_option == 1)  /* Fullscreen */
            {
                LrgGraphicsSettings *gfx = get_graphics_settings ();
                self->fullscreen = !self->fullscreen;
                if (gfx != NULL)
                    lrg_graphics_settings_set_fullscreen_mode (gfx, self->fullscreen);
                apply_fullscreen (self);
            }
            else if (self->selected_option == 2)  /* VSync */
            {
                LrgGraphicsSettings *gfx = get_graphics_settings ();
                self->vsync = !self->vsync;
                if (gfx != NULL)
                    lrg_graphics_settings_set_vsync (gfx, self->vsync);
            }
            break;

        case SETTINGS_TAB_AUDIO:
            if (self->selected_option == 0)  /* Master */
            {
                LrgAudioSettings *aud = get_audio_settings ();
                self->master_volume = MIN (100, self->master_volume + 10);
                if (aud != NULL)
                    lrg_audio_settings_set_master_volume (aud, self->master_volume / 100.0);
            }
            else if (self->selected_option == 1)  /* Music */
            {
                LrgAudioSettings *aud = get_audio_settings ();
                self->music_volume = MIN (100, self->music_volume + 10);
                if (aud != NULL)
                    lrg_audio_settings_set_music_volume (aud, self->music_volume / 100.0);
            }
            else if (self->selected_option == 2)  /* SFX */
            {
                LrgAudioSettings *aud = get_audio_settings ();
                self->sfx_volume = MIN (100, self->sfx_volume + 10);
                if (aud != NULL)
                    lrg_audio_settings_set_sfx_volume (aud, self->sfx_volume / 100.0);
            }
            break;

        case SETTINGS_TAB_GAMEPLAY:
            if (self->selected_option == 0)  /* Auto-Save */
            {
                LpGameplaySettings *gp = get_gameplay_settings ();
                self->auto_save = !self->auto_save;
                if (gp != NULL)
                    lp_gameplay_settings_set_autosave_enabled (gp, self->auto_save);
            }
            else if (self->selected_option == 1)  /* Tutorials */
            {
                LpGameplaySettings *gp = get_gameplay_settings ();
                self->tutorials = !self->tutorials;
                if (gp != NULL)
                    lp_gameplay_settings_set_show_notifications (gp, self->tutorials);
            }
            else if (self->selected_option == 2)  /* Difficulty */
            {
                if (self->difficulty < 2)
                {
                    LpGameplaySettings *gp = get_gameplay_settings ();
                    self->difficulty++;
                    if (gp != NULL)
                        lp_gameplay_settings_set_difficulty (gp, (LpDifficulty)self->difficulty);
                }
            }
            else if (self->selected_option == 3)  /* Game Speed */
            {
                if (self->game_speed < 3)
                {
                    LpGameplaySettings *gp = get_gameplay_settings ();
                    self->game_speed++;
                    if (gp != NULL)
                        lp_gameplay_settings_set_game_speed (gp, (LpGameSpeed)self->game_speed);
                }
            }
            break;

        default:
            break;
        }
    }

    /* Cancel (ESC or B button) to return to previous state */
    if (LP_INPUT_CANCEL_PRESSED ())
    {
        LpGame *game;
        LrgGameStateManager *manager;

        lp_log_info ("Returning from settings");

        game = lp_game_get_from_state (state);
        manager = lrg_game_template_get_state_manager (
            LRG_GAME_TEMPLATE (game));
        lrg_game_state_manager_pop (manager);
    }
}

static void
draw_option (LpStateSettings *self,
             gint             base_x,
             const gchar     *option_label,
             const gchar     *value,
             gint             y,
             gboolean         selected,
             GrlColor        *text_color,
             GrlColor        *selected_color,
             GrlColor        *value_color)
{
    GrlColor *label_c = selected ? selected_color : text_color;

    if (selected)
    {
        draw_label (get_pool_label (self), ">", base_x, y, 18, selected_color);
    }

    draw_label (get_pool_label (self), option_label, base_x + 20, y, 18, label_c);
    draw_label (get_pool_label (self), value, base_x + 280, y, 18, value_color);

    if (selected)
    {
        draw_label (get_pool_label (self), "<", base_x + 260, y, 18, selected_color);
        draw_label (get_pool_label (self), ">", base_x + 380, y, 18, selected_color);
    }
}

static void
lp_state_settings_draw (LrgGameState *state)
{
    LpStateSettings *self = LP_STATE_SETTINGS (state);
    LpGame *game = lp_game_get_from_state (state);
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) panel_color = NULL;
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) selected_color = NULL;
    g_autoptr(GrlColor) value_color = NULL;
    gint screen_w, screen_h;
    gint center_x, center_y;
    gint panel_x, panel_y, panel_w, panel_h;
    gint content_x, content_y;
    gint i;
    gchar value_str[64];

    const gchar *tab_names[] = {
        "Graphics",
        "Audio",
        "Gameplay",
        "Controls"
    };

    const gchar *resolution_strs[] = {
        "1280x720",
        "1920x1080",
        "2560x1440"
    };

    const gchar *difficulties[] = {
        "Easy",
        "Normal",
        "Hard"
    };

    const gchar *game_speeds[] = {
        "1x (Normal)",
        "2x (Fast)",
        "4x (Faster)",
        "10x (Fastest)"
    };

    /* Reset label pool for this frame */
    reset_label_pool (self);

    /* Get virtual resolution (render target size) for UI positioning */
    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
    center_x = screen_w / 2;
    center_y = screen_h / 2;

    /* Calculate panel dimensions (80% of screen) */
    panel_w = (screen_w * 80) / 100;
    panel_h = (screen_h * 55) / 100;
    panel_x = (screen_w - panel_w) / 2;
    panel_y = center_y - (panel_h / 2) + 40;

    /* Content starts inside the panel */
    content_x = panel_x + 40;
    content_y = panel_y + 40;

    /* Colors */
    bg_color = grl_color_new (20, 20, 30, 240);
    panel_color = grl_color_new (30, 30, 45, 255);
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    selected_color = grl_color_new (255, 215, 0, 255);
    value_color = grl_color_new (150, 200, 150, 255);

    /* Draw semi-transparent background */
    grl_draw_rectangle (0, 0, screen_w, screen_h, bg_color);

    /* Draw title */
    draw_label (self->label_title, "SETTINGS",
                center_x - 80, panel_y - 60, 40, title_color);

    /* Draw tabs */
    for (i = 0; i < SETTINGS_TAB_COUNT; i++)
    {
        GrlColor *color;
        gint tab_x;
        gint tab_spacing;

        tab_spacing = panel_w / SETTINGS_TAB_COUNT;
        tab_x = panel_x + (i * tab_spacing) + 20;
        color = (i == (gint)self->current_tab) ? selected_color : text_color;

        draw_label (get_pool_label (self), tab_names[i], tab_x, panel_y - 20, 20, color);

        if (i == (gint)self->current_tab)
        {
            /* Underline current tab */
            grl_draw_rectangle (tab_x, panel_y - 0, 80, 2, selected_color);
        }
    }

    /* Draw content panel */
    grl_draw_rectangle (panel_x, panel_y, panel_w, panel_h, panel_color);

    /* Draw content for selected tab */
    switch (self->current_tab)
    {
    case SETTINGS_TAB_GRAPHICS:
        draw_option (self, content_x, "Resolution:", resolution_strs[self->resolution_idx],
                     content_y, self->selected_option == 0,
                     text_color, selected_color, value_color);

        draw_option (self, content_x, "Fullscreen:", self->fullscreen ? "On" : "Off",
                     content_y + 40, self->selected_option == 1,
                     text_color, selected_color, value_color);

        draw_option (self, content_x, "VSync:", self->vsync ? "On" : "Off",
                     content_y + 80, self->selected_option == 2,
                     text_color, selected_color, value_color);
        break;

    case SETTINGS_TAB_AUDIO:
        g_snprintf (value_str, sizeof (value_str), "%d%%", self->master_volume);
        draw_option (self, content_x, "Master Volume:", value_str,
                     content_y, self->selected_option == 0,
                     text_color, selected_color, value_color);

        g_snprintf (value_str, sizeof (value_str), "%d%%", self->music_volume);
        draw_option (self, content_x, "Music Volume:", value_str,
                     content_y + 40, self->selected_option == 1,
                     text_color, selected_color, value_color);

        g_snprintf (value_str, sizeof (value_str), "%d%%", self->sfx_volume);
        draw_option (self, content_x, "SFX Volume:", value_str,
                     content_y + 80, self->selected_option == 2,
                     text_color, selected_color, value_color);
        break;

    case SETTINGS_TAB_GAMEPLAY:
        draw_option (self, content_x, "Auto-Save:", self->auto_save ? "On" : "Off",
                     content_y, self->selected_option == 0,
                     text_color, selected_color, value_color);

        draw_option (self, content_x, "Tutorials:", self->tutorials ? "On" : "Off",
                     content_y + 40, self->selected_option == 1,
                     text_color, selected_color, value_color);

        draw_option (self, content_x, "Difficulty:", difficulties[self->difficulty],
                     content_y + 80, self->selected_option == 2,
                     text_color, selected_color, value_color);

        draw_option (self, content_x, "Game Speed:", game_speeds[self->game_speed],
                     content_y + 120, self->selected_option == 3,
                     text_color, selected_color, value_color);
        break;

    case SETTINGS_TAB_CONTROLS:
        draw_label (get_pool_label (self), "Key Bindings (read-only):",
                    content_x, content_y, 18, title_color);
        draw_label (get_pool_label (self), "Navigate Menu:",
                    content_x + 20, content_y + 40, 16, text_color);
        draw_label (get_pool_label (self), "Arrow Keys / WASD",
                    content_x + 260, content_y + 40, 16, dim_color);
        draw_label (get_pool_label (self), "Select / Confirm:",
                    content_x + 20, content_y + 70, 16, text_color);
        draw_label (get_pool_label (self), "Enter / Space",
                    content_x + 260, content_y + 70, 16, dim_color);
        draw_label (get_pool_label (self), "Back / Cancel:",
                    content_x + 20, content_y + 100, 16, text_color);
        draw_label (get_pool_label (self), "Escape",
                    content_x + 260, content_y + 100, 16, dim_color);
        draw_label (get_pool_label (self), "Switch Tabs:",
                    content_x + 20, content_y + 130, 16, text_color);
        draw_label (get_pool_label (self), "Tab",
                    content_x + 260, content_y + 130, 16, dim_color);
        break;

    default:
        break;
    }

    /* Draw instructions at bottom */
    draw_label (get_pool_label (self), "UP/DOWN: Select    LEFT/RIGHT: Change    TAB: Switch Tab    ESC: Return",
                center_x - 340, screen_h - 50, 16, dim_color);
}

static gboolean
lp_state_settings_handle_input (LrgGameState *state,
                                gpointer      event)
{
    (void)state;
    (void)event;
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_settings_class_init (LpStateSettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_settings_dispose;

    state_class->enter = lp_state_settings_enter;
    state_class->exit = lp_state_settings_exit;
    state_class->update = lp_state_settings_update;
    state_class->draw = lp_state_settings_draw;
    state_class->handle_input = lp_state_settings_handle_input;
}

static void
lp_state_settings_init (LpStateSettings *self)
{
    guint i;

    /* Settings menu is transparent (can show from main menu or pause) */
    lrg_game_state_set_name (LRG_GAME_STATE (self), "Settings");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), TRUE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->current_tab = SETTINGS_TAB_GRAPHICS;
    self->selected_option = 0;

    /* Default graphics settings */
    self->resolution_idx = 0;  /* 1280x720 */
    self->fullscreen = FALSE;
    self->vsync = TRUE;

    /* Default audio settings */
    self->master_volume = 100;
    self->music_volume = 80;
    self->sfx_volume = 100;

    /* Default gameplay settings */
    self->auto_save = TRUE;
    self->tutorials = TRUE;
    self->difficulty = 1;  /* Normal */
    self->game_speed = 0;  /* 1x Normal */

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool for dynamic text (tabs, options, controls, instructions) */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 60; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_state_settings_new:
 *
 * Creates a new settings menu overlay state.
 *
 * Returns: (transfer full): A new #LpStateSettings
 */
LpStateSettings *
lp_state_settings_new (void)
{
    return g_object_new (LP_TYPE_STATE_SETTINGS, NULL);
}
