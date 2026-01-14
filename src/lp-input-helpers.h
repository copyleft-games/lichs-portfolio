/* lp-input-helpers.h - Input Helper Macros
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Provides convenience macros for checking input from multiple sources
 * (keyboard, vim keys, and gamepad) with a single call.
 *
 * All gamepads are supported automatically via graylib/raylib's SDL backend:
 * - Xbox controllers (wired/wireless)
 * - PlayStation controllers (DS4/DualSense)
 * - Nintendo Switch Pro controller
 * - Steam Deck controls
 * - Generic XInput/DirectInput controllers
 */

#pragma once

#include <graylib.h>

/* Controller index for single-player game */
#define LP_GAMEPAD_INDEX (0)

/* ==========================================================================
 * Navigation Macros
 * ========================================================================== */

/**
 * LP_INPUT_NAV_UP_PRESSED:
 *
 * Checks if navigation up was just pressed.
 * Keyboard: UP arrow, K (vim)
 * Gamepad: D-pad up
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_NAV_UP_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_UP) || \
     grl_input_is_key_pressed (GRL_KEY_K) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_UP))

/**
 * LP_INPUT_NAV_DOWN_PRESSED:
 *
 * Checks if navigation down was just pressed.
 * Keyboard: DOWN arrow, J (vim)
 * Gamepad: D-pad down
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_NAV_DOWN_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_DOWN) || \
     grl_input_is_key_pressed (GRL_KEY_J) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN))

/**
 * LP_INPUT_NAV_LEFT_PRESSED:
 *
 * Checks if navigation left was just pressed.
 * Keyboard: LEFT arrow, H (vim)
 * Gamepad: D-pad left
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_NAV_LEFT_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_LEFT) || \
     grl_input_is_key_pressed (GRL_KEY_H) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT))

/**
 * LP_INPUT_NAV_RIGHT_PRESSED:
 *
 * Checks if navigation right was just pressed.
 * Keyboard: RIGHT arrow, L (vim)
 * Gamepad: D-pad right
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_NAV_RIGHT_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_RIGHT) || \
     grl_input_is_key_pressed (GRL_KEY_L) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT))

/* ==========================================================================
 * Action Macros
 * ========================================================================== */

/**
 * LP_INPUT_CONFIRM_PRESSED:
 *
 * Checks if confirm/select action was just pressed.
 * Keyboard: ENTER, SPACE
 * Gamepad: A/Cross (bottom face button)
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_CONFIRM_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_ENTER) || \
     grl_input_is_key_pressed (GRL_KEY_SPACE) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN))

/**
 * LP_INPUT_CANCEL_PRESSED:
 *
 * Checks if cancel/back action was just pressed.
 * Keyboard: ESCAPE
 * Gamepad: B/Circle (right face button)
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_CANCEL_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_ESCAPE) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))

/* ==========================================================================
 * Tab/Category Switching Macros
 * ========================================================================== */

/**
 * LP_INPUT_TAB_NEXT_PRESSED:
 *
 * Checks if next tab/category was just pressed.
 * Keyboard: TAB
 * Gamepad: RB/R1 (right bumper)
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_TAB_NEXT_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_TAB) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1))

/**
 * LP_INPUT_TAB_PREV_PRESSED:
 *
 * Checks if previous tab/category was just pressed.
 * Keyboard: SHIFT+TAB (not directly supported, use separate check)
 * Gamepad: LB/L1 (left bumper)
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_TAB_PREV_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1))

/* ==========================================================================
 * Special Action Macros
 * ========================================================================== */

/**
 * LP_INPUT_TOGGLE_PRESSED:
 *
 * Checks if toggle/alternate action was just pressed.
 * Keyboard: R
 * Gamepad: Y/Triangle (top face button)
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_TOGGLE_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_R) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP))

/**
 * LP_INPUT_START_PRESSED:
 *
 * Checks if start/menu button was just pressed.
 * Keyboard: (none - use ESCAPE for pause)
 * Gamepad: Start/Options (middle right button)
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_START_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT))

/* ==========================================================================
 * Value Adjustment Macros (for settings)
 * ========================================================================== */

/**
 * LP_INPUT_VALUE_DEC_PRESSED:
 *
 * Checks if decrease value was just pressed.
 * Keyboard: LEFT arrow, A
 * Gamepad: D-pad left
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_VALUE_DEC_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_LEFT) || \
     grl_input_is_key_pressed (GRL_KEY_A) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT))

/**
 * LP_INPUT_VALUE_INC_PRESSED:
 *
 * Checks if increase value was just pressed.
 * Keyboard: RIGHT arrow, D
 * Gamepad: D-pad right
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_INPUT_VALUE_INC_PRESSED() \
    (grl_input_is_key_pressed (GRL_KEY_RIGHT) || \
     grl_input_is_key_pressed (GRL_KEY_D) || \
     grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT))

/* ==========================================================================
 * Gamepad-Only Helpers (for widget event forwarding)
 * ========================================================================== */

/**
 * LP_GAMEPAD_NAV_UP_PRESSED:
 *
 * Checks if gamepad D-pad up was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_NAV_UP_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_UP))

/**
 * LP_GAMEPAD_NAV_DOWN_PRESSED:
 *
 * Checks if gamepad D-pad down was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_NAV_DOWN_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN))

/**
 * LP_GAMEPAD_NAV_LEFT_PRESSED:
 *
 * Checks if gamepad D-pad left was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_NAV_LEFT_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT))

/**
 * LP_GAMEPAD_NAV_RIGHT_PRESSED:
 *
 * Checks if gamepad D-pad right was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_NAV_RIGHT_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT))

/**
 * LP_GAMEPAD_CONFIRM_PRESSED:
 *
 * Checks if gamepad A/Cross button was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_CONFIRM_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN))

/**
 * LP_GAMEPAD_CANCEL_PRESSED:
 *
 * Checks if gamepad B/Circle button was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_CANCEL_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))

/**
 * LP_GAMEPAD_TOGGLE_PRESSED:
 *
 * Checks if gamepad Y/Triangle button was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_TOGGLE_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP))

/**
 * LP_GAMEPAD_TAB_NEXT_PRESSED:
 *
 * Checks if gamepad RB/R1 button was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_TAB_NEXT_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1))

/**
 * LP_GAMEPAD_TAB_PREV_PRESSED:
 *
 * Checks if gamepad LB/L1 button was just pressed.
 * Used for forwarding gamepad input to widgets as keyboard events.
 *
 * Returns: %TRUE if pressed this frame
 */
#define LP_GAMEPAD_TAB_PREV_PRESSED() \
    (grl_input_is_gamepad_button_pressed (LP_GAMEPAD_INDEX, GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1))
