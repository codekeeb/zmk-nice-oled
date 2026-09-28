# -*- coding: utf-8 -*-
"""Genera boards/shields/nice_oled/assets/codekeeb_logo.c: la animacion
CODE/KEEB de la pantalla derecha (logo fijo, la pildora como un cristal
de 6 caras que gira, destello por las letras y brillo al final).

    python tools/codekeeb_logo/gen.py

El dibujo se hace en vertical (32 x 69, la franja visible del panel de
128x32) y se guarda como las demas animaciones: 69 x 68 girado, porque el
widget se coloca en coordenadas del panel (18, -18). Se regenera; no se
edita el .c a mano.
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cristal import seq_cristal

AQUI = os.path.dirname(os.path.abspath(__file__))
SALIDA = os.path.join(AQUI, "..", "..", "boards", "shields", "nice_oled", "assets", "codekeeb_logo.c")
MS_FOTOGRAMA = 75


def guardar(frame):
    """vista 32x69 -> imagen guardada 69x68 (fila r, columna c)."""
    W, H = 69, 68
    rows = []
    for r in range(H):
        bits = []
        for c in range(W):
            x, y = r - 18, 68 - c
            bits.append(1 if 0 <= x < 32 and 0 <= y < 69 and frame.p[y][x] else 0)
        row = bytearray((W + 7) // 8)
        for c, b in enumerate(bits):
            if b: row[c >> 3] |= 0x80 >> (c & 7)
        rows.append(row)
    return b"".join(rows)


def main():
    frames = seq_cristal()
    out = ["/* Animacion CODE/KEEB de la pantalla derecha. GENERADO por",
           " * tools/codekeeb_logo/gen.py: no editar a mano. %d fotogramas," % len(frames),
           " * %d ms cada uno. */" % MS_FOTOGRAMA, "#include <lvgl.h>", "",
           "#ifndef LV_ATTRIBUTE_MEM_ALIGN", "#define LV_ATTRIBUTE_MEM_ALIGN", "#endif", ""]
    for i, f in enumerate(frames):
        data = guardar(f)
        n = "codekeeb_logo_%02d" % i
        out.append("static const LV_ATTRIBUTE_MEM_ALIGN uint8_t %s_map[] = {" % n)
        out.append("#if CONFIG_NICE_OLED_WIDGET_INVERTED")
        out.append("    0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,")
        out.append("#else")
        out.append("    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff,")
        out.append("#endif")
        for k in range(0, len(data), 18):
            out.append("    " + ", ".join("0x%02x" % b for b in data[k:k + 18]) + ",")
        out.append("};")
        out.append("const lv_img_dsc_t %s = {" % n)
        out.append("    .header.cf = LV_IMG_CF_INDEXED_1BIT,")
        out.append("    .header.always_zero = 0,")
        out.append("    .header.reserved = 0,")
        out.append("    .header.w = 69,")
        out.append("    .header.h = 68,")
        out.append("    .data_size = %d," % (len(data) + 8))
        out.append("    .data = %s_map," % n)
        out.append("};")
        out.append("")
    open(SALIDA, "w", encoding="utf8", newline="\n").write("\n".join(out))
    print("%s: %d fotogramas" % (os.path.normpath(SALIDA), len(frames)))


if __name__ == "__main__":
    main()
