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

/* CODEKEEB PATCH: real (non-NULL) data/config structs, same pattern as
 * every stock ZMK behavior (see behavior_caps_word.c) -- even though this
 * behavior doesn't need per-instance state today, the earlier
 * NULL/NULL/NULL/NULL registration left it structurally different from
 * every working Studio-editable behavior in this tree, and ZMK Studio's
 * set_layer_binding rejected it with INVALID_BEHAVIOR while every other
 * zero-param behavior with real data/config structs (caps_word,
 * studio_unlock, trans, etc.) worked fine. */
struct behavior_oled_anim_config {
    uint8_t _unused;
};

struct behavior_oled_anim_data {
    uint8_t _unused;
};

static const struct behavior_oled_anim_config behavior_oled_anim_config_0 = {0};
static struct behavior_oled_anim_data behavior_oled_anim_data_0 = {0};

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
};

#define OLED_ANIM_INST(n)                                                                        \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &behavior_oled_anim_data_##n,                         \
                            &behavior_oled_anim_config_##n, POST_KERNEL,                          \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_oled_anim_driver_api);

DT_INST_FOREACH_STATUS_OKAY(OLED_ANIM_INST)

/* CODEKEEB TEMP DEBUG: log this device's readiness and real local_id at
 * boot, late enough that the local-id map is populated. Remove once the
 * Studio set_layer_binding INVALID_BEHAVIOR bug is diagnosed. */
static int oled_anim_debug_dump(void) {
    const struct device *dev = DEVICE_DT_GET(DT_DRV_INST(0));
    LOG_ERR("CODEKEEB DEBUG oledanim: dev->name=\"%s\" ready=%d local_id=%u", dev->name,
            device_is_ready(dev), zmk_behavior_get_local_id(dev->name));
    return 0;
}
SYS_INIT(oled_anim_debug_dump, APPLICATION, 99);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
