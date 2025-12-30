/* lp-floating-text.h - Floating Text Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Displays floating text that drifts upward and fades out.
 * Used for gold change popups and value feedback.
 */

#ifndef LP_FLOATING_TEXT_H
#define LP_FLOATING_TEXT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_FLOATING_TEXT (lp_floating_text_get_type ())
G_DECLARE_FINAL_TYPE (LpFloatingText, lp_floating_text, LP, FLOATING_TEXT, LrgWidget)

/**
 * lp_floating_text_new:
 * @text: the text to display
 * @x: starting X position
 * @y: starting Y position
 * @color: (transfer none): text color
 *
 * Creates a new floating text widget.
 *
 * Returns: (transfer full): a new #LpFloatingText
 */
LpFloatingText *lp_floating_text_new (const gchar *text,
                                      gfloat       x,
                                      gfloat       y,
                                      GrlColor    *color);

/**
 * lp_floating_text_spawn_gold:
 * @parent: the container to add the text to
 * @amount: the gold amount to display
 * @positive: %TRUE for gain, %FALSE for loss
 * @x: spawn X position
 * @y: spawn Y position
 *
 * Convenience function to spawn a gold change floating text.
 * Uses gold color for positive, red for negative.
 */
void lp_floating_text_spawn_gold (LrgContainer *parent,
                                  LrgBigNumber *amount,
                                  gboolean      positive,
                                  gfloat        x,
                                  gfloat        y);

/**
 * lp_floating_text_get_text:
 * @self: a #LpFloatingText
 *
 * Gets the displayed text.
 *
 * Returns: (transfer none): the text string
 */
const gchar *lp_floating_text_get_text (LpFloatingText *self);

/**
 * lp_floating_text_set_text:
 * @self: a #LpFloatingText
 * @text: the new text
 *
 * Sets the displayed text.
 */
void lp_floating_text_set_text (LpFloatingText *self,
                                const gchar    *text);

/**
 * lp_floating_text_get_lifetime:
 * @self: a #LpFloatingText
 *
 * Gets the total lifetime in seconds.
 *
 * Returns: lifetime in seconds
 */
gfloat lp_floating_text_get_lifetime (LpFloatingText *self);

/**
 * lp_floating_text_set_lifetime:
 * @self: a #LpFloatingText
 * @lifetime: lifetime in seconds
 *
 * Sets the total lifetime before the text disappears.
 */
void lp_floating_text_set_lifetime (LpFloatingText *self,
                                    gfloat          lifetime);

/**
 * lp_floating_text_get_velocity_y:
 * @self: a #LpFloatingText
 *
 * Gets the vertical velocity (negative = upward).
 *
 * Returns: velocity in pixels per second
 */
gfloat lp_floating_text_get_velocity_y (LpFloatingText *self);

/**
 * lp_floating_text_set_velocity_y:
 * @self: a #LpFloatingText
 * @velocity_y: velocity in pixels per second (negative = upward)
 *
 * Sets the vertical velocity.
 */
void lp_floating_text_set_velocity_y (LpFloatingText *self,
                                      gfloat          velocity_y);

/**
 * lp_floating_text_get_alpha:
 * @self: a #LpFloatingText
 *
 * Gets the current alpha (opacity).
 *
 * Returns: alpha value 0.0 to 1.0
 */
gfloat lp_floating_text_get_alpha (LpFloatingText *self);

/**
 * lp_floating_text_is_finished:
 * @self: a #LpFloatingText
 *
 * Checks if the floating text has finished its animation.
 *
 * Returns: %TRUE if finished and should be removed
 */
gboolean lp_floating_text_is_finished (LpFloatingText *self);

/**
 * lp_floating_text_update:
 * @self: a #LpFloatingText
 * @delta: time elapsed since last update in seconds
 *
 * Updates the floating text animation.
 */
void lp_floating_text_update (LpFloatingText *self,
                              gfloat          delta);

G_END_DECLS

#endif /* LP_FLOATING_TEXT_H */
