/*
 * SPDX-License-Identifier: MIT
 *
 * Seleccion en caliente de la animacion del periferico.
 */

#pragma once

#include <stdint.h>

/* Debe coincidir con la tabla sel_anims[] de animation.c (BUILD_ASSERT). */
#define NICE_OLED_ANIM_COUNT 5

/* Indice de animacion actual (persistido en settings). */
uint8_t nice_oled_anim_get(void);

/* Avanza a la siguiente animacion, persiste y notifica via evento. */
void nice_oled_anim_next(void);
