/*
 * SPDX-License-Identifier: MIT
 *
 * Nucleo de la seleccion de animacion: indice actual, persistencia en
 * settings y evento de cambio. Se compila en AMBAS mitades (el behavior
 * tiene locality global); el listener que redibuja solo existe en el
 * periferico (animation.c).
 */

#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

#include <zmk/event_manager.h>

#include <nice_oled/events/oled_anim_changed.h>
#include <nice_oled/oled_anim.h>

ZMK_EVENT_IMPL(zmk_oled_anim_changed);

static uint8_t current_anim = 0;

uint8_t nice_oled_anim_get(void) { return current_anim; }

static void raise_changed(void) {
    ZMK_EVENT_RAISE(new_zmk_oled_anim_changed(
        (struct zmk_oled_anim_changed){.index = current_anim}));
}

#if IS_ENABLED(CONFIG_SETTINGS)
static int oled_anim_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                                  void *cb_arg) {
    const char *next;

    if (settings_name_steq(name, "anim", &next) && !next) {
        if (len != sizeof(current_anim)) {
            return -EINVAL;
        }

        int rc = read_cb(cb_arg, &current_anim, sizeof(current_anim));
        if (rc >= 0) {
            if (current_anim >= NICE_OLED_ANIM_COUNT) {
                current_anim = 0;
            }
            /* Si la pantalla ya se dibujo, el evento la actualiza. */
            raise_changed();
            return 0;
        }

        return rc;
    }

    return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(nice_oled, "nice_oled", NULL, oled_anim_settings_set, NULL, NULL);
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

void nice_oled_anim_next(void) {
    current_anim = (current_anim + 1) % NICE_OLED_ANIM_COUNT;

    raise_changed();

#if IS_ENABLED(CONFIG_SETTINGS)
    settings_save_one("nice_oled/anim", &current_anim, sizeof(current_anim));
#endif
}
