/* lp-widget-synergy-indicator.h - Synergy Bonus Display Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Visual widget that displays active synergies and total bonus.
 * Shows synergy count and multiplier, with expandable details.
 */

#ifndef LP_WIDGET_SYNERGY_INDICATOR_H
#define LP_WIDGET_SYNERGY_INDICATOR_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_WIDGET_SYNERGY_INDICATOR (lp_widget_synergy_indicator_get_type ())

G_DECLARE_FINAL_TYPE (LpWidgetSynergyIndicator, lp_widget_synergy_indicator,
                      LP, WIDGET_SYNERGY_INDICATOR, LrgWidget)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_widget_synergy_indicator_new:
 *
 * Creates a new synergy indicator widget.
 * Automatically connects to the default LpSynergyManager.
 *
 * Returns: (transfer full): A new #LpWidgetSynergyIndicator
 */
LpWidgetSynergyIndicator * lp_widget_synergy_indicator_new (void);

/* ==========================================================================
 * Synergy Display
 * ========================================================================== */

/**
 * lp_widget_synergy_indicator_get_synergy_count:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets the number of currently active synergies being displayed.
 *
 * Returns: The number of active synergies
 */
guint lp_widget_synergy_indicator_get_synergy_count (LpWidgetSynergyIndicator *self);

/**
 * lp_widget_synergy_indicator_get_total_bonus:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets the total synergy bonus multiplier.
 *
 * Returns: The bonus multiplier (1.0 = no bonus)
 */
gdouble lp_widget_synergy_indicator_get_total_bonus (LpWidgetSynergyIndicator *self);

/* ==========================================================================
 * Appearance Options
 * ========================================================================== */

/**
 * lp_widget_synergy_indicator_get_show_details:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets whether detailed synergy list is shown.
 *
 * Returns: %TRUE if showing details
 */
gboolean lp_widget_synergy_indicator_get_show_details (LpWidgetSynergyIndicator *self);

/**
 * lp_widget_synergy_indicator_set_show_details:
 * @self: an #LpWidgetSynergyIndicator
 * @show: whether to show details
 *
 * Sets whether to display the detailed synergy list.
 */
void lp_widget_synergy_indicator_set_show_details (LpWidgetSynergyIndicator *self,
                                                    gboolean                  show);

/**
 * lp_widget_synergy_indicator_get_compact:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Gets whether the widget is in compact mode.
 *
 * Returns: %TRUE if in compact mode
 */
gboolean lp_widget_synergy_indicator_get_compact (LpWidgetSynergyIndicator *self);

/**
 * lp_widget_synergy_indicator_set_compact:
 * @self: an #LpWidgetSynergyIndicator
 * @compact: whether to use compact mode
 *
 * Sets compact mode. In compact mode, only shows icon and bonus.
 */
void lp_widget_synergy_indicator_set_compact (LpWidgetSynergyIndicator *self,
                                               gboolean                  compact);

/* ==========================================================================
 * Refresh
 * ========================================================================== */

/**
 * lp_widget_synergy_indicator_refresh:
 * @self: an #LpWidgetSynergyIndicator
 *
 * Forces a refresh of the synergy display from the manager.
 */
void lp_widget_synergy_indicator_refresh (LpWidgetSynergyIndicator *self);

G_END_DECLS

#endif /* LP_WIDGET_SYNERGY_INDICATOR_H */
