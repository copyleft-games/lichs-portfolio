/* lp-megaproject.c - Multi-Century Project System Implementation
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-megaproject.h"
#include "../lp-log.h"

/* ==========================================================================
 * LpMegaprojectPhase - Boxed Type Implementation
 * ========================================================================== */

G_DEFINE_BOXED_TYPE (LpMegaprojectPhase, lp_megaproject_phase,
                     lp_megaproject_phase_copy, lp_megaproject_phase_free)

LpMegaprojectPhase *
lp_megaproject_phase_new (const gchar *name,
                          guint        years)
{
    LpMegaprojectPhase *phase;

    phase = g_new0 (LpMegaprojectPhase, 1);
    phase->name = g_strdup (name);
    phase->years = years;
    phase->effect_type = NULL;
    phase->effect_value = 0.0;

    return phase;
}

LpMegaprojectPhase *
lp_megaproject_phase_copy (const LpMegaprojectPhase *phase)
{
    LpMegaprojectPhase *copy;

    if (phase == NULL)
        return NULL;

    copy = g_new0 (LpMegaprojectPhase, 1);
    copy->name = g_strdup (phase->name);
    copy->years = phase->years;
    copy->effect_type = g_strdup (phase->effect_type);
    copy->effect_value = phase->effect_value;

    return copy;
}

void
lp_megaproject_phase_free (LpMegaprojectPhase *phase)
{
    if (phase == NULL)
        return;

    g_free (phase->name);
    g_free (phase->effect_type);
    g_free (phase);
}

/* ==========================================================================
 * LpMegaproject - Private Structure
 * ========================================================================== */

struct _LpMegaproject
{
    GObject parent_instance;

    /* Identity */
    gchar *id;
    gchar *name;
    gchar *description;

    /* Configuration */
    LrgBigNumber *cost_per_year;
    guint         unlock_level;
    guint         discovery_risk;    /* per decade, 0-100 */

    /* Phases */
    GPtrArray *phases;               /* of LpMegaprojectPhase */
    guint      total_duration;       /* calculated from phases */

    /* Progress state */
    LpMegaprojectState state;
    guint              years_invested;
    guint              current_phase_index;
    guint              years_in_current_phase;

    /* Completed phase effects (cached) */
    gdouble  property_income_bonus;
    gboolean has_instant_travel;
    gboolean has_seizure_immunity;
};

/* ==========================================================================
 * Interface Declarations
 * ========================================================================== */

static void lp_megaproject_saveable_init (LrgSaveableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LpMegaproject, lp_megaproject, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (LRG_TYPE_SAVEABLE,
                                                lp_megaproject_saveable_init))

/* ==========================================================================
 * Property Definitions
 * ========================================================================== */

enum
{
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_DESCRIPTION,
    PROP_STATE,
    PROP_TOTAL_DURATION,
    PROP_YEARS_INVESTED,
    PROP_CURRENT_PHASE_INDEX,
    PROP_PROGRESS,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* ==========================================================================
 * Signal Definitions
 * ========================================================================== */

enum
{
    SIGNAL_STATE_CHANGED,
    SIGNAL_PHASE_COMPLETED,
    SIGNAL_DISCOVERED,
    SIGNAL_DESTROYED,
    SIGNAL_COMPLETED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

/*
 * Recalculates the total duration from all phases.
 */
static void
recalculate_total_duration (LpMegaproject *self)
{
    guint total;
    guint i;

    total = 0;
    for (i = 0; i < self->phases->len; i++)
    {
        LpMegaprojectPhase *phase;

        phase = g_ptr_array_index (self->phases, i);
        total += phase->years;
    }
    self->total_duration = total;
}

/*
 * Updates cached effect values from completed phases.
 */
static void
update_cached_effects (LpMegaproject *self)
{
    guint i;

    self->property_income_bonus = 0.0;
    self->has_instant_travel = FALSE;
    self->has_seizure_immunity = FALSE;

    /* Sum effects from all completed phases */
    for (i = 0; i < self->current_phase_index; i++)
    {
        LpMegaprojectPhase *phase;

        phase = g_ptr_array_index (self->phases, i);
        if (phase->effect_type == NULL)
            continue;

        if (g_strcmp0 (phase->effect_type, "property_income_bonus") == 0)
        {
            self->property_income_bonus += phase->effect_value;
        }
        else if (g_strcmp0 (phase->effect_type, "agent_travel") == 0)
        {
            self->has_instant_travel = TRUE;
        }
        else if (g_strcmp0 (phase->effect_type, "property_immune_seizure") == 0)
        {
            self->has_seizure_immunity = TRUE;
        }
    }
}

/*
 * Sets the state and emits notification.
 */
static void
set_state (LpMegaproject      *self,
           LpMegaprojectState  new_state)
{
    LpMegaprojectState old_state;

    if (self->state == new_state)
        return;

    old_state = self->state;
    self->state = new_state;

    g_signal_emit (self, signals[SIGNAL_STATE_CHANGED], 0, old_state, new_state);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STATE]);
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_megaproject_finalize (GObject *object)
{
    LpMegaproject *self;

    self = LP_MEGAPROJECT (object);

    g_free (self->id);
    g_free (self->name);
    g_free (self->description);
    g_clear_pointer (&self->cost_per_year, lrg_big_number_free);
    g_ptr_array_unref (self->phases);

    G_OBJECT_CLASS (lp_megaproject_parent_class)->finalize (object);
}

static void
lp_megaproject_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    LpMegaproject *self;

    self = LP_MEGAPROJECT (object);

    switch (prop_id)
    {
        case PROP_ID:
            g_value_set_string (value, self->id);
            break;
        case PROP_NAME:
            g_value_set_string (value, self->name);
            break;
        case PROP_DESCRIPTION:
            g_value_set_string (value, self->description);
            break;
        case PROP_STATE:
            g_value_set_enum (value, self->state);
            break;
        case PROP_TOTAL_DURATION:
            g_value_set_uint (value, self->total_duration);
            break;
        case PROP_YEARS_INVESTED:
            g_value_set_uint (value, self->years_invested);
            break;
        case PROP_CURRENT_PHASE_INDEX:
            g_value_set_uint (value, self->current_phase_index);
            break;
        case PROP_PROGRESS:
            g_value_set_float (value, lp_megaproject_get_progress (self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
lp_megaproject_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    LpMegaproject *self;

    self = LP_MEGAPROJECT (object);

    switch (prop_id)
    {
        case PROP_ID:
            g_free (self->id);
            self->id = g_value_dup_string (value);
            break;
        case PROP_NAME:
            g_free (self->name);
            self->name = g_value_dup_string (value);
            break;
        case PROP_DESCRIPTION:
            g_free (self->description);
            self->description = g_value_dup_string (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
lp_megaproject_class_init (LpMegaprojectClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lp_megaproject_finalize;
    object_class->get_property = lp_megaproject_get_property;
    object_class->set_property = lp_megaproject_set_property;

    /* Properties */
    properties[PROP_ID] =
        g_param_spec_string ("id", "ID",
                             "Unique identifier",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_NAME] =
        g_param_spec_string ("name", "Name",
                             "Display name",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_DESCRIPTION] =
        g_param_spec_string ("description", "Description",
                             "Project description",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    properties[PROP_STATE] =
        g_param_spec_enum ("state", "State",
                           "Current project state",
                           LP_TYPE_MEGAPROJECT_STATE,
                           LP_MEGAPROJECT_STATE_LOCKED,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_TOTAL_DURATION] =
        g_param_spec_uint ("total-duration", "Total Duration",
                           "Total years to complete",
                           0, G_MAXUINT, 0,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_YEARS_INVESTED] =
        g_param_spec_uint ("years-invested", "Years Invested",
                           "Years invested so far",
                           0, G_MAXUINT, 0,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_CURRENT_PHASE_INDEX] =
        g_param_spec_uint ("current-phase-index", "Current Phase Index",
                           "Index of current phase",
                           0, G_MAXUINT, 0,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties[PROP_PROGRESS] =
        g_param_spec_float ("progress", "Progress",
                            "Completion progress (0.0 to 1.0)",
                            0.0f, 1.0f, 0.0f,
                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /* Signals */

    /**
     * LpMegaproject::state-changed:
     * @self: an #LpMegaproject
     * @old_state: Previous state
     * @new_state: New state
     *
     * Emitted when project state changes.
     */
    signals[SIGNAL_STATE_CHANGED] =
        g_signal_new ("state-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 2,
                      LP_TYPE_MEGAPROJECT_STATE,
                      LP_TYPE_MEGAPROJECT_STATE);

    /**
     * LpMegaproject::phase-completed:
     * @self: an #LpMegaproject
     * @phase_index: Index of completed phase
     * @phase: The completed phase
     *
     * Emitted when a phase completes.
     */
    signals[SIGNAL_PHASE_COMPLETED] =
        g_signal_new ("phase-completed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 2,
                      G_TYPE_UINT,
                      LP_TYPE_MEGAPROJECT_PHASE);

    /**
     * LpMegaproject::discovered:
     * @self: an #LpMegaproject
     *
     * Emitted when project is discovered by enemies.
     */
    signals[SIGNAL_DISCOVERED] =
        g_signal_new ("discovered",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpMegaproject::destroyed:
     * @self: an #LpMegaproject
     *
     * Emitted when project is destroyed.
     */
    signals[SIGNAL_DESTROYED] =
        g_signal_new ("destroyed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);

    /**
     * LpMegaproject::completed:
     * @self: an #LpMegaproject
     *
     * Emitted when project fully completes.
     */
    signals[SIGNAL_COMPLETED] =
        g_signal_new ("completed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);
}

static void
lp_megaproject_init (LpMegaproject *self)
{
    self->id = NULL;
    self->name = NULL;
    self->description = NULL;
    self->cost_per_year = lrg_big_number_new (0.0);
    self->unlock_level = 0;
    self->discovery_risk = 0;
    self->phases = g_ptr_array_new_with_free_func (
        (GDestroyNotify) lp_megaproject_phase_free);
    self->total_duration = 0;
    self->state = LP_MEGAPROJECT_STATE_LOCKED;
    self->years_invested = 0;
    self->current_phase_index = 0;
    self->years_in_current_phase = 0;
    self->property_income_bonus = 0.0;
    self->has_instant_travel = FALSE;
    self->has_seizure_immunity = FALSE;
}

/* ==========================================================================
 * LrgSaveable Implementation
 * ========================================================================== */

static const gchar *
lp_megaproject_get_save_id (LrgSaveable *saveable)
{
    LpMegaproject *self;

    self = LP_MEGAPROJECT (saveable);
    return self->id;
}

static gboolean
lp_megaproject_save (LrgSaveable    *saveable,
                     LrgSaveContext *ctx,
                     GError        **error)
{
    LpMegaproject *self;
    guint i;

    self = LP_MEGAPROJECT (saveable);

    lrg_save_context_write_string (ctx, "id", self->id);
    lrg_save_context_write_string (ctx, "name", self->name);
    lrg_save_context_write_string (ctx, "description", self->description);
    lrg_save_context_write_double (ctx, "cost-per-year-mantissa",
                                   lrg_big_number_get_mantissa (self->cost_per_year));
    lrg_save_context_write_int (ctx, "cost-per-year-exponent",
                                lrg_big_number_get_exponent (self->cost_per_year));
    lrg_save_context_write_uint (ctx, "unlock-level", self->unlock_level);
    lrg_save_context_write_uint (ctx, "discovery-risk", self->discovery_risk);
    lrg_save_context_write_uint (ctx, "state", self->state);
    lrg_save_context_write_uint (ctx, "years-invested", self->years_invested);
    lrg_save_context_write_uint (ctx, "current-phase-index", self->current_phase_index);
    lrg_save_context_write_uint (ctx, "years-in-current-phase", self->years_in_current_phase);

    /* Save phases using section-based approach */
    lrg_save_context_write_uint (ctx, "phase-count", self->phases->len);
    for (i = 0; i < self->phases->len; i++)
    {
        LpMegaprojectPhase *phase;
        g_autofree gchar *section_name = NULL;

        phase = g_ptr_array_index (self->phases, i);
        section_name = g_strdup_printf ("phase-%u", i);

        lrg_save_context_begin_section (ctx, section_name);
        lrg_save_context_write_string (ctx, "name", phase->name);
        lrg_save_context_write_uint (ctx, "years", phase->years);
        if (phase->effect_type != NULL)
        {
            lrg_save_context_write_string (ctx, "effect-type", phase->effect_type);
            lrg_save_context_write_double (ctx, "effect-value", phase->effect_value);
        }
        lrg_save_context_end_section (ctx);
    }

    return TRUE;
}

static gboolean
lp_megaproject_load (LrgSaveable    *saveable,
                     LrgSaveContext *ctx,
                     GError        **error)
{
    LpMegaproject *self;
    guint phase_count;
    guint i;

    self = LP_MEGAPROJECT (saveable);

    g_free (self->id);
    self->id = lrg_save_context_read_string (ctx, "id", NULL);

    g_free (self->name);
    self->name = lrg_save_context_read_string (ctx, "name", NULL);

    g_free (self->description);
    self->description = lrg_save_context_read_string (ctx, "description", NULL);

    g_clear_pointer (&self->cost_per_year, lrg_big_number_free);
    {
        gdouble mantissa = lrg_save_context_read_double (ctx, "cost-per-year-mantissa", 0.0);
        gint64 exponent = lrg_save_context_read_int (ctx, "cost-per-year-exponent", 0);
        if (mantissa == 0.0)
            self->cost_per_year = lrg_big_number_new_zero ();
        else
            self->cost_per_year = lrg_big_number_new_from_parts (mantissa, exponent);
    }

    self->unlock_level = lrg_save_context_read_uint (ctx, "unlock-level", 0);
    self->discovery_risk = lrg_save_context_read_uint (ctx, "discovery-risk", 0);
    self->state = lrg_save_context_read_uint (ctx, "state", LP_MEGAPROJECT_STATE_LOCKED);
    self->years_invested = lrg_save_context_read_uint (ctx, "years-invested", 0);
    self->current_phase_index = lrg_save_context_read_uint (ctx, "current-phase-index", 0);
    self->years_in_current_phase = lrg_save_context_read_uint (ctx, "years-in-current-phase", 0);

    /* Load phases */
    g_ptr_array_set_size (self->phases, 0);
    phase_count = lrg_save_context_read_uint (ctx, "phase-count", 0);

    for (i = 0; i < phase_count; i++)
    {
        g_autofree gchar *section_name = NULL;

        section_name = g_strdup_printf ("phase-%u", i);

        if (lrg_save_context_enter_section (ctx, section_name))
        {
            LpMegaprojectPhase *phase;
            g_autofree gchar *name = NULL;
            guint years;

            name = lrg_save_context_read_string (ctx, "name", "Unknown");
            years = lrg_save_context_read_uint (ctx, "years", 0);

            phase = lp_megaproject_phase_new (name, years);
            phase->effect_type = lrg_save_context_read_string (ctx, "effect-type", NULL);
            phase->effect_value = lrg_save_context_read_double (ctx, "effect-value", 0.0);

            g_ptr_array_add (self->phases, phase);
            lrg_save_context_leave_section (ctx);
        }
    }

    recalculate_total_duration (self);
    update_cached_effects (self);

    return TRUE;
}

static void
lp_megaproject_saveable_init (LrgSaveableInterface *iface)
{
    iface->get_save_id = lp_megaproject_get_save_id;
    iface->save = lp_megaproject_save;
    iface->load = lp_megaproject_load;
}

/* ==========================================================================
 * Public API - Construction
 * ========================================================================== */

LpMegaproject *
lp_megaproject_new (const gchar *id,
                    const gchar *name)
{
    return g_object_new (LP_TYPE_MEGAPROJECT,
                         "id", id,
                         "name", name,
                         NULL);
}

LpMegaproject *
lp_megaproject_load_from_yaml (const gchar  *file_path,
                               GError      **error)
{
    g_autoptr(LpMegaproject) project = NULL;
    g_autoptr(GKeyFile) keyfile = NULL;
    g_autofree gchar *id = NULL;
    g_autofree gchar *name = NULL;
    g_autofree gchar *description = NULL;
    gdouble cost_per_year;
    guint unlock_level;
    guint discovery_risk;
    g_auto(GStrv) groups = NULL;
    gsize n_groups;
    gsize i;

    /* For now use GKeyFile as a simplified loader. In production,
     * this would use yaml-glib to parse the YAML format. */
    keyfile = g_key_file_new ();
    if (!g_key_file_load_from_file (keyfile, file_path, G_KEY_FILE_NONE, error))
        return NULL;

    /* Read basic properties */
    id = g_key_file_get_string (keyfile, "megaproject", "id", NULL);
    if (id == NULL)
    {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                     "Megaproject file missing 'id' field");
        return NULL;
    }

    name = g_key_file_get_string (keyfile, "megaproject", "name", NULL);
    if (name == NULL)
        name = g_strdup (id);

    project = lp_megaproject_new (id, name);

    description = g_key_file_get_string (keyfile, "megaproject", "description", NULL);
    if (description != NULL)
        lp_megaproject_set_description (project, description);

    cost_per_year = g_key_file_get_double (keyfile, "megaproject", "cost_per_year", NULL);
    if (cost_per_year > 0)
    {
        g_autoptr(LrgBigNumber) cost = lrg_big_number_new (cost_per_year);
        lp_megaproject_set_cost_per_year (project, cost);
    }

    unlock_level = g_key_file_get_integer (keyfile, "megaproject", "unlock_level", NULL);
    lp_megaproject_set_unlock_level (project, unlock_level);

    discovery_risk = g_key_file_get_integer (keyfile, "megaproject", "discovery_risk", NULL);
    lp_megaproject_set_discovery_risk (project, discovery_risk);

    /* Load phases from [phase.N] groups */
    groups = g_key_file_get_groups (keyfile, &n_groups);
    for (i = 0; i < n_groups; i++)
    {
        if (g_str_has_prefix (groups[i], "phase."))
        {
            LpMegaprojectPhase *phase;
            g_autofree gchar *phase_name = NULL;
            guint phase_years;
            g_autofree gchar *effect_type = NULL;
            gdouble effect_value;

            phase_name = g_key_file_get_string (keyfile, groups[i], "name", NULL);
            if (phase_name == NULL)
                phase_name = g_strdup ("Unknown Phase");

            phase_years = g_key_file_get_integer (keyfile, groups[i], "years", NULL);

            phase = lp_megaproject_phase_new (phase_name, phase_years);

            effect_type = g_key_file_get_string (keyfile, groups[i], "effect_type", NULL);
            if (effect_type != NULL)
            {
                phase->effect_type = g_steal_pointer (&effect_type);
                effect_value = g_key_file_get_double (keyfile, groups[i], "effect_value", NULL);
                phase->effect_value = effect_value;
            }

            lp_megaproject_add_phase (project, phase);
        }
    }

    return g_steal_pointer (&project);
}

/* ==========================================================================
 * Public API - Properties Access
 * ========================================================================== */

const gchar *
lp_megaproject_get_id (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), NULL);
    return self->id;
}

const gchar *
lp_megaproject_get_name (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), NULL);
    return self->name;
}

const gchar *
lp_megaproject_get_description (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), NULL);
    return self->description;
}

LpMegaprojectState
lp_megaproject_get_state (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), LP_MEGAPROJECT_STATE_LOCKED);
    return self->state;
}

guint
lp_megaproject_get_total_duration (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0);
    return self->total_duration;
}

const LrgBigNumber *
lp_megaproject_get_cost_per_year (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), NULL);
    return self->cost_per_year;
}

guint
lp_megaproject_get_unlock_level (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0);
    return self->unlock_level;
}

guint
lp_megaproject_get_discovery_risk (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0);
    return self->discovery_risk;
}

/* ==========================================================================
 * Public API - Progress Tracking
 * ========================================================================== */

guint
lp_megaproject_get_years_invested (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0);
    return self->years_invested;
}

guint
lp_megaproject_get_years_remaining (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0);

    if (self->years_invested >= self->total_duration)
        return 0;

    return self->total_duration - self->years_invested;
}

gfloat
lp_megaproject_get_progress (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0.0f);

    if (self->total_duration == 0)
        return 0.0f;

    return (gfloat) self->years_invested / (gfloat) self->total_duration;
}

const LpMegaprojectPhase *
lp_megaproject_get_current_phase (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), NULL);

    if (self->current_phase_index >= self->phases->len)
        return NULL;

    return g_ptr_array_index (self->phases, self->current_phase_index);
}

guint
lp_megaproject_get_current_phase_index (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0);
    return self->current_phase_index;
}

GPtrArray *
lp_megaproject_get_phases (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), NULL);
    return self->phases;
}

/* ==========================================================================
 * Public API - State Management
 * ========================================================================== */

gboolean
lp_megaproject_can_start (LpMegaproject *self,
                          guint          phylactery_level)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);

    /* Must be in available state and meet level requirement */
    if (self->state != LP_MEGAPROJECT_STATE_AVAILABLE)
        return FALSE;

    return phylactery_level >= self->unlock_level;
}

gboolean
lp_megaproject_start (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);

    if (self->state != LP_MEGAPROJECT_STATE_AVAILABLE)
        return FALSE;

    set_state (self, LP_MEGAPROJECT_STATE_ACTIVE);
    return TRUE;
}

gboolean
lp_megaproject_pause (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);

    if (self->state != LP_MEGAPROJECT_STATE_ACTIVE &&
        self->state != LP_MEGAPROJECT_STATE_DISCOVERED)
        return FALSE;

    set_state (self, LP_MEGAPROJECT_STATE_PAUSED);
    return TRUE;
}

gboolean
lp_megaproject_resume (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);

    if (self->state != LP_MEGAPROJECT_STATE_PAUSED)
        return FALSE;

    set_state (self, LP_MEGAPROJECT_STATE_ACTIVE);
    return TRUE;
}

gboolean
lp_megaproject_advance_years (LpMegaproject *self,
                              guint          years)
{
    guint remaining;

    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);

    if (self->state != LP_MEGAPROJECT_STATE_ACTIVE)
        return FALSE;

    remaining = years;
    while (remaining > 0 && self->current_phase_index < self->phases->len)
    {
        LpMegaprojectPhase *phase;
        guint years_needed;

        phase = g_ptr_array_index (self->phases, self->current_phase_index);
        years_needed = phase->years - self->years_in_current_phase;

        if (remaining >= years_needed)
        {
            /* Complete this phase */
            remaining -= years_needed;
            self->years_invested += years_needed;
            self->years_in_current_phase = 0;

            g_signal_emit (self, signals[SIGNAL_PHASE_COMPLETED], 0,
                          self->current_phase_index, phase);

            self->current_phase_index++;
            update_cached_effects (self);
        }
        else
        {
            /* Partial progress in current phase */
            self->years_in_current_phase += remaining;
            self->years_invested += remaining;
            remaining = 0;
        }
    }

    /* Notify of progress changes */
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_YEARS_INVESTED]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROGRESS]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_PHASE_INDEX]);

    /* Check for completion */
    if (self->current_phase_index >= self->phases->len)
    {
        set_state (self, LP_MEGAPROJECT_STATE_COMPLETE);
        g_signal_emit (self, signals[SIGNAL_COMPLETED], 0);
        return TRUE;
    }

    return TRUE;
}

gboolean
lp_megaproject_is_complete (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);
    return self->state == LP_MEGAPROJECT_STATE_COMPLETE;
}

/* ==========================================================================
 * Public API - Risk Management
 * ========================================================================== */

gboolean
lp_megaproject_roll_discovery (LpMegaproject *self)
{
    guint roll;

    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);

    if (self->state != LP_MEGAPROJECT_STATE_ACTIVE)
        return FALSE;

    if (self->discovery_risk == 0)
        return FALSE;

    roll = g_random_int_range (0, 100);
    if (roll < self->discovery_risk)
    {
        set_state (self, LP_MEGAPROJECT_STATE_DISCOVERED);
        g_signal_emit (self, signals[SIGNAL_DISCOVERED], 0);
        return TRUE;
    }

    return FALSE;
}

gboolean
lp_megaproject_is_discovered (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);
    return self->state == LP_MEGAPROJECT_STATE_DISCOVERED;
}

void
lp_megaproject_destroy (LpMegaproject *self)
{
    g_return_if_fail (LP_IS_MEGAPROJECT (self));

    set_state (self, LP_MEGAPROJECT_STATE_DESTROYED);
    g_signal_emit (self, signals[SIGNAL_DESTROYED], 0);
}

gboolean
lp_megaproject_hide (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);

    if (self->state != LP_MEGAPROJECT_STATE_DISCOVERED)
        return FALSE;

    set_state (self, LP_MEGAPROJECT_STATE_ACTIVE);
    return TRUE;
}

/* ==========================================================================
 * Public API - Phase Management
 * ========================================================================== */

void
lp_megaproject_add_phase (LpMegaproject      *self,
                          LpMegaprojectPhase *phase)
{
    g_return_if_fail (LP_IS_MEGAPROJECT (self));
    g_return_if_fail (phase != NULL);

    g_ptr_array_add (self->phases, phase);
    recalculate_total_duration (self);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TOTAL_DURATION]);
}

/* ==========================================================================
 * Public API - Effect Queries
 * ========================================================================== */

gdouble
lp_megaproject_get_property_income_bonus (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), 0.0);
    return self->property_income_bonus;
}

gboolean
lp_megaproject_has_agent_instant_travel (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);
    return self->has_instant_travel;
}

gboolean
lp_megaproject_has_property_seizure_immunity (LpMegaproject *self)
{
    g_return_val_if_fail (LP_IS_MEGAPROJECT (self), FALSE);
    return self->has_seizure_immunity;
}

/* ==========================================================================
 * Public API - Configuration
 * ========================================================================== */

void
lp_megaproject_set_description (LpMegaproject *self,
                                const gchar   *description)
{
    g_return_if_fail (LP_IS_MEGAPROJECT (self));

    g_free (self->description);
    self->description = g_strdup (description);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESCRIPTION]);
}

void
lp_megaproject_set_cost_per_year (LpMegaproject      *self,
                                  const LrgBigNumber *cost)
{
    g_return_if_fail (LP_IS_MEGAPROJECT (self));

    g_clear_pointer (&self->cost_per_year, lrg_big_number_free);
    if (cost != NULL)
        self->cost_per_year = lrg_big_number_copy (cost);
    else
        self->cost_per_year = lrg_big_number_new (0.0);
}

void
lp_megaproject_set_unlock_level (LpMegaproject *self,
                                 guint          level)
{
    g_return_if_fail (LP_IS_MEGAPROJECT (self));
    self->unlock_level = level;
}

void
lp_megaproject_set_discovery_risk (LpMegaproject *self,
                                   guint          risk)
{
    g_return_if_fail (LP_IS_MEGAPROJECT (self));
    self->discovery_risk = MIN (risk, 100);
}

/* ==========================================================================
 * Public API - Reset
 * ========================================================================== */

void
lp_megaproject_reset (LpMegaproject *self)
{
    g_return_if_fail (LP_IS_MEGAPROJECT (self));

    self->years_invested = 0;
    self->current_phase_index = 0;
    self->years_in_current_phase = 0;
    self->property_income_bonus = 0.0;
    self->has_instant_travel = FALSE;
    self->has_seizure_immunity = FALSE;

    /* Reset to locked or available based on unlock level */
    if (self->unlock_level == 0)
        set_state (self, LP_MEGAPROJECT_STATE_AVAILABLE);
    else
        set_state (self, LP_MEGAPROJECT_STATE_LOCKED);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_YEARS_INVESTED]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROGRESS]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_PHASE_INDEX]);
}
