/* lp-investment-financial.h - Financial Investment
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Financial investments represent monetary instruments: bonds,
 * promissory notes, loans, and insurance pools. They offer
 * fixed returns but carry risk of default by the issuer.
 *
 * Key mechanic: Kingdom default risk. When a kingdom defaults
 * on its debts, bond values collapse but can be bought cheaply.
 * Owning all of a kingdom's debt provides special opportunities.
 */

#ifndef LP_INVESTMENT_FINANCIAL_H
#define LP_INVESTMENT_FINANCIAL_H

#include <glib-object.h>
#include "lp-investment.h"

G_BEGIN_DECLS

#define LP_TYPE_INVESTMENT_FINANCIAL (lp_investment_financial_get_type ())

G_DECLARE_FINAL_TYPE (LpInvestmentFinancial, lp_investment_financial,
                      LP, INVESTMENT_FINANCIAL, LpInvestment)

/**
 * LpFinancialType:
 * @LP_FINANCIAL_TYPE_CROWN_BOND: Kingdom treasury bonds (safest)
 * @LP_FINANCIAL_TYPE_NOBLE_DEBT: Loans to noble houses
 * @LP_FINANCIAL_TYPE_MERCHANT_NOTE: Merchant promissory notes
 * @LP_FINANCIAL_TYPE_INSURANCE: Insurance pools (collect premiums)
 * @LP_FINANCIAL_TYPE_USURY: High-interest loans (risky but profitable)
 *
 * Subtypes of financial investments.
 */
typedef enum
{
    LP_FINANCIAL_TYPE_CROWN_BOND,
    LP_FINANCIAL_TYPE_NOBLE_DEBT,
    LP_FINANCIAL_TYPE_MERCHANT_NOTE,
    LP_FINANCIAL_TYPE_INSURANCE,
    LP_FINANCIAL_TYPE_USURY
} LpFinancialType;

GType lp_financial_type_get_type (void) G_GNUC_CONST;
#define LP_TYPE_FINANCIAL_TYPE (lp_financial_type_get_type ())

/**
 * LpDebtStatus:
 * @LP_DEBT_STATUS_PERFORMING: Payments being made normally
 * @LP_DEBT_STATUS_DELINQUENT: Payments behind schedule
 * @LP_DEBT_STATUS_DEFAULT: Debtor has defaulted
 *
 * Status of a debt instrument.
 */
typedef enum
{
    LP_DEBT_STATUS_PERFORMING,
    LP_DEBT_STATUS_DELINQUENT,
    LP_DEBT_STATUS_DEFAULT
} LpDebtStatus;

GType lp_debt_status_get_type (void) G_GNUC_CONST;
#define LP_TYPE_DEBT_STATUS (lp_debt_status_get_type ())

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_investment_financial_new:
 * @id: unique identifier
 * @name: display name
 * @financial_type: the #LpFinancialType
 *
 * Creates a new financial investment.
 *
 * Returns: (transfer full): A new #LpInvestmentFinancial
 */
LpInvestmentFinancial *
lp_investment_financial_new (const gchar     *id,
                             const gchar     *name,
                             LpFinancialType  financial_type);

/**
 * lp_investment_financial_new_with_value:
 * @id: unique identifier
 * @name: display name
 * @financial_type: the #LpFinancialType
 * @face_value: (transfer full): face value of the instrument
 * @interest_rate: annual interest rate (e.g., 0.05 for 5%)
 *
 * Creates a new financial investment with specified value and rate.
 *
 * Returns: (transfer full): A new #LpInvestmentFinancial
 */
LpInvestmentFinancial *
lp_investment_financial_new_with_value (const gchar     *id,
                                        const gchar     *name,
                                        LpFinancialType  financial_type,
                                        LrgBigNumber    *face_value,
                                        gdouble          interest_rate);

/* ==========================================================================
 * Financial-Specific Methods
 * ========================================================================== */

/**
 * lp_investment_financial_get_financial_type:
 * @self: an #LpInvestmentFinancial
 *
 * Gets the financial instrument subtype.
 *
 * Returns: The #LpFinancialType
 */
LpFinancialType
lp_investment_financial_get_financial_type (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_get_debt_status:
 * @self: an #LpInvestmentFinancial
 *
 * Gets the current debt status.
 *
 * Returns: The #LpDebtStatus
 */
LpDebtStatus
lp_investment_financial_get_debt_status (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_set_debt_status:
 * @self: an #LpInvestmentFinancial
 * @status: the new #LpDebtStatus
 *
 * Sets the debt status.
 */
void
lp_investment_financial_set_debt_status (LpInvestmentFinancial *self,
                                         LpDebtStatus           status);

/**
 * lp_investment_financial_get_interest_rate:
 * @self: an #LpInvestmentFinancial
 *
 * Gets the fixed interest rate.
 *
 * Returns: Interest rate (e.g., 0.05 for 5%)
 */
gdouble
lp_investment_financial_get_interest_rate (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_set_interest_rate:
 * @self: an #LpInvestmentFinancial
 * @rate: the interest rate
 *
 * Sets the interest rate.
 */
void
lp_investment_financial_set_interest_rate (LpInvestmentFinancial *self,
                                           gdouble                rate);

/**
 * lp_investment_financial_get_face_value:
 * @self: an #LpInvestmentFinancial
 *
 * Gets the face (principal) value of the instrument.
 * This is different from current market value.
 *
 * Returns: (transfer none): Face value as #LrgBigNumber
 */
LrgBigNumber *
lp_investment_financial_get_face_value (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_set_face_value:
 * @self: an #LpInvestmentFinancial
 * @value: (transfer full): the face value
 *
 * Sets the face value.
 */
void
lp_investment_financial_set_face_value (LpInvestmentFinancial *self,
                                        LrgBigNumber          *value);

/**
 * lp_investment_financial_get_maturity_year:
 * @self: an #LpInvestmentFinancial
 *
 * Gets the year when this instrument matures (for bonds).
 * 0 means no maturity (perpetual or insurance).
 *
 * Returns: Maturity year, or 0 if none
 */
guint64
lp_investment_financial_get_maturity_year (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_set_maturity_year:
 * @self: an #LpInvestmentFinancial
 * @year: the maturity year
 *
 * Sets the maturity year.
 */
void
lp_investment_financial_set_maturity_year (LpInvestmentFinancial *self,
                                           guint64                year);

/**
 * lp_investment_financial_get_issuer_id:
 * @self: an #LpInvestmentFinancial
 *
 * Gets the ID of the issuing entity (kingdom, noble house, etc.).
 *
 * Returns: (transfer none) (nullable): Issuer ID
 */
const gchar *
lp_investment_financial_get_issuer_id (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_set_issuer_id:
 * @self: an #LpInvestmentFinancial
 * @issuer_id: (nullable): the issuer ID
 *
 * Sets the issuer ID.
 */
void
lp_investment_financial_set_issuer_id (LpInvestmentFinancial *self,
                                       const gchar           *issuer_id);

/**
 * lp_investment_financial_calculate_interest_payment:
 * @self: an #LpInvestmentFinancial
 *
 * Calculates the annual interest payment.
 *
 * Returns: (transfer full): Annual interest amount
 */
LrgBigNumber *
lp_investment_financial_calculate_interest_payment (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_is_defaulted:
 * @self: an #LpInvestmentFinancial
 *
 * Checks if this instrument has defaulted.
 *
 * Returns: %TRUE if in default
 */
gboolean
lp_investment_financial_is_defaulted (LpInvestmentFinancial *self);

/**
 * lp_investment_financial_is_matured:
 * @self: an #LpInvestmentFinancial
 * @current_year: the current game year
 *
 * Checks if this instrument has matured.
 *
 * Returns: %TRUE if matured
 */
gboolean
lp_investment_financial_is_matured (LpInvestmentFinancial *self,
                                    guint64                current_year);

/**
 * lp_investment_financial_get_default_recovery_rate:
 * @self: an #LpInvestmentFinancial
 *
 * Gets the recovery rate if the instrument defaults.
 * This is the percentage of face value recovered.
 *
 * Returns: Recovery rate (e.g., 0.3 for 30%)
 */
gdouble
lp_investment_financial_get_default_recovery_rate (LpInvestmentFinancial *self);

G_END_DECLS

#endif /* LP_INVESTMENT_FINANCIAL_H */
