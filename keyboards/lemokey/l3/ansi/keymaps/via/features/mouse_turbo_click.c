// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file mouse_turbo_click.c
 * @brief Mouse Turbo Click implementation
 *
 * For full documentation, see
 * <https://getreuer.info/posts/keyboards/mouse-turbo-click>
 */

#include "features/mouse_turbo_click.h"

// This library relies on that mouse keys and the deferred execution API are
// enabled, which we check for here. Enable them in your rules.mk by setting:
//   MOUSEKEY_ENABLE = yes
//   DEFERRED_EXEC_ENABLE = yes
// If `MOUSE_TURBO_CLICK_KEY` has been defined to click a non-mouse key instead,
// then mouse keys is no longer required.
#if !defined(MOUSEKEY_ENABLE) && !defined(MOUSE_TURBO_CLICK_KEY)
#error "mouse_turbo_click: Please set `MOUSEKEY_ENABLE = yes` in rules.mk."
#elif !defined(DEFERRED_EXEC_ENABLE)
#error "mouse_turbo_click: Please set `DEFERRED_EXEC_ENABLE = yes` in rules.mk."
#else

// The keycode to be repeatedly clicked, `KC_MS_BTN1` mouse button 1 by default.
#ifndef MOUSE_TURBO_CLICK_KEY
#define MOUSE_TURBO_CLICK_KEY KC_MS_BTN1
#endif  // MOUSE_TURBO_CLICK_KEY

// The click period in milliseconds. For instance a period of 200 ms would be 5
// clicks per second. Smaller period implies faster clicking.
//
// WARNING: The keyboard might become unresponsive if the period is too small.
// I suggest setting this no smaller than 10.
#ifndef MOUSE_TURBO_CLICK_PERIOD
#define MOUSE_TURBO_CLICK_PERIOD 10
#endif  // MOUSE_TURBO_CLICK_PERIOD

static bool turbo_enabled = false; // Globally accessible within the file, but not outside.

bool is_turbo_click_active(void) {
  return turbo_enabled;
}

static deferred_token click_token = INVALID_DEFERRED_TOKEN;
static bool click_registered = false;

// Callback used with deferred execution. It alternates between registering and
// unregistering (pressing and releasing) `MOUSE_TURBO_CLICK_KEY`.
static uint32_t turbo_click_callback(uint32_t trigger_time, void* cb_arg) {
  if (click_registered) {
    unregister_code16(MOUSE_TURBO_CLICK_KEY);
    click_registered = false;
  } else {
    click_registered = true;
    register_code16(MOUSE_TURBO_CLICK_KEY);
  }
  return MOUSE_TURBO_CLICK_PERIOD / 2;  // Execute again in half a period.
}

// Starts Turbo Click, begins the `turbo_click_callback()` callback.
static void turbo_click_start(void) {
  if (click_token == INVALID_DEFERRED_TOKEN) {
    uint32_t next_delay_ms = turbo_click_callback(0, NULL);
    click_token = defer_exec(next_delay_ms, turbo_click_callback, NULL);
  }
}

// Stops Turbo Click, cancels the callback.
static void turbo_click_stop(void) {
  if (click_token != INVALID_DEFERRED_TOKEN) {
    cancel_deferred_exec(click_token);
    click_token = INVALID_DEFERRED_TOKEN;
    if (click_registered) {
      // If `MOUSE_TURBO_CLICK_KEY` is currently registered, release it.
      unregister_code16(MOUSE_TURBO_CLICK_KEY);
      click_registered = false;
    }
  }
}

bool process_mouse_turbo_click(uint16_t keycode, keyrecord_t* record, uint16_t turbo_click_keycode) {
  if (keycode == turbo_click_keycode) {
    if (record->event.pressed) {  // Key press event.
      turbo_enabled = !turbo_enabled;  // Toggle the state of turbo click.
      if (turbo_enabled) {
        turbo_click_start();  // Start turbo click if toggled on.
      } else {
        turbo_click_stop();   // Stop turbo click if toggled off.
      }
    }
    return false;  // Do not propagate the key event further.
  }
  
  return true;  // Propagate other key events normally.
}



#endif