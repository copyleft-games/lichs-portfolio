/* lp-agent.h - Base Agent Class
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Base class for all agent types. Agents are mortals (or immortals) who serve
 * the lich by managing investments and gathering intelligence. Each agent type
 * has different characteristics for longevity, skill transfer, and loyalty.
 *
 * This is a derivable type - subclass it for specific agent types.
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_AGENT_H
#define LP_AGENT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_AGENT (lp_agent_get_type ())

G_DECLARE_DERIVABLE_TYPE (LpAgent, lp_agent, LP, AGENT, GObject)

/**
 * LpAgentClass:
 * @parent_class: parent class
 * @on_year_passed: called each simulated year for aging and updates
 * @on_death: handle agent death (cleanup, trigger succession)
 * @on_betrayal: handle agent betrayal (exposure increase)
 * @can_recruit: check if agent can recruit successors
 * @get_agent_type: get the specific agent type
 *
 * Virtual methods for agent subclasses.
 */
struct _LpAgentClass
{
    GObjectClass parent_class;

    /*< public >*/

    /**
     * LpAgentClass::on_year_passed:
     * @self: an #LpAgent
     *
     * Called each simulated year. Handles aging, loyalty decay,
     * and any subclass-specific year-end processing.
     */
    void (*on_year_passed) (LpAgent *self);

    /**
     * LpAgentClass::on_death:
     * @self: an #LpAgent
     *
     * Called when the agent dies (of old age or other causes).
     * Subclasses override to handle succession logic.
     */
    void (*on_death) (LpAgent *self);

    /**
     * LpAgentClass::on_betrayal:
     * @self: an #LpAgent
     *
     * Called when the agent betrays the lich.
     * This typically increases exposure based on knowledge level.
     */
    void (*on_betrayal) (LpAgent *self);

    /**
     * LpAgentClass::can_recruit:
     * @self: an #LpAgent
     *
     * Checks if this agent can recruit successors.
     *
     * Returns: %TRUE if the agent can recruit
     */
    gboolean (*can_recruit) (LpAgent *self);

    /**
     * LpAgentClass::get_agent_type:
     * @self: an #LpAgent
     *
     * Gets the specific agent type for this instance.
     * Subclasses override to return their type.
     *
     * Returns: The #LpAgentType
     */
    LpAgentType (*get_agent_type) (LpAgent *self);

    /*< private >*/
    gpointer _reserved[8];
};

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_agent_new:
 * @id: unique identifier for this agent
 * @name: display name
 *
 * Creates a new base agent. Note: For gameplay, use subclass constructors.
 *
 * Returns: (transfer full): A new #LpAgent
 */
LpAgent *
lp_agent_new (const gchar *id,
              const gchar *name);

/* ==========================================================================
 * Property Getters/Setters
 * ========================================================================== */

/**
 * lp_agent_get_id:
 * @self: an #LpAgent
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The agent ID
 */
const gchar *
lp_agent_get_id (LpAgent *self);

/**
 * lp_agent_get_name:
 * @self: an #LpAgent
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The agent name
 */
const gchar *
lp_agent_get_name (LpAgent *self);

/**
 * lp_agent_set_name:
 * @self: an #LpAgent
 * @name: the new name
 *
 * Sets the display name.
 */
void
lp_agent_set_name (LpAgent     *self,
                   const gchar *name);

/**
 * lp_agent_get_age:
 * @self: an #LpAgent
 *
 * Gets the current age in years.
 *
 * Returns: The agent's age
 */
guint
lp_agent_get_age (LpAgent *self);

/**
 * lp_agent_set_age:
 * @self: an #LpAgent
 * @age: the new age
 *
 * Sets the age.
 */
void
lp_agent_set_age (LpAgent *self,
                  guint    age);

/**
 * lp_agent_get_max_age:
 * @self: an #LpAgent
 *
 * Gets the maximum lifespan.
 *
 * Returns: Maximum age before death
 */
guint
lp_agent_get_max_age (LpAgent *self);

/**
 * lp_agent_set_max_age:
 * @self: an #LpAgent
 * @max_age: the new maximum age
 *
 * Sets the maximum lifespan.
 */
void
lp_agent_set_max_age (LpAgent *self,
                      guint    max_age);

/**
 * lp_agent_get_loyalty:
 * @self: an #LpAgent
 *
 * Gets the loyalty level (0-100).
 *
 * Returns: Loyalty score
 */
gint
lp_agent_get_loyalty (LpAgent *self);

/**
 * lp_agent_set_loyalty:
 * @self: an #LpAgent
 * @loyalty: the new loyalty (clamped to 0-100)
 *
 * Sets the loyalty level.
 */
void
lp_agent_set_loyalty (LpAgent *self,
                      gint     loyalty);

/**
 * lp_agent_get_competence:
 * @self: an #LpAgent
 *
 * Gets the competence level (0-100).
 *
 * Returns: Competence score
 */
gint
lp_agent_get_competence (LpAgent *self);

/**
 * lp_agent_set_competence:
 * @self: an #LpAgent
 * @competence: the new competence (clamped to 0-100)
 *
 * Sets the competence level.
 */
void
lp_agent_set_competence (LpAgent *self,
                         gint     competence);

/**
 * lp_agent_get_cover_status:
 * @self: an #LpAgent
 *
 * Gets the cover identity status.
 *
 * Returns: The #LpCoverStatus
 */
LpCoverStatus
lp_agent_get_cover_status (LpAgent *self);

/**
 * lp_agent_set_cover_status:
 * @self: an #LpAgent
 * @status: the new #LpCoverStatus
 *
 * Sets the cover status.
 */
void
lp_agent_set_cover_status (LpAgent       *self,
                           LpCoverStatus  status);

/**
 * lp_agent_get_knowledge_level:
 * @self: an #LpAgent
 *
 * Gets how much the agent knows about their true master.
 *
 * Returns: The #LpKnowledgeLevel
 */
LpKnowledgeLevel
lp_agent_get_knowledge_level (LpAgent *self);

/**
 * lp_agent_set_knowledge_level:
 * @self: an #LpAgent
 * @level: the new #LpKnowledgeLevel
 *
 * Sets the knowledge level.
 */
void
lp_agent_set_knowledge_level (LpAgent          *self,
                              LpKnowledgeLevel  level);

/**
 * lp_agent_get_traits:
 * @self: an #LpAgent
 *
 * Gets the array of traits this agent possesses.
 *
 * Returns: (transfer none) (element-type LpTrait): Array of traits
 */
GPtrArray *
lp_agent_get_traits (LpAgent *self);

/**
 * lp_agent_add_trait:
 * @self: an #LpAgent
 * @trait: (transfer none): the #LpTrait to add
 *
 * Adds a trait to this agent.
 */
void
lp_agent_add_trait (LpAgent *self,
                    LpTrait *trait);

/**
 * lp_agent_remove_trait:
 * @self: an #LpAgent
 * @trait: the #LpTrait to remove
 *
 * Removes a trait from this agent.
 *
 * Returns: %TRUE if the trait was removed
 */
gboolean
lp_agent_remove_trait (LpAgent *self,
                       LpTrait *trait);

/**
 * lp_agent_has_trait:
 * @self: an #LpAgent
 * @trait_id: the trait ID to check for
 *
 * Checks if the agent has a specific trait.
 *
 * Returns: %TRUE if the agent has the trait
 */
gboolean
lp_agent_has_trait (LpAgent     *self,
                    const gchar *trait_id);

/**
 * lp_agent_get_assigned_investments:
 * @self: an #LpAgent
 *
 * Gets investments assigned to this agent.
 *
 * Returns: (transfer none) (element-type LpInvestment): Array of investments
 */
GPtrArray *
lp_agent_get_assigned_investments (LpAgent *self);

/**
 * lp_agent_assign_investment:
 * @self: an #LpAgent
 * @investment: (transfer none): the #LpInvestment to assign
 *
 * Assigns an investment to this agent.
 */
void
lp_agent_assign_investment (LpAgent      *self,
                            LpInvestment *investment);

/**
 * lp_agent_unassign_investment:
 * @self: an #LpAgent
 * @investment: the #LpInvestment to unassign
 *
 * Removes an investment assignment.
 *
 * Returns: %TRUE if the investment was unassigned
 */
gboolean
lp_agent_unassign_investment (LpAgent      *self,
                              LpInvestment *investment);

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

/**
 * lp_agent_on_year_passed:
 * @self: an #LpAgent
 *
 * Called to process a year passing for this agent.
 */
void
lp_agent_on_year_passed (LpAgent *self);

/**
 * lp_agent_on_death:
 * @self: an #LpAgent
 *
 * Called when the agent dies.
 */
void
lp_agent_on_death (LpAgent *self);

/**
 * lp_agent_on_betrayal:
 * @self: an #LpAgent
 *
 * Called when the agent betrays the lich.
 */
void
lp_agent_on_betrayal (LpAgent *self);

/**
 * lp_agent_can_recruit:
 * @self: an #LpAgent
 *
 * Checks if the agent can recruit successors.
 *
 * Returns: %TRUE if recruitment is possible
 */
gboolean
lp_agent_can_recruit (LpAgent *self);

/**
 * lp_agent_get_agent_type:
 * @self: an #LpAgent
 *
 * Gets the specific agent type.
 *
 * Returns: The #LpAgentType
 */
LpAgentType
lp_agent_get_agent_type (LpAgent *self);

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

/**
 * lp_agent_is_alive:
 * @self: an #LpAgent
 *
 * Checks if the agent is still alive.
 *
 * Returns: %TRUE if age < max_age
 */
gboolean
lp_agent_is_alive (LpAgent *self);

/**
 * lp_agent_get_years_remaining:
 * @self: an #LpAgent
 *
 * Gets the estimated years of life remaining.
 *
 * Returns: Years until max_age
 */
guint
lp_agent_get_years_remaining (LpAgent *self);

/**
 * lp_agent_get_income_modifier:
 * @self: an #LpAgent
 *
 * Gets the income modifier from agent competence and traits.
 * Applied to investments managed by this agent.
 *
 * Returns: Income modifier (1.0 = no change)
 */
gdouble
lp_agent_get_income_modifier (LpAgent *self);

/**
 * lp_agent_get_exposure_contribution:
 * @self: an #LpAgent
 *
 * Gets this agent's contribution to overall exposure.
 * Based on cover status and knowledge level.
 *
 * Returns: Exposure contribution (0-100 scale)
 */
guint
lp_agent_get_exposure_contribution (LpAgent *self);

/**
 * lp_agent_roll_betrayal:
 * @self: an #LpAgent
 *
 * Rolls for potential betrayal based on loyalty and knowledge.
 *
 * Returns: %TRUE if the agent betrays
 */
gboolean
lp_agent_roll_betrayal (LpAgent *self);

G_END_DECLS

#endif /* LP_AGENT_H */
