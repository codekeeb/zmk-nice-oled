/*
 * SPDX-License-Identifier: MIT
 *
 * Seleccion en caliente de la vista de WPM de la pantalla CENTRAL.
 *
 * Las opciones del modulo (luna, bongo cat, numero, velocimetro,
 * grafico...) eran excluyentes en tiempo de compilacion: solo una
 * llegaba al binario. Aqui se compilan todas y se elige cual se ve,
 * igual que el periferico hace con su animacion.
 */

#pragma once

#include <stdint.h>

/* Debe coincidir con el orden de wpm_view.c y con la lista de la web. */
/* El bongo cat "responsive" queda fuera: sus imagenes viven en dos
   ficheros con los MISMOS simbolos (uno por tamano de pantalla) y el
   modulo no expone con cual quedarse -- su asset esta comentado en el
   CMakeLists de origen. Compilar los dos da multiple definition. */
#define NICE_OLED_WPM_VIEW_BONGO       0
#define NICE_OLED_WPM_VIEW_LUNA        1
#define NICE_OLED_WPM_VIEW_NUMBER      2
#define NICE_OLED_WPM_VIEW_SPEEDOMETER 3
#define NICE_OLED_WPM_VIEW_GRAPH       4
#define NICE_OLED_WPM_VIEW_COUNT       5

/* Vista actual (persistida en settings). */
uint8_t nice_oled_wpm_view_get(void);

/* Avanza a la siguiente vista, persiste y notifica via evento. */
void nice_oled_wpm_view_next(void);

/* Fija una vista concreta. Devuelve 0, o -EINVAL si no existe. */
int nice_oled_wpm_view_set(uint8_t idx);
