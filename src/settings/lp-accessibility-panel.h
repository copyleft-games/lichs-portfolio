/* lp-accessibility-panel.h - Accessibility settings panel widget
 *
 * Copyright 2026 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include <libregnum.h>

G_BEGIN_DECLS

#define LP_TYPE_ACCESSIBILITY_PANEL (lp_accessibility_panel_get_type ())

G_DECLARE_FINAL_TYPE (LpAccessibilityPanel, lp_accessibility_panel, LP, ACCESSIBILITY_PANEL, LrgWidget)

/**
 * lp_accessibility_panel_new:
 * @settings: an #LrgAccessibilitySettings to edit
 *
 * Creates a new accessibility settings panel that allows
 * editing all accessibility options.
 *
 * Returns: (transfer full): A new #LpAccessibilityPanel
 */
LpAccessibilityPanel *
lp_accessibility_panel_new (LrgAccessibilitySettings *settings);

/**
 * lp_accessibility_panel_apply:
 * @self: an #LpAccessibilityPanel
 *
 * Applies the current panel settings to the game.
 * Triggers colorblind filter updates, UI rescaling, etc.
 */
void
lp_accessibility_panel_apply (LpAccessibilityPanel *self);

G_END_DECLS
