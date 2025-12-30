/* lp-exposure-manager.c - Exposure Tracking Singleton
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-exposure-manager.h"

/* Exposure thresholds for each level */
#define THRESHOLD_SCRUTINY  (25)
#define THRESHOLD_SUSPICION (50)
#define THRESHOLD_HUNT      (75)
#define THRESHOLD_CRUSADE   (100)

/* Default values */
#define DEFAULT_EXPOSURE    (0)
#define DEFAULT_DECAY_RATE  (5)
#define MAX_EXPOSURE        (100)

struct _LpExposureManager
{
    GObject parent_instance;

    guint exposure;      /* Current exposure value (0-100) */
    guint decay_rate;    /* Decay per year during slumber */
};

enum
{
    PROP_0,
    PROP_EXPOSURE,
    PROP_DECAY_RATE,
    N_PROPS
};

enum
{
    SIGNAL_THRESHOLD_CROSSED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static LpExposureManager *default_manager = NULL;

G_DEFINE_TYPE (LpExposureManager, lp_exposure_manager, G_TYPE_OBJECT)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_exposure_manager_get_default:
 *
 * Gets the default exposure manager instance.
 *
 * Returns: (transfer none): The default #LpExposureManager
 */
LpExposureManager *
lp_exposure_manager_get_default (void)
{
    if (default_manager == NULL)
        default_manager = g_object_new (LP_TYPE_EXPOSURE_MANAGER, NULL);

    return default_manager;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_exposure_manager_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    LpExposureManager *self = LP_EXPOSURE_MANAGER (object);

    switch (prop_id)
    {
    case PROP_EXPOSURE:
        g_value_set_uint (value, self->exposure);
        break;

    case PROP_DECAY_RATE:
        g_value_set_uint (value, self->decay_rate);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_exposure_manager_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    LpExposureManager *self = LP_EXPOSURE_MANAGER (object);

    switch (prop_id)
    {
    case PROP_EXPOSURE:
        lp_exposure_manager_set_exposure (self, g_value_get_uint (value));
        break;

    case PROP_DECAY_RATE:
        lp_exposure_manager_set_decay_rate (self, g_value_get_uint (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
lp_exposure_manager_finalize (GObject *object)
{
    lp_log_debug ("Finalizing exposure manager");

    if (default_manager == LP_EXPOSURE_MANAGER (object))
        default_manager = NULL;

    G_OBJECT_CLASS (lp_exposure_manager_parent_class)->finalize (object);
}

static void
lp_exposure_manager_class_init (LpExposureManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = lp_exposure_manager_get_property;
    object_class->set_property = lp_exposure_manager_set_property;
    object_class->finalize = lp_exposure_manager_finalize;

    /**
     * LpExposureManager:exposure:
     *
     * The current exposure value (0-100).
     */
    properties[PROP_EXPOSURE] =
        g_param_spec_uint ("exposure",
                           "Exposure",
                           "Current exposure value (0-100)",
                           0, MAX_EXPOSURE, DEFAULT_EXPOSURE,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    /**
     * LpExposureManager:decay-rate:
     *
     * The exposure decay rate per year.
     */
    properties[PROP_DECAY_RATE] =
        g_param_spec_uint ("decay-rate",
                           "Decay Rate",
                           "Exposure decay per year",
                           0, MAX_EXPOSURE, DEFAULT_DECAY_RATE,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY |
                           G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * LpExposureManager::threshold-crossed:
     * @self: the #LpExposureManager
     * @old_level: the previous #LpExposureLevel
     * @new_level: the new #LpExposureLevel
     *
     * Emitted when the exposure crosses a threshold boundary.
     */
    signals[SIGNAL_THRESHOLD_CROSSED] =
        g_signal_new ("threshold-crossed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      LP_TYPE_EXPOSURE_LEVEL,
                      LP_TYPE_EXPOSURE_LEVEL);
}

static void
lp_exposure_manager_init (LpExposureManager *self)
{
    self->exposure = DEFAULT_EXPOSURE;
    self->decay_rate = DEFAULT_DECAY_RATE;
}

/* ==========================================================================
 * Exposure Value
 * ========================================================================== */

/**
 * lp_exposure_manager_get_exposure:
 * @self: an #LpExposureManager
 *
 * Gets the current exposure value.
 *
 * Returns: The current exposure (0-100)
 */
guint
lp_exposure_manager_get_exposure (LpExposureManager *self)
{
    g_return_val_if_fail (LP_IS_EXPOSURE_MANAGER (self), 0);

    return self->exposure;
}

/**
 * lp_exposure_manager_set_exposure:
 * @self: an #LpExposureManager
 * @value: the new exposure value
 *
 * Sets the exposure value, clamped to 0-100.
 */
void
lp_exposure_manager_set_exposure (LpExposureManager *self,
                                  guint              value)
{
    LpExposureLevel old_level;
    LpExposureLevel new_level;
    guint clamped;

    g_return_if_fail (LP_IS_EXPOSURE_MANAGER (self));

    clamped = MIN (value, MAX_EXPOSURE);

    if (self->exposure == clamped)
        return;

    old_level = lp_exposure_manager_get_level (self);
    self->exposure = clamped;
    new_level = lp_exposure_manager_get_level (self);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EXPOSURE]);

    /* Emit signal if we crossed a threshold */
    if (old_level != new_level)
    {
        lp_log_info ("Exposure threshold crossed: %d -> %d (value: %u)",
                     old_level, new_level, clamped);
        g_signal_emit (self, signals[SIGNAL_THRESHOLD_CROSSED], 0,
                       old_level, new_level);
    }
}

/**
 * lp_exposure_manager_add_exposure:
 * @self: an #LpExposureManager
 * @amount: amount to add (can be negative)
 *
 * Adds to the current exposure value.
 */
void
lp_exposure_manager_add_exposure (LpExposureManager *self,
                                  gint               amount)
{
    gint new_value;

    g_return_if_fail (LP_IS_EXPOSURE_MANAGER (self));

    new_value = (gint)self->exposure + amount;
    lp_exposure_manager_set_exposure (self, (guint)CLAMP (new_value, 0, MAX_EXPOSURE));
}

/* ==========================================================================
 * Exposure Level
 * ========================================================================== */

/**
 * lp_exposure_manager_get_level_for_value:
 * @value: an exposure value (0-100)
 *
 * Gets the exposure level for a given value.
 *
 * Returns: The #LpExposureLevel for the value
 */
LpExposureLevel
lp_exposure_manager_get_level_for_value (guint value)
{
    if (value >= THRESHOLD_CRUSADE)
        return LP_EXPOSURE_LEVEL_CRUSADE;
    else if (value >= THRESHOLD_HUNT)
        return LP_EXPOSURE_LEVEL_HUNT;
    else if (value >= THRESHOLD_SUSPICION)
        return LP_EXPOSURE_LEVEL_SUSPICION;
    else if (value >= THRESHOLD_SCRUTINY)
        return LP_EXPOSURE_LEVEL_SCRUTINY;
    else
        return LP_EXPOSURE_LEVEL_HIDDEN;
}

/**
 * lp_exposure_manager_get_level:
 * @self: an #LpExposureManager
 *
 * Gets the current exposure level.
 *
 * Returns: The current #LpExposureLevel
 */
LpExposureLevel
lp_exposure_manager_get_level (LpExposureManager *self)
{
    g_return_val_if_fail (LP_IS_EXPOSURE_MANAGER (self),
                          LP_EXPOSURE_LEVEL_HIDDEN);

    return lp_exposure_manager_get_level_for_value (self->exposure);
}

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
lp_exposure_manager_get_decay_rate (LpExposureManager *self)
{
    g_return_val_if_fail (LP_IS_EXPOSURE_MANAGER (self), DEFAULT_DECAY_RATE);

    return self->decay_rate;
}

/**
 * lp_exposure_manager_set_decay_rate:
 * @self: an #LpExposureManager
 * @rate: the decay rate per year
 *
 * Sets the exposure decay rate per year.
 */
void
lp_exposure_manager_set_decay_rate (LpExposureManager *self,
                                    guint              rate)
{
    g_return_if_fail (LP_IS_EXPOSURE_MANAGER (self));

    if (self->decay_rate == rate)
        return;

    self->decay_rate = rate;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DECAY_RATE]);
}

/**
 * lp_exposure_manager_apply_decay:
 * @self: an #LpExposureManager
 * @years: number of years to apply decay for
 *
 * Applies exposure decay for the given number of years.
 */
void
lp_exposure_manager_apply_decay (LpExposureManager *self,
                                 guint              years)
{
    guint decay_amount;

    g_return_if_fail (LP_IS_EXPOSURE_MANAGER (self));

    if (years == 0 || self->exposure == 0)
        return;

    decay_amount = self->decay_rate * years;

    lp_log_debug ("Applying exposure decay: %u years * %u rate = %u",
                  years, self->decay_rate, decay_amount);

    lp_exposure_manager_add_exposure (self, -(gint)decay_amount);
}

/* ==========================================================================
 * Reset
 * ========================================================================== */

/**
 * lp_exposure_manager_reset:
 * @self: an #LpExposureManager
 *
 * Resets the exposure manager to initial state.
 */
void
lp_exposure_manager_reset (LpExposureManager *self)
{
    g_return_if_fail (LP_IS_EXPOSURE_MANAGER (self));

    lp_log_debug ("Resetting exposure manager");

    self->exposure = DEFAULT_EXPOSURE;
    self->decay_rate = DEFAULT_DECAY_RATE;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EXPOSURE]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DECAY_RATE]);
}
