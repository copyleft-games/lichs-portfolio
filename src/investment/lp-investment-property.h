/* lp-investment-property.h - Property Investment
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Property investments represent real estate: land, buildings, mines,
 * and other physical holdings. They offer low risk and steady returns,
 * and typically survive political upheaval better than other assets.
 *
 * Subtypes include:
 * - Agricultural land
 * - Urban property
 * - Mining rights
 * - Timber forests
 */

#ifndef LP_INVESTMENT_PROPERTY_H
#define LP_INVESTMENT_PROPERTY_H

#include <glib-object.h>
#include "lp-investment.h"

G_BEGIN_DECLS

#define LP_TYPE_INVESTMENT_PROPERTY (lp_investment_property_get_type ())

G_DECLARE_FINAL_TYPE (LpInvestmentProperty, lp_investment_property,
                      LP, INVESTMENT_PROPERTY, LpInvestment)

/**
 * LpPropertyType:
 * @LP_PROPERTY_TYPE_AGRICULTURAL: Farmland, vineyards, pastures
 * @LP_PROPERTY_TYPE_URBAN: City buildings, shops, warehouses
 * @LP_PROPERTY_TYPE_MINING: Mines, quarries, extraction rights
 * @LP_PROPERTY_TYPE_TIMBER: Forests and lumber rights
 * @LP_PROPERTY_TYPE_COASTAL: Ports, harbors, fishing rights
 *
 * Subtypes of property investments.
 */
typedef enum
{
    LP_PROPERTY_TYPE_AGRICULTURAL,
    LP_PROPERTY_TYPE_URBAN,
    LP_PROPERTY_TYPE_MINING,
    LP_PROPERTY_TYPE_TIMBER,
    LP_PROPERTY_TYPE_COASTAL
} LpPropertyType;

GType lp_property_type_get_type (void) G_GNUC_CONST;
#define LP_TYPE_PROPERTY_TYPE (lp_property_type_get_type ())

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_investment_property_new:
 * @id: unique identifier
 * @name: display name
 * @property_type: the #LpPropertyType
 *
 * Creates a new property investment.
 *
 * Returns: (transfer full): A new #LpInvestmentProperty
 */
LpInvestmentProperty *
lp_investment_property_new (const gchar    *id,
                            const gchar    *name,
                            LpPropertyType  property_type);

/**
 * lp_investment_property_new_with_value:
 * @id: unique identifier
 * @name: display name
 * @property_type: the #LpPropertyType
 * @value: (transfer full): initial value
 *
 * Creates a new property investment with specified value.
 *
 * Returns: (transfer full): A new #LpInvestmentProperty
 */
LpInvestmentProperty *
lp_investment_property_new_with_value (const gchar    *id,
                                       const gchar    *name,
                                       LpPropertyType  property_type,
                                       LrgBigNumber   *value);

/* ==========================================================================
 * Property-Specific Methods
 * ========================================================================== */

/**
 * lp_investment_property_get_property_type:
 * @self: an #LpInvestmentProperty
 *
 * Gets the property subtype.
 *
 * Returns: The #LpPropertyType
 */
LpPropertyType
lp_investment_property_get_property_type (LpInvestmentProperty *self);

/**
 * lp_investment_property_get_stability_bonus:
 * @self: an #LpInvestmentProperty
 *
 * Gets the stability bonus. Property investments have higher stability,
 * meaning they retain value better during crises.
 *
 * Returns: Stability bonus (1.0 = normal, >1.0 = more stable)
 */
gdouble
lp_investment_property_get_stability_bonus (LpInvestmentProperty *self);

/**
 * lp_investment_property_set_stability_bonus:
 * @self: an #LpInvestmentProperty
 * @bonus: the stability bonus
 *
 * Sets the stability bonus.
 */
void
lp_investment_property_set_stability_bonus (LpInvestmentProperty *self,
                                            gdouble               bonus);

/**
 * lp_investment_property_get_improvements:
 * @self: an #LpInvestmentProperty
 *
 * Gets the number of improvements made to this property.
 * Improvements increase value and returns.
 *
 * Returns: Number of improvements
 */
guint
lp_investment_property_get_improvements (LpInvestmentProperty *self);

/**
 * lp_investment_property_add_improvement:
 * @self: an #LpInvestmentProperty
 * @cost: (transfer full): cost of the improvement
 *
 * Adds an improvement to the property, increasing its value.
 *
 * Returns: %TRUE if improvement was applied
 */
gboolean
lp_investment_property_add_improvement (LpInvestmentProperty *self,
                                        LrgBigNumber         *cost);

/**
 * lp_investment_property_get_upkeep_cost:
 * @self: an #LpInvestmentProperty
 *
 * Calculates the annual upkeep cost for this property.
 *
 * Returns: (transfer full): Annual upkeep cost
 */
LrgBigNumber *
lp_investment_property_get_upkeep_cost (LpInvestmentProperty *self);

/**
 * lp_investment_property_is_developed:
 * @self: an #LpInvestmentProperty
 *
 * Checks if the property is fully developed (max improvements).
 *
 * Returns: %TRUE if fully developed
 */
gboolean
lp_investment_property_is_developed (LpInvestmentProperty *self);

G_END_DECLS

#endif /* LP_INVESTMENT_PROPERTY_H */
