/* lp-screen-slumber.h - Slumber Configuration Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Screen for configuring slumber duration, dormant orders, and wake conditions.
 */

#ifndef LP_SCREEN_SLUMBER_H
#define LP_SCREEN_SLUMBER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SCREEN_SLUMBER (lp_screen_slumber_get_type ())

G_DECLARE_FINAL_TYPE (LpScreenSlumber, lp_screen_slumber,
                      LP, SCREEN_SLUMBER, LrgContainer)

/**
 * lp_screen_slumber_new:
 *
 * Creates a new slumber configuration screen.
 *
 * Returns: (transfer full): A new #LpScreenSlumber
 */
LpScreenSlumber * lp_screen_slumber_new (void);

/**
 * lp_screen_slumber_get_duration:
 * @self: an #LpScreenSlumber
 *
 * Gets the selected slumber duration in years.
 *
 * Returns: The slumber duration (minimum 25 years)
 */
guint lp_screen_slumber_get_duration (LpScreenSlumber *self);

/**
 * lp_screen_slumber_set_duration:
 * @self: an #LpScreenSlumber
 * @years: the slumber duration in years
 *
 * Sets the slumber duration.
 */
void lp_screen_slumber_set_duration (LpScreenSlumber *self,
                                      guint            years);

/**
 * LpScreenSlumber::slumber-confirmed:
 * @self: the #LpScreenSlumber
 * @duration: the selected duration in years
 *
 * Emitted when the user confirms the slumber configuration.
 */

G_END_DECLS

#endif /* LP_SCREEN_SLUMBER_H */
