/* lp-theme.h - Dark Fantasy Theme Configuration
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Configures the libregnum theme singleton with the dark fantasy
 * aesthetic for Lich's Portfolio.
 */

#ifndef LP_THEME_H
#define LP_THEME_H

#include <glib.h>
#include <libregnum.h>

G_BEGIN_DECLS

/* ==========================================================================
 * Theme Configuration
 * ========================================================================== */

/**
 * lp_theme_configure_default:
 *
 * Configures the default LrgTheme singleton with the dark fantasy
 * color scheme for Lich's Portfolio.
 *
 * Call this once during application startup before creating any UI widgets.
 *
 * Color Palette:
 * - Primary: Deep purple (#2d1b4e)
 * - Secondary: Bone white (#e8e0d5)
 * - Accent: Gold (#c9a227)
 * - Background: Near black (#0a0a0f)
 * - Surface: Dark purple (#1a1025)
 * - Text: Off-white (#d4d0c8)
 * - Text Secondary: Muted gray (#8a8580)
 * - Border: Dark purple border (#3d2b5e)
 * - Error: Blood red (#9e2a2a)
 * - Success: Emerald (#2a9e4a)
 */
void lp_theme_configure_default (void);

/* ==========================================================================
 * Theme Color Accessors
 *
 * These provide access to game-specific colors not in the base LrgTheme.
 * ========================================================================== */

/**
 * lp_theme_get_gold_color:
 *
 * Gets the gold accent color used for wealth/currency display.
 *
 * Returns: (transfer none): The gold color
 */
const GrlColor * lp_theme_get_gold_color (void);

/**
 * lp_theme_get_danger_color:
 *
 * Gets the danger/warning color used for exposure and threats.
 *
 * Returns: (transfer none): The danger color (deep red)
 */
const GrlColor * lp_theme_get_danger_color (void);

/**
 * lp_theme_get_hidden_color:
 *
 * Gets the color used for hidden/stealth status.
 *
 * Returns: (transfer none): The hidden color (dark blue)
 */
const GrlColor * lp_theme_get_hidden_color (void);

/**
 * lp_theme_get_scrutiny_color:
 *
 * Gets the color used for scrutiny exposure level.
 *
 * Returns: (transfer none): The scrutiny color (yellow)
 */
const GrlColor * lp_theme_get_scrutiny_color (void);

/**
 * lp_theme_get_suspicion_color:
 *
 * Gets the color used for suspicion exposure level.
 *
 * Returns: (transfer none): The suspicion color (orange)
 */
const GrlColor * lp_theme_get_suspicion_color (void);

/**
 * lp_theme_get_hunt_color:
 *
 * Gets the color used for hunt exposure level.
 *
 * Returns: (transfer none): The hunt color (red-orange)
 */
const GrlColor * lp_theme_get_hunt_color (void);

/**
 * lp_theme_get_crusade_color:
 *
 * Gets the color used for crusade exposure level.
 *
 * Returns: (transfer none): The crusade color (bright red)
 */
const GrlColor * lp_theme_get_crusade_color (void);

/**
 * lp_theme_get_synergy_color:
 *
 * Gets the color used for active synergy indicators.
 *
 * Returns: (transfer none): The synergy color (cyan)
 */
const GrlColor * lp_theme_get_synergy_color (void);

/**
 * lp_theme_get_inactive_color:
 *
 * Gets the color used for inactive/disabled elements.
 *
 * Returns: (transfer none): The inactive color (dark gray)
 */
const GrlColor * lp_theme_get_inactive_color (void);

G_END_DECLS

#endif /* LP_THEME_H */
