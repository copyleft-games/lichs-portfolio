/* lp-state-save-slots.h - Save/Load Slots Selection State
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Presents a list of save slots for saving or loading game progress.
 */

#ifndef LP_STATE_SAVE_SLOTS_H
#define LP_STATE_SAVE_SLOTS_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_STATE_SAVE_SLOTS (lp_state_save_slots_get_type ())

G_DECLARE_FINAL_TYPE (LpStateSaveSlots, lp_state_save_slots,
                      LP, STATE_SAVE_SLOTS, LrgGameState)

/**
 * LpSaveSlotsMode:
 * @LP_SAVE_SLOTS_MODE_SAVE: Show slots for saving
 * @LP_SAVE_SLOTS_MODE_LOAD: Show slots for loading
 *
 * Operating mode for the save slots screen.
 */
typedef enum
{
    LP_SAVE_SLOTS_MODE_SAVE,
    LP_SAVE_SLOTS_MODE_LOAD
} LpSaveSlotsMode;

/**
 * lp_state_save_slots_new:
 * @mode: The #LpSaveSlotsMode (save or load)
 *
 * Creates a new save slots selection state.
 *
 * Returns: (transfer full): A new #LpStateSaveSlots
 */
LpStateSaveSlots * lp_state_save_slots_new (LpSaveSlotsMode mode);

/**
 * lp_state_save_slots_get_mode:
 * @self: an #LpStateSaveSlots
 *
 * Gets the current mode (save or load).
 *
 * Returns: The #LpSaveSlotsMode
 */
LpSaveSlotsMode lp_state_save_slots_get_mode (LpStateSaveSlots *self);

G_END_DECLS

#endif /* LP_STATE_SAVE_SLOTS_H */
