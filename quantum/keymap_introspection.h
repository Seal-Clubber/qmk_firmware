// Copyright 2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Key mapping

// Get the number of layers defined in the keymap, stored in firmware rather than any other persistent storage
uint8_t keymap_layer_count_raw(void);
// Get the number of layers defined in the keymap, potentially stored dynamically
uint8_t keymap_layer_count(void);

// Get the keycode for the keymap location, stored in firmware rather than any other persistent storage
uint16_t keycode_at_keymap_location_raw(uint8_t layer_num, uint8_t row, uint8_t column);
// Get the keycode for the keymap location, potentially stored dynamically
uint16_t keycode_at_keymap_location(uint8_t layer_num, uint8_t row, uint8_t column);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Encoder mapping

#if defined(ENCODER_ENABLE) && defined(ENCODER_MAP_ENABLE)

// Get the number of layers defined in the encoder map, stored in firmware rather than any other persistent storage
uint8_t encodermap_layer_count_raw(void);
// Get the number of layers defined in the encoder map, potentially stored dynamically
uint8_t encodermap_layer_count(void);

// Get the keycode for the encoder mapping location, stored in firmware rather than any other persistent storage
uint16_t keycode_at_encodermap_location_raw(uint8_t layer_num, uint8_t encoder_idx, bool clockwise);
// Get the keycode for the encoder mapping location, potentially stored dynamically
uint16_t keycode_at_encodermap_location(uint8_t layer_num, uint8_t encoder_idx, bool clockwise);

#endif // defined(ENCODER_ENABLE) && defined(ENCODER_MAP_ENABLE)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Combos

#if defined(COMBO_ENABLE)

// Forward declaration of combo_t so we don't need to deal with header reordering
struct combo_t;
typedef struct combo_t combo_t;

// Get the number of combos defined in the user's keymap, stored in firmware rather than any other persistent storage
uint16_t combo_count_raw(void);
// Get the number of combos defined in the user's keymap, potentially stored dynamically
uint16_t combo_count(void);

// Get the keycode for the encoder mapping location, stored in firmware rather than any other persistent storage
combo_t* combo_get_raw(uint16_t combo_idx);
// Get the keycode for the encoder mapping location, potentially stored dynamically
combo_t* combo_get(uint16_t combo_idx);

#endif // defined(COMBO_ENABLE)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Key Cancellation

#if defined(KEY_CANCELLATION_ENABLE)
// Forward declaration of key_cancellation_t so we don't need to deal with header reordering
struct key_cancellation_t;
typedef struct key_cancellation_t key_cancellation_t;
// Get the number of key cancellations defined in the user's keymap, stored in firmware rather than any other persistent storage
uint16_t key_cancellation_count_raw(void);
// Get the number of key cancellations defined in the user's keymap, potentially stored dynamically
uint16_t key_cancellation_count(void);
// Get the keycodes for the key cancellation, stored in firmware rather than any other persistent storage
key_cancellation_t key_cancellation_get_raw(uint16_t idx);
// Get the keycodes for the key cancellation, stored in firmware, potentially stored dynamically
key_cancellation_t key_cancellation_get(uint16_t idx);
#endif // defined(KEY_CANCELLATION_ENABLE)