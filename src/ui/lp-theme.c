/* lp-theme.c - Dark Fantasy Theme Configuration
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_UI
#include "../lp-log.h"

#include "lp-theme.h"

/* ==========================================================================
 * Static Color Storage
 *
 * Game-specific colors that extend the base LrgTheme palette.
 * ========================================================================== */

static GrlColor *gold_color = NULL;
static GrlColor *danger_color = NULL;
static GrlColor *hidden_color = NULL;
static GrlColor *scrutiny_color = NULL;
static GrlColor *suspicion_color = NULL;
static GrlColor *hunt_color = NULL;
static GrlColor *crusade_color = NULL;
static GrlColor *synergy_color = NULL;
static GrlColor *inactive_color = NULL;

/* ==========================================================================
 * Helper: Initialize Custom Colors
 * ========================================================================== */

static void
initialize_custom_colors (void)
{
    /* Free existing colors if re-initializing */
    g_clear_pointer (&gold_color, grl_color_free);
    g_clear_pointer (&danger_color, grl_color_free);
    g_clear_pointer (&hidden_color, grl_color_free);
    g_clear_pointer (&scrutiny_color, grl_color_free);
    g_clear_pointer (&suspicion_color, grl_color_free);
    g_clear_pointer (&hunt_color, grl_color_free);
    g_clear_pointer (&crusade_color, grl_color_free);
    g_clear_pointer (&synergy_color, grl_color_free);
    g_clear_pointer (&inactive_color, grl_color_free);

    /* Gold - for wealth display (#c9a227) */
    gold_color = grl_color_new (0xc9, 0xa2, 0x27, 0xff);

    /* Danger - blood red (#9e2a2a) */
    danger_color = grl_color_new (0x9e, 0x2a, 0x2a, 0xff);

    /* Hidden - dark blue (#1a3a5c) */
    hidden_color = grl_color_new (0x1a, 0x3a, 0x5c, 0xff);

    /* Scrutiny - yellow (#c9b327) */
    scrutiny_color = grl_color_new (0xc9, 0xb3, 0x27, 0xff);

    /* Suspicion - orange (#c97327) */
    suspicion_color = grl_color_new (0xc9, 0x73, 0x27, 0xff);

    /* Hunt - red-orange (#c94a27) */
    hunt_color = grl_color_new (0xc9, 0x4a, 0x27, 0xff);

    /* Crusade - bright red (#c92727) */
    crusade_color = grl_color_new (0xc9, 0x27, 0x27, 0xff);

    /* Synergy - cyan (#27c9c9) */
    synergy_color = grl_color_new (0x27, 0xc9, 0xc9, 0xff);

    /* Inactive - dark gray (#3a3a3a) */
    inactive_color = grl_color_new (0x3a, 0x3a, 0x3a, 0xff);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

/**
 * lp_theme_configure_default:
 *
 * Configures the default theme singleton with dark fantasy colors.
 */
void
lp_theme_configure_default (void)
{
    LrgTheme *theme;
    g_autoptr(GrlColor) primary = NULL;
    g_autoptr(GrlColor) secondary = NULL;
    g_autoptr(GrlColor) accent = NULL;
    g_autoptr(GrlColor) background = NULL;
    g_autoptr(GrlColor) surface = NULL;
    g_autoptr(GrlColor) text = NULL;
    g_autoptr(GrlColor) text_secondary = NULL;
    g_autoptr(GrlColor) border = NULL;
    g_autoptr(GrlColor) error = NULL;
    g_autoptr(GrlColor) success = NULL;

    lp_log_info ("Configuring dark fantasy theme");

    theme = lrg_theme_get_default ();

    /* Create base colors */
    primary = grl_color_new (0x2d, 0x1b, 0x4e, 0xff);         /* Deep purple */
    secondary = grl_color_new (0xe8, 0xe0, 0xd5, 0xff);       /* Bone white */
    accent = grl_color_new (0xc9, 0xa2, 0x27, 0xff);          /* Gold */
    background = grl_color_new (0x0a, 0x0a, 0x0f, 0xff);      /* Near black */
    surface = grl_color_new (0x1a, 0x10, 0x25, 0xff);         /* Dark purple */
    text = grl_color_new (0xd4, 0xd0, 0xc8, 0xff);            /* Off-white */
    text_secondary = grl_color_new (0x8a, 0x85, 0x80, 0xff);  /* Muted gray */
    border = grl_color_new (0x3d, 0x2b, 0x5e, 0xff);          /* Dark purple border */
    error = grl_color_new (0x9e, 0x2a, 0x2a, 0xff);           /* Blood red */
    success = grl_color_new (0x2a, 0x9e, 0x4a, 0xff);         /* Emerald */

    /* Apply to theme */
    lrg_theme_set_primary_color (theme, primary);
    lrg_theme_set_secondary_color (theme, secondary);
    lrg_theme_set_accent_color (theme, accent);
    lrg_theme_set_background_color (theme, background);
    lrg_theme_set_surface_color (theme, surface);
    lrg_theme_set_text_color (theme, text);
    lrg_theme_set_text_secondary_color (theme, text_secondary);
    lrg_theme_set_border_color (theme, border);
    lrg_theme_set_error_color (theme, error);
    lrg_theme_set_success_color (theme, success);

    /* Typography */
    lrg_theme_set_font_size_small (theme, 12.0f);
    lrg_theme_set_font_size_normal (theme, 16.0f);
    lrg_theme_set_font_size_large (theme, 24.0f);

    /* Spacing */
    lrg_theme_set_padding_small (theme, 4.0f);
    lrg_theme_set_padding_normal (theme, 8.0f);
    lrg_theme_set_padding_large (theme, 16.0f);
    lrg_theme_set_border_width (theme, 1.0f);
    lrg_theme_set_corner_radius (theme, 4.0f);

    /* Initialize game-specific colors */
    initialize_custom_colors ();

    lp_log_debug ("Theme configuration complete");
}

/* ==========================================================================
 * Custom Color Accessors
 * ========================================================================== */

const GrlColor *
lp_theme_get_gold_color (void)
{
    if (gold_color == NULL)
        initialize_custom_colors ();

    return gold_color;
}

const GrlColor *
lp_theme_get_danger_color (void)
{
    if (danger_color == NULL)
        initialize_custom_colors ();

    return danger_color;
}

const GrlColor *
lp_theme_get_hidden_color (void)
{
    if (hidden_color == NULL)
        initialize_custom_colors ();

    return hidden_color;
}

const GrlColor *
lp_theme_get_scrutiny_color (void)
{
    if (scrutiny_color == NULL)
        initialize_custom_colors ();

    return scrutiny_color;
}

const GrlColor *
lp_theme_get_suspicion_color (void)
{
    if (suspicion_color == NULL)
        initialize_custom_colors ();

    return suspicion_color;
}

const GrlColor *
lp_theme_get_hunt_color (void)
{
    if (hunt_color == NULL)
        initialize_custom_colors ();

    return hunt_color;
}

const GrlColor *
lp_theme_get_crusade_color (void)
{
    if (crusade_color == NULL)
        initialize_custom_colors ();

    return crusade_color;
}

const GrlColor *
lp_theme_get_synergy_color (void)
{
    if (synergy_color == NULL)
        initialize_custom_colors ();

    return synergy_color;
}

const GrlColor *
lp_theme_get_inactive_color (void)
{
    if (inactive_color == NULL)
        initialize_custom_colors ();

    return inactive_color;
}
