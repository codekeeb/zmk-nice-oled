/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

struct zmk_wpm_view_changed {
    uint8_t index;
};

ZMK_EVENT_DECLARE(zmk_wpm_view_changed);
