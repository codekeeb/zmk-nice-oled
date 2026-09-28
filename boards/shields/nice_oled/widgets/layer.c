#include "layer.h"
#include <fonts.h>
#include <zephyr/kernel.h>

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_label_dsc_t label_dsc;
    char text[10] = {};

    if (state->layer_label == NULL) {
        sprintf(text, "Layer %i", state->layer_index);
    } else {
        strncpy(text, state->layer_label, 9);
        to_uppercase(text);
    }

#if IS_ENABLED(CONFIG_NICE_EPAPER_ON)
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono_16, LV_TEXT_ALIGN_CENTER);
    const lv_coord_t y = CONFIG_NICE_OLED_WIDGET_LAYER_CUSTOM_Y;
#else
    /* CODEKEEB: el panel del Sofle solo deja ver 32 px de ancho y con la
       fuente de 16 (8 px por letra) caben 4 letras: "LOWER" salia "LOWE".
       Se usa la mayor fuente con la que cabe el nombre, manteniendo la
       linea base de la de 16 para que el texto no salte al cambiar de capa
       (los clientes pueden renombrar capas desde ZMK Studio). */
    static const lv_font_t *const fonts[] = {&pixel_operator_mono_16, &pixel_operator_mono_12,
                                             &pixel_operator_mono_8};
    const lv_font_t *font = fonts[ARRAY_SIZE(fonts) - 1];
    for (size_t i = 0; i < ARRAY_SIZE(fonts); i++) {
        if (lv_txt_get_width(text, strlen(text), fonts[i], 0, LV_TEXT_FLAG_NONE) <=
            CONFIG_NICE_OLED_WIDGET_LAYER_WIDTH) {
            font = fonts[i];
            break;
        }
    }
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, font, LV_TEXT_ALIGN_LEFT);
    const lv_coord_t y = CONFIG_NICE_OLED_WIDGET_LAYER_CUSTOM_Y +
                         (fonts[0]->line_height - fonts[0]->base_line) -
                         (font->line_height - font->base_line);
#endif // CONFIG_NICE_EPAPER_ON

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_RESPONSIVE_BONGO_CAT)
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);
#endif
    lv_canvas_draw_text(canvas, CONFIG_NICE_OLED_WIDGET_LAYER_CUSTOM_X, y, 68, &label_dsc, text);
}
