#!/usr/bin/env python3
"""
RetroMania Arcade — High Quality Sprite Improver
=================================================
- Parses SVG files (gateway, lamp, sensor, traffic, camera) using ElementTree
- Renders them to high-quality PNGs using Pillow with anti-aliasing
- Generates MUCH better game sprites: ninja, enemies, tiles, backgrounds, items
- Uses 2x → 1x downscale trick for smooth anti-aliased edges
- Uses Pillow drawing primitives (arcs, pieslice, chords, etc.) for smooth shapes

Run from project root: python3 tools/improve_sprites.py
"""

import math
import os
import re
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw

ASSETS_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "assets")
# SVGs are in the root-level application_v1.27/assets/ directory
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
SVG_DIR = os.path.join(PROJECT_ROOT, "application_v1.27", "assets")
os.makedirs(ASSETS_DIR, exist_ok=True)

def save_png(name, img):
    """Save an image as PNG, PRESERVING alpha channel for game engine.
    Game rendersystem uses: if(pixel & 0xFF000000) to check transparency.
    """
    path = os.path.join(ASSETS_DIR, name)
    if img.mode == 'RGBA':
        img.save(path, "PNG")
    elif img.mode == 'RGB':
        img.save(path, "PNG")
    else:
        img = img.convert('RGBA')
        img.save(path, "PNG")
    print(f"  ✓ {name:30s} ({img.size[0]}x{img.size[1]} mode={img.mode})")


# ================================================================
# SVG Parser & Renderer
# ================================================================

def parse_color(color_str, default=(0, 0, 0)):
    """Parse an SVG color string to an RGB tuple."""
    if not color_str:
        return default
    c = color_str.strip()
    if c.startswith('#'):
        c = c[1:]
        if len(c) == 3:
            c = ''.join([x*2 for x in c])
        if len(c) == 6:
            return (int(c[0:2], 16), int(c[2:4], 16), int(c[4:6], 16))
    # Named colors
    named = {
        'red': (255, 0, 0), 'green': (0, 255, 0), 'blue': (0, 0, 255),
        'white': (255, 255, 255), 'black': (0, 0, 0), 'gray': (128, 128, 128),
        'yellow': (255, 255, 0), 'orange': (255, 165, 0), 'purple': (128, 0, 128),
        'cyan': (0, 255, 255), 'magenta': (255, 0, 255), 'silver': (192, 192, 192),
        'navy': (0, 0, 128), 'teal': (0, 128, 128), 'maroon': (128, 0, 0),
        'olive': (128, 128, 0), 'lime': (0, 255, 0), 'aqua': (0, 255, 255),
        'fuchsia': (255, 0, 255), 'none': None,
    }
    return named.get(c.lower(), default)


def parse_opacity(style_str, tag_attribs):
    """Parse opacity from style string or tag attributes."""
    # Check direct attribute
    if 'opacity' in tag_attribs:
        try:
            return float(tag_attribs['opacity'])
        except ValueError:
            pass
    if 'fill-opacity' in tag_attribs:
        try:
            return float(tag_attribs['fill-opacity'])
        except ValueError:
            pass
    # Check style string
    if not style_str:
        return 1.0
    for part in style_str.split(';'):
        part = part.strip()
        if part.startswith('opacity:'):
            try:
                return float(part.split(':')[1])
            except ValueError:
                pass
        if part.startswith('fill-opacity:'):
            try:
                return float(part.split(':')[1])
            except ValueError:
                pass
    return 1.0


def parse_style_color(style_str, attribs, key='fill'):
    """Parse a color from style string or attribute, with fallback."""
    if key in attribs:
        c = parse_color(attribs[key])
        if c is not None:
            return c
    if style_str:
        for part in style_str.split(';'):
            part = part.strip()
            if part.startswith(key + ':'):
                val = part.split(':', 1)[1].strip()
                c = parse_color(val)
                if c is not None:
                    return c
    # Default based on key
    return (0, 0, 0) if key == 'fill' else None


def parse_style_stroke_width(style_str, attribs):
    """Parse stroke-width from style or attribute."""
    if 'stroke-width' in attribs:
        try:
            return float(attribs['stroke-width'])
        except ValueError:
            pass
    if style_str:
        for part in style_str.split(';'):
            part = part.strip()
            if part.startswith('stroke-width:'):
                try:
                    return float(part.split(':')[1])
                except ValueError:
                    pass
    return 1.0


def svg_to_pixels(svg_path, target_size=128):
    """
    Parse a simple SVG file and render it to a high-quality RGBA image.
    Handles: rect, circle, path (limited), ellipse via cx/cy/rx/ry.
    """
    tree = ET.parse(svg_path)
    root = tree.getroot()

    # Get viewBox dimensions
    view_box = root.get('viewBox', '0 0 24 24')
    vb_parts = list(map(float, view_box.split()))
    if len(vb_parts) == 4:
        vb_x, vb_y, vb_w, vb_h = vb_parts
    else:
        # Fall back to width/height attributes
        vb_w = float(root.get('width', 24))
        vb_h = float(root.get('height', 24))
        vb_x = vb_y = 0

    if vb_w == 0 or vb_h == 0:
        vb_w, vb_h = 24, 24

    # Create high-res image (4x for anti-aliasing)
    scale = target_size / max(vb_w, vb_h)
    render_size = int(vb_w * scale * 2)  # 2x for AA
    render_size = max(render_size, 8)

    img = Image.new('RGBA', (render_size, render_size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    def svg_to_img_coords(x, y):
        """Convert SVG coordinates to image pixel coordinates."""
        px = (x - vb_x) * scale * 2
        py = (y - vb_y) * scale * 2
        return px, py

    def svg_to_img_len(l):
        """Convert SVG length to image pixels."""
        return l * scale * 2

    def render_element(elem):
        """Recursively render an SVG element and its children."""
        tag = elem.tag.split('}')[-1] if '}' in elem.tag else elem.tag
        attrib = elem.attrib
        style = attrib.get('style', '')

        fill_color = parse_style_color(style, attrib, 'fill')
        stroke_color = parse_style_color(style, attrib, 'stroke')
        stroke_width = parse_style_stroke_width(style, attrib)
        opacity_val = parse_opacity(style, attrib)

        if tag == 'rect':
            x = float(attrib.get('x', 0))
            y = float(attrib.get('y', 0))
            w = float(attrib.get('width', 0))
            h = float(attrib.get('height', 0))
            rx = float(attrib.get('rx', 0))
            ry = float(attrib.get('ry', rx))

            p1 = svg_to_img_coords(x, y)
            p2 = svg_to_img_coords(x + w, y + h)
            sw = svg_to_img_len(stroke_width) if stroke_color else 0

            if fill_color:
                if rx > 0 or ry > 0:
                    # Rounded rectangle
                    rrx = svg_to_img_len(rx)
                    rry = svg_to_img_len(ry)
                    draw.rounded_rectangle([p1, p2], radius=max(rrx, rry),
                                           fill=fill_color + (int(255 * opacity_val),))
                else:
                    draw.rectangle([p1, p2], fill=fill_color + (int(255 * opacity_val),))

            if stroke_color:
                draw.rectangle([p1, p2], outline=stroke_color + (255,), width=max(1, int(sw)))

        elif tag == 'circle':
            cx = float(attrib.get('cx', 0))
            cy = float(attrib.get('cy', 0))
            r = float(attrib.get('r', 0))

            pc = svg_to_img_coords(cx, cy)
            pr = svg_to_img_len(r)
            sw = svg_to_img_len(stroke_width) if stroke_color else 0

            if fill_color:
                draw.ellipse([pc[0] - pr, pc[1] - pr, pc[0] + pr, pc[1] + pr],
                             fill=fill_color + (int(255 * opacity_val),))
            if stroke_color:
                draw.ellipse([pc[0] - pr, pc[1] - pr, pc[0] + pr, pc[1] + pr],
                             outline=stroke_color + (255,), width=max(1, int(sw)))

        elif tag == 'ellipse':
            cx = float(attrib.get('cx', 0))
            cy = float(attrib.get('cy', 0))
            rx = float(attrib.get('rx', 0))
            ry = float(attrib.get('ry', 0))

            pc = svg_to_img_coords(cx, cy)
            prx = svg_to_img_len(rx)
            pry = svg_to_img_len(ry)

            if fill_color:
                draw.ellipse([pc[0] - prx, pc[1] - pry, pc[0] + prx, pc[1] + pry],
                             fill=fill_color + (int(255 * opacity_val),))
            if stroke_color:
                draw.ellipse([pc[0] - prx, pc[1] - pry, pc[0] + prx, pc[1] + pry],
                             outline=stroke_color + (255,), width=max(1, int(svg_to_img_len(stroke_width))))

        elif tag == 'path':
            d = attrib.get('d', '')
            if d and fill_color:
                # Parse path commands (simplified: just handle M, L, C, Q, Z)
                # For our simple SVGs, we can approximate with points
                commands = re.findall(r'([MLCQZ])\s*([-\d.,\s]*)', d.upper())
                points = []
                current_pos = (0, 0)
                for cmd, params_str in commands:
                    params = [float(x) for x in re.findall(r'-?\d+\.?\d*', params_str)]
                    if cmd == 'M' and len(params) >= 2:
                        pt = svg_to_img_coords(params[0], params[1])
                        points = [pt]
                        current_pos = pt
                    elif cmd == 'L' and len(params) >= 2:
                        pt = svg_to_img_coords(params[0], params[1])
                        points.append(pt)
                        current_pos = pt
                    elif cmd == 'C' and len(params) >= 6:
                        # Approximate cubic bezier with line
                        pt = svg_to_img_coords(params[4], params[5])
                        points.append(pt)
                        current_pos = pt
                    elif cmd == 'Q' and len(params) >= 4:
                        pt = svg_to_img_coords(params[2], params[3])
                        points.append(pt)
                        current_pos = pt
                    elif cmd == 'H' and len(params) >= 1:
                        pt = svg_to_img_coords(params[0], current_pos[1] / (scale * 2) + vb_y)
                        points.append(pt)
                        current_pos = pt
                    elif cmd == 'V' and len(params) >= 1:
                        pt = svg_to_img_coords(current_pos[0] / (scale * 2) + vb_x, params[0])
                        points.append(pt)
                        current_pos = pt
                    # 'Z' closes the path (we'll just polygon fill)

                if len(points) >= 3:
                    sw_val = int(svg_to_img_len(stroke_width)) if stroke_color else 0
                    draw.polygon(points, fill=fill_color + (int(255 * opacity_val),))
                    if stroke_color and sw_val > 0:
                        draw.polygon(points, outline=stroke_color + (255,), width=sw_val)

        # Process child elements
        for child in elem:
            render_element(child)

    # Process root children
    for child in root:
        render_element(child)

    # Downscale to target with smooth anti-aliasing
    final_size = int(max(vb_w, vb_h) * scale)
    if final_size <= 0:
        final_size = target_size

    img_resized = img.resize((final_size, final_size), Image.LANCZOS)
    return img_resized


# ================================================================
# High Quality Sprite Generators
# ================================================================




def make_ninja_idle_aa():
    """High quality ninja idle sprite at 64x64 with anti-aliasing."""
    S = 128  # Work at 2x
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # ---- Body (dark blue ninja gi) ----
    # Upper body (shifted slightly up to fit 48x48)
    draw.ellipse([S*0.30, S*0.28, S*0.70, S*0.56], fill=(55, 55, 75, 255))
    # Lower body
    draw.ellipse([S*0.25, S*0.52, S*0.75, S*0.82], fill=(45, 45, 65, 255))

    # ---- Head ----
    # Face
    draw.ellipse([S*0.30, S*0.10, S*0.70, S*0.38], fill=(255, 200, 155, 255))
    # Hair / bandana top
    draw.ellipse([S*0.28, S*0.06, S*0.72, S*0.24], fill=(40, 40, 40, 255))
    # Bandana stripe
    draw.rectangle([S*0.28, S*0.20, S*0.72, S*0.24], fill=(180, 35, 35, 255))

    # Eyes (white)
    draw.ellipse([S*0.34, S*0.22, S*0.42, S*0.30], fill=(255, 255, 255, 255))
    draw.ellipse([S*0.58, S*0.22, S*0.66, S*0.30], fill=(255, 255, 255, 255))
    # Pupils
    draw.ellipse([S*0.37, S*0.24, S*0.41, S*0.28], fill=(30, 30, 30, 255))
    draw.ellipse([S*0.59, S*0.24, S*0.63, S*0.28], fill=(30, 30, 30, 255))

    # ---- Arms ----
    # Left arm
    draw.ellipse([S*0.18, S*0.30, S*0.30, S*0.48], fill=(255, 200, 155, 255))
    draw.ellipse([S*0.18, S*0.45, S*0.30, S*0.55], fill=(55, 55, 75, 255))
    # Right arm
    draw.ellipse([S*0.70, S*0.30, S*0.82, S*0.48], fill=(255, 200, 155, 255))
    draw.ellipse([S*0.70, S*0.45, S*0.82, S*0.55], fill=(55, 55, 75, 255))

    # ---- Belt ----
    draw.rectangle([S*0.28, S*0.48, S*0.72, S*0.52], fill=(180, 35, 35, 255))

    # ---- Legs ----
    draw.ellipse([S*0.30, S*0.60, S*0.48, S*0.88], fill=(45, 45, 65, 255))
    draw.ellipse([S*0.52, S*0.60, S*0.70, S*0.88], fill=(45, 45, 65, 255))

    # ---- Feet ----
    draw.ellipse([S*0.28, S*0.82, S*0.46, S*0.94], fill=(35, 35, 45, 255))
    draw.ellipse([S*0.54, S*0.82, S*0.72, S*0.94], fill=(35, 35, 45, 255))

    # ---- Sword hilt (visible over right shoulder) ----
    draw.line([(S*0.68, S*0.12), (S*0.68, S*0.30)], fill=(160, 120, 50, 255), width=4)
    draw.line([(S*0.68, S*0.08), (S*0.68, S*0.12)], fill=(180, 180, 190, 255), width=5)

    # ---- Highlights ----
    # Gi highlight
    draw.ellipse([S*0.35, S*0.33, S*0.48, S*0.42], fill=(80, 80, 100, 60))

    return img.resize((48, 48), Image.LANCZOS)


def make_ninja_walk1_aa():
    """High quality ninja walk1 sprite."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Upper body (slightly tilted)
    draw.ellipse([S*0.30, S*0.30, S*0.72, S*0.58], fill=(55, 55, 75, 255))
    # Lower body
    draw.ellipse([S*0.25, S*0.55, S*0.70, S*0.82], fill=(45, 45, 65, 255))

    # Head
    draw.ellipse([S*0.30, S*0.10, S*0.72, S*0.38], fill=(255, 200, 155, 255))
    draw.ellipse([S*0.28, S*0.06, S*0.74, S*0.24], fill=(40, 40, 40, 255))
    draw.rectangle([S*0.28, S*0.20, S*0.74, S*0.24], fill=(180, 35, 35, 255))
    draw.ellipse([S*0.34, S*0.22, S*0.42, S*0.30], fill=(255, 255, 255, 255))
    draw.ellipse([S*0.58, S*0.22, S*0.66, S*0.30], fill=(255, 255, 255, 255))
    draw.ellipse([S*0.37, S*0.24, S*0.41, S*0.28], fill=(30, 30, 30, 255))
    draw.ellipse([S*0.59, S*0.24, S*0.63, S*0.28], fill=(30, 30, 30, 255))

    # Right arm forward
    draw.ellipse([S*0.72, S*0.32, S*0.88, S*0.48], fill=(255, 200, 155, 255))
    draw.ellipse([S*0.72, S*0.44, S*0.88, S*0.56], fill=(55, 55, 75, 255))
    # Hand
    draw.ellipse([S*0.80, S*0.52, S*0.88, S*0.58], fill=(255, 200, 155, 255))

    # Left arm back
    draw.ellipse([S*0.16, S*0.34, S*0.28, S*0.48], fill=(255, 200, 155, 255))
    draw.ellipse([S*0.16, S*0.44, S*0.28, S*0.52], fill=(55, 55, 75, 255))

    # Belt
    draw.rectangle([S*0.27, S*0.48, S*0.73, S*0.52], fill=(180, 35, 35, 255))

    # Legs — walking (right forward)
    draw.ellipse([S*0.50, S*0.60, S*0.72, S*0.88], fill=(45, 45, 65, 255))
    draw.ellipse([S*0.28, S*0.60, S*0.44, S*0.80], fill=(45, 45, 65, 255))

    # Feet
    draw.ellipse([S*0.52, S*0.82, S*0.74, S*0.94], fill=(35, 35, 45, 255))
    draw.ellipse([S*0.26, S*0.76, S*0.42, S*0.86], fill=(35, 35, 45, 255))

    # Sword
    draw.line([(S*0.70, S*0.12), (S*0.70, S*0.30)], fill=(160, 120, 50, 255), width=4)

    return img.resize((48, 48), Image.LANCZOS)


def make_ninja_walk2_aa():
    """High quality ninja walk2 sprite (mirror of walk1)."""
    img = make_ninja_walk1_aa()
    return img.transpose(Image.FLIP_LEFT_RIGHT)


def make_ninja_jump_aa():
    """High quality ninja jump sprite — arms up, legs tucked."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Body (compact)
    draw.ellipse([S*0.30, S*0.30, S*0.70, S*0.56], fill=(55, 55, 75, 255))

    # Head (slightly higher)
    draw.ellipse([S*0.30, S*0.06, S*0.70, S*0.32], fill=(255, 200, 155, 255))
    draw.ellipse([S*0.28, S*0.02, S*0.72, S*0.18], fill=(40, 40, 40, 255))
    draw.rectangle([S*0.28, S*0.16, S*0.72, S*0.20], fill=(180, 35, 35, 255))
    draw.ellipse([S*0.34, S*0.18, S*0.42, S*0.26], fill=(255, 255, 255, 255))
    draw.ellipse([S*0.58, S*0.18, S*0.66, S*0.26], fill=(255, 255, 255, 255))
    draw.ellipse([S*0.37, S*0.20, S*0.41, S*0.24], fill=(30, 30, 30, 255))
    draw.ellipse([S*0.59, S*0.20, S*0.63, S*0.24], fill=(30, 30, 30, 255))

    # Arms up
    draw.ellipse([S*0.22, S*0.10, S*0.34, S*0.28], fill=(55, 55, 75, 255))
    draw.ellipse([S*0.66, S*0.10, S*0.78, S*0.28], fill=(55, 55, 75, 255))

    # Belt
    draw.rectangle([S*0.28, S*0.44, S*0.72, S*0.48], fill=(180, 35, 35, 255))

    # Legs tucked
    draw.ellipse([S*0.32, S*0.50, S*0.48, S*0.70], fill=(45, 45, 65, 255))
    draw.ellipse([S*0.52, S*0.50, S*0.68, S*0.70], fill=(45, 45, 65, 255))

    # Feet bunched
    draw.ellipse([S*0.34, S*0.66, S*0.46, S*0.76], fill=(35, 35, 45, 255))
    draw.ellipse([S*0.54, S*0.66, S*0.66, S*0.76], fill=(35, 35, 45, 255))

    return img.resize((48, 48), Image.LANCZOS)


def make_ninja_attack_aa():
    """High quality ninja attack sprite — sword slash."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Body (twisted)
    draw.ellipse([S*0.28, S*0.30, S*0.66, S*0.58], fill=(55, 55, 75, 255))

    # Head
    draw.ellipse([S*0.30, S*0.10, S*0.66, S*0.38], fill=(255, 200, 155, 255))
    draw.ellipse([S*0.28, S*0.06, S*0.68, S*0.24], fill=(40, 40, 40, 255))
    draw.rectangle([S*0.28, S*0.20, S*0.68, S*0.24], fill=(180, 35, 35, 255))
    draw.ellipse([S*0.34, S*0.22, S*0.42, S*0.30], fill=(255, 255, 255, 255))
    draw.ellipse([S*0.54, S*0.22, S*0.62, S*0.30], fill=(255, 255, 255, 255))
    draw.ellipse([S*0.37, S*0.24, S*0.41, S*0.28], fill=(30, 30, 30, 255))
    draw.ellipse([S*0.55, S*0.24, S*0.59, S*0.28], fill=(30, 30, 30, 255))

    # Sword arm extended right
    draw.ellipse([S*0.68, S*0.30, S*0.90, S*0.44], fill=(55, 55, 75, 255))

    # Sword blade
    draw.line([(S*0.84, S*0.30), (S*1.02, S*0.20)], fill=(200, 200, 210, 255), width=5)
    draw.line([(S*0.84, S*0.30), (S*0.90, S*0.42)], fill=(160, 120, 50, 255), width=4)

    # Slash trail effect
    draw.line([(S*0.96, S*0.18), (S*1.04, S*0.24)], fill=(255, 255, 200, 120), width=3)

    # Belt
    draw.rectangle([S*0.26, S*0.48, S*0.68, S*0.52], fill=(180, 35, 35, 255))

    # Legs
    draw.ellipse([S*0.28, S*0.60, S*0.46, S*0.86], fill=(45, 45, 65, 255))
    draw.ellipse([S*0.50, S*0.60, S*0.66, S*0.86], fill=(45, 45, 65, 255))
    draw.ellipse([S*0.26, S*0.80, S*0.44, S*0.94], fill=(35, 35, 45, 255))
    draw.ellipse([S*0.52, S*0.80, S*0.68, S*0.94], fill=(35, 35, 45, 255))

    return img.resize((48, 48), Image.LANCZOS)


# ================================================================
# HIGH QUALITY ENEMIES
# ================================================================

def make_enemy_bouncer_aa():
    """High quality red slime bouncer with smooth shading."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Body — rounded slime shape with gradient
    cx, cy = S//2, S//2 + 4
    for r in range(40, 0, -1):
        alpha = 255
        t = r / 40.0
        r_col = int(200 * (0.6 + 0.4 * t))
        g_col = int(50 * (0.5 + 0.5 * t))
        b_col = int(50 * (0.5 + 0.5 * t))
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(r_col, g_col, b_col, alpha))

    # Highlight
    draw.ellipse([cx - 15, cy - 20, cx - 5, cy - 10], fill=(255, 150, 150, 100))

    # Eyes
    draw.ellipse([cx - 14, cy - 10, cx - 6, cy], fill=(255, 255, 220, 255))
    draw.ellipse([cx + 6, cy - 10, cx + 14, cy], fill=(255, 255, 220, 255))
    draw.ellipse([cx - 10, cy - 8, cx - 7, cy - 4], fill=(30, 30, 30, 255))
    draw.ellipse([cx + 7, cy - 8, cx + 10, cy - 4], fill=(30, 30, 30, 255))

    return img.resize((44, 44), Image.LANCZOS)


def make_enemy_chaser_aa():
    """High quality blue bat chaser with smooth shading."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Body
    cx, cy = S//2, S//2
    for r in range(24, 0, -1):
        t = r / 24.0
        r_col = int(80 + 60 * t)
        g_col = int(80 + 60 * t)
        b_col = int(200 + 55 * t)
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(r_col, g_col, b_col, 255))

    # Wings — left
    draw.polygon([(cx-20, cy-5), (cx-50, cy-20), (cx-48, cy), (cx-40, cy+5), (cx-20, cy+5)],
                 fill=(60, 60, 140, 200))
    # Wings — right
    draw.polygon([(cx+20, cy-5), (cx+50, cy-20), (cx+48, cy), (cx+40, cy+5), (cx+20, cy+5)],
                 fill=(60, 60, 140, 200))

    # Eyes (glowing)
    draw.ellipse([cx - 12, cy - 8, cx - 4, cy + 2], fill=(255, 255, 120, 255))
    draw.ellipse([cx + 4, cy - 8, cx + 12, cy + 2], fill=(255, 255, 120, 255))
    draw.ellipse([cx - 9, cy - 6, cx - 6, cy - 1], fill=(180, 40, 40, 255))
    draw.ellipse([cx + 6, cy - 6, cx + 9, cy - 1], fill=(180, 40, 40, 255))

    # Body highlight
    draw.ellipse([cx - 8, cy - 14, cx - 2, cy - 6], fill=(150, 150, 240, 80))

    return img.resize((44, 44), Image.LANCZOS)


def make_enemy_boss_aa():
    """High quality purple boss at 128x128."""
    S = 256  # Work at 2x for AA
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    cx, cy = S//2, S//2 + 8

    # Body — layered circles for smooth shading
    for r in range(80, 10, -2):
        t = r / 80.0
        r_col = int(120 + 60 * t)
        g_col = int(40 + 30 * t)
        b_col = int(140 + 70 * t)
        alpha = 255
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(r_col, g_col, b_col, alpha))

    # Body highlight
    draw.ellipse([cx - 30, cy - 50, cx + 10, cy - 20], fill=(180, 100, 200, 60))

    # Eyes (large, angry)
    draw.ellipse([cx - 30, cy - 30, cx - 10, cy - 10], fill=(255, 255, 100, 255))
    draw.ellipse([cx + 10, cy - 30, cx + 30, cy - 10], fill=(255, 255, 100, 255))
    # Pupils
    draw.ellipse([cx - 24, cy - 26, cx - 16, cy - 14], fill=(200, 30, 30, 255))
    draw.ellipse([cx + 16, cy - 26, cx + 24, cy - 14], fill=(200, 30, 30, 255))

    # Mouth
    draw.ellipse([cx - 24, cy + 6, cx + 24, cy + 22], fill=(60, 20, 60, 255))
    # Teeth
    for tx in range(cx - 16, cx + 20, 8):
        draw.rectangle([tx, cy + 8, tx + 5, cy + 16], fill=(180, 180, 180, 255))

    # Horns
    draw.polygon([(cx - 50, cy - 40), (cx - 35, cy - 70), (cx - 30, cy - 40)],
                 fill=(80, 30, 100, 255))
    draw.polygon([(cx + 50, cy - 40), (cx + 35, cy - 70), (cx + 30, cy - 40)],
                 fill=(80, 30, 100, 255))

    # Arms
    draw.ellipse([cx - 90, cy - 10, cx - 60, cy + 20], fill=(100, 50, 120, 255))
    draw.ellipse([cx + 60, cy - 10, cx + 90, cy + 20], fill=(100, 50, 120, 255))

    # Glow aura
    for r in range(95, 85, -1):
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], outline=(80, 40, 100, 30 + (95-r)*3), width=2)

    return img.resize((128, 128), Image.LANCZOS)


# ================================================================
# HIGH QUALITY TILES (64x64)
# ================================================================

def make_tile_wall_aa():
    """Stone wall tile with brick pattern at 64x64 using 2x rendering."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Base stone color gradient
    for y in range(S):
        t = y / S
        r = int(70 + 30 * t)
        g = int(62 + 25 * t)
        b = int(50 + 20 * t)
        draw.line([(0, y), (S, y)], fill=(r, g, b))

    # Brick pattern
    brick_h = 30
    brick_w = 60
    for by in range(0, S, brick_h + 2):
        offset = (brick_w // 2) if ((by // (brick_h + 2)) % 2 == 1) else 0
        for bx in range(-offset, S, brick_w + 2):
            if bx + brick_w < 0:
                continue
            # Mortar (darker border)
            draw.rectangle([bx, by, bx + brick_w, by + brick_h], fill=(45, 38, 30, 255))
            # Stone with slight noise variation
            stone_r = 75 + (bx * 7 + by * 13) % 20
            stone_g = 65 + (bx * 5 + by * 11) % 18
            stone_b = 50 + (bx * 3 + by * 7) % 15
            draw.rectangle([bx + 2, by + 2, bx + brick_w - 2, by + brick_h - 2],
                          fill=(stone_r, stone_g, stone_b))

    return img.resize((64, 64), Image.LANCZOS)


def make_tile_floor_aa():
    """Cobblestone floor tile."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    for y in range(S):
        t = y / S
        noise = ((y * 11) % 15) - 7
        r = int(100 + 20 * t + noise)
        g = int(85 + 15 * t + noise)
        b = int(60 + 10 * t + noise)
        draw.line([(0, y), (S, y)], fill=(r, g, b))

    # Cobblestone circles
    for cy in range(16, S, 32):
        for cx in range(16, S, 32):
            r = 12 + ((cx + cy) % 4)
            c_r = 90 + (cx * 3 + cy * 7) % 20
            c_g = 75 + (cx * 3 + cy * 7) % 18
            c_b = 50 + (cx * 3 + cy * 7) % 15
            draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(c_r, c_g, c_b))

    return img.resize((64, 64), Image.LANCZOS)


def make_tile_door_aa():
    """Wooden door tile with planks and handle."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Wood gradient
    for y in range(S):
        wood = 80 + (y % 12) * 3
        r = int(130 + wood * 0.5)
        g = int(80 + wood * 0.3)
        b = int(40 + wood * 0.2)
        draw.line([(0, y), (S, y)], fill=(r, g, b))

    # Vertical plank lines
    for px in range(10, S, 24):
        draw.line([(px, 0), (px, S)], fill=(55, 35, 15, 200), width=3)

    # Door handle
    draw.ellipse([S - 30, S//2 - 10, S - 18, S//2 + 2], fill=(180, 160, 60, 255))
    draw.ellipse([S - 28, S//2 - 8, S - 20, S//2], fill=(200, 180, 80, 255))

    return img.resize((64, 64), Image.LANCZOS)


def make_tile_goal_aa():
    """Glowing portal goal tile."""
    S = 128
    img = Image.new('RGBA', (S, S), (10, 10, 30, 255))
    draw = ImageDraw.Draw(img)

    cx, cy = S//2, S//2

    # Outer glow layers
    for r in range(55, 0, -2):
        t = r / 55.0
        r_col = int(60 + 195 * (1 - t))
        g_col = int(20 + 60 * (1 - t))
        b_col = int(100 + 155 * (1 - t))
        alpha = int(200 * (1 - t))
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(r_col, g_col, b_col, alpha))

    # Inner bright core
    draw.ellipse([cx - 12, cy - 12, cx + 12, cy + 12], fill=(200, 180, 220, 255))

    # Swirl lines
    for angle in range(0, 360, 60):
        rad = math.radians(angle)
        r1, r2 = 20, 40
        x1 = cx + int(r1 * math.cos(rad))
        y1 = cy + int(r1 * math.sin(rad))
        x2 = cx + int(r2 * math.cos(rad))
        y2 = cy + int(r2 * math.sin(rad))
        draw.line([(x1, y1), (x2, y2)], fill=(255, 220, 255, 100), width=2)

    return img.resize((64, 64), Image.LANCZOS)


def make_tile_spike_aa():
    """Spike trap tile with metal points."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Base
    draw.rectangle([0, S-20, S, S], fill=(55, 50, 45, 255))

    # Spikes
    for i in range(0, 5):
        spike_x = i * (S // 5) + (S // 10)
        for h in range(1, 36):
            width = max(1, 12 - h // 3)
            brightness = 60 + h * 4
            y_top = S - 20 - h
            draw.polygon([
                (spike_x - width, S - 20),
                (spike_x, y_top),
                (spike_x + width, S - 20)
            ], fill=(brightness, brightness - 10, brightness - 20))

    return img.resize((64, 64), Image.LANCZOS)


def make_tile_platform_aa():
    """Floating platform tile with grass top."""
    S = 128
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Dirt body
    for y in range(S//4, S):
        t = (y - S//4) / (S - S//4)
        noise = ((y * 7) % 12) - 6
        r = int(85 + 25 * t + noise)
        g = int(60 + 20 * t + noise)
        b = int(35 + 15 * t + noise)
        draw.line([(0, y), (S, y)], fill=(r, g, b))

    # Grass top
    for y in range(S//4 - 8, S//4 + 10):
        shade = 30 + ((y * 3) % 15)
        r = int(20 + shade * 0.3)
        g = int(90 + shade)
        b = int(15 + shade * 0.2)
        draw.line([(0, y), (S, y)], fill=(r, g, b))

    # Grass tufts
    for tx in range(8, S, 16):
        tuft_h = 6 + (tx * 3) % 6
        draw.line([(tx, S//4 - 8), (tx - 3, S//4 - 8 - tuft_h)],
                  fill=(30, 140, 30, 200), width=2)
        draw.line([(tx, S//4 - 8), (tx + 3, S//4 - 8 - tuft_h)],
                  fill=(25, 130, 25, 200), width=2)

    return img.resize((64, 64), Image.LANCZOS)


# ================================================================
# ITEMS (32x32 - render at 64x64 then downscale)
# ================================================================

def make_item_coin_aa():
    """Gold coin with dollar sign, anti-aliased."""
    S = 64
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    cx, cy = S//2, S//2

    # Coin body with gradient
    for r in range(22, 0, -1):
        t = r / 22.0
        r_col = int(200 + 55 * t)
        g_col = int(160 + 40 * t)
        b_col = int(40 + 30 * t)
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(r_col, g_col, b_col))

    # Inner ring
    draw.ellipse([cx - 18, cy - 18, cx + 18, cy + 18], outline=(220, 190, 60, 200), width=2)

    # Dollar sign
    draw.rectangle([cx - 3, cy - 10, cx + 3, cy - 8], fill=(200, 180, 60, 255))
    draw.line([(cx - 5, cy - 8), (cx + 5, cy - 8)], fill=(200, 180, 60, 255), width=2)
    draw.line([(cx - 5, cy + 8), (cx + 5, cy + 8)], fill=(200, 180, 60, 255), width=2)
    draw.line([(cx, cy - 12), (cx, cy + 12)], fill=(200, 180, 60, 255), width=3)

    # Shine highlight
    draw.ellipse([cx - 12, cy - 12, cx - 5, cy - 5], fill=(255, 240, 150, 80))

    return img.resize((32, 32), Image.LANCZOS)


def make_item_heart_aa():
    """Heart pickup with smooth shape."""
    S = 64
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    cx, cy = S//2, S//2 + 2

    # Heart shape using ellipses and triangle
    # Two circles at top
    draw.ellipse([cx - 18, cy - 14, cx - 2, cy + 6], fill=(220, 50, 50, 255))
    draw.ellipse([cx + 2, cy - 14, cx + 18, cy + 6], fill=(220, 50, 50, 255))
    # Triangle bottom
    draw.polygon([(cx - 20, cy + 2), (cx + 20, cy + 2), (cx, cy + 24)],
                 fill=(220, 50, 50, 255))

    # Highlight
    draw.ellipse([cx - 10, cy - 8, cx - 4, cy], fill=(255, 120, 120, 80))
    draw.ellipse([cx + 4, cy - 8, cx + 10, cy], fill=(255, 120, 120, 80))

    return img.resize((32, 32), Image.LANCZOS)


def make_item_star_aa():
    """Star powerup with glow."""
    S = 64
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    cx, cy = S//2, S//2

    # Outer glow
    for r in [20, 18, 16]:
        alpha = 40 - r
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(255, 230, 50, alpha))

    # Star shape (5-pointed)
    points = []
    for i in range(10):
        angle = -math.pi / 2 + i * math.pi / 5
        r = 18 if i % 2 == 0 else 8
        px = cx + int(r * math.cos(angle))
        py = cy + int(r * math.sin(angle))
        points.append((px, py))
    draw.polygon(points, fill=(255, 220, 40, 255))

    # Inner bright core
    draw.ellipse([cx - 4, cy - 4, cx + 4, cy + 4], fill=(255, 255, 200, 255))

    return img.resize((32, 32), Image.LANCZOS)


# ================================================================
# IMPROVED BACKGROUNDS (800x600)
# ================================================================

def make_bg_forest_aa():
    """Improved forest background with depth layers."""
    W, H = 1600, 1200  # 2x for AA
    img = Image.new('RGBA', (W, H), (0, 0, 0, 255))
    draw = ImageDraw.Draw(img)

    # Sky gradient (dawn)
    for y in range(int(H * 0.7)):
        t = y / (H * 0.7)
        r = int(50 + 80 * t)
        g = int(80 + 60 * t)
        b = int(140 + 40 * t)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    # Sun
    sun_x, sun_y = W//2, int(H * 0.15)
    for r in range(120, 0, -1):
        alpha = max(0, min(255, 60 - r // 2))
        draw.ellipse([sun_x - r, sun_y - r, sun_x + r, sun_y + r],
                     fill=(255, 220, 100, alpha))
    draw.ellipse([sun_x - 30, sun_y - 30, sun_x + 30, sun_y + 30],
                 fill=(255, 240, 180, 255))

    # Far mountains (distant)
    for mx, mh in [(W*0.15, H*0.25), (W*0.40, H*0.30), (W*0.65, H*0.28), (W*0.85, H*0.22)]:
        pts = [(0, int(H*0.7))]
        for x in range(0, W, 10):
            rel_x = (x - (mx - W*0.15)) / (W*0.3)
            height = mh * max(0, math.cos(rel_x * math.pi / 2)) if abs(rel_x) < 1 else 0
            pts.append((x, int(H*0.7 - height)))
        pts.append((W, int(H*0.7)))
        draw.polygon(pts, fill=(70, 90, 60, 180))

    # Tree line (mid-ground)
    for tx in range(0, W, 40):
        tree_h = int(H * 0.3 + (tx * 17) % int(H * 0.15))
        # Trunk
        trunk_w = 6 + (tx * 13) % 4
        draw.rectangle([tx - trunk_w, int(H*0.7) - tree_h, tx + trunk_w, int(H*0.7)],
                       fill=(55, 40, 25))
        # Foliage (layered circles)
        foliage_cy = int(H*0.7) - tree_h + 20
        for r in range(50, 15, -5):
            alpha = 200 - r * 2
            f_x = tx + (r * 3) % 20 - 10
            draw.ellipse([f_x - r, foliage_cy - r, f_x + r, foliage_cy + r],
                         fill=(20 + r, 70 + r // 2, 15, alpha))

    # Ground
    for y in range(int(H*0.68), H):
        t = (y - H*0.68) / (H*0.32)
        r = int(40 + 60 * t)
        g = int(90 + 70 * t)
        b = int(25 + 20 * t)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    return img.resize((800, 600), Image.LANCZOS)


def make_bg_cave_aa():
    """Improved cave background with crystal glow."""
    W, H = 1600, 1200
    img = Image.new('RGBA', (W, H), (0, 0, 0, 255))
    draw = ImageDraw.Draw(img)

    # Cave walls gradient
    for y in range(H):
        t = y / H
        noise = ((y * 13) % 20) - 10
        r = int(25 + 15 * t + noise)
        g = int(20 + 12 * t + noise)
        b = int(30 + 12 * t + noise)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    # Stalactites
    for sx in range(0, W, 60):
        height = int(60 + (sx * 7) % 150)
        for y in range(height):
            width = max(4, 14 - y // 15)
            draw.polygon([
                (sx - width, 0),
                (sx, y),
                (sx + width, 0)
            ], fill=(50, 42, 38))

    # Glowing crystals
    for cx, c_cy in [(W*0.15, H*0.35), (W*0.40, H*0.25), (W*0.65, H*0.30), (W*0.85, H*0.35)]:
        # Glow aura
        for r in range(60, 10, -5):
            alpha = max(0, 30 - r // 2)
            draw.ellipse([cx - r, c_cy - r, cx + r, c_cy + r],
                        fill=(80, 180, 255, alpha))
        # Crystal body
        draw.polygon([(cx, c_cy - 25), (cx - 10, c_cy + 5), (cx + 10, c_cy + 5)],
                     fill=(100, 200, 255, 200))
        draw.polygon([(cx - 3, c_cy - 18), (cx - 12, c_cy + 8), (cx + 4, c_cy + 8)],
                     fill=(60, 150, 220, 150))

    return img.resize((800, 600), Image.LANCZOS)


def make_bg_mountain_aa():
    """Improved mountain background with snow caps and depth."""
    W, H = 1600, 1200
    img = Image.new('RGBA', (W, H), (0, 0, 0, 255))
    draw = ImageDraw.Draw(img)

    # Sky gradient (dusk)
    for y in range(int(H * 0.65)):
        t = y / (H * 0.65)
        r = int(60 + 120 * t)
        g = int(80 + 80 * t)
        b = int(130 + 30 * t)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    # Clouds
    for c_x, c_w, c_y in [(W*0.1, 200, H*0.08), (W*0.5, 280, H*0.12), (W*0.75, 180, H*0.06)]:
        cx_i, cw_i = int(c_x), int(c_w)
        for bx in range(cx_i, cx_i + cw_i, 30):
            br = 15 + (bx * 7) % 15
            draw.ellipse([bx - br, int(c_y) - br//2, bx + br, int(c_y) + br//2],
                        fill=(200, 180, 160, 100))

    # Far mountains
    for mx, mh in [(W*0.12, H*0.30), (W*0.30, H*0.42), (W*0.55, H*0.38), (W*0.78, H*0.32)]:
        pts = [(0, int(H*0.65))]
        for x in range(0, W, 5):
            rel_x = (x - mx) / (W*0.25)
            if abs(rel_x) < 1:
                height = mh * math.cos(rel_x * math.pi / 2) ** 2
            else:
                height = 0
            pts.append((x, int(H*0.65 - height)))
        pts.append((W, int(H*0.65)))
        draw.polygon(pts, fill=(60 + int(mh*0.3), 55 + int(mh*0.2), 70))

        # Snow caps
        if mh > H*0.3:
            for x in range(0, W, 5):
                rel_x = (x - mx) / (W*0.25)
                if abs(rel_x) < 0.2:
                    height = mh * math.cos(rel_x * math.pi / 2) ** 2
                    if height > mh * 0.8:
                        draw.line([(x, int(H*0.65 - height)),
                                  (x, int(H*0.65 - height * 0.85))],
                                 fill=(220, 220, 235), width=3)

    # Mid-ground hills
    pts = [(0, int(H*0.65))]
    for x in range(0, W, 10):
        h = 40 + 20 * math.sin(x * 0.01) + 15 * math.sin(x * 0.025)
        pts.append((x, int(H*0.65 - h)))
    pts.append((W, int(H*0.65)))
    draw.polygon(pts, fill=(50, 70, 45))

    # Ground
    for y in range(int(H*0.63), H):
        t = (y - H*0.63) / (H*0.37)
        r = int(45 + 65 * t)
        g = int(65 + 55 * t)
        b = int(35 + 30 * t)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    return img.resize((800, 600), Image.LANCZOS)


def make_bg_castle_aa():
    """Improved castle background with dark atmosphere."""
    W, H = 1600, 1200
    img = Image.new('RGBA', (W, H), (0, 0, 0, 255))
    draw = ImageDraw.Draw(img)

    # Dark sky gradient
    for y in range(H):
        t = y / H
        r = int(18 + 30 * t)
        g = int(14 + 25 * t)
        b = int(35 + 30 * t)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    # Moon
    moon_x, moon_y = W*0.8, H*0.12
    for r in range(80, 0, -1):
        alpha = max(0, 40 - r)
        draw.ellipse([moon_x - r, moon_y - r, moon_x + r, moon_y + r],
                     fill=(200, 200, 220, alpha))
    draw.ellipse([moon_x - 30, moon_y - 30, moon_x + 30, moon_y + 30],
                 fill=(240, 240, 255, 255))

    # Castle silhouette (main keep)
    # Base
    draw.rectangle([int(W*0.35), int(H*0.30), int(W*0.65), int(H*0.70)],
                   fill=(40, 35, 48))

    # Towers
    towers = [(W*0.25, H*0.28), (W*0.35, H*0.22), (W*0.65, H*0.24), (W*0.75, H*0.30)]
    for tx, th in towers:
        tw = 30
        draw.rectangle([int(tx - tw), int(H*0.70 - th), int(tx + tw), int(H*0.70)],
                       fill=(35, 30, 42))

    # Battlements
    for bx in range(int(W*0.34), int(W*0.66), 20):
        draw.rectangle([bx, int(H*0.24), bx + 12, int(H*0.30)],
                       fill=(45, 38, 50))

    # Glowing windows
    for wx, wy in [(W*0.39, H*0.36), (W*0.45, H*0.36), (W*0.55, H*0.36), (W*0.61, H*0.36),
                   (W*0.42, H*0.46), (W*0.50, H*0.46), (W*0.58, H*0.46)]:
        # Window glow
        for r in range(16, 0, -1):
            draw.ellipse([wx - r, wy - r, wx + r, wy + r], fill=(200, 180, 60, 40 - r))
        # Window frame
        draw.rectangle([int(wx - 6), int(wy - 8), int(wx + 6), int(wy + 8)],
                       fill=(180, 160, 50))

    # Ground
    for y in range(int(H*0.68), H):
        t = (y - H*0.68) / (H*0.32)
        r = int(35 + 25 * t)
        g = int(42 + 20 * t)
        b = int(30 + 18 * t)
        draw.line([(0, y), (W, y)], fill=(r, g, b))

    return img.resize((800, 600), Image.LANCZOS)


# ================================================================
# SVG-BASED SPRITES
# ================================================================

def make_powerup_lamp():
    """Convert lamp.svg to a glowing powerup icon at 32x32."""
    svg_path = os.path.join(SVG_DIR, "lamp.svg")
    if os.path.exists(svg_path):
        img = svg_to_pixels(svg_path, target_size=64)
    else:
        # Fallback: draw a lamp
        img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.ellipse([16, 8, 48, 40], fill=(255, 215, 0, 255))
        draw.rectangle([28, 40, 36, 52], fill=(140, 140, 140, 255))
        draw.rectangle([22, 52, 42, 56], fill=(100, 100, 100, 255))
    return img.resize((32, 32), Image.LANCZOS)


def make_powerup_shield():
    """Convert gateway.svg to a shield/barrier powerup."""
    svg_path = os.path.join(SVG_DIR, "gateway.svg")
    if os.path.exists(svg_path):
        img = svg_to_pixels(svg_path, target_size=64)
    else:
        img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.rectangle([8, 24, 56, 52], fill=(30, 60, 120, 255))
        draw.rounded_rectangle([8, 24, 56, 52], radius=6, fill=(30, 60, 120, 255))
        draw.ellipse([20, 32, 28, 40], fill=(0, 255, 136, 255))
        draw.rectangle([28, 36, 48, 40], fill=(0, 128, 255, 255))
    return img.resize((32, 32), Image.LANCZOS)


def make_powerup_radar():
    """Convert sensor.svg to a radar/detection icon."""
    svg_path = os.path.join(SVG_DIR, "sensor.svg")
    if os.path.exists(svg_path):
        img = svg_to_pixels(svg_path, target_size=64)
    else:
        img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.ellipse([24, 24, 40, 40], fill=(0, 212, 255, 255))
        draw.arc([16, 16, 48, 48], 0, 360, fill=(0, 212, 255, 200), width=2)
        draw.arc([8, 8, 56, 56], 0, 360, fill=(0, 212, 255, 150), width=2)
    return img.resize((32, 32), Image.LANCZOS)


def make_ui_traffic():
    """Convert traffic.svg to a UI indicator icon."""
    svg_path = os.path.join(SVG_DIR, "traffic.svg")
    if os.path.exists(svg_path):
        img = svg_to_pixels(svg_path, target_size=48)
    else:
        img = Image.new('RGBA', (48, 48), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.rectangle([14, 4, 34, 44], fill=(50, 50, 50, 255))
        draw.ellipse([18, 8, 30, 20], fill=(255, 51, 68, 255))
        draw.ellipse([18, 20, 30, 32], fill=(255, 170, 0, 255))
        draw.ellipse([18, 32, 30, 44], fill=(0, 255, 136, 255))
    return img.resize((24, 24), Image.LANCZOS)


def make_deco_camera():
    """Convert camera.svg to a decorative element."""
    svg_path = os.path.join(SVG_DIR, "camera.svg")
    if os.path.exists(svg_path):
        img = svg_to_pixels(svg_path, target_size=64)
    else:
        img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.rectangle([8, 18, 56, 48], fill=(140, 140, 140, 255))
        draw.ellipse([20, 28, 44, 44], fill=(50, 50, 50, 255))
        draw.rectangle([48, 26, 62, 36], fill=(100, 100, 100, 255))
    return img.resize((32, 32), Image.LANCZOS)


# ================================================================
# GENERATE ALL IMPROVED SPRITES
# ================================================================

def main():
    print("🎮 RetroMania Arcade — High Quality Sprite Improver\n")
    print(f"Output: {ASSETS_DIR}\n")
    print(f"SVG source: {SVG_DIR}\n" if os.path.exists(SVG_DIR) else "(no SVG directory found)\n")

    total = 0

    print("--- NINJA SPRITES (48x48, anti-aliased) ---")
    save_png("ninja_idle.png", make_ninja_idle_aa())
    save_png("ninja_walk1.png", make_ninja_walk1_aa())
    save_png("ninja_walk2.png", make_ninja_walk2_aa())
    save_png("ninja_jump.png", make_ninja_jump_aa())
    save_png("ninja_attack.png", make_ninja_attack_aa())
    total += 5

    print("\n--- ENEMY SPRITES (anti-aliased) ---")
    save_png("enemy_bouncer.png", make_enemy_bouncer_aa())
    save_png("enemy_chaser.png", make_enemy_chaser_aa())
    save_png("enemy_boss.png", make_enemy_boss_aa())
    total += 3

    print("\n--- MAZE TILES (64x64, anti-aliased) ---")
    save_png("tile_wall.png", make_tile_wall_aa())
    save_png("tile_floor.png", make_tile_floor_aa())
    save_png("tile_door.png", make_tile_door_aa())
    save_png("tile_goal.png", make_tile_goal_aa())
    save_png("tile_spike.png", make_tile_spike_aa())
    save_png("tile_platform.png", make_tile_platform_aa())
    total += 6

    print("\n--- SCENE BACKGROUNDS (800x600, improved) ---")
    save_png("bg_forest.png", make_bg_forest_aa())
    save_png("bg_cave.png", make_bg_cave_aa())
    save_png("bg_mountain.png", make_bg_mountain_aa())
    save_png("bg_castle.png", make_bg_castle_aa())
    total += 4

    print("\n--- ITEMS (32x32, anti-aliased) ---")
    save_png("item_coin.png", make_item_coin_aa())
    save_png("item_heart.png", make_item_heart_aa())
    save_png("item_star.png", make_item_star_aa())
    total += 3

    print("\n--- SVG-DERIVED SPRITES ---")
    # Check for SVG files
    svg_files = [f for f in os.listdir(SVG_DIR) if f.endswith('.svg')] if os.path.exists(SVG_DIR) else []
    if svg_files:
        print(f"  Found SVGs: {', '.join(svg_files)}")
        save_png("powerup_lamp.png", make_powerup_lamp())
        save_png("powerup_shield.png", make_powerup_shield())
        save_png("powerup_radar.png", make_powerup_radar())
        save_png("ui_traffic.png", make_ui_traffic())
        save_png("deco_camera.png", make_deco_camera())
        total += 5
    else:
        print("  No SVG files found, using fallback drawing")
        save_png("powerup_lamp.png", make_powerup_lamp())
        save_png("powerup_shield.png", make_powerup_shield())
        save_png("powerup_radar.png", make_powerup_radar())
        save_png("ui_traffic.png", make_ui_traffic())
        save_png("deco_camera.png", make_deco_camera())
        total += 5

    print(f"\n{'='*50}")
    print(f"✅ Generated {total} improved PNG sprites in assets/")
    print(f"{'='*50}")

if __name__ == "__main__":
    main()
