/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

struct zmk_oled_anim_changed {
    uint8_t index;
};

ZMK_EVENT_DECLARE(zmk_oled_anim_changed);
