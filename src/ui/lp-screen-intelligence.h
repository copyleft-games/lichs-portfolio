/* lp-screen-intelligence.h - Intelligence Reports Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Screen for viewing intelligence reports, predictions, and competitor info.
 */

#ifndef LP_SCREEN_INTELLIGENCE_H
#define LP_SCREEN_INTELLIGENCE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SCREEN_INTELLIGENCE (lp_screen_intelligence_get_type ())

G_DECLARE_FINAL_TYPE (LpScreenIntelligence, lp_screen_intelligence,
                      LP, SCREEN_INTELLIGENCE, LrgContainer)

/**
 * lp_screen_intelligence_new:
 *
 * Creates a new intelligence screen.
 *
 * Returns: (transfer full): A new #LpScreenIntelligence
 */
LpScreenIntelligence * lp_screen_intelligence_new (void);

/**
 * lp_screen_intelligence_refresh:
 * @self: an #LpScreenIntelligence
 *
 * Refreshes the intelligence reports.
 */
void lp_screen_intelligence_refresh (LpScreenIntelligence *self);

G_END_DECLS

#endif /* LP_SCREEN_INTELLIGENCE_H */
