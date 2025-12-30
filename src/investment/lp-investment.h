/* lp-investment.h - Base Investment Class
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Base class for all investment types. Investments are financial assets
 * that the lich owns and manages through their agents. Each investment
 * type has different risk/reward profiles and responds differently to
 * world events.
 *
 * This is a derivable type - subclass it for specific investment types.
 * Implements LrgSaveable for persistence.
 */

#ifndef LP_INVESTMENT_H
#define LP_INVESTMENT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_INVESTMENT (lp_investment_get_type ())

G_DECLARE_DERIVABLE_TYPE (LpInvestment, lp_investment, LP, INVESTMENT, GObject)

/**
 * LpInvestmentClass:
 * @parent_class: parent class
 * @calculate_returns: calculate returns over a period of years
 * @apply_event: apply effects of a world event
 * @can_sell: check if investment can be sold
 * @get_risk_modifier: get current risk modifier for the investment
 * @get_description: get a detailed description of the investment
 *
 * Virtual methods for investment subclasses.
 */
struct _LpInvestmentClass
{
    GObjectClass parent_class;

    /*< public >*/

    /**
     * LpInvestmentClass::calculate_returns:
     * @self: an #LpInvestment
     * @years: number of years to calculate
     *
     * Calculates the returns for this investment over the given period.
     * Subclasses override this to implement asset-class-specific returns.
     *
     * Returns: (transfer full): The calculated returns as #LrgBigNumber
     */
    LrgBigNumber * (*calculate_returns) (LpInvestment *self,
                                         guint         years);

    /**
     * LpInvestmentClass::apply_event:
     * @self: an #LpInvestment
     * @event: the #LpEvent to apply
     *
     * Applies the effects of a world event to this investment.
     * Subclasses override this to handle asset-class-specific events.
     */
    void (*apply_event) (LpInvestment *self,
                         LpEvent      *event);

    /**
     * LpInvestmentClass::can_sell:
     * @self: an #LpInvestment
     *
     * Checks if this investment can currently be sold.
     * Some investments may have lock-in periods or other restrictions.
     *
     * Returns: %TRUE if the investment can be sold
     */
    gboolean (*can_sell) (LpInvestment *self);

    /**
     * LpInvestmentClass::get_risk_modifier:
     * @self: an #LpInvestment
     *
     * Gets the current risk modifier for this investment.
     * This affects return calculations and event vulnerability.
     *
     * Returns: Risk modifier (1.0 = normal risk)
     */
    gdouble (*get_risk_modifier) (LpInvestment *self);

    /**
     * LpInvestmentClass::get_base_return_rate:
     * @self: an #LpInvestment
     *
     * Gets the base annual return rate for this investment type.
     * Subclasses override to provide asset-class-specific rates.
     *
     * Returns: Base annual return rate (e.g., 0.05 for 5%)
     */
    gdouble (*get_base_return_rate) (LpInvestment *self);

    /*< private >*/
    gpointer _reserved[8];
};

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_investment_new:
 * @id: unique identifier for this investment
 * @name: display name
 * @asset_class: the #LpAssetClass
 *
 * Creates a new investment with the given properties.
 * Note: For actual gameplay, use the subclass constructors instead.
 *
 * Returns: (transfer full): A new #LpInvestment
 */
LpInvestment *
lp_investment_new (const gchar  *id,
                   const gchar  *name,
                   LpAssetClass  asset_class);

/* ==========================================================================
 * Properties Getters/Setters
 * ========================================================================== */

/**
 * lp_investment_get_id:
 * @self: an #LpInvestment
 *
 * Gets the unique identifier.
 *
 * Returns: (transfer none): The investment ID
 */
const gchar *
lp_investment_get_id (LpInvestment *self);

/**
 * lp_investment_get_name:
 * @self: an #LpInvestment
 *
 * Gets the display name.
 *
 * Returns: (transfer none): The investment name
 */
const gchar *
lp_investment_get_name (LpInvestment *self);

/**
 * lp_investment_set_name:
 * @self: an #LpInvestment
 * @name: the new name
 *
 * Sets the display name.
 */
void
lp_investment_set_name (LpInvestment *self,
                        const gchar  *name);

/**
 * lp_investment_get_description:
 * @self: an #LpInvestment
 *
 * Gets the description.
 *
 * Returns: (transfer none) (nullable): The description, or %NULL
 */
const gchar *
lp_investment_get_description (LpInvestment *self);

/**
 * lp_investment_set_description:
 * @self: an #LpInvestment
 * @description: (nullable): the new description
 *
 * Sets the description.
 */
void
lp_investment_set_description (LpInvestment *self,
                               const gchar  *description);

/**
 * lp_investment_get_asset_class:
 * @self: an #LpInvestment
 *
 * Gets the asset class.
 *
 * Returns: The #LpAssetClass
 */
LpAssetClass
lp_investment_get_asset_class (LpInvestment *self);

/**
 * lp_investment_get_risk_level:
 * @self: an #LpInvestment
 *
 * Gets the risk level.
 *
 * Returns: The #LpRiskLevel
 */
LpRiskLevel
lp_investment_get_risk_level (LpInvestment *self);

/**
 * lp_investment_set_risk_level:
 * @self: an #LpInvestment
 * @level: the new #LpRiskLevel
 *
 * Sets the risk level.
 */
void
lp_investment_set_risk_level (LpInvestment *self,
                              LpRiskLevel   level);

/**
 * lp_investment_get_purchase_price:
 * @self: an #LpInvestment
 *
 * Gets the original purchase price.
 *
 * Returns: (transfer none): The purchase price as #LrgBigNumber
 */
LrgBigNumber *
lp_investment_get_purchase_price (LpInvestment *self);

/**
 * lp_investment_set_purchase_price:
 * @self: an #LpInvestment
 * @price: (transfer full): the new purchase price
 *
 * Sets the purchase price.
 */
void
lp_investment_set_purchase_price (LpInvestment *self,
                                  LrgBigNumber *price);

/**
 * lp_investment_get_current_value:
 * @self: an #LpInvestment
 *
 * Gets the current market value.
 *
 * Returns: (transfer none): The current value as #LrgBigNumber
 */
LrgBigNumber *
lp_investment_get_current_value (LpInvestment *self);

/**
 * lp_investment_set_current_value:
 * @self: an #LpInvestment
 * @value: (transfer full): the new current value
 *
 * Sets the current value.
 */
void
lp_investment_set_current_value (LpInvestment *self,
                                 LrgBigNumber *value);

/**
 * lp_investment_get_purchase_year:
 * @self: an #LpInvestment
 *
 * Gets the year of purchase.
 *
 * Returns: The purchase year
 */
guint64
lp_investment_get_purchase_year (LpInvestment *self);

/**
 * lp_investment_set_purchase_year:
 * @self: an #LpInvestment
 * @year: the purchase year
 *
 * Sets the purchase year.
 */
void
lp_investment_set_purchase_year (LpInvestment *self,
                                 guint64       year);

/**
 * lp_investment_get_region_id:
 * @self: an #LpInvestment
 *
 * Gets the ID of the region where this investment is located.
 *
 * Returns: (transfer none) (nullable): The region ID, or %NULL if not region-bound
 */
const gchar *
lp_investment_get_region_id (LpInvestment *self);

/**
 * lp_investment_set_region_id:
 * @self: an #LpInvestment
 * @region_id: (nullable): the region ID
 *
 * Sets the region ID.
 */
void
lp_investment_set_region_id (LpInvestment *self,
                             const gchar  *region_id);

/* ==========================================================================
 * Virtual Method Wrappers
 * ========================================================================== */

/**
 * lp_investment_calculate_returns:
 * @self: an #LpInvestment
 * @years: number of years to calculate
 *
 * Calculates the returns for this investment over the given period.
 *
 * Returns: (transfer full): The calculated returns as #LrgBigNumber
 */
LrgBigNumber *
lp_investment_calculate_returns (LpInvestment *self,
                                 guint         years);

/**
 * lp_investment_apply_event:
 * @self: an #LpInvestment
 * @event: the #LpEvent to apply
 *
 * Applies the effects of a world event to this investment.
 */
void
lp_investment_apply_event (LpInvestment *self,
                           LpEvent      *event);

/**
 * lp_investment_can_sell:
 * @self: an #LpInvestment
 *
 * Checks if this investment can currently be sold.
 *
 * Returns: %TRUE if the investment can be sold
 */
gboolean
lp_investment_can_sell (LpInvestment *self);

/**
 * lp_investment_get_risk_modifier:
 * @self: an #LpInvestment
 *
 * Gets the current risk modifier.
 *
 * Returns: Risk modifier (1.0 = normal risk)
 */
gdouble
lp_investment_get_risk_modifier (LpInvestment *self);

/**
 * lp_investment_get_base_return_rate:
 * @self: an #LpInvestment
 *
 * Gets the base annual return rate.
 *
 * Returns: Base annual return rate (e.g., 0.05 for 5%)
 */
gdouble
lp_investment_get_base_return_rate (LpInvestment *self);

/* ==========================================================================
 * Utility Methods
 * ========================================================================== */

/**
 * lp_investment_get_age:
 * @self: an #LpInvestment
 * @current_year: the current game year
 *
 * Gets the age of this investment in years.
 *
 * Returns: Years since purchase
 */
guint64
lp_investment_get_age (LpInvestment *self,
                       guint64       current_year);

/**
 * lp_investment_get_return_percentage:
 * @self: an #LpInvestment
 *
 * Calculates the total return percentage since purchase.
 *
 * Returns: Return as a percentage (e.g., 150.0 for 150% return)
 */
gdouble
lp_investment_get_return_percentage (LpInvestment *self);

/**
 * lp_investment_get_exposure_contribution:
 * @self: an #LpInvestment
 *
 * Gets the contribution to the player's exposure level.
 * Larger/riskier investments increase exposure more.
 *
 * Returns: Exposure contribution (0-100 scale)
 */
guint
lp_investment_get_exposure_contribution (LpInvestment *self);

G_END_DECLS

#endif /* LP_INVESTMENT_H */
