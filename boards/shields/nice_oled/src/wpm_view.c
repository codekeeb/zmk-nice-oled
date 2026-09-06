/*
 * SPDX-License-Identifier: MIT
 *
 * Nucleo de la seleccion de vista de WPM: indice actual, persistencia
 * en settings y evento de cambio. Mismo patron que oled_anim.c, pero
 * para la pantalla CENTRAL.
 *
 * Se compila en ambas mitades porque el behavior que la cambia tiene
 * locality global; el que redibuja es el central.
 */

#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

#include <zmk/event_manager.h>

#include <nice_oled/events/wpm_view_changed.h>
#include <nice_oled/wpm_view.h>

ZMK_EVENT_IMPL(zmk_wpm_view_changed);

static uint8_t current_view = NICE_OLED_WPM_VIEW_BONGO;

uint8_t nice_oled_wpm_view_get(void) { return current_view; }

static void raise_changed(void) {
    raise_zmk_wpm_view_changed((struct zmk_wpm_view_changed){.index = current_view});
}

#if IS_ENABLED(CONFIG_SETTINGS)
static int wpm_view_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                                 void *cb_arg) {
    if (settings_name_steq(name, "view", NULL)) {
        uint8_t v;

        if (len != sizeof(v)) {
            return -EINVAL;
        }

        int rc = read_cb(cb_arg, &v, sizeof(v));
        if (rc >= 0) {
            if (v >= NICE_OLED_WPM_VIEW_COUNT) {
                v = NICE_OLED_WPM_VIEW_BONGO;
            }
            current_view = v;
            return 0;
        }

        return rc;
    }

    return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(nice_oled_wpm, "nice_oled_wpm", NULL, wpm_view_settings_set, NULL,
                               NULL);
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

static void persist(void) {
#if IS_ENABLED(CONFIG_SETTINGS)
    settings_save_one("nice_oled_wpm/view", &current_view, sizeof(current_view));
#endif
}

void nice_oled_wpm_view_next(void) {
    current_view = (current_view + 1) % NICE_OLED_WPM_VIEW_COUNT;

    raise_changed();
    persist();
}

int nice_oled_wpm_view_set(uint8_t idx) {
    if (idx >= NICE_OLED_WPM_VIEW_COUNT) {
        return -EINVAL;
    }

    if (idx == current_view) {
        return 0;
    }

    current_view = idx;

    raise_changed();
    persist();

    return 0;
}
