/* lp-state-save-slots.c - Save/Load Slots Selection State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_GAMESTATE
#include "../lp-log.h"

#include "lp-state-save-slots.h"
#include "lp-state-wake.h"
#include "../core/lp-game.h"
#include "../core/lp-game-data.h"
#include "../save/lp-save-manager.h"
#include "../lp-input-helpers.h"
#include <graylib.h>
#include <libregnum.h>

/* Maximum slots to display */
#define MAX_SLOTS (10)

/* Slot info cache */
typedef struct
{
    gboolean exists;
    guint64  year;
    guint64  total_years;
    gint64   timestamp;
} SlotInfo;

struct _LpStateSaveSlots
{
    LrgGameState parent_instance;

    LpSaveSlotsMode mode;
    gint            selected_slot;
    gboolean        confirm_overwrite;

    /* Slot information cache */
    SlotInfo        slots[MAX_SLOTS];

    /* UI Labels */
    LrgLabel  *label_title;
    GPtrArray *label_pool;
    guint      label_pool_index;
};

G_DEFINE_TYPE (LpStateSaveSlots, lp_state_save_slots, LRG_TYPE_GAME_STATE)

enum
{
    PROP_0,
    PROP_MODE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* ==========================================================================
 * Label Helpers
 * ========================================================================== */

static void
draw_label (LrgLabel       *label,
            const gchar    *text,
            gfloat          x,
            gfloat          y,
            gfloat          font_size,
            const GrlColor *color)
{
    lrg_label_set_text (label, text);
    lrg_widget_set_position (LRG_WIDGET (label), x, y);
    lrg_label_set_font_size (label, font_size);
    lrg_label_set_color (label, color);
    lrg_widget_draw (LRG_WIDGET (label));
}

static LrgLabel *
get_pool_label (LpStateSaveSlots *self)
{
    LrgLabel *label;

    if (self->label_pool_index >= self->label_pool->len)
        return g_ptr_array_index (self->label_pool, self->label_pool->len - 1);

    label = g_ptr_array_index (self->label_pool, self->label_pool_index);
    self->label_pool_index++;

    return label;
}

static void
reset_label_pool (LpStateSaveSlots *self)
{
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Private Helpers
 * ========================================================================== */

static void
refresh_slot_info (LpStateSaveSlots *self)
{
    LpSaveManager *save_mgr;
    guint i;

    save_mgr = lp_save_manager_get_default ();

    for (i = 0; i < MAX_SLOTS; i++)
    {
        self->slots[i].exists = lp_save_manager_slot_exists (save_mgr, i);

        if (self->slots[i].exists)
        {
            lp_save_manager_get_slot_info (save_mgr, i,
                                           &self->slots[i].year,
                                           &self->slots[i].total_years,
                                           &self->slots[i].timestamp);
        }
        else
        {
            self->slots[i].year = 0;
            self->slots[i].total_years = 0;
            self->slots[i].timestamp = 0;
        }
    }
}

static void
format_timestamp (gint64 timestamp, gchar *buffer, gsize buffer_size)
{
    g_autoptr(GDateTime) dt = NULL;

    if (timestamp <= 0)
    {
        g_snprintf (buffer, buffer_size, "Unknown");
        return;
    }

    dt = g_date_time_new_from_unix_local (timestamp);
    if (dt != NULL)
    {
        g_autofree gchar *formatted = g_date_time_format (dt, "%Y-%m-%d %H:%M");
        g_snprintf (buffer, buffer_size, "%s", formatted ? formatted : "Unknown");
    }
    else
    {
        g_snprintf (buffer, buffer_size, "Unknown");
    }
}

static void
perform_save (LpStateSaveSlots *self, guint slot)
{
    LpGame *game;
    LpGameData *game_data;
    LpSaveManager *save_mgr;
    g_autoptr(GError) error = NULL;

    game = lp_game_get_from_state (LRG_GAME_STATE (self));
    game_data = lp_game_get_game_data (game);
    save_mgr = lp_save_manager_get_default ();

    if (lp_save_manager_save_game (save_mgr, game_data, slot, &error))
    {
        lp_log_info ("Game saved to slot %u", slot);
        refresh_slot_info (self);
        self->confirm_overwrite = FALSE;
    }
    else
    {
        lp_log_error ("Failed to save game: %s",
                      error ? error->message : "Unknown error");
    }
}

static void
perform_load (LpStateSaveSlots *self, guint slot)
{
    LpGame *game;
    LpGameData *game_data;
    LpSaveManager *save_mgr;
    LrgGameStateManager *manager;
    g_autoptr(GError) error = NULL;

    game = lp_game_get_from_state (LRG_GAME_STATE (self));
    game_data = lp_game_get_game_data (game);
    save_mgr = lp_save_manager_get_default ();
    manager = lrg_game_template_get_state_manager (LRG_GAME_TEMPLATE (game));

    if (lp_save_manager_load_game (save_mgr, game_data, slot, &error))
    {
        lp_log_info ("Game loaded from slot %u", slot);

        /* Pop this state and push wake state */
        lrg_game_state_manager_pop (manager);
        lrg_game_state_manager_push (manager, LRG_GAME_STATE (lp_state_wake_new ()));
    }
    else
    {
        lp_log_error ("Failed to load game: %s",
                      error ? error->message : "Unknown error");
    }
}

/* ==========================================================================
 * GObject Virtual Methods
 * ========================================================================== */

static void
lp_state_save_slots_dispose (GObject *object)
{
    LpStateSaveSlots *self = LP_STATE_SAVE_SLOTS (object);

    g_clear_object (&self->label_title);
    g_clear_pointer (&self->label_pool, g_ptr_array_unref);

    G_OBJECT_CLASS (lp_state_save_slots_parent_class)->dispose (object);
}

static void
lp_state_save_slots_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    LpStateSaveSlots *self = LP_STATE_SAVE_SLOTS (object);

    switch (prop_id)
    {
    case PROP_MODE:
        g_value_set_int (value, (gint)self->mode);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_state_save_slots_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    LpStateSaveSlots *self = LP_STATE_SAVE_SLOTS (object);

    switch (prop_id)
    {
    case PROP_MODE:
        self->mode = (LpSaveSlotsMode)g_value_get_int (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

/* ==========================================================================
 * LrgGameState Virtual Methods
 * ========================================================================== */

static void
lp_state_save_slots_enter (LrgGameState *state)
{
    LpStateSaveSlots *self = LP_STATE_SAVE_SLOTS (state);

    lp_log_info ("Entering save slots (%s mode)",
                 self->mode == LP_SAVE_SLOTS_MODE_SAVE ? "save" : "load");

    self->selected_slot = 0;
    self->confirm_overwrite = FALSE;

    refresh_slot_info (self);
}

static void
lp_state_save_slots_exit (LrgGameState *state)
{
    lp_log_info ("Exiting save slots");
}

static void
lp_state_save_slots_update (LrgGameState *state, gdouble delta)
{
    LpStateSaveSlots *self = LP_STATE_SAVE_SLOTS (state);
    LpGame *game;
    LrgGameStateManager *manager;

    (void)delta;

    game = lp_game_get_from_state (state);
    manager = lrg_game_template_get_state_manager (LRG_GAME_TEMPLATE (game));

    /* Navigate slots */
    if (LP_INPUT_NAV_UP_PRESSED ())
    {
        self->selected_slot--;
        if (self->selected_slot < 0)
            self->selected_slot = MAX_SLOTS - 1;
        self->confirm_overwrite = FALSE;
    }

    if (LP_INPUT_NAV_DOWN_PRESSED ())
    {
        self->selected_slot++;
        if (self->selected_slot >= MAX_SLOTS)
            self->selected_slot = 0;
        self->confirm_overwrite = FALSE;
    }

    /* Confirm/Select */
    if (LP_INPUT_CONFIRM_PRESSED ())
    {
        if (self->mode == LP_SAVE_SLOTS_MODE_SAVE)
        {
            if (self->slots[self->selected_slot].exists && !self->confirm_overwrite)
            {
                /* First press on existing slot: ask for confirmation */
                self->confirm_overwrite = TRUE;
            }
            else
            {
                /* Empty slot or confirmed overwrite */
                perform_save (self, (guint)self->selected_slot);
            }
        }
        else  /* Load mode */
        {
            if (self->slots[self->selected_slot].exists)
            {
                perform_load (self, (guint)self->selected_slot);
            }
        }
    }

    /* Delete slot with DELETE key */
    if (grl_input_is_key_pressed (GRL_KEY_DELETE) ||
        grl_input_is_key_pressed (GRL_KEY_X))
    {
        if (self->slots[self->selected_slot].exists)
        {
            LpSaveManager *save_mgr = lp_save_manager_get_default ();
            g_autoptr(GError) error = NULL;

            if (lp_save_manager_delete_slot (save_mgr, (guint)self->selected_slot, &error))
            {
                lp_log_info ("Deleted slot %d", self->selected_slot);
                refresh_slot_info (self);
            }
            else
            {
                lp_log_error ("Failed to delete slot: %s",
                              error ? error->message : "Unknown error");
            }
        }
    }

    /* Cancel */
    if (LP_INPUT_CANCEL_PRESSED ())
    {
        if (self->confirm_overwrite)
        {
            self->confirm_overwrite = FALSE;
        }
        else
        {
            lrg_game_state_manager_pop (manager);
        }
    }
}

static void
lp_state_save_slots_draw (LrgGameState *state)
{
    LpStateSaveSlots *self = LP_STATE_SAVE_SLOTS (state);
    LpGame *game = lp_game_get_from_state (state);
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) panel_color = NULL;
    g_autoptr(GrlColor) title_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) dim_color = NULL;
    g_autoptr(GrlColor) selected_color = NULL;
    g_autoptr(GrlColor) empty_color = NULL;
    g_autoptr(GrlColor) warn_color = NULL;
    gint screen_w, screen_h;
    gint center_x;
    gint panel_x, panel_y, panel_w, panel_h;
    gint content_x, content_y;
    gint i;

    /* Reset label pool for this frame */
    reset_label_pool (self);

    /* Get virtual resolution */
    screen_w = lrg_game_2d_template_get_virtual_width (LRG_GAME_2D_TEMPLATE (game));
    screen_h = lrg_game_2d_template_get_virtual_height (LRG_GAME_2D_TEMPLATE (game));
    center_x = screen_w / 2;

    /* Calculate panel dimensions */
    panel_w = (screen_w * 70) / 100;
    panel_h = (screen_h * 75) / 100;
    panel_x = (screen_w - panel_w) / 2;
    panel_y = (screen_h - panel_h) / 2;

    content_x = panel_x + 30;
    content_y = panel_y + 60;

    /* Colors */
    bg_color = grl_color_new (20, 20, 30, 220);
    panel_color = grl_color_new (35, 35, 50, 255);
    title_color = grl_color_new (180, 150, 200, 255);
    text_color = grl_color_new (200, 200, 200, 255);
    dim_color = grl_color_new (100, 100, 100, 255);
    selected_color = grl_color_new (255, 215, 0, 255);
    empty_color = grl_color_new (80, 80, 100, 255);
    warn_color = grl_color_new (255, 100, 100, 255);

    /* Draw background overlay */
    grl_draw_rectangle (0, 0, screen_w, screen_h, bg_color);

    /* Draw panel */
    grl_draw_rectangle (panel_x, panel_y, panel_w, panel_h, panel_color);

    /* Draw title */
    {
        const gchar *title;

        title = (self->mode == LP_SAVE_SLOTS_MODE_SAVE) ? "SAVE GAME" : "LOAD GAME";
        draw_label (self->label_title, title,
                    center_x - 80, panel_y + 20, 32, title_color);
    }

    /* Draw slots */
    for (i = 0; i < MAX_SLOTS; i++)
    {
        gchar slot_text[128];
        gchar info_text[128];
        GrlColor *slot_color;
        gint slot_y;
        gboolean is_selected;

        slot_y = content_y + (i * 40);
        is_selected = (i == self->selected_slot);

        if (self->slots[i].exists)
        {
            gchar timestamp_str[64];
            format_timestamp (self->slots[i].timestamp, timestamp_str, sizeof (timestamp_str));

            g_snprintf (slot_text, sizeof (slot_text), "Slot %d", i + 1);
            g_snprintf (info_text, sizeof (info_text),
                        "Year %lu | Total: %lu years | %s",
                        (gulong)self->slots[i].year,
                        (gulong)self->slots[i].total_years,
                        timestamp_str);
            slot_color = is_selected ? selected_color : text_color;
        }
        else
        {
            g_snprintf (slot_text, sizeof (slot_text), "Slot %d", i + 1);
            g_snprintf (info_text, sizeof (info_text), "< Empty >");
            slot_color = is_selected ? selected_color : empty_color;
        }

        /* Selection indicator */
        if (is_selected)
        {
            draw_label (get_pool_label (self), ">", content_x - 20, slot_y, 18, selected_color);
        }

        /* Slot name */
        draw_label (get_pool_label (self), slot_text, content_x, slot_y, 18, slot_color);

        /* Slot info */
        draw_label (get_pool_label (self), info_text, content_x + 100, slot_y, 16,
                    is_selected ? text_color : dim_color);
    }

    /* Draw confirmation message for overwrite */
    if (self->confirm_overwrite)
    {
        draw_label (get_pool_label (self), "Press ENTER again to overwrite, ESC to cancel",
                    center_x - 220, panel_y + panel_h - 60, 16, warn_color);
    }

    /* Draw instructions */
    {
        const gchar *instructions;

        if (self->mode == LP_SAVE_SLOTS_MODE_SAVE)
        {
            instructions = "UP/DOWN: Select    ENTER: Save    X/DEL: Delete    ESC: Cancel";
        }
        else
        {
            instructions = "UP/DOWN: Select    ENTER: Load    X/DEL: Delete    ESC: Cancel";
        }

        draw_label (get_pool_label (self), instructions,
                    center_x - 300, panel_y + panel_h - 30, 14, dim_color);
    }
}

static gboolean
lp_state_save_slots_handle_input (LrgGameState *state, gpointer event)
{
    (void)state;
    (void)event;
    return FALSE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_state_save_slots_class_init (LpStateSaveSlotsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    LrgGameStateClass *state_class = LRG_GAME_STATE_CLASS (klass);

    object_class->dispose = lp_state_save_slots_dispose;
    object_class->get_property = lp_state_save_slots_get_property;
    object_class->set_property = lp_state_save_slots_set_property;

    state_class->enter = lp_state_save_slots_enter;
    state_class->exit = lp_state_save_slots_exit;
    state_class->update = lp_state_save_slots_update;
    state_class->draw = lp_state_save_slots_draw;
    state_class->handle_input = lp_state_save_slots_handle_input;

    properties[PROP_MODE] =
        g_param_spec_int ("mode", "Mode",
                          "Save or load mode",
                          LP_SAVE_SLOTS_MODE_SAVE, LP_SAVE_SLOTS_MODE_LOAD,
                          LP_SAVE_SLOTS_MODE_LOAD,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
lp_state_save_slots_init (LpStateSaveSlots *self)
{
    guint i;

    lrg_game_state_set_name (LRG_GAME_STATE (self), "SaveSlots");
    lrg_game_state_set_transparent (LRG_GAME_STATE (self), TRUE);
    lrg_game_state_set_blocking (LRG_GAME_STATE (self), TRUE);

    self->mode = LP_SAVE_SLOTS_MODE_LOAD;
    self->selected_slot = 0;
    self->confirm_overwrite = FALSE;

    /* Create labels */
    self->label_title = lrg_label_new (NULL);

    /* Create label pool */
    self->label_pool = g_ptr_array_new_with_free_func (g_object_unref);
    for (i = 0; i < 50; i++)
        g_ptr_array_add (self->label_pool, lrg_label_new (NULL));
    self->label_pool_index = 0;
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

LpStateSaveSlots *
lp_state_save_slots_new (LpSaveSlotsMode mode)
{
    return g_object_new (LP_TYPE_STATE_SAVE_SLOTS,
                         "mode", (gint)mode,
                         NULL);
}

LpSaveSlotsMode
lp_state_save_slots_get_mode (LpStateSaveSlots *self)
{
    g_return_val_if_fail (LP_IS_STATE_SAVE_SLOTS (self), LP_SAVE_SLOTS_MODE_LOAD);
    return self->mode;
}
