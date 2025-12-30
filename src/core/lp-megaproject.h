/* lp-megaproject.h - Multi-Century Project System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpMegaproject represents multi-century investments that provide powerful
 * late-game benefits but require sustained effort across many slumber cycles.
 *
 * Each megaproject has:
 * - Multiple phases with incremental benefits
 * - Discovery risk that can destroy progress
 * - Ongoing gold cost per year
 * - Phylactery level unlock requirement
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_MEGAPROJECT_H
#define LP_MEGAPROJECT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_MEGAPROJECT (lp_megaproject_get_type ())

G_DECLARE_FINAL_TYPE (LpMegaproject, lp_megaproject, LP, MEGAPROJECT, GObject)

/* ==========================================================================
 * LpMegaprojectPhase - Boxed type for project phases
 * ========================================================================== */

#define LP_TYPE_MEGAPROJECT_PHASE (lp_megaproject_phase_get_type ())

/**
 * LpMegaprojectPhase:
 * @name: Phase name (e.g., "Survey", "Construction")
 * @years: Years required to complete this phase
 * @effect_type: Type of effect when phase completes (nullable)
 * @effect_value: Numeric value for the effect
 *
 * Represents a single phase within a megaproject.
 * Effects are applied when the phase completes.
 */
typedef struct _LpMegaprojectPhase LpMegaprojectPhase;

struct _LpMegaprojectPhase
{
    gchar   *name;
    guint    years;
    gchar   *effect_type;     /* e.g., "property_income_bonus", "agent_travel" */
    gdouble  effect_value;
};

GType                  lp_megaproject_phase_get_type (void) G_GNUC_CONST;
LpMegaprojectPhase    *lp_megaproject_phase_new      (const gchar *name,
                                                      guint        years);
LpMegaprojectPhase    *lp_megaproject_phase_copy     (const LpMegaprojectPhase *phase);
void                   lp_megaproject_phase_free     (LpMegaprojectPhase *phase);

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_megaproject_new:
 * @id: Unique identifier
 * @name: Display name
 *
 * Creates a new megaproject with basic settings.
 *
 * Returns: (transfer full): A new #LpMegaproject
 */
LpMegaproject *
lp_megaproject_new (const gchar *id,
                    const gchar *name);

/**
 * lp_megaproject_load_from_yaml:
 * @file_path: Path to YAML definition file
 * @error: Return location for error
 *
 * Loads a megaproject definition from a YAML file.
 *
 * Returns: (transfer full) (nullable): A new #LpMegaproject, or %NULL on error
 */
LpMegaproject *
lp_megaproject_load_from_yaml (const gchar  *file_path,
                               GError      **error);

/* ==========================================================================
 * Properties Access
 * ========================================================================== */

/**
 * lp_megaproject_get_id:
 * @self: an #LpMegaproject
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The project ID
 */
const gchar *
lp_megaproject_get_id (LpMegaproject *self);

/**
 * lp_megaproject_get_name:
 * @self: an #LpMegaproject
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The project name
 */
const gchar *
lp_megaproject_get_name (LpMegaproject *self);

/**
 * lp_megaproject_get_description:
 * @self: an #LpMegaproject
 *
 * Gets the project description.
 *
 * Returns: (transfer none): The description
 */
const gchar *
lp_megaproject_get_description (LpMegaproject *self);

/**
 * lp_megaproject_get_state:
 * @self: an #LpMegaproject
 *
 * Gets the current project state.
 *
 * Returns: The project state
 */
LpMegaprojectState
lp_megaproject_get_state (LpMegaproject *self);

/**
 * lp_megaproject_get_total_duration:
 * @self: an #LpMegaproject
 *
 * Gets the total duration in years across all phases.
 *
 * Returns: Total years to complete
 */
guint
lp_megaproject_get_total_duration (LpMegaproject *self);

/**
 * lp_megaproject_get_cost_per_year:
 * @self: an #LpMegaproject
 *
 * Gets the gold cost per year to maintain progress.
 *
 * Returns: (transfer none): Cost per year
 */
const LrgBigNumber *
lp_megaproject_get_cost_per_year (LpMegaproject *self);

/**
 * lp_megaproject_get_unlock_level:
 * @self: an #LpMegaproject
 *
 * Gets the required phylactery level to unlock this project.
 *
 * Returns: Required level
 */
guint
lp_megaproject_get_unlock_level (LpMegaproject *self);

/**
 * lp_megaproject_get_discovery_risk:
 * @self: an #LpMegaproject
 *
 * Gets the discovery risk per decade (percentage).
 *
 * Returns: Risk percentage (0-100)
 */
guint
lp_megaproject_get_discovery_risk (LpMegaproject *self);

/* ==========================================================================
 * Progress Tracking
 * ========================================================================== */

/**
 * lp_megaproject_get_years_invested:
 * @self: an #LpMegaproject
 *
 * Gets the total years invested so far.
 *
 * Returns: Years invested
 */
guint
lp_megaproject_get_years_invested (LpMegaproject *self);

/**
 * lp_megaproject_get_years_remaining:
 * @self: an #LpMegaproject
 *
 * Gets the years remaining until completion.
 *
 * Returns: Years remaining
 */
guint
lp_megaproject_get_years_remaining (LpMegaproject *self);

/**
 * lp_megaproject_get_progress:
 * @self: an #LpMegaproject
 *
 * Gets the overall completion percentage.
 *
 * Returns: Progress from 0.0 to 1.0
 */
gfloat
lp_megaproject_get_progress (LpMegaproject *self);

/**
 * lp_megaproject_get_current_phase:
 * @self: an #LpMegaproject
 *
 * Gets the current phase being worked on.
 *
 * Returns: (transfer none) (nullable): Current phase, or %NULL if complete
 */
const LpMegaprojectPhase *
lp_megaproject_get_current_phase (LpMegaproject *self);

/**
 * lp_megaproject_get_current_phase_index:
 * @self: an #LpMegaproject
 *
 * Gets the index of the current phase (0-based).
 *
 * Returns: Phase index
 */
guint
lp_megaproject_get_current_phase_index (LpMegaproject *self);

/**
 * lp_megaproject_get_phases:
 * @self: an #LpMegaproject
 *
 * Gets the array of all phases.
 *
 * Returns: (transfer none) (element-type LpMegaprojectPhase): The phases array
 */
GPtrArray *
lp_megaproject_get_phases (LpMegaproject *self);

/* ==========================================================================
 * State Management
 * ========================================================================== */

/**
 * lp_megaproject_can_start:
 * @self: an #LpMegaproject
 * @phylactery_level: Current phylactery level
 *
 * Checks if this project can be started.
 *
 * Returns: %TRUE if project can be started
 */
gboolean
lp_megaproject_can_start (LpMegaproject *self,
                          guint          phylactery_level);

/**
 * lp_megaproject_start:
 * @self: an #LpMegaproject
 *
 * Starts the project. Must be in AVAILABLE state.
 *
 * Returns: %TRUE if successfully started
 */
gboolean
lp_megaproject_start (LpMegaproject *self);

/**
 * lp_megaproject_pause:
 * @self: an #LpMegaproject
 *
 * Pauses the project. Progress is preserved but no cost incurred.
 *
 * Returns: %TRUE if successfully paused
 */
gboolean
lp_megaproject_pause (LpMegaproject *self);

/**
 * lp_megaproject_resume:
 * @self: an #LpMegaproject
 *
 * Resumes a paused project.
 *
 * Returns: %TRUE if successfully resumed
 */
gboolean
lp_megaproject_resume (LpMegaproject *self);

/**
 * lp_megaproject_advance_years:
 * @self: an #LpMegaproject
 * @years: Number of years to advance
 *
 * Advances project progress by the given years.
 * Must be in ACTIVE state. Checks for phase transitions.
 *
 * Returns: %TRUE if progress was made (project still active)
 */
gboolean
lp_megaproject_advance_years (LpMegaproject *self,
                              guint          years);

/**
 * lp_megaproject_is_complete:
 * @self: an #LpMegaproject
 *
 * Checks if the project is complete.
 *
 * Returns: %TRUE if in COMPLETE state
 */
gboolean
lp_megaproject_is_complete (LpMegaproject *self);

/* ==========================================================================
 * Risk Management
 * ========================================================================== */

/**
 * lp_megaproject_roll_discovery:
 * @self: an #LpMegaproject
 *
 * Rolls for discovery risk. Should be called once per decade while active.
 * If discovered, project moves to DISCOVERED state.
 *
 * Returns: %TRUE if project was discovered
 */
gboolean
lp_megaproject_roll_discovery (LpMegaproject *self);

/**
 * lp_megaproject_is_discovered:
 * @self: an #LpMegaproject
 *
 * Checks if the project has been discovered by enemies.
 *
 * Returns: %TRUE if in DISCOVERED state
 */
gboolean
lp_megaproject_is_discovered (LpMegaproject *self);

/**
 * lp_megaproject_destroy:
 * @self: an #LpMegaproject
 *
 * Destroys the project due to enemy action.
 * Called when a discovered project is attacked.
 */
void
lp_megaproject_destroy (LpMegaproject *self);

/**
 * lp_megaproject_hide:
 * @self: an #LpMegaproject
 *
 * Attempts to re-hide a discovered project.
 * Requires significant resources (handled externally).
 *
 * Returns: %TRUE if successfully hidden
 */
gboolean
lp_megaproject_hide (LpMegaproject *self);

/* ==========================================================================
 * Phase Management
 * ========================================================================== */

/**
 * lp_megaproject_add_phase:
 * @self: an #LpMegaproject
 * @phase: (transfer full): Phase to add
 *
 * Adds a phase to the project. Used during construction or YAML loading.
 */
void
lp_megaproject_add_phase (LpMegaproject      *self,
                          LpMegaprojectPhase *phase);

/* ==========================================================================
 * Effect Queries
 * ========================================================================== */

/**
 * lp_megaproject_get_property_income_bonus:
 * @self: an #LpMegaproject
 *
 * Gets the cumulative property income bonus from completed phases.
 *
 * Returns: Bonus multiplier (0.0 = no bonus)
 */
gdouble
lp_megaproject_get_property_income_bonus (LpMegaproject *self);

/**
 * lp_megaproject_has_agent_instant_travel:
 * @self: an #LpMegaproject
 *
 * Checks if completed phases grant instant agent travel.
 *
 * Returns: %TRUE if instant travel is available
 */
gboolean
lp_megaproject_has_agent_instant_travel (LpMegaproject *self);

/**
 * lp_megaproject_has_property_seizure_immunity:
 * @self: an #LpMegaproject
 *
 * Checks if completed phases grant property seizure immunity.
 *
 * Returns: %TRUE if immunity is granted
 */
gboolean
lp_megaproject_has_property_seizure_immunity (LpMegaproject *self);

/* ==========================================================================
 * Configuration
 * ========================================================================== */

/**
 * lp_megaproject_set_description:
 * @self: an #LpMegaproject
 * @description: New description
 *
 * Sets the project description.
 */
void
lp_megaproject_set_description (LpMegaproject *self,
                                const gchar   *description);

/**
 * lp_megaproject_set_cost_per_year:
 * @self: an #LpMegaproject
 * @cost: Cost per year
 *
 * Sets the ongoing cost per year.
 */
void
lp_megaproject_set_cost_per_year (LpMegaproject      *self,
                                  const LrgBigNumber *cost);

/**
 * lp_megaproject_set_unlock_level:
 * @self: an #LpMegaproject
 * @level: Required phylactery level
 *
 * Sets the unlock requirement.
 */
void
lp_megaproject_set_unlock_level (LpMegaproject *self,
                                 guint          level);

/**
 * lp_megaproject_set_discovery_risk:
 * @self: an #LpMegaproject
 * @risk: Risk percentage per decade (0-100)
 *
 * Sets the discovery risk.
 */
void
lp_megaproject_set_discovery_risk (LpMegaproject *self,
                                   guint          risk);

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_megaproject_reset:
 * @self: an #LpMegaproject
 *
 * Resets project progress to initial state (LOCKED or AVAILABLE).
 * Used for prestige or full game reset.
 */
void
lp_megaproject_reset (LpMegaproject *self);

G_END_DECLS

#endif /* LP_MEGAPROJECT_H */
