/* lp-widget-exposure-meter.h - Exposure Tracking Widget
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Visual widget that displays the player's current exposure level.
 * The meter changes color based on the exposure threshold crossed.
 */

#ifndef LP_WIDGET_EXPOSURE_METER_H
#define LP_WIDGET_EXPOSURE_METER_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_WIDGET_EXPOSURE_METER (lp_widget_exposure_meter_get_type ())

G_DECLARE_FINAL_TYPE (LpWidgetExposureMeter, lp_widget_exposure_meter, LP, WIDGET_EXPOSURE_METER, LrgWidget)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_widget_exposure_meter_new:
 *
 * Creates a new exposure meter widget.
 *
 * Returns: (transfer full): A new #LpWidgetExposureMeter
 */
LpWidgetExposureMeter * lp_widget_exposure_meter_new (void);

/* ==========================================================================
 * Exposure Value
 * ========================================================================== */

/**
 * lp_widget_exposure_meter_get_value:
 * @self: an #LpWidgetExposureMeter
 *
 * Gets the current exposure value (0-100).
 *
 * Returns: The current exposure value
 */
guint lp_widget_exposure_meter_get_value (LpWidgetExposureMeter *self);

/**
 * lp_widget_exposure_meter_set_value:
 * @self: an #LpWidgetExposureMeter
 * @value: the exposure value (0-100)
 *
 * Sets the current exposure value. Value is clamped to 0-100.
 */
void lp_widget_exposure_meter_set_value (LpWidgetExposureMeter *self,
                                          guint                  value);

/* ==========================================================================
 * Exposure Level
 * ========================================================================== */

/**
 * lp_widget_exposure_meter_get_level:
 * @self: an #LpWidgetExposureMeter
 *
 * Gets the current exposure level based on the value thresholds.
 *
 * Returns: The current #LpExposureLevel
 */
LpExposureLevel lp_widget_exposure_meter_get_level (LpWidgetExposureMeter *self);

/* ==========================================================================
 * Appearance Options
 * ========================================================================== */

/**
 * lp_widget_exposure_meter_get_show_label:
 * @self: an #LpWidgetExposureMeter
 *
 * Gets whether the exposure label is shown.
 *
 * Returns: %TRUE if showing label
 */
gboolean lp_widget_exposure_meter_get_show_label (LpWidgetExposureMeter *self);

/**
 * lp_widget_exposure_meter_set_show_label:
 * @self: an #LpWidgetExposureMeter
 * @show: whether to show the label
 *
 * Sets whether to display the exposure level label.
 */
void lp_widget_exposure_meter_set_show_label (LpWidgetExposureMeter *self,
                                               gboolean               show);

/**
 * lp_widget_exposure_meter_get_show_percentage:
 * @self: an #LpWidgetExposureMeter
 *
 * Gets whether the percentage is shown.
 *
 * Returns: %TRUE if showing percentage
 */
gboolean lp_widget_exposure_meter_get_show_percentage (LpWidgetExposureMeter *self);

/**
 * lp_widget_exposure_meter_set_show_percentage:
 * @self: an #LpWidgetExposureMeter
 * @show: whether to show the percentage
 *
 * Sets whether to display the exposure percentage.
 */
void lp_widget_exposure_meter_set_show_percentage (LpWidgetExposureMeter *self,
                                                    gboolean               show);

/**
 * lp_widget_exposure_meter_get_orientation:
 * @self: an #LpWidgetExposureMeter
 *
 * Gets the meter orientation.
 *
 * Returns: The meter orientation
 */
LrgOrientation lp_widget_exposure_meter_get_orientation (LpWidgetExposureMeter *self);

/**
 * lp_widget_exposure_meter_set_orientation:
 * @self: an #LpWidgetExposureMeter
 * @orientation: the meter orientation
 *
 * Sets the meter orientation (horizontal or vertical).
 */
void lp_widget_exposure_meter_set_orientation (LpWidgetExposureMeter *self,
                                                LrgOrientation         orientation);

/* ==========================================================================
 * Signals
 * ========================================================================== */

/**
 * LpWidgetExposureMeter::level-changed:
 * @self: the #LpWidgetExposureMeter
 * @old_level: the previous #LpExposureLevel
 * @new_level: the new #LpExposureLevel
 *
 * Emitted when the exposure level threshold is crossed.
 */

G_END_DECLS

#endif /* LP_WIDGET_EXPOSURE_METER_H */
