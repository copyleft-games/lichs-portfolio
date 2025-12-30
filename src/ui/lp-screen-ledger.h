/* lp-screen-ledger.h - Discovery Ledger Screen
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Screen for viewing discovered secrets and hidden information.
 */

#ifndef LP_SCREEN_LEDGER_H
#define LP_SCREEN_LEDGER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_SCREEN_LEDGER (lp_screen_ledger_get_type ())

G_DECLARE_FINAL_TYPE (LpScreenLedger, lp_screen_ledger,
                      LP, SCREEN_LEDGER, LrgContainer)

/**
 * lp_screen_ledger_new:
 *
 * Creates a new ledger screen.
 *
 * Returns: (transfer full): A new #LpScreenLedger
 */
LpScreenLedger * lp_screen_ledger_new (void);

/**
 * lp_screen_ledger_get_ledger:
 * @self: an #LpScreenLedger
 *
 * Gets the ledger being displayed.
 *
 * Returns: (transfer none) (nullable): The #LpLedger
 */
LpLedger * lp_screen_ledger_get_ledger (LpScreenLedger *self);

/**
 * lp_screen_ledger_set_ledger:
 * @self: an #LpScreenLedger
 * @ledger: (nullable): the ledger to display
 *
 * Sets the ledger to display.
 */
void lp_screen_ledger_set_ledger (LpScreenLedger *self,
                                   LpLedger       *ledger);

/**
 * lp_screen_ledger_refresh:
 * @self: an #LpScreenLedger
 *
 * Refreshes the ledger display.
 */
void lp_screen_ledger_refresh (LpScreenLedger *self);

G_END_DECLS

#endif /* LP_SCREEN_LEDGER_H */
