/* lp-tooltip.h - Contextual Help Tooltip Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Displays contextual help information when hovering over UI elements.
 * Supports both static text and dynamic content generation.
 */

#ifndef LP_TOOLTIP_H
#define LP_TOOLTIP_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_TOOLTIP (lp_tooltip_get_type ())

G_DECLARE_FINAL_TYPE (LpTooltip, lp_tooltip, LP, TOOLTIP, LrgWidget)

/**
 * LpTooltipPosition:
 * @LP_TOOLTIP_POSITION_AUTO: Automatically choose best position
 * @LP_TOOLTIP_POSITION_ABOVE: Display above the target
 * @LP_TOOLTIP_POSITION_BELOW: Display below the target
 * @LP_TOOLTIP_POSITION_LEFT: Display to the left of target
 * @LP_TOOLTIP_POSITION_RIGHT: Display to the right of target
 *
 * Position preference for tooltip display.
 */
typedef enum
{
    LP_TOOLTIP_POSITION_AUTO,
    LP_TOOLTIP_POSITION_ABOVE,
    LP_TOOLTIP_POSITION_BELOW,
    LP_TOOLTIP_POSITION_LEFT,
    LP_TOOLTIP_POSITION_RIGHT
} LpTooltipPosition;

/**
 * lp_tooltip_new:
 *
 * Creates a new tooltip widget.
 *
 * Returns: (transfer full): A new #LpTooltip
 */
LpTooltip * lp_tooltip_new (void);

/* ==========================================================================
 * Content
 * ========================================================================== */

/**
 * lp_tooltip_set_title:
 * @self: an #LpTooltip
 * @title: (nullable): the tooltip title
 *
 * Sets the tooltip title (displayed in larger font).
 */
void lp_tooltip_set_title (LpTooltip   *self,
                           const gchar *title);

/**
 * lp_tooltip_get_title:
 * @self: an #LpTooltip
 *
 * Gets the tooltip title.
 *
 * Returns: (transfer none) (nullable): The title
 */
const gchar * lp_tooltip_get_title (LpTooltip *self);

/**
 * lp_tooltip_set_text:
 * @self: an #LpTooltip
 * @text: (nullable): the tooltip text
 *
 * Sets the tooltip body text.
 */
void lp_tooltip_set_text (LpTooltip   *self,
                          const gchar *text);

/**
 * lp_tooltip_get_text:
 * @self: an #LpTooltip
 *
 * Gets the tooltip body text.
 *
 * Returns: (transfer none) (nullable): The text
 */
const gchar * lp_tooltip_get_text (LpTooltip *self);

/**
 * lp_tooltip_set_hint:
 * @self: an #LpTooltip
 * @hint: (nullable): a short hint or tip
 *
 * Sets an optional hint shown in smaller text at bottom.
 */
void lp_tooltip_set_hint (LpTooltip   *self,
                          const gchar *hint);

/**
 * lp_tooltip_get_hint:
 * @self: an #LpTooltip
 *
 * Gets the tooltip hint text.
 *
 * Returns: (transfer none) (nullable): The hint
 */
const gchar * lp_tooltip_get_hint (LpTooltip *self);

/* ==========================================================================
 * Display Control
 * ========================================================================== */

/**
 * lp_tooltip_show_at:
 * @self: an #LpTooltip
 * @x: screen x coordinate
 * @y: screen y coordinate
 * @position: preferred position relative to point
 *
 * Shows the tooltip at the specified screen position.
 */
void lp_tooltip_show_at (LpTooltip          *self,
                         gfloat              x,
                         gfloat              y,
                         LpTooltipPosition   position);

/**
 * lp_tooltip_show_for_widget:
 * @self: an #LpTooltip
 * @widget: the target widget
 * @position: preferred position relative to widget
 *
 * Shows the tooltip positioned relative to a widget.
 */
void lp_tooltip_show_for_widget (LpTooltip          *self,
                                 LrgWidget          *widget,
                                 LpTooltipPosition   position);

/**
 * lp_tooltip_hide:
 * @self: an #LpTooltip
 *
 * Hides the tooltip.
 */
void lp_tooltip_hide (LpTooltip *self);

/**
 * lp_tooltip_is_visible:
 * @self: an #LpTooltip
 *
 * Checks if the tooltip is currently visible.
 *
 * Returns: %TRUE if visible
 */
gboolean lp_tooltip_is_visible (LpTooltip *self);

/* ==========================================================================
 * Appearance
 * ========================================================================== */

/**
 * lp_tooltip_set_max_width:
 * @self: an #LpTooltip
 * @width: maximum width in pixels (0 = no limit)
 *
 * Sets the maximum width for text wrapping.
 */
void lp_tooltip_set_max_width (LpTooltip *self,
                               gfloat     width);

/**
 * lp_tooltip_set_delay:
 * @self: an #LpTooltip
 * @delay_ms: delay before showing in milliseconds
 *
 * Sets the delay before the tooltip appears.
 */
void lp_tooltip_set_delay (LpTooltip *self,
                           guint      delay_ms);

G_END_DECLS

#endif /* LP_TOOLTIP_H */
