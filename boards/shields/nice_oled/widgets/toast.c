/*
 * SPDX-License-Identifier: MIT
 *
 * CODEKEEB: short notice over the peripheral screen ("toast").
 *
 * Shows a title and a word for a moment, then the screen goes back to its
 * animation. Used by the RGB firmware to name the effect that has just
 * been picked.
 *
 * nice_oled_toast_show() may be called from any context (the RGB state
 * reaches the peripheral from the split BLE thread), so it only copies the
 * text and hands the drawing to the display work queue, where LVGL lives.
 *
 * Geometry. The status screen draws on a 160x160 canvas in portrait and
 * rotate_canvas() turns it 90 degrees onto the panel. On the Sofle's
 * 128x32 SSD1306 only the canvas strip x 0..31, y 32..159 is visible.
 * Widgets such as the animation are placed straight in panel (landscape)
 * coordinates. The toast does the same: it composes the text in a 32x128
 * portrait buffer and copies it, turned the way rotate_canvas() turns the
 * canvas, into a 128x32 canvas that covers the whole panel.
 */

#include <string.h>

#include <zephyr/kernel.h>
#include <zmk/display.h>

#include <nice_oled/toast.h>

#include "util.h"

LV_FONT_DECLARE(pixel_operator_mono_8);
LV_FONT_DECLARE(pixel_operator_mono_12);

/* With the stock 4 KB LVGL pool and ZMK's 2 KB display stack the
 * peripheral overflowed and stopped. The pool default lives in
 * Kconfig.defconfig; the stack must be set in the keyboard's .conf
 * (CONFIG_ZMK_DISPLAY_DEDICATED_THREAD_STACK_SIZE=4096), because ZMK's own
 * default wins over a module's. */
BUILD_ASSERT(CONFIG_LV_Z_MEM_POOL_SIZE >= 8192, "toast needs the larger LVGL pool");
BUILD_ASSERT(CONFIG_ZMK_DISPLAY_DEDICATED_THREAD_STACK_SIZE >= 4096,
             "set CONFIG_ZMK_DISPLAY_DEDICATED_THREAD_STACK_SIZE=4096 in the .conf");

#define PANEL_W 128 /* landscape, as the panel is wired */
#define PANEL_H 32
#define PORT_W PANEL_H /* portrait, as it is read */
#define PORT_H PANEL_W

static lv_obj_t *toast_cv;  /* visible, landscape */
static lv_obj_t *port_cv;   /* hidden, portrait scratch */
static lv_color_t toast_buf[PANEL_W * PANEL_H];
static lv_color_t port_buf[PORT_W * PORT_H];

static K_MUTEX_DEFINE(toast_lock);
static char pending_title[12];
static char pending_text[24];
static uint16_t pending_ms;

static void toast_hide_cb(struct k_work *work) {
    if (toast_cv) {
        lv_obj_add_flag(toast_cv, LV_OBJ_FLAG_HIDDEN);
    }
}

static K_WORK_DELAYABLE_DEFINE(toast_hide_work, toast_hide_cb);

static void toast_draw(const char *title, const char *text) {
    lv_canvas_fill_bg(port_cv, (LVGL_BACKGROUND), LV_OPA_COVER);

    lv_draw_label_dsc_t small;
    init_label_dsc(&small, (LVGL_FOREGROUND), &pixel_operator_mono_8, LV_TEXT_ALIGN_CENTER);

    /* Short words get the bigger font: 12 px is 6 px a letter, so up to
     * five letters fit the 32 px width; 8 px is 4 px a letter, eight. */
    lv_draw_label_dsc_t big;
    bool fits_big = strlen(text) <= 5;
    init_label_dsc(&big, (LVGL_FOREGROUND), fits_big ? &pixel_operator_mono_12 : &pixel_operator_mono_8,
                   LV_TEXT_ALIGN_CENTER);

    lv_draw_rect_dsc_t line;
    init_rect_dsc(&line, (LVGL_FOREGROUND));

    /* Centered on the 128 px length of the screen. */
    lv_coord_t y = PORT_H / 2 - 16;
    lv_canvas_draw_text(port_cv, 0, y, PORT_W, &small, title);
    lv_canvas_draw_rect(port_cv, 4, y + 11, PORT_W - 8, 1, &line);
    lv_canvas_draw_text(port_cv, 0, y + 16, PORT_W, &big, text);

    /* Portrait (px, py) -> landscape: the same quarter turn rotate_canvas()
     * applies, so the text reads the right way up like everything else. */
    for (int py = 0; py < PORT_H; py++) {
        for (int px = 0; px < PORT_W; px++) {
            toast_buf[px * PANEL_W + (PANEL_W - 1 - py)] = port_buf[py * PORT_W + px];
        }
    }

    lv_obj_invalidate(toast_cv);
}

static void toast_show_cb(struct k_work *work) {
    char title[sizeof(pending_title)];
    char text[sizeof(pending_text)];
    uint16_t ms;

    k_mutex_lock(&toast_lock, K_FOREVER);
    memcpy(title, pending_title, sizeof(title));
    memcpy(text, pending_text, sizeof(text));
    ms = pending_ms;
    k_mutex_unlock(&toast_lock);

    if (!toast_cv) {
        return;
    }

    toast_draw(title, text);
    lv_obj_clear_flag(toast_cv, LV_OBJ_FLAG_HIDDEN);
    /* The animation is recreated when it changes, which puts it on top:
     * bring the toast back above it. */
    lv_obj_move_foreground(toast_cv);

    k_work_reschedule_for_queue(zmk_display_work_q(), &toast_hide_work, K_MSEC(ms));
}

static K_WORK_DEFINE(toast_show_work, toast_show_cb);

void nice_oled_toast_show(const char *title, const char *text, uint16_t ms) {
    k_mutex_lock(&toast_lock, K_FOREVER);
    strncpy(pending_title, title ? title : "", sizeof(pending_title) - 1);
    pending_title[sizeof(pending_title) - 1] = '\0';
    strncpy(pending_text, text ? text : "", sizeof(pending_text) - 1);
    pending_text[sizeof(pending_text) - 1] = '\0';
    pending_ms = ms;
    k_mutex_unlock(&toast_lock);

    k_work_submit_to_queue(zmk_display_work_q(), &toast_show_work);
}

void nice_oled_toast_init(lv_obj_t *parent) {
    port_cv = lv_canvas_create(parent);
    lv_canvas_set_buffer(port_cv, port_buf, PORT_W, PORT_H, LV_IMG_CF_TRUE_COLOR);
    lv_obj_add_flag(port_cv, LV_OBJ_FLAG_HIDDEN);

    toast_cv = lv_canvas_create(parent);
    lv_canvas_set_buffer(toast_cv, toast_buf, PANEL_W, PANEL_H, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(toast_cv, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(toast_cv, LV_OBJ_FLAG_HIDDEN);
}
