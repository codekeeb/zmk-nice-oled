/*
 * SPDX-License-Identifier: MIT
 *
 * Behaviors de la vista de WPM de la pantalla CENTRAL:
 *   &wpmview      cicla a la siguiente (para una tecla)
 *   &wpmset <n>   fija una concreta (lo usa el RPC de ZMK Studio)
 *
 * La pantalla es la del central, asi que no hace falta reenviar nada al
 * periferico -- al contrario que la animacion del OLED derecho.
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include <nice_oled/wpm_view.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ---- &wpmview: cicla ---- */
#define DT_DRV_COMPAT zmk_behavior_wpm_view

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int wpm_view_pressed(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    nice_oled_wpm_view_next();
    LOG_INF("wpmview: vista %d", nice_oled_wpm_view_get());
    return ZMK_BEHAVIOR_OPAQUE;
}

static int wpm_view_released(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wpm_view_driver_api = {
    .binding_pressed = wpm_view_pressed,
    .binding_released = wpm_view_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define WPM_VIEW_INST(n)                                                                         \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wpm_view_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WPM_VIEW_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

/* ---- &wpmset <n>: fija ---- */
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT zmk_behavior_wpm_set

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int wpm_set_pressed(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    nice_oled_wpm_view_set((uint8_t)binding->param1);
    LOG_INF("wpmset: vista %d", nice_oled_wpm_view_get());
    return ZMK_BEHAVIOR_OPAQUE;
}

static int wpm_set_released(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_wpm_set_driver_api = {
    .binding_pressed = wpm_set_pressed,
    .binding_released = wpm_set_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define WPM_SET_INST(n)                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_wpm_set_driver_api);

DT_INST_FOREACH_STATUS_OKAY(WPM_SET_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
