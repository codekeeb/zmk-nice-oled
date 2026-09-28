# -*- coding: utf-8 -*-
"""Opcion 1 enriquecida con el lenguaje de la gema (texto SIEMPRE fijo).

  1a cristal: la pildora es un prisma de cristal de 6 caras que gira sobre
     su eje, iluminado por caras con varios niveles de tramado, aristas y
     brillo solido cuando una cara mira a la luz. El destello de las
     letras termina en un brillo "-o-".
  1b gema:    la pildora fluye como en la opcion 1 y, debajo del logo, una
     gema (bipiramide de 4 caras) gira en 3D con la misma iluminacion.

Todo se calcula (geometria 3D real, sombreado plano y tramado Bayer), no
se dibuja a mano. Cada ciclo cierra sin salto.
"""
import math
from logoanim import Img, BAYER, BIG, shear_x, capsule, rot_ccw

SLOPE = 0.25


def dither(level, x, y):
    """level 0..1 -> encendido segun Bayer 4x4 (0 = negro, 1 = solido)."""
    return BAYER[y % 4][x % 4] < level * 16


def quant(i):
    """intensidad de luz -> nivel de tramado, en escalones como la gema."""
    if i > 0.92: return 1.0      # la cara mira a la luz: solido
    if i > 0.70: return 0.75
    if i > 0.45: return 0.5      # damero
    if i > 0.22: return 0.25
    if i > 0.05: return 0.0625   # puntos sueltos
    return 0.0


def norm(v):
    l = math.sqrt(sum(c * c for c in v)); return tuple(c / l for c in v)


# ------------------------------------------------------------------ logo
def logo_layout(pill_w=10):
    """Coordenadas de disenio (69x32, antes de girar) del texto y la pildora."""
    letters = []
    x0 = 21
    for i, ch in enumerate("CODE"):
        letters.append((BIG[ch], x0 + 2 + i * 11, 3))
    for i, ch in enumerate("KEEB"):
        letters.append((BIG[ch], x0 + i * 11, 17))
    pill = (2, 2, pill_w, 28)          # x0, y0, ancho, alto (de pie)
    return letters, pill


def draw_letters(im, letters, glint=None):
    for glyph, lx, ly in letters:
        gh = len(glyph)
        for y, row in enumerate(glyph):
            for x, v in enumerate(row):
                if not v: continue
                X, Y = lx + x + shear_x(y, gh, SLOPE), ly + y
                if glint is not None:
                    u = X + (im.h - 1 - Y) * SLOPE
                    if glint <= u < glint + 2:
                        continue                      # la banda del destello
                im.set(X, Y)


def draw_flare(im, cx, cy, size):
    """brillo al estilo de la gema: raya horizontal con nucleo de 3x3 y
    una raya vertical mas corta. size 0..3."""
    if size <= 0: return
    arm = (0, 2, 5, 7)[size]
    for d in range(-arm, arm + 1):
        im.set(cx + d, cy)
    for d in range(-(arm // 2), arm // 2 + 1):
        im.set(cx, cy + d)
    if size >= 2:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1): im.set(cx + dx, cy + dy)


def pill_crystal(im, pill, phi):
    """la pildora como prisma de 6 caras girando sobre su eje largo."""
    px0, py0, w, h = pill
    edge, inner = capsule(w, h)
    L = norm((0.45, -0.5, 0.74))       # luz arriba-derecha en el disenio
    r = w / 2.0
    N = 6
    def facet(x):
        u = max(-0.999, min(0.999, ((x + 0.5) - r) / r))
        a = math.asin(u) + phi
        return int(math.floor(a / (2 * math.pi / N))) % N
    for (x, y) in inner:
        k = facet(x)
        th = (k + 0.5) * 2 * math.pi / N - phi      # normal de la cara
        nx, nz = math.sin(th), math.cos(th)
        # los extremos redondeados miran tambien arriba/abajo
        ny = 0.0
        if y + 0.5 < r: ny = -((r - (y + 0.5)) / r)
        elif y + 0.5 > h - r: ny = ((y + 0.5) - (h - r)) / r
        n = norm((nx * (1 - abs(ny)), ny, nz * (1 - abs(ny)) + 1e-6))
        I = max(0.0, n[0] * L[0] + n[1] * L[1] + n[2] * L[2])
        X, Y = px0 + x + shear_x(y, h, SLOPE), py0 + y
        # arista entre caras: linea encendida, como en la gema
        if facet(x) != facet(x + 1) and 0 < x < w - 1:
            im.set(X, Y); continue
        if dither(quant(I), X, Y): im.set(X, Y)
    for (x, y) in edge:
        im.set(px0 + x + shear_x(y, h, SLOPE), py0 + y)


def pill_flow(im, pill, t):
    """la pildora de la opcion 1: degradado en tramado que fluye."""
    px0, py0, w, h = pill
    edge, inner = capsule(w, h)
    for x, y in edge:
        im.set(px0 + x + shear_x(y, h, SLOPE), py0 + y)
    for x, y in inner:
        f = (h - y) / h
        dens = 0.35 + 0.45 * (1 - f) + 0.30 * math.sin(2 * math.pi * (f * 1.3 + t))
        X, Y = px0 + x + shear_x(y, h, SLOPE), py0 + y
        if dither(max(0, min(1, dens)), X, Y): im.set(X, Y)


# ------------------------------------------------------------------ gema
def gem(im, cx, cy, rw, rh, phi):
    """bipiramide de base cuadrada girando sobre el eje vertical, con
    sombreado plano por caras, tramado y aristas (vista de frente)."""
    L = norm((-0.55, -0.55, 0.63))     # luz arriba-izquierda, como la gema
    top, bot = (0.0, -rh, 0.0), (0.0, rh, 0.0)
    ring = [(rw * math.cos(phi + k * math.pi / 2), 0.0, rw * math.sin(phi + k * math.pi / 2))
            for k in range(4)]
    faces = []
    for k in range(4):
        a, b = ring[k], ring[(k + 1) % 4]
        faces.append((top, a, b)); faces.append((bot, b, a))
    zbuf = {}
    for tri in faces:
        (x1, y1, z1), (x2, y2, z2), (x3, y3, z3) = tri
        ux, uy, uz = x2 - x1, y2 - y1, z2 - z1
        vx, vy, vz = x3 - x1, y3 - y1, z3 - z1
        n = (uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx)
        if n[2] <= 0: continue                     # cara de espaldas
        n = norm(n)
        I = max(0.0, sum(n[i] * L[i] for i in range(3)))
        lvl = quant(I)
        xs = [p[0] for p in tri]; ys = [p[1] for p in tri]
        for py in range(int(math.floor(min(ys))), int(math.ceil(max(ys))) + 1):
            for px in range(int(math.floor(min(xs))), int(math.ceil(max(xs))) + 1):
                # dentro del triangulo (baricentricas en 2D)
                d = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3)
                if abs(d) < 1e-9: continue
                l1 = ((y2 - y3) * (px + .5 - x3) + (x3 - x2) * (py + .5 - y3)) / d
                l2 = ((y3 - y1) * (px + .5 - x3) + (x1 - x3) * (py + .5 - y3)) / d
                l3 = 1 - l1 - l2
                if min(l1, l2, l3) < -0.02: continue
                z = l1 * z1 + l2 * z2 + l3 * z3
                edgep = min(l1, l2, l3) < 0.09
                key = (px, py)
                if key in zbuf and zbuf[key][0] >= z: continue
                zbuf[key] = (z, edgep, lvl)
    for (px, py), (z, edgep, lvl) in zbuf.items():
        X, Y = cx + px, cy + py
        if edgep or dither(lvl, X, Y): im.set(X, Y)
        else: im.set(X, Y, 0)


# ---------------------------------------------------------- secuencias
def paste(dst, src, ox, oy):
    for y in range(src.h):
        for x in range(src.w):
            if src.p[y][x]: dst.set(ox + x, oy + y)


def seq_cristal(n=48):
    """1a: 32 x 69 (zona estandar de las animaciones)."""
    letters, pill = logo_layout(10)
    frames = []
    for i in range(n):
        t = i / n
        d = Img(69, 32)
        phi = t * 2 * math.pi / 3           # 2 caras por ciclo: cierra sin salto
        pill_crystal(d, pill, phi)
        k = i - (n - 14)                    # destello en los ultimos 14
        g = 16 + k * 4 if 0 <= k < 12 else None
        draw_letters(d, letters, g)
        v = rot_ccw(d)
        if 10 <= k <= 14:                   # brillo al final del texto
            draw_flare(v, 25, 4, (1, 2, 3, 2, 1)[k - 10])
        frames.append(v)
    return frames


def seq_gema(n=48):
    """1b: 32 x 95 (baja hasta el borde del panel): logo + gema debajo."""
    letters, pill = logo_layout(8)
    frames = []
    for i in range(n):
        t = i / n
        d = Img(69, 32)
        pill_flow(d, pill, t)
        k = i - (n - 14)
        g = 16 + k * 4 if 0 <= k < 12 else None
        draw_letters(d, letters, g)
        f = Img(32, 95)
        paste(f, rot_ccw(d), 0, 0)
        if 10 <= k <= 14:
            draw_flare(f, 25, 4, (1, 2, 3, 2, 1)[k - 10])
        # dos cuartos de vuelta por ciclo (la base cuadrada se repite cada
        # cuarto, asi que cierra sin salto)
        gem(f, 16, 83, 8.0, 11.0, t * math.pi)
        frames.append(f)
    return frames
