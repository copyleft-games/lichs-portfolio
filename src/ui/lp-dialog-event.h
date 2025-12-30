/* lp-dialog-event.h - Event Dialog
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Dialog for displaying game events and choices.
 * Shows narrative text, Malachar's commentary, and choice buttons.
 */

#ifndef LP_DIALOG_EVENT_H
#define LP_DIALOG_EVENT_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"

G_BEGIN_DECLS

#define LP_TYPE_DIALOG_EVENT (lp_dialog_event_get_type ())

G_DECLARE_FINAL_TYPE (LpDialogEvent, lp_dialog_event,
                      LP, DIALOG_EVENT, LrgContainer)

/* ==========================================================================
 * Construction
 * ========================================================================== */

/**
 * lp_dialog_event_new:
 *
 * Creates a new event dialog.
 *
 * Returns: (transfer full): A new #LpDialogEvent
 */
LpDialogEvent * lp_dialog_event_new (void);

/* ==========================================================================
 * Event Display
 * ========================================================================== */

/**
 * lp_dialog_event_get_event:
 * @self: an #LpDialogEvent
 *
 * Gets the event being displayed.
 *
 * Returns: (transfer none) (nullable): The #LpEvent, or %NULL
 */
LpEvent * lp_dialog_event_get_event (LpDialogEvent *self);

/**
 * lp_dialog_event_set_event:
 * @self: an #LpDialogEvent
 * @event: (nullable): the event to display
 *
 * Sets the event to display in the dialog.
 */
void lp_dialog_event_set_event (LpDialogEvent *self,
                                 LpEvent       *event);

/* ==========================================================================
 * Choice Selection
 * ========================================================================== */

/**
 * lp_dialog_event_get_selected_choice:
 * @self: an #LpDialogEvent
 *
 * Gets the index of the selected choice.
 *
 * Returns: The selected choice index, or -1 if none
 */
gint lp_dialog_event_get_selected_choice (LpDialogEvent *self);

/**
 * lp_dialog_event_select_choice:
 * @self: an #LpDialogEvent
 * @index: the choice index to select
 *
 * Selects a choice by index.
 */
void lp_dialog_event_select_choice (LpDialogEvent *self,
                                     gint           index);

/**
 * lp_dialog_event_confirm_choice:
 * @self: an #LpDialogEvent
 *
 * Confirms the currently selected choice.
 */
void lp_dialog_event_confirm_choice (LpDialogEvent *self);

/* ==========================================================================
 * Signals
 * ========================================================================== */

/**
 * LpDialogEvent::choice-confirmed:
 * @self: the #LpDialogEvent
 * @choice_index: the confirmed choice index
 *
 * Emitted when a choice is confirmed.
 */

/**
 * LpDialogEvent::dismissed:
 * @self: the #LpDialogEvent
 *
 * Emitted when the dialog is dismissed without a choice.
 */

G_END_DECLS

#endif /* LP_DIALOG_EVENT_H */
