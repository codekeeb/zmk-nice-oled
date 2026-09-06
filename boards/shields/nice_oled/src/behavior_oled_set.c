/*
 * SPDX-License-Identifier: MIT
 *
 * Behavior &oledset: fija la animacion del OLED por indice.
 *
 * A diferencia de &oledanim (que cicla y se pensó para una tecla), este
 * existe para que el central pueda decirle al PERIFERICO que animacion
 * poner: la pantalla animada es la suya y el estado es local a cada
 * mitad, asi que una llamada directa en el central no se veria. Lo usa
 * el handler RPC de ZMK Studio, del mismo modo que el RGB usa "rgbsync".
 */

#define DT_DRV_COMPAT zmk_behavior_oled_set

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include <nice_oled/oled_anim.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    nice_oled_anim_set((uint8_t)binding->param1);
    LOG_INF("oledset: animacion %d", nice_oled_anim_get());
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_oled_set_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define OLED_SET_INST(n)                                                                         \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_oled_set_driver_api);

DT_INST_FOREACH_STATUS_OKAY(OLED_SET_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
