# -*- coding: utf-8 -*-
"""Propuestas de animacion pixel art para la OLED derecha (CODE/KEEB).

Zona util de la animacion en la pantalla derecha: 32 x 69 px (lienzo
x 0..31, y 73..141), 1 bit. Dos variantes:
  A: logo completo TUMBADO (se disenia en 69x32 y se gira), como la marca:
     pildora a la izquierda, CODE sobre KEEB.
  B: DE PIE en 32x69: CODE / pildora / KEEB, la pildora hace de "/".
Animacion: la pildora se enciende, las letras se teclean con cursor, la
pildora fluye (degradado en tramado), pasa un brillo y se borra.
"""
import math

# ---------------- glifos (1 = encendido), estilo del logotipo -------------
def G(rows):
    return [[1 if ch == "#" else 0 for ch in r] for r in rows]

BIG = {  # 9 x 12, trazo de 3
    "C": G(["..#######", ".########", "#########", "###......", "###......", "###......",
            "###......", "###......", "###......", "#########", ".########", "..#######"]),
    "O": G(["..#######", ".########", "#########", "###...###", "###...###", "###...###",
            "###...###", "###...###", "###...###", "#########", "########.", "#######.."]),
    "D": G(["#######..", "########.", "#########", "###...###", "###...###", "###...###",
            "###...###", "###...###", "###...###", "#########", "########.", "#######.."]),
    "E": G(["#########", "#########", "#########", "###......", "###......", "#######..",
            "#######..", "###......", "###......", "#########", "#########", "#########"]),
    "K": G(["###...###", "###..###.", "###.###..", "######...", "#####....", "####.....",
            "#####....", "######...", "###.###..", "###..###.", "###...###", "###...###"]),
    "B": G(["########.", "#########", "###...###", "###...###", "###..###.", "#######..",
            "########.", "###...###", "###...###", "#########", "#########", "########."]),
}
SMALL = {  # 6 x 9, trazo de 2
    "C": G([".#####", "######", "##....", "##....", "##....", "##....", "##....", "######", ".#####"]),
    "O": G([".#####", "######", "##..##", "##..##", "##..##", "##..##", "##..##", "######", "#####."]),
    "D": G(["#####.", "######", "##..##", "##..##", "##..##", "##..##", "##..##", "######", "#####."]),
    "E": G(["######", "######", "##....", "##....", "#####.", "##....", "##....", "######", "######"]),
    "K": G(["##..##", "##.##.", "####..", "###...", "###...", "####..", "##.##.", "##..##", "##..##"]),
    "B": G(["#####.", "######", "##..##", "#####.", "#####.", "##..##", "##..##", "######", "#####."]),
}

BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


class Img:
    def __init__(s, w, h): s.w, s.h = w, h; s.p = [[0] * w for _ in range(h)]
    def set(s, x, y, v=1):
        if 0 <= x < s.w and 0 <= y < s.h: s.p[y][x] = v
    def get(s, x, y): return s.p[y][x] if 0 <= x < s.w and 0 <= y < s.h else 0


def shear_x(y, h, slope):
    """desplazamiento a la derecha de la fila y (arriba se desplaza mas)."""
    return int((h - 1 - y) * slope)


def capsule(w, h):
    """pildora de pie, extremos redondeados, como mascara (borde, interior)."""
    r = w / 2.0
    edge, inner = set(), set()
    for y in range(h):
        for x in range(w):
            cx = x + 0.5
            cy = y + 0.5
            if cy < r: dy = r - cy
            elif cy > h - r: dy = cy - (h - r)
            else: dy = 0
            d = math.hypot(cx - r, dy)
            if d <= r:
                if d > r - 1.2: edge.add((x, y))
                else: inner.add((x, y))
    return edge, inner


class Scene:
    """Elementos colocados (con su inclinacion ya aplicada) y su orden."""
    def __init__(s, w, h):
        s.w, s.h = w, h
        s.letters = []      # (orden, [(x,y)...], x_cursor, y_cursor, ancho)
        s.pill = None       # (orden, edge, inner, rows) coords finales

    def add_letter(s, order, glyph, x0, y0, slope):
        gh = len(glyph)
        pts = [(x0 + x + shear_x(y, gh, slope), y0 + y)
               for y, row in enumerate(glyph) for x, v in enumerate(row) if v]
        # cursor: barra de 2 px en cursiva donde va a caer la letra
        s.letters.append((order, pts, x0, y0, gh, slope))

    def add_pill(s, order, x0, y0, w, h, slope):
        e, i = capsule(w, h)
        sh = lambda p: (x0 + p[0] + shear_x(p[1], h, slope), y0 + p[1])
        s.pill = (order, [sh(p) for p in e], [sh(p) for p in i], (y0, y0 + h))


def render(scene, shown, pill_level, flow_t, cursor, glint):
    """shown: nº de elementos visibles (en orden). pill_level 0..1 encendido
    de la pildora (de abajo arriba al aparecer). flow_t fase del tramado.
    cursor: (x,y,w) o None. glint: posicion de la banda de brillo o None."""
    im = Img(scene.w, scene.h)
    elems = sorted([(o, "L", pts, 0) for (o, pts, *_r) in scene.letters] +
                   ([(scene.pill[0], "P", None, 0)] if scene.pill else []), key=lambda t: t[0])
    for idx, (o, kind, pts, _) in enumerate(elems):
        if idx >= shown: break
        if kind == "L":
            for x, y in pts: im.set(x, y)
        else:
            order, edge, inner, (py0, py1) = scene.pill
            ph = py1 - py0
            for x, y in edge:
                if (py1 - y) <= pill_level * ph + 0.5: im.set(x, y)
            for x, y in inner:
                if (py1 - y) > pill_level * ph + 0.5: continue
                # degradado: denso abajo (naranja), ralo arriba (cian), y una
                # onda que recorre la pildora de abajo arriba
                f = (py1 - y) / ph
                dens = 0.35 + 0.45 * (1 - f) + 0.30 * math.sin(2 * math.pi * (f * 1.3 + flow_t))
                dens = max(0.0, min(1.0, dens))
                if BAYER[y % 4][x % 4] < dens * 16: im.set(x, y)
    if glint is not None:
        # banda diagonal (misma inclinacion que la cursiva) que apaga pixeles
        for y in range(im.h):
            for x in range(im.w):
                u = x + (im.h - 1 - y) * 0.25
                if glint <= u < glint + 2: im.set(x, y, 0)
    if cursor:
        cx, cy, ch, sl = cursor
        for y in range(ch):
            x = cx + shear_x(y, ch, sl)
            im.set(x, cy + y); im.set(x + 1, cy + y)
    return im


def sequence(scene):
    """Lista de fotogramas (Img) de un ciclo, a 100 ms cada uno."""
    n = len(scene.letters) + (1 if scene.pill else 0)
    order_cursor = {}
    for (o, pts, cx, cy, gh, sl) in scene.letters: order_cursor[o] = (cx, cy, gh, sl)
    def cur_for(k):   # cursor bajo el siguiente elemento a teclear
        return order_cursor.get(k)
    frames = []
    pill_order = scene.pill[0] if scene.pill else -1
    # 1) cursor parpadeando sobre la pantalla vacia
    for b in (1, 0, 1, 0):
        frames.append(render(scene, 0, 0, 0, cur_for(0) if b else None, None))
    # 2) tecleo
    t = 0.0
    for k in range(n):
        if k == pill_order:
            for lv in (0.25, 0.5, 0.75, 1.0):
                t += 0.06
                frames.append(render(scene, k + 1, lv, t, None, None))
        else:
            frames.append(render(scene, k, 1.0, t, cur_for(k), None))
            t += 0.06
            frames.append(render(scene, k + 1, 1.0, t, cur_for(k + 1), None))
    # 3) la pildora fluye, cursor parpadeando al final
    last = None
    for i in range(16):
        t += 0.06
        frames.append(render(scene, n, 1.0, t, None, None))
    # 4) brillo que cruza
    for g in range(-4, scene.w + 12, 5):
        t += 0.06
        frames.append(render(scene, n, 1.0, t, None, g))
    for i in range(6):
        t += 0.06
        frames.append(render(scene, n, 1.0, t, None, None))
    # 5) retroceso: se borra hacia atras
    for k in range(n, -1, -1):
        if k - 1 == pill_order:
            for lv in (0.66, 0.33):
                t += 0.06
                frames.append(render(scene, k, lv, t, None, None))
        frames.append(render(scene, k, 1.0, t, cur_for(k), None))
    for b in (0, 1):
        frames.append(render(scene, 0, 0, 0, cur_for(0) if b else None, None))
    return frames


def scene_tumbado():
    """A: 69 x 32 (se gira luego). Pildora a la izquierda, CODE sobre KEEB."""
    s = Scene(69, 32)
    slope = 0.25
    s.add_pill(0, 3, 2, 8, 28, slope)
    x0 = 19
    for i, ch in enumerate("CODE"):
        s.add_letter(1 + i, BIG[ch], x0 + 2 + i * 11, 3, slope)
    for i, ch in enumerate("KEEB"):
        s.add_letter(5 + i, BIG[ch], x0 + i * 11, 17, slope)
    return s


def scene_de_pie():
    """B: 32 x 69. CODE / pildora / KEEB."""
    s = Scene(32, 69)
    slope = 0.25
    for i, ch in enumerate("CODE"):
        s.add_letter(i, SMALL[ch], 1 + i * 7, 8, slope)
    s.add_pill(4, 12, 21, 6, 26, 0.25)
    for i, ch in enumerate("KEEB"):
        s.add_letter(5 + i, SMALL[ch], 1 + i * 7, 51, slope)
    return s


def rot_ccw(im):
    """girar 90 antihorario: la lectura queda de abajo arriba."""
    out = Img(im.h, im.w)
    for y in range(im.h):
        for x in range(im.w):
            if im.p[y][x]: out.set(y, im.w - 1 - x)
    return out
