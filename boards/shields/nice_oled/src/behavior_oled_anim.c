/*
 * SPDX-License-Identifier: MIT
 *
 * Behavior &oledanim: cicla la animacion de la OLED del periferico.
 * Locality global para que se ejecute tambien en la mitad que tiene la
 * pantalla animada.
 */

#define DT_DRV_COMPAT zmk_behavior_oled_anim

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
    nice_oled_anim_next();
    LOG_INF("oledanim: animacion %d", nice_oled_anim_get());
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_oled_anim_driver_api = {
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
/* CODEKEEB PATCH -- this is what made &oledanim impossible to assign to a
 * key from ZMK Studio. Studio's set_layer_binding calls
 * zmk_behavior_validate_binding(), which calls
 * behavior_get_parameter_metadata(). That helper returns -ENODEV when a
 * driver sets NEITHER .get_parameter_metadata NOR .parameter_metadata
 * (drivers/behavior.h), and set_layer_binding maps any negative result to
 * SET_LAYER_BINDING_RESP_INVALID_PARAMETERS -- so the behavior worked
 * perfectly when pressed physically (metadata is never consulted at
 * runtime) but Studio always refused to bind it. Every stock zero-param
 * ZMK behavior declares this exact helper (see behavior_caps_word.c). */
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define OLED_ANIM_INST(n)                                                                        \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_oled_anim_driver_api);

DT_INST_FOREACH_STATUS_OKAY(OLED_ANIM_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
