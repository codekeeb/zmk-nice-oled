/*
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <lvgl.h>

/* Show `title` over `text` on the peripheral screen for `ms` milliseconds.
 * Safe from any thread: the drawing runs on the display work queue. */
void nice_oled_toast_show(const char *title, const char *text, uint16_t ms);

/* Called by the screen once its canvas exists. */
void nice_oled_toast_init(lv_obj_t *parent);
