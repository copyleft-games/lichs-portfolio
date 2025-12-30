/* lp-screen-megaprojects.h - Megaprojects Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Screen for viewing and managing multi-century megaprojects.
 */

#ifndef LP_SCREEN_MEGAPROJECTS_H
#define LP_SCREEN_MEGAPROJECTS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SCREEN_MEGAPROJECTS (lp_screen_megaprojects_get_type ())

G_DECLARE_FINAL_TYPE (LpScreenMegaprojects, lp_screen_megaprojects,
                      LP, SCREEN_MEGAPROJECTS, LrgContainer)

/**
 * lp_screen_megaprojects_new:
 *
 * Creates a new megaprojects screen.
 *
 * Returns: (transfer full): A new #LpScreenMegaprojects
 */
LpScreenMegaprojects * lp_screen_megaprojects_new (void);

/**
 * lp_screen_megaprojects_refresh:
 * @self: an #LpScreenMegaprojects
 *
 * Refreshes the megaprojects display.
 */
void lp_screen_megaprojects_refresh (LpScreenMegaprojects *self);

G_END_DECLS

#endif /* LP_SCREEN_MEGAPROJECTS_H */
