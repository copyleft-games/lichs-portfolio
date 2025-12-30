/* lp-event.h - World Event Base Class
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Base class for all world events. Events occur during slumber periods
 * and can affect kingdoms, regions, investments, and agents.
 *
 * This is a derivable type - subclass it for specific event types:
 * - LpEventEconomic: Market crashes, trade discoveries
 * - LpEventPolitical: Wars, successions, revolutions
 * - LpEventMagical: Artifacts, divine intervention
 * - LpEventPersonal: Agent deaths, betrayals
 *
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_EVENT_H
#define LP_EVENT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

/* ==========================================================================
 * LpEventChoice - Boxed type for player choices
 * ========================================================================== */

#define LP_TYPE_EVENT_CHOICE (lp_event_choice_get_type ())

/**
 * LpEventChoice:
 * @id: unique identifier for this choice
 * @text: display text for the choice
 * @consequence: description of what will happen
 * @requires_gold: whether gold payment is needed
 * @gold_cost: cost if requires_gold is TRUE
 * @requires_agent: whether an agent is needed to execute
 *
 * Represents a player choice for an event.
 */
struct _LpEventChoice
{
    gchar   *id;
    gchar   *text;
    gchar   *consequence;
    gboolean requires_gold;
    guint64  gold_cost;
    gboolean requires_agent;
};

GType
lp_event_choice_get_type (void) G_GNUC_CONST;

/**
 * lp_event_choice_new:
 * @id: unique identifier
 * @text: display text
 *
 * Creates a new event choice.
 *
 * Returns: (transfer full): A new #LpEventChoice
 */
LpEventChoice *
lp_event_choice_new (const gchar *id,
                     const gchar *text);

/**
 * lp_event_choice_copy:
 * @choice: an #LpEventChoice
 *
 * Creates a copy of an event choice.
 *
 * Returns: (transfer full): A copy of @choice
 */
LpEventChoice *
lp_event_choice_copy (const LpEventChoice *choice);

/**
 * lp_event_choice_free:
 * @choice: an #LpEventChoice
 *
 * Frees an event choice.
 */
void
lp_event_choice_free (LpEventChoice *choice);

/* ==========================================================================
 * LpEvent - Derivable base class
 * ========================================================================== */

#define LP_TYPE_EVENT (lp_event_get_type ())

G_DECLARE_DERIVABLE_TYPE (LpEvent, lp_event, LP, EVENT, GObject)

/**
 * LpEventClass:
 * @parent_class: parent class
 * @apply_effects: apply event effects to the world simulation
 * @get_choices: get available player choices (may return NULL)
 * @get_investment_modifier: get modifier for affected investments
 * @get_narrative_text: get flavor text for display
 * @can_occur: check if event preconditions are met
 *
 * Virtual methods for event subclasses.
 */
struct _LpEventClass
{
    GObjectClass parent_class;

    /*< public >*/

    /**
     * LpEventClass::apply_effects:
     * @self: an #LpEvent
     * @simulation: the #LpWorldSimulation
     *
     * Applies this event's effects to the world simulation.
     * Subclasses override to implement specific effects.
     */
    void (*apply_effects) (LpEvent           *self,
                           LpWorldSimulation *simulation);

    /**
     * LpEventClass::get_choices:
     * @self: an #LpEvent
     *
     * Gets the available player choices for this event.
     *
     * Returns: (transfer full) (element-type LpEventChoice) (nullable):
     *          Array of choices, or %NULL if no choices
     */
    GPtrArray *(*get_choices) (LpEvent *self);

    /**
     * LpEventClass::get_investment_modifier:
     * @self: an #LpEvent
     * @investment: the #LpInvestment to check
     *
     * Gets the income modifier this event applies to an investment.
     *
     * Returns: Modifier (1.0 = no change)
     */
    gdouble (*get_investment_modifier) (LpEvent      *self,
                                        LpInvestment *investment);

    /**
     * LpEventClass::get_narrative_text:
     * @self: an #LpEvent
     *
     * Gets the narrative/flavor text for this event.
     *
     * Returns: (transfer full): Narrative text
     */
    gchar *(*get_narrative_text) (LpEvent *self);

    /**
     * LpEventClass::can_occur:
     * @self: an #LpEvent
     * @simulation: the #LpWorldSimulation
     *
     * Checks if this event's preconditions are met.
     *
     * Returns: %TRUE if the event can occur
     */
    gboolean (*can_occur) (LpEvent           *self,
                           LpWorldSimulation *simulation);

    /*< private >*/
    gpointer _reserved[8];
};

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_event_new:
 * @id: unique identifier
 * @name: display name
 * @event_type: the #LpEventType
 *
 * Creates a new base event. For gameplay, use subclass constructors.
 *
 * Returns: (transfer full): A new #LpEvent
 */
LpEvent *
lp_event_new (const gchar *id,
              const gchar *name,
              LpEventType  event_type);

/* ==========================================================================
 * Property Accessors
 * ========================================================================== */

/**
 * lp_event_get_id:
 * @self: an #LpEvent
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The event ID
 */
const gchar *
lp_event_get_id (LpEvent *self);

/**
 * lp_event_get_name:
 * @self: an #LpEvent
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The event name
 */
const gchar *
lp_event_get_name (LpEvent *self);

/**
 * lp_event_set_name:
 * @self: an #LpEvent
 * @name: the new name
 *
 * Sets the display name.
 */
void
lp_event_set_name (LpEvent     *self,
                   const gchar *name);

/**
 * lp_event_get_description:
 * @self: an #LpEvent
 *
 * Gets the description.
 *
 * Returns: (transfer none): The description
 */
const gchar *
lp_event_get_description (LpEvent *self);

/**
 * lp_event_set_description:
 * @self: an #LpEvent
 * @description: the new description
 *
 * Sets the description.
 */
void
lp_event_set_description (LpEvent     *self,
                          const gchar *description);

/**
 * lp_event_get_event_type:
 * @self: an #LpEvent
 *
 * Gets the event type.
 *
 * Returns: The #LpEventType
 */
LpEventType
lp_event_get_event_type (LpEvent *self);

/**
 * lp_event_get_severity:
 * @self: an #LpEvent
 *
 * Gets the event severity.
 *
 * Returns: The #LpEventSeverity
 */
LpEventSeverity
lp_event_get_severity (LpEvent *self);

/**
 * lp_event_set_severity:
 * @self: an #LpEvent
 * @severity: the #LpEventSeverity
 *
 * Sets the event severity.
 */
void
lp_event_set_severity (LpEvent         *self,
                       LpEventSeverity  severity);

/**
 * lp_event_get_year_occurred:
 * @self: an #LpEvent
 *
 * Gets the year the event occurred.
 *
 * Returns: The year
 */
guint64
lp_event_get_year_occurred (LpEvent *self);

/**
 * lp_event_set_year_occurred:
 * @self: an #LpEvent
 * @year: the year
 *
 * Sets the year the event occurred.
 */
void
lp_event_set_year_occurred (LpEvent *self,
                            guint64  year);

/**
 * lp_event_get_affects_region_id:
 * @self: an #LpEvent
 *
 * Gets the ID of the affected region.
 *
 * Returns: (transfer none) (nullable): The region ID, or %NULL
 */
const gchar *
lp_event_get_affects_region_id (LpEvent *self);

/**
 * lp_event_set_affects_region_id:
 * @self: an #LpEvent
 * @region_id: (nullable): the region ID
 *
 * Sets the affected region.
 */
void
lp_event_set_affects_region_id (LpEvent     *self,
                                const gchar *region_id);

/**
 * lp_event_get_affects_kingdom_id:
 * @self: an #LpEvent
 *
 * Gets the ID of the affected kingdom.
 *
 * Returns: (transfer none) (nullable): The kingdom ID, or %NULL
 */
const gchar *
lp_event_get_affects_kingdom_id (LpEvent *self);

/**
 * lp_event_set_affects_kingdom_id:
 * @self: an #LpEvent
 * @kingdom_id: (nullable): the kingdom ID
 *
 * Sets the affected kingdom.
 */
void
lp_event_set_affects_kingdom_id (LpEvent     *self,
                                 const gchar *kingdom_id);

/**
 * lp_event_get_duration_years:
 * @self: an #LpEvent
 *
 * Gets the event duration in years (0 = instant).
 *
 * Returns: Duration in years
 */
guint
lp_event_get_duration_years (LpEvent *self);

/**
 * lp_event_set_duration_years:
 * @self: an #LpEvent
 * @years: duration in years
 *
 * Sets the event duration.
 */
void
lp_event_set_duration_years (LpEvent *self,
                             guint    years);

/**
 * lp_event_get_is_active:
 * @self: an #LpEvent
 *
 * Gets whether the event is currently active.
 *
 * Returns: %TRUE if active
 */
gboolean
lp_event_get_is_active (LpEvent *self);

/**
 * lp_event_set_is_active:
 * @self: an #LpEvent
 * @active: whether active
 *
 * Sets whether the event is active.
 */
void
lp_event_set_is_active (LpEvent  *self,
                        gboolean  active);

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

/**
 * lp_event_apply_effects:
 * @self: an #LpEvent
 * @simulation: the #LpWorldSimulation
 *
 * Applies this event's effects to the simulation.
 */
void
lp_event_apply_effects (LpEvent           *self,
                        LpWorldSimulation *simulation);

/**
 * lp_event_get_choices:
 * @self: an #LpEvent
 *
 * Gets available player choices.
 *
 * Returns: (transfer full) (element-type LpEventChoice) (nullable): Choices array
 */
GPtrArray *
lp_event_get_choices (LpEvent *self);

/**
 * lp_event_get_investment_modifier:
 * @self: an #LpEvent
 * @investment: the #LpInvestment
 *
 * Gets the modifier for an investment.
 *
 * Returns: Modifier (1.0 = no change)
 */
gdouble
lp_event_get_investment_modifier (LpEvent      *self,
                                  LpInvestment *investment);

/**
 * lp_event_get_narrative_text:
 * @self: an #LpEvent
 *
 * Gets narrative text.
 *
 * Returns: (transfer full): Narrative text
 */
gchar *
lp_event_get_narrative_text (LpEvent *self);

/**
 * lp_event_can_occur:
 * @self: an #LpEvent
 * @simulation: the #LpWorldSimulation
 *
 * Checks if preconditions are met.
 *
 * Returns: %TRUE if can occur
 */
gboolean
lp_event_can_occur (LpEvent           *self,
                    LpWorldSimulation *simulation);

/* ==========================================================================
 * Event Lifecycle
 * ========================================================================== */

/**
 * lp_event_tick_year:
 * @self: an #LpEvent
 *
 * Advances the event by one year. Decrements duration and
 * deactivates when duration reaches zero.
 *
 * Returns: %TRUE if the event is still active
 */
gboolean
lp_event_tick_year (LpEvent *self);

/**
 * lp_event_resolve:
 * @self: an #LpEvent
 * @choice_id: (nullable): ID of the player's choice
 *
 * Resolves the event, optionally with a player choice.
 * Deactivates the event.
 */
void
lp_event_resolve (LpEvent     *self,
                  const gchar *choice_id);

G_END_DECLS

#endif /* LP_EVENT_H */
