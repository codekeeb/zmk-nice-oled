/*
 * SPDX-License-Identifier: MIT
 *
 * Seleccion en caliente de la animacion del periferico.
 */

#pragma once

#include <stdint.h>

/* Debe coincidir con la tabla sel_anims[] de animation.c (BUILD_ASSERT). */
#define NICE_OLED_ANIM_COUNT 6

/* Indice de animacion actual (persistido en settings). */
uint8_t nice_oled_anim_get(void);

/* Avanza a la siguiente animacion, persiste y notifica via evento. */
void nice_oled_anim_next(void);

/* CODEKEEB: fija una animacion concreta, para poder elegirla desde ZMK
 * Studio en vez de ir ciclando con &oledanim. Persiste igual que _next.
 * Devuelve 0, o -EINVAL si el indice no existe. */
int nice_oled_anim_set(uint8_t idx);
