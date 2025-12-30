/* lp-state-analyze.h - World Analysis Game State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The analyze state provides the main gameplay view.
 * Shows the world map, portfolio overview, available investments,
 * and agent status.
 */

#ifndef LP_STATE_ANALYZE_H
#define LP_STATE_ANALYZE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_ANALYZE (lp_state_analyze_get_type ())

G_DECLARE_FINAL_TYPE (LpStateAnalyze, lp_state_analyze,
                      LP, STATE_ANALYZE, LrgGameState)

/**
 * lp_state_analyze_new:
 *
 * Creates a new world analysis state.
 *
 * Returns: (transfer full): A new #LpStateAnalyze
 */
LpStateAnalyze *
lp_state_analyze_new (void);

G_END_DECLS

#endif /* LP_STATE_ANALYZE_H */
