/* lp-gameplay-settings.c - Game-specific settings group implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-gameplay-settings.h"
#include <gio/gio.h>

/**
 * SECTION:lp-gameplay-settings
 * @title: LpGameplaySettings
 * @short_description: Game-specific settings group
 *
 * #LpGameplaySettings manages gameplay-specific settings unique to
 * Lich's Portfolio, including autosave configuration and event
 * notification preferences.
 */

/* Default values */
#define DEFAULT_AUTOSAVE_ENABLED    TRUE
#define DEFAULT_AUTOSAVE_INTERVAL   5
#define DEFAULT_PAUSE_ON_EVENTS     TRUE
#define DEFAULT_SHOW_NOTIFICATIONS  TRUE
#define DEFAULT_DIFFICULTY          LP_DIFFICULTY_NORMAL
#define DEFAULT_GAME_SPEED          LP_GAME_SPEED_NORMAL

/* Limits */
#define MIN_AUTOSAVE_INTERVAL       1
#define MAX_AUTOSAVE_INTERVAL       60

struct _LpGameplaySettings
{
    LrgSettingsGroup parent_instance;

    /* Autosave settings */
    gboolean autosave_enabled;
    guint    autosave_interval;

    /* Event settings */
    gboolean pause_on_events;
    gboolean show_notifications;

    /* Difficulty and speed settings */
    LpDifficulty difficulty;
    LpGameSpeed  game_speed;
};

G_DEFINE_TYPE (LpGameplaySettings, lp_gameplay_settings, LRG_TYPE_SETTINGS_GROUP)

enum
{
    PROP_0,
    PROP_AUTOSAVE_ENABLED,
    PROP_AUTOSAVE_INTERVAL,
    PROP_PAUSE_ON_EVENTS,
    PROP_SHOW_NOTIFICATIONS,
    PROP_DIFFICULTY,
    PROP_GAME_SPEED,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Helper to emit changed signal and mark dirty.
 */
static void
emit_changed (LpGameplaySettings *self,
              const gchar        *property_name)
{
    lrg_settings_group_mark_dirty (LRG_SETTINGS_GROUP (self));
    g_signal_emit_by_name (self, "changed", property_name);
}

/* ==========================================================================
 * Virtual Method Implementations
 * ========================================================================== */

static void
lp_gameplay_settings_apply (LrgSettingsGroup *group)
{
    /*
     * Gameplay settings don't need to apply to any system.
     * They are read directly when needed.
     */
    (void)group;
}

static void
lp_gameplay_settings_reset (LrgSettingsGroup *group)
{
    LpGameplaySettings *self = LP_GAMEPLAY_SETTINGS (group);

    self->autosave_enabled = DEFAULT_AUTOSAVE_ENABLED;
    self->autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
    self->pause_on_events = DEFAULT_PAUSE_ON_EVENTS;
    self->show_notifications = DEFAULT_SHOW_NOTIFICATIONS;
    self->difficulty = DEFAULT_DIFFICULTY;
    self->game_speed = DEFAULT_GAME_SPEED;

    emit_changed (self, NULL);
}

static const gchar *
lp_gameplay_settings_get_group_name (LrgSettingsGroup *group)
{
    (void)group;
    return "gameplay";
}

static GVariant *
lp_gameplay_settings_serialize (LrgSettingsGroup  *group,
                                GError           **error)
{
    LpGameplaySettings *self = LP_GAMEPLAY_SETTINGS (group);
    GVariantBuilder builder;

    (void)error;

    g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{sv}"));

    g_variant_builder_add (&builder, "{sv}", "autosave_enabled",
                           g_variant_new_boolean (self->autosave_enabled));
    g_variant_builder_add (&builder, "{sv}", "autosave_interval",
                           g_variant_new_uint32 (self->autosave_interval));
    g_variant_builder_add (&builder, "{sv}", "pause_on_events",
                           g_variant_new_boolean (self->pause_on_events));
    g_variant_builder_add (&builder, "{sv}", "show_notifications",
                           g_variant_new_boolean (self->show_notifications));
    g_variant_builder_add (&builder, "{sv}", "difficulty",
                           g_variant_new_int32 ((gint32)self->difficulty));
    g_variant_builder_add (&builder, "{sv}", "game_speed",
                           g_variant_new_int32 ((gint32)self->game_speed));

    return g_variant_builder_end (&builder);
}

static gboolean
lp_gameplay_settings_deserialize (LrgSettingsGroup  *group,
                                  GVariant          *data,
                                  GError           **error)
{
    LpGameplaySettings *self = LP_GAMEPLAY_SETTINGS (group);
    GVariant *value;

    if (!g_variant_is_of_type (data, G_VARIANT_TYPE ("a{sv}")))
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                     "Expected a{sv} variant for gameplay settings");
        return FALSE;
    }

    value = g_variant_lookup_value (data, "autosave_enabled", G_VARIANT_TYPE_BOOLEAN);
    if (value)
    {
        self->autosave_enabled = g_variant_get_boolean (value);
        g_variant_unref (value);
    }

    value = g_variant_lookup_value (data, "autosave_interval", G_VARIANT_TYPE_UINT32);
    if (value)
    {
        self->autosave_interval = CLAMP (g_variant_get_uint32 (value),
                                          MIN_AUTOSAVE_INTERVAL,
                                          MAX_AUTOSAVE_INTERVAL);
        g_variant_unref (value);
    }

    value = g_variant_lookup_value (data, "pause_on_events", G_VARIANT_TYPE_BOOLEAN);
    if (value)
    {
        self->pause_on_events = g_variant_get_boolean (value);
        g_variant_unref (value);
    }

    value = g_variant_lookup_value (data, "show_notifications", G_VARIANT_TYPE_BOOLEAN);
    if (value)
    {
        self->show_notifications = g_variant_get_boolean (value);
        g_variant_unref (value);
    }

    value = g_variant_lookup_value (data, "difficulty", G_VARIANT_TYPE_INT32);
    if (value)
    {
        gint32 diff = g_variant_get_int32 (value);
        self->difficulty = CLAMP (diff, LP_DIFFICULTY_EASY, LP_DIFFICULTY_HARD);
        g_variant_unref (value);
    }

    value = g_variant_lookup_value (data, "game_speed", G_VARIANT_TYPE_INT32);
    if (value)
    {
        gint32 speed = g_variant_get_int32 (value);
        self->game_speed = CLAMP (speed, LP_GAME_SPEED_NORMAL, LP_GAME_SPEED_FASTEST);
        g_variant_unref (value);
    }

    return TRUE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_gameplay_settings_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    LpGameplaySettings *self = LP_GAMEPLAY_SETTINGS (object);

    switch (prop_id)
    {
    case PROP_AUTOSAVE_ENABLED:
        g_value_set_boolean (value, self->autosave_enabled);
        break;
    case PROP_AUTOSAVE_INTERVAL:
        g_value_set_uint (value, self->autosave_interval);
        break;
    case PROP_PAUSE_ON_EVENTS:
        g_value_set_boolean (value, self->pause_on_events);
        break;
    case PROP_SHOW_NOTIFICATIONS:
        g_value_set_boolean (value, self->show_notifications);
        break;
    case PROP_DIFFICULTY:
        g_value_set_int (value, (gint)self->difficulty);
        break;
    case PROP_GAME_SPEED:
        g_value_set_int (value, (gint)self->game_speed);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_gameplay_settings_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    LpGameplaySettings *self = LP_GAMEPLAY_SETTINGS (object);

    switch (prop_id)
    {
    case PROP_AUTOSAVE_ENABLED:
        lp_gameplay_settings_set_autosave_enabled (self, g_value_get_boolean (value));
        break;
    case PROP_AUTOSAVE_INTERVAL:
        lp_gameplay_settings_set_autosave_interval (self, g_value_get_uint (value));
        break;
    case PROP_PAUSE_ON_EVENTS:
        lp_gameplay_settings_set_pause_on_events (self, g_value_get_boolean (value));
        break;
    case PROP_SHOW_NOTIFICATIONS:
        lp_gameplay_settings_set_show_notifications (self, g_value_get_boolean (value));
        break;
    case PROP_DIFFICULTY:
        lp_gameplay_settings_set_difficulty (self, (LpDifficulty)g_value_get_int (value));
        break;
    case PROP_GAME_SPEED:
        lp_gameplay_settings_set_game_speed (self, (LpGameSpeed)g_value_get_int (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_gameplay_settings_class_init (LpGameplaySettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgSettingsGroupClass *group_class = LRG_SETTINGS_GROUP_CLASS (klass);

    object_class->get_property = lp_gameplay_settings_get_property;
    object_class->set_property = lp_gameplay_settings_set_property;

    /* Override virtual methods */
    group_class->apply = lp_gameplay_settings_apply;
    group_class->reset = lp_gameplay_settings_reset;
    group_class->get_group_name = lp_gameplay_settings_get_group_name;
    group_class->serialize = lp_gameplay_settings_serialize;
    group_class->deserialize = lp_gameplay_settings_deserialize;

    /* Install properties */
    properties[PROP_AUTOSAVE_ENABLED] =
        g_param_spec_boolean ("autosave-enabled", "Autosave Enabled",
                              "Whether autosave is enabled",
                              DEFAULT_AUTOSAVE_ENABLED,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_AUTOSAVE_INTERVAL] =
        g_param_spec_uint ("autosave-interval", "Autosave Interval",
                           "Autosave interval in minutes",
                           MIN_AUTOSAVE_INTERVAL, MAX_AUTOSAVE_INTERVAL,
                           DEFAULT_AUTOSAVE_INTERVAL,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_PAUSE_ON_EVENTS] =
        g_param_spec_boolean ("pause-on-events", "Pause On Events",
                              "Whether to pause on major events",
                              DEFAULT_PAUSE_ON_EVENTS,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_SHOW_NOTIFICATIONS] =
        g_param_spec_boolean ("show-notifications", "Show Notifications",
                              "Whether to show event notifications",
                              DEFAULT_SHOW_NOTIFICATIONS,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_DIFFICULTY] =
        g_param_spec_int ("difficulty", "Difficulty",
                          "Game difficulty level (0=Easy, 1=Normal, 2=Hard)",
                          LP_DIFFICULTY_EASY, LP_DIFFICULTY_HARD,
                          DEFAULT_DIFFICULTY,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_GAME_SPEED] =
        g_param_spec_int ("game-speed", "Game Speed",
                          "Simulation speed multiplier (0=1x, 1=2x, 2=4x, 3=10x)",
                          LP_GAME_SPEED_NORMAL, LP_GAME_SPEED_FASTEST,
                          DEFAULT_GAME_SPEED,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_gameplay_settings_init (LpGameplaySettings *self)
{
    self->autosave_enabled = DEFAULT_AUTOSAVE_ENABLED;
    self->autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
    self->pause_on_events = DEFAULT_PAUSE_ON_EVENTS;
    self->show_notifications = DEFAULT_SHOW_NOTIFICATIONS;
    self->difficulty = DEFAULT_DIFFICULTY;
    self->game_speed = DEFAULT_GAME_SPEED;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

LpGameplaySettings *
lp_gameplay_settings_new (void)
{
    return g_object_new (LP_TYPE_GAMEPLAY_SETTINGS, NULL);
}

gboolean
lp_gameplay_settings_get_autosave_enabled (LpGameplaySettings *self)
{
    g_return_val_if_fail (LP_IS_GAMEPLAY_SETTINGS (self), FALSE);
    return self->autosave_enabled;
}

void
lp_gameplay_settings_set_autosave_enabled (LpGameplaySettings *self,
                                           gboolean            enabled)
{
    g_return_if_fail (LP_IS_GAMEPLAY_SETTINGS (self));

    enabled = !!enabled;

    if (self->autosave_enabled != enabled)
    {
        self->autosave_enabled = enabled;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AUTOSAVE_ENABLED]);
        emit_changed (self, "autosave-enabled");
    }
}

guint
lp_gameplay_settings_get_autosave_interval (LpGameplaySettings *self)
{
    g_return_val_if_fail (LP_IS_GAMEPLAY_SETTINGS (self), DEFAULT_AUTOSAVE_INTERVAL);
    return self->autosave_interval;
}

void
lp_gameplay_settings_set_autosave_interval (LpGameplaySettings *self,
                                            guint               minutes)
{
    g_return_if_fail (LP_IS_GAMEPLAY_SETTINGS (self));

    minutes = CLAMP (minutes, MIN_AUTOSAVE_INTERVAL, MAX_AUTOSAVE_INTERVAL);

    if (self->autosave_interval != minutes)
    {
        self->autosave_interval = minutes;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AUTOSAVE_INTERVAL]);
        emit_changed (self, "autosave-interval");
    }
}

gboolean
lp_gameplay_settings_get_pause_on_events (LpGameplaySettings *self)
{
    g_return_val_if_fail (LP_IS_GAMEPLAY_SETTINGS (self), FALSE);
    return self->pause_on_events;
}

void
lp_gameplay_settings_set_pause_on_events (LpGameplaySettings *self,
                                          gboolean            pause)
{
    g_return_if_fail (LP_IS_GAMEPLAY_SETTINGS (self));

    pause = !!pause;

    if (self->pause_on_events != pause)
    {
        self->pause_on_events = pause;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAUSE_ON_EVENTS]);
        emit_changed (self, "pause-on-events");
    }
}

gboolean
lp_gameplay_settings_get_show_notifications (LpGameplaySettings *self)
{
    g_return_val_if_fail (LP_IS_GAMEPLAY_SETTINGS (self), FALSE);
    return self->show_notifications;
}

void
lp_gameplay_settings_set_show_notifications (LpGameplaySettings *self,
                                             gboolean            show)
{
    g_return_if_fail (LP_IS_GAMEPLAY_SETTINGS (self));

    show = !!show;

    if (self->show_notifications != show)
    {
        self->show_notifications = show;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SHOW_NOTIFICATIONS]);
        emit_changed (self, "show-notifications");
    }
}

LpDifficulty
lp_gameplay_settings_get_difficulty (LpGameplaySettings *self)
{
    g_return_val_if_fail (LP_IS_GAMEPLAY_SETTINGS (self), DEFAULT_DIFFICULTY);
    return self->difficulty;
}

void
lp_gameplay_settings_set_difficulty (LpGameplaySettings *self,
                                     LpDifficulty        difficulty)
{
    g_return_if_fail (LP_IS_GAMEPLAY_SETTINGS (self));

    difficulty = CLAMP (difficulty, LP_DIFFICULTY_EASY, LP_DIFFICULTY_HARD);

    if (self->difficulty != difficulty)
    {
        self->difficulty = difficulty;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DIFFICULTY]);
        emit_changed (self, "difficulty");
    }
}

LpGameSpeed
lp_gameplay_settings_get_game_speed (LpGameplaySettings *self)
{
    g_return_val_if_fail (LP_IS_GAMEPLAY_SETTINGS (self), DEFAULT_GAME_SPEED);
    return self->game_speed;
}

void
lp_gameplay_settings_set_game_speed (LpGameplaySettings *self,
                                     LpGameSpeed         speed)
{
    g_return_if_fail (LP_IS_GAMEPLAY_SETTINGS (self));

    speed = CLAMP (speed, LP_GAME_SPEED_NORMAL, LP_GAME_SPEED_FASTEST);

    if (self->game_speed != speed)
    {
        self->game_speed = speed;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_GAME_SPEED]);
        emit_changed (self, "game-speed");
    }
}

gdouble
lp_gameplay_settings_get_speed_multiplier (LpGameplaySettings *self)
{
    g_return_val_if_fail (LP_IS_GAMEPLAY_SETTINGS (self), 1.0);

    switch (self->game_speed)
    {
    case LP_GAME_SPEED_NORMAL:
        return 1.0;
    case LP_GAME_SPEED_FAST:
        return 2.0;
    case LP_GAME_SPEED_FASTER:
        return 4.0;
    case LP_GAME_SPEED_FASTEST:
        return 10.0;
    default:
        return 1.0;
    }
}
