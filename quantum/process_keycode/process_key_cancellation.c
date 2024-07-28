// Copyright 2024 Harrison Chan (@xelus22)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "process_key_cancellation.h"
#include <string.h>
#include "keycodes.h"
#include "keycode_config.h"
#include "action_util.h"
#include "keymap_introspection.h"
#include "debug.h"

#define KEYREPORT_BUFFER_SIZE 10

// key cancellation up stroke buffer
uint16_t buffer_keyreports[KEYREPORT_BUFFER_SIZE];

// is the next free one
int buffer_keyreport_count = 0;

// temp buffer to store keycodes
uint16_t buffer_keyreports_temp[KEYREPORT_BUFFER_SIZE];

/**
 * @brief function for querying the enabled state of key cancellation
 *
 * @return true if enabled
 * @return false if disabled
 */
bool key_cancellation_is_enabled(void) {
    return keymap_config.key_cancellation_enable;
}

/**
 * @brief Enables key cancellation and saves state to eeprom
 *
 */
void key_cancellation_enable(void) {
    keymap_config.key_cancellation_enable = true;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Disables key cancellation and saves state to eeprom
 *
 */
void key_cancellation_disable(void) {
    keymap_config.key_cancellation_enable = false;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Toggles key cancellation's status and save state to eeprom
 *
 */
void key_cancellation_toggle(void) {
    keymap_config.key_cancellation_enable = !keymap_config.key_cancellation_enable;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief function for querying the enabled state of key cancellation recovery
 *
 * @return true if enabled
 * @return false if disabled
 */
bool key_cancellation_recovery_is_enabled(void) {
    // should check both
    return keymap_config.key_cancellation_enable && keymap_config.key_cancellation_recovery_enable;
}

/**
 * @brief Enables key cancellation recovery and saves state to eeprom
 *
 */
void key_cancellation_recovery_enable(void) {
    keymap_config.key_cancellation_recovery_enable = true;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Disables key cancellation recovery and saves state to eeprom
 *
 */
void key_cancellation_recovery_disable(void) {
    keymap_config.key_cancellation_recovery_enable = false;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Toggles key cancellation recovery's status and save state to eeprom
 *
 */
void key_cancellation_recovery_toggle(void) {
    keymap_config.key_cancellation_recovery_enable = !keymap_config.key_cancellation_recovery_enable;
    eeconfig_update_keymap(keymap_config.raw);
}


/**
 * @brief handler for user to override whether key cancellation should process this keypress
 *
 * @param keycode Keycode registered by matrix press, per keymap
 * @param record keyrecord_t structure
 * @return true Allow key cancellation
 * @return false Stop processing and escape from key cancellation
 */
__attribute__((weak)) bool process_key_cancellation_user(uint16_t keycode, keyrecord_t *record) {
    return true;
}

// check if key already in buffer
bool key_cancellation_is_key_in_buffer(uint16_t keycode) {
    for (int i = 0; i < buffer_keyreport_count; i++) {
        if (buffer_keyreports[i] == keycode) {
            return true;
        }
    }
    return false;
}

void add_key_buffer(uint16_t keycode) {
    if (key_cancellation_is_key_in_buffer(keycode)) {
        return;
    }

    if (buffer_keyreport_count >= KEYREPORT_BUFFER_SIZE) {
        return;
    }

    buffer_keyreports[buffer_keyreport_count] = keycode;
    buffer_keyreport_count++;
}

// remove keycode and shift buffer
void del_key_buffer(uint16_t keycode) {
    for (int i = 0; i < buffer_keyreport_count; i++) {
        if (buffer_keyreports[i] == keycode) {
            for (int j = i; j < buffer_keyreport_count - 1; j++) {
                buffer_keyreports[j] = buffer_keyreports[j + 1];
            }
            buffer_keyreport_count--;
            break;
        }
    }
}

// remove keycode, dont care about shifting buffer
void del_key_buffer_temp(uint16_t keycode, int end_index) {
    for (int i = 0; i < end_index; i++) {
        if (buffer_keyreports_temp[i] == keycode) {
            buffer_keyreports_temp[i] = 0;
            break;
        }
    }
}

// check if keycode is in the cancellation press list
bool key_cancellation_is_key_in_press_list(uint16_t keycode) {
    for (int i = 0; i < key_cancellation_count(); i++) {
        if (key_cancellation_get(i).press == keycode) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Process handler for key_cancellation feature
 *
 * @param keycode Keycode registered by matrix press, per keymap
 * @param record keyrecord_t structure
 * @return true Continue processing keycodes, and send to host
 * @return false Stop processing keycodes, and don't send to host
 */
bool process_key_cancellation(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case QK_KEY_CANCELLATION_ON:
                key_cancellation_enable();
                return false;
            case QK_KEY_CANCELLATION_OFF:
                key_cancellation_disable();
                return false;
            case QK_KEY_CANCELLATION_TOGGLE:
                key_cancellation_toggle();
                return false;
            case QK_KEY_CANCELLATION_RECOVERY_ON:
                key_cancellation_recovery_enable();
                return false;
            case QK_KEY_CANCELLATION_RECOVERY_OFF:
                key_cancellation_recovery_disable();
                return false;
            case QK_KEY_CANCELLATION_RECOVERY_TOGGLE:
                key_cancellation_recovery_toggle();
                return false;
            default:
                break;
        }
    }

    if (!keymap_config.key_cancellation_enable) {
        return true;
    }

/*
    if (!record->event.pressed) {
        return true;
    }
*/

    // only supports basic keycodes
    if (!IS_BASIC_KEYCODE(keycode)) {
        return true;
    }

    if (!process_key_cancellation_user(keycode, record)) {
        return true;
    }

// if key cancellation recovery is not enabled then do not process key up events
    if (!keymap_config.key_cancellation_recovery_enable && !record->event.pressed) {
        return true;
    }

    // only key cancellation to buffer if required
    if (keymap_config.key_cancellation_recovery_enable) {
        if (key_cancellation_is_key_in_press_list(keycode)) {
            if (record->event.pressed) {
                add_key_buffer(keycode);
            } else {
                del_key_buffer(keycode);
            }
        }

        if (buffer_keyreport_count == 0) {
            return true;
        }
    }

    // no key cancellation to process

    // copy buffer to temp buffer
    memcpy(buffer_keyreports_temp, buffer_keyreports, sizeof(buffer_keyreports));

    ac_dprintf("buffer_keyreport_count: %d\n", buffer_keyreport_count);

    if (record->event.pressed) {
        for (int i = 0; i < key_cancellation_count(); i++) {
            key_cancellation_t key_cancellation = key_cancellation_get(i);
            if (keycode == key_cancellation.press) {
                del_key(key_cancellation.unpress);
            }
        }
    } else {
        for (int j = buffer_keyreport_count - 1; j >= 0; j--) {
            for (int i = 0; i < key_cancellation_count(); i++) {
                key_cancellation_t key_cancellation = key_cancellation_get(i);
                // if key in buffer is the same as the key cancellation press
                if (key_cancellation.press == buffer_keyreports_temp[j]) {
                    // if key cancellation unpress is in buffer
                    if (key_cancellation_is_key_in_buffer(key_cancellation.unpress)) {
                        // remove key cancellation unpress from buffer
                        del_key_buffer_temp(key_cancellation.unpress, j);
                    }
                }
            }
        }

        // print temp buffer
        for (int i = 0; i < buffer_keyreport_count; i++) {
            ac_dprintf("buffer_keyreports_temp[%d]: %d\n", i, buffer_keyreports_temp[i]);
        }

        // print buffer
        for (int i = 0; i < buffer_keyreport_count; i++) {
            ac_dprintf("buffer_keyreports[%d]: %d\n", i, buffer_keyreports[i]);
        }

        // compare buffer and temp buffer
        for (int i = 0; i < buffer_keyreport_count; i++) {
            if (buffer_keyreports[i] != buffer_keyreports_temp[i]) {
                if (buffer_keyreports_temp[i] == 0) {
                    del_key(buffer_keyreports[i]);
                }
            } else {
                add_key(buffer_keyreports_temp[i]);
            }
        }
    }

    return true;
}