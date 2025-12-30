/* lp-exposure-manager.h - Exposure Tracking Singleton
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tracks the lich's visibility to mortal institutions.
 * Exposure ranges from 0-100 and affects gameplay through thresholds.
 */

#ifndef LP_EXPOSURE_MANAGER_H
#define LP_EXPOSURE_MANAGER_H

#include <glib-object.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_EXPOSURE_MANAGER (lp_exposure_manager_get_type ())

G_DECLARE_FINAL_TYPE (LpExposureManager, lp_exposure_manager,
                      LP, EXPOSURE_MANAGER, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_exposure_manager_get_default:
 *
 * Gets the default exposure manager instance.
 * Creates it if it doesn't exist.
 *
 * Returns: (transfer none): The default #LpExposureManager instance
 */
LpExposureManager *
lp_exposure_manager_get_default (void);

/* ==========================================================================
 * Exposure Value
 * ========================================================================== */

/**
 * lp_exposure_manager_get_exposure:
 * @self: an #LpExposureManager
 *
 * Gets the current exposure value (0-100).
 *
 * Returns: The current exposure value
 */
guint
lp_exposure_manager_get_exposure (LpExposureManager *self);

/**
 * lp_exposure_manager_set_exposure:
 * @self: an #LpExposureManager
 * @value: the new exposure value (clamped to 0-100)
 *
 * Sets the exposure value directly. Emits ::threshold-crossed
 * if the exposure level changes.
 */
void
lp_exposure_manager_set_exposure (LpExposureManager *self,
                                  guint              value);

/**
 * lp_exposure_manager_add_exposure:
 * @self: an #LpExposureManager
 * @amount: amount to add (can be negative)
 *
 * Adds to the current exposure value. Result is clamped to 0-100.
 * Emits ::threshold-crossed if the exposure level changes.
 */
void
lp_exposure_manager_add_exposure (LpExposureManager *self,
                                  gint               amount);

/* ==========================================================================
 * Exposure Level
 * ========================================================================== */

/**
 * lp_exposure_manager_get_level:
 * @self: an #LpExposureManager
 *
 * Gets the current exposure level based on the exposure value.
 *
 * Thresholds:
 * - HIDDEN: 0-24
 * - SCRUTINY: 25-49
 * - SUSPICION: 50-74
 * - HUNT: 75-99
 * - CRUSADE: 100
 *
 * Returns: The current #LpExposureLevel
 */
LpExposureLevel
lp_exposure_manager_get_level (LpExposureManager *self);

/**
 * lp_exposure_manager_get_level_for_value:
 * @value: an exposure value (0-100)
 *
 * Gets the exposure level for a given value.
 * Utility function for UI/calculations.
 *
 * Returns: The #LpExposureLevel for the given value
 */
LpExposureLevel
lp_exposure_manager_get_level_for_value (guint value);

/* ==========================================================================
 * Decay
 * ========================================================================== */

/**
 * lp_exposure_manager_get_decay_rate:
 * @self: an #LpExposureManager
 *
 * Gets the exposure decay rate per year.
 *
 * Returns: The decay rate
 */
guint
lp_exposure_manager_get_decay_rate (LpExposureManager *self);

/**
 * lp_exposure_manager_set_decay_rate:
 * @self: an #LpExposureManager
 * @rate: the decay rate per year
 *
 * Sets the exposure decay rate per year.
 */
void
lp_exposure_manager_set_decay_rate (LpExposureManager *self,
                                    guint              rate);

/**
 * lp_exposure_manager_apply_decay:
 * @self: an #LpExposureManager
 * @years: number of years to apply decay for
 *
 * Applies exposure decay for the given number of years.
 * Called at the end of each slumber period.
 */
void
lp_exposure_manager_apply_decay (LpExposureManager *self,
                                 guint              years);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_exposure_manager_reset:
 * @self: an #LpExposureManager
 *
 * Resets the exposure manager to initial state.
 * Called when starting a new game or after prestige.
 */
void
lp_exposure_manager_reset (LpExposureManager *self);

G_END_DECLS

#endif /* LP_EXPOSURE_MANAGER_H */
