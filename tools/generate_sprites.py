#!/usr/bin/env python3
"""
RetroMania Arcade — Sprite & Asset Generator
Generates all PNG assets programmatically using Pillow.
Run from project root: python3 tools/generate_sprites.py
"""

from PIL import Image
import os

ASSETS_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "assets")
os.makedirs(ASSETS_DIR, exist_ok=True)

SA = 64  # Standard sprite size (64x64)

def save_png(name, img):
    path = os.path.join(ASSETS_DIR, name)
    img.save(path, "PNG")
    print(f"  ✓ {name}  ({img.size[0]}x{img.size[1]})")

# ================================================================
# 1. NINJA SPRITES (64x64 each, different poses)
# ================================================================
def rgb(r, g, b, a=255):
    return (r, g, b, a)

def make_ninja_idle():
    """Ninja standing — front facing"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Head (bandana mask)
    for y in range(8, 20):
        for x in range(16, 48):
            # Face skin
            if y >= 12 and y <= 18 and x >= 18 and x <= 45:
                if x >= 24 and x <= 39 and y == 12:  # bandana line
                    p[x,y] = rgb(180, 40, 40)
                elif x >= 22 and x <= 32 and y >= 14 and y <= 17:  # eyes area
                    if y == 15: p[x,y] = rgb(255,255,255)
                    elif y == 16 and (x < 26 or x > 30): p[x,y] = rgb(255,200,150)
                    else: p[x,y] = rgb(255,200,150)
                else:
                    p[x,y] = rgb(255,200,150)
            elif y >= 8 and y <= 11 and x >= 20 and x <= 43:  # hair/bandana top
                p[x,y] = rgb(40, 40, 40)
            elif y == 19 and x >= 18 and x <= 45:  # chin
                p[x,y] = rgb(255,200,150)
    # Eyes
    for ey in range(14, 17):
        for ex in [24, 25, 33, 34]:
            if 14 <= ey <= 16:
                p[ex, ey] = rgb(40, 40, 40)
    # Body (gi)
    for y in range(20, 42):
        for x in range(18, 46):
            if y < 30:  # upper body
                p[x,y] = rgb(60, 60, 80)
            else:  # lower body
                p[x,y] = rgb(50, 50, 70)
    # Belt
    for y in [29,30]:
        for x in range(18, 46):
            p[x,y] = rgb(180, 40, 40)
    # Arms
    for y in range(20, 38):
        for x in [14,15,16,17,47,48,49]:
            if x < 20 and y < 35:
                p[x,y] = rgb(255,200,150) if y < 30 else rgb(60,60,80)
            elif x > 44 and y < 35:
                p[x,y] = rgb(255,200,150) if y < 30 else rgb(60,60,80)
    # Hands
    for y in range(34, 38):
        for x in [14,15,16,17,47,48,49]:
            p[x,y] = rgb(255,200,150)
    # Legs
    for y in range(38, 56):
        for x in [20,21,22,23,24,25, 32,33,34,35,36,37]:
            p[x,y] = rgb(50, 50, 70)
    # Feet
    for y in range(54, 58):
        for x in [18,19,20,21,22,23,24,25, 32,33,34,35,36,37,38,39]:
            p[x,y] = rgb(40, 40, 50)
    # Sword (on back, visible above right shoulder)
    for y in range(6, 22):
        for x in [46,47]:
            if y < 16: p[x,y] = rgb(180, 180, 190)
            elif y < 20: p[x,y] = rgb(120, 80, 40)
    return img

def make_ninja_walk1():
    """Ninja walking — right leg forward"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Head (same as idle)
    for y in range(8, 20):
        for x in range(16, 48):
            if y >= 12 and y <= 18 and x >= 18 and x <= 45:
                if x >= 24 and x <= 39 and y == 12:
                    p[x,y] = rgb(180, 40, 40)
                elif x >= 22 and x <= 32 and y >= 14 and y <= 17:
                    if y == 15: p[x,y] = rgb(255,255,255)
                    elif y == 16 and (x < 26 or x > 30): p[x,y] = rgb(255,200,150)
                    else: p[x,y] = rgb(255,200,150)
                else:
                    p[x,y] = rgb(255,200,150)
            elif y >= 8 and y <= 11 and x >= 20 and x <= 43:
                p[x,y] = rgb(40, 40, 40)
            elif y == 19 and x >= 18 and x <= 45:
                p[x,y] = rgb(255,200,150)
    for ey in range(14, 17):
        for ex in [24, 25, 33, 34]:
            p[ex, ey] = rgb(40, 40, 40)
    # Body — slightly tilted
    for y in range(20, 40):
        for x in range(18, 46):
            if y < 30:
                p[x,y] = rgb(60, 60, 80)
            else:
                p[x,y] = rgb(50, 50, 70)
    for y in [29,30]:
        for x in range(18, 46):
            p[x,y] = rgb(180, 40, 40)
    # Arms swinging
    for y in range(20, 38):
        for x in [12,13,14,15,16,48,49,50,51]:
            if x < 18 and y > 24 and y < 36:
                p[x,y] = rgb(255,200,150) if y < 32 else rgb(60,60,80)
            elif x > 46 and y > 20 and y < 32:
                p[x,y] = rgb(255,200,150) if y < 28 else rgb(60,60,80)
    # Hands
    for y in [28,29,30,31]:
        for x in [12,13,14,15]:
            p[x,y] = rgb(255,200,150)
    for y in [34,35,36,37]:
        for x in [48,49,50,51]:
            p[x,y] = rgb(255,200,150)
    # Legs — walking pose (right forward, left back)
    for y in range(36, 56):
        for x in [24,25,26,27,28,29]:
            p[x,y] = rgb(50, 50, 70)
    for y in range(36, 56):
        for x in [18,19,20,21]:
            if y < 50:
                p[x,y] = rgb(50, 50, 70)
    # Feet
    for y in range(54, 58):
        for x in [24,25,26,27,28,29,30,31]:
            p[x,y] = rgb(40, 40, 50)
    for y in range(50, 54):
        for x in [16,17,18,19,20]:
            p[x,y] = rgb(40, 40, 50)
    return img

def make_ninja_walk2():
    """Ninja walking — left leg forward"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Same head as idle
    for y in range(8, 20):
        for x in range(16, 48):
            if y >= 12 and y <= 18 and x >= 18 and x <= 45:
                if x >= 24 and x <= 39 and y == 12:
                    p[x,y] = rgb(180, 40, 40)
                elif x >= 22 and x <= 32 and y >= 14 and y <= 17:
                    if y == 15: p[x,y] = rgb(255,255,255)
                    elif y == 16 and (x < 26 or x > 30): p[x,y] = rgb(255,200,150)
                    else: p[x,y] = rgb(255,200,150)
                else:
                    p[x,y] = rgb(255,200,150)
            elif y >= 8 and y <= 11 and x >= 20 and x <= 43:
                p[x,y] = rgb(40, 40, 40)
            elif y == 19 and x >= 18 and x <= 45:
                p[x,y] = rgb(255,200,150)
    for ey in range(14, 17):
        for ex in [24, 25, 33, 34]:
            p[ex, ey] = rgb(40, 40, 40)
    # Body
    for y in range(20, 40):
        for x in range(18, 46):
            if y < 30:
                p[x,y] = rgb(60, 60, 80)
            else:
                p[x,y] = rgb(50, 50, 70)
    for y in [29,30]:
        for x in range(18, 46):
            p[x,y] = rgb(180, 40, 40)
    # Arms (opposite swing)
    for y in range(20, 38):
        for x in [12,13,14,15,16,48,49,50,51]:
            if x < 18 and y > 20 and y < 32:
                p[x,y] = rgb(255,200,150) if y < 28 else rgb(60,60,80)
            elif x > 46 and y > 24 and y < 36:
                p[x,y] = rgb(255,200,150) if y < 32 else rgb(60,60,80)
    for y in [34,35,36,37]:
        for x in [12,13,14,15]:
            p[x,y] = rgb(255,200,150)
    for y in [28,29,30,31]:
        for x in [48,49,50,51]:
            p[x,y] = rgb(255,200,150)
    # Legs — walking pose (left forward, right back)
    for y in range(36, 56):
        for x in [34,35,36,37,38,39]:
            p[x,y] = rgb(50, 50, 70)
    for y in range(36, 56):
        for x in [28,29,30,31]:
            if y < 50:
                p[x,y] = rgb(50, 50, 70)
    for y in range(54, 58):
        for x in [30,31,32,33,34,35,36,37]:
            p[x,y] = rgb(40, 40, 50)
    for y in range(50, 54):
        for x in [26,27,28,29,30]:
            p[x,y] = rgb(40, 40, 50)
    return img

def make_ninja_jump():
    """Ninja jumping — arms up, legs tucked"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Head (slightly higher)
    for y in range(4, 16):
        for x in range(16, 48):
            if y >= 8 and y <= 14 and x >= 18 and x <= 45:
                if x >= 24 and x <= 39 and y == 8:
                    p[x,y] = rgb(180, 40, 40)
                elif x >= 22 and x <= 32 and y >= 10 and y <= 13:
                    if y == 11: p[x,y] = rgb(255,255,255)
                    elif y == 12 and (x < 26 or x > 30): p[x,y] = rgb(255,200,150)
                    else: p[x,y] = rgb(255,200,150)
                else:
                    p[x,y] = rgb(255,200,150)
            elif y >= 4 and y <= 7 and x >= 20 and x <= 43:
                p[x,y] = rgb(40, 40, 40)
            elif y == 15 and x >= 18 and x <= 45:
                p[x,y] = rgb(255,200,150)
    for ey in range(10, 13):
        for ex in [24, 25, 33, 34]:
            p[ex, ey] = rgb(40, 40, 40)
    # Body
    for y in range(16, 36):
        for x in range(18, 46):
            p[x,y] = rgb(60, 60, 80)
    for y in [25,26]:
        for x in range(18, 46):
            p[x,y] = rgb(180, 40, 40)
    # Arms up
    for y in range(5, 18):
        for x in [46,47,48,49,50]:
            if y < 14:
                p[x,y] = rgb(60,60,80)
    for y in range(5, 18):
        for x in [12,13,14,15,16]:
            if y < 14:
                p[x,y] = rgb(60,60,80)
    # Legs tucked
    for y in range(34, 44):
        for x in [20,21,22,23,24,25, 32,33,34,35,36,37]:
            p[x,y] = rgb(50, 50, 70)
    return img

def make_ninja_attack():
    """Ninja attacking — sword slash pose"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Head
    for y in range(8, 20):
        for x in range(16, 48):
            if y >= 12 and y <= 18 and x >= 18 and x <= 45:
                if x >= 24 and x <= 39 and y == 12:
                    p[x,y] = rgb(180, 40, 40)
                elif x >= 22 and x <= 32 and y >= 14 and y <= 17:
                    if y == 15: p[x,y] = rgb(255,255,255)
                    elif y == 16 and (x < 26 or x > 30): p[x,y] = rgb(255,200,150)
                    else: p[x,y] = rgb(255,200,150)
                else:
                    p[x,y] = rgb(255,200,150)
            elif y >= 8 and y <= 11 and x >= 20 and x <= 43:
                p[x,y] = rgb(40, 40, 40)
            elif y == 19 and x >= 18 and x <= 45:
                p[x,y] = rgb(255,200,150)
    for ey in range(14, 17):
        for ex in [24, 25, 33, 34]:
            p[ex, ey] = rgb(40, 40, 40)
    # Body twisted
    for y in range(20, 38):
        for x in range(16, 44):
            if y < 30:
                p[x,y] = rgb(60, 60, 80)
            else:
                p[x,y] = rgb(50, 50, 70)
    for y in [27,28]:
        for x in range(16, 44):
            p[x,y] = rgb(180, 40, 40)
    # Sword arm extended right
    for y in range(20, 26):
        for x in range(44, 62):
            p[x,y] = rgb(60,60,80)
    # Sword blade
    for y in range(18, 26):
        for x in range(56, 64):
            p[x,y] = rgb(200, 200, 210)
    for y in range(18, 26):
        for x in [54,55]:
            p[x,y] = rgb(150, 120, 60)
    # Legs
    for y in range(36, 56):
        for x in [22,23,24,25,26,27, 34,35,36,37,38,39]:
            p[x,y] = rgb(50, 50, 70)
    for y in range(54, 58):
        for x in [20,21,22,23,24,25,26,27, 34,35,36,37,38,39,40,41]:
            p[x,y] = rgb(40, 40, 50)
    return img

# ================================================================
# 2. ENEMY SPRITES
# ================================================================
def make_enemy_bouncer():
    """Red slime bouncer 48x48"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Slime body
    for y in range(16, 54):
        for x in range(12, 52):
            dx, dy = x - 32, y - 32
            dist = (dx*dx + dy*dy) ** 0.5
            if dist < 20:
                # Round slime shape
                shade = 1.0 - (abs(y - 32) / 20.0)
                r = int(200 * (0.6 + 0.4 * shade))
                g = int(50 * (0.5 + 0.5 * shade))
                b = int(50 * (0.5 + 0.5 * shade))
                p[x,y] = rgb(r, g, b)
    # Eyes
    for ey in range(24, 30):
        for ex in [24, 25, 38, 39]:
            p[ex, ey] = rgb(255, 255, 200)
    for ey in range(25, 28):
        for ex in [25, 39]:
            p[ex, ey] = rgb(30, 30, 30)
    return img

def make_enemy_chaser():
    """Blue bat chaser 56x56"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Body
    for y in range(20, 44):
        for x in range(20, 44):
            dx, dy = x - 32, y - 32
            dist = (dx*dx + dy*dy) ** 0.5
            if dist < 14:
                shade = 1.0 - (dist / 14.0)
                r = int(80 * (0.5 + 0.5 * shade))
                g = int(80 * (0.5 + 0.5 * shade))
                b = int(200 * (0.5 + 0.5 * shade))
                p[x,y] = rgb(r, g, b)
    # Wings
    for y in range(16, 44):
        for x in range(8, 20):
            dx, dy = x - 14, y - 30
            if dx*dx + dy*dy < 56:
                p[x,y] = rgb(40 + (y-16)*2, 40 + (y-16), 120 + (y-16))
    for y in range(16, 44):
        for x in range(44, 56):
            dx, dy = x - 50, y - 30
            if dx*dx + dy*dy < 56:
                p[x,y] = rgb(40 + (y-16)*2, 40 + (y-16), 120 + (y-16))
    # Eyes (glowing)
    for ey in range(22, 30):
        for ex in [25, 26, 37, 38]:
            p[ex, ey] = rgb(255, 255, 100)
    for ey in range(24, 28):
        for ex in [26, 38]:
            p[ex, ey] = rgb(180, 50, 50)
    return img

def make_enemy_boss():
    """Big purple boss 128x128 (scaled down during render via w/h in entity)"""
    size = 128
    img = Image.new("RGBA", (size, size), (0,0,0,0))
    p = img.load()
    # Body
    cx, cy = size//2, size//2 + 8
    for y in range(10, size-10):
        for x in range(10, size-10):
            dx, dy = x - cx, y - cy
            dist = (dx*dx + dy*dy) ** 0.5
            if dist < 48:
                shade = 1.0 - (dist / 48.0)
                r = int(120 + 50 * shade)
                g = int(40 + 20 * shade)
                b = int(140 + 60 * shade)
                p[x,y] = rgb(r, g, b)
    # Eyes
    for ey in range(cy-18, cy-8):
        for ex in [cx-15, cx-14, cx+13, cx+14]:
            p[ex, ey] = rgb(255, 255, 100)
            p[ex-1, ey] = rgb(255, 255, 100)
            p[ex+1, ey] = rgb(255, 255, 100)
    for ey in range(cy-14, cy-10):
        for ex in [cx-14, cx+14]:
            p[ex, ey] = rgb(200, 30, 30)
            p[ex-1, ey] = rgb(200, 30, 30)
    # Mouth
    for y in range(cy+4, cy+12):
        for x in range(cx-14, cx+15):
            p[x,y] = rgb(60, 20, 60)
    for ey in range(cy+6, cy+10):
        for ex in range(cx-8, cx+9):
            if abs(ex - cx) % 4 < 2:
                p[ex, ey] = rgb(180, 50, 50)
    # Horns
    for y in range(12, size//2-6):
        for x in [cx-28, cx-27, cx+26, cx+27]:
            if y < cx-10:
                p[x, y] = rgb(80, 30, 100)
                p[x-1, y] = rgb(80, 30, 100) if x > cx else p[x+1,y]
    return img

# ================================================================
# 3. MAZE/LABYRINTH TILES (64x64)
# ================================================================
def make_tile_wall():
    """Stone wall tile"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    for y in range(SA):
        for x in range(SA):
            # Base stone color with variation
            base_r, base_g, base_b = 80, 72, 60
            noise = ((x * 7 + y * 13) % 20) - 10
            r = max(0, min(255, base_r + noise))
            g = max(0, min(255, base_g + noise))
            b = max(0, min(255, base_b + noise))
            p[x,y] = rgb(r, g, b)
    # Brick pattern
    for by in range(0, SA, 16):
        for bx in range(0, SA, 32):
            offset = 16 if (by // 16) % 2 else 0
            for y in range(by, min(by+15, SA)):
                for x in range(bx - offset, min(bx + 32 - offset, SA)):
                    if x >= 0:
                        # Mortar lines
                        if y == by or y == by+14 or x == bx - offset or x == bx + 31 - offset:
                            if x >= 0 and x < SA:
                                p[x,y] = rgb(50, 45, 38)
    return img

def make_tile_floor():
    """Floor tile (stone/dirt)"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    for y in range(SA):
        for x in range(SA):
            noise = ((x * 3 + y * 11) % 15) - 7
            r = max(0, min(255, 110 + noise))
            g = max(0, min(255, 95 + noise))
            b = max(0, min(255, 70 + noise))
            p[x,y] = rgb(r, g, b)
    return img

def make_tile_door():
    """Wooden door tile"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    for y in range(SA):
        for x in range(SA):
            wood = 90 + (y % 8) * 3 + ((x * 7) % 6)
            r = max(0, min(255, 120 + wood))
            g = max(0, min(255, 80 + wood // 2))
            b = max(0, min(255, 40 + wood // 3))
            p[x,y] = rgb(r, g, b)
    # Vertical planks
    for plank in range(0, SA, 12):
        for y in range(SA):
            if plank > 0:
                p[plank, y] = rgb(60, 40, 20)
    # Door handle
    for y in range(28, 36):
        for x in range(52, 56):
            p[x,y] = rgb(180, 160, 60)
    return img

def make_tile_goal():
    """Goal/portal tile — glowing swirl"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    for y in range(SA):
        for x in range(SA):
            p[x,y] = rgb(10, 10, 30)
    # Portal glow
    cx, cy = SA//2, SA//2
    for y in range(8, SA-8):
        for x in range(8, SA-8):
            dist = ((x - cx)**2 + (y - cy)**2) ** 0.5
            if dist < 24:
                bright = max(0, 1.0 - dist / 24.0)
                r = int(60 + 195 * bright)
                g = int(20 + 60 * bright)
                b = int(100 + 155 * bright)
                p[x,y] = rgb(r, g, b)
    return img

def make_tile_spike():
    """Spike trap tile"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Base
    for y in range(SA-8, SA):
        for x in range(SA):
            p[x,y] = rgb(60, 55, 50)
    # Spikes
    for spike in range(0, SA, 8):
        for h in range(20):
            for w in range(3):
                sx = spike + 2 + w
                sy = SA - 8 - h
                if sy >= 0 and sy < SA and sx >= 0 and sx < SA:
                    brightness = 60 + h * 6
                    p[sx, sy] = rgb(brightness, brightness - 20, brightness - 30)
    return img

def make_tile_platform():
    """Floating platform tile (grass-topped)"""
    img = Image.new("RGBA", (SA, SA), (0,0,0,0))
    p = img.load()
    # Dirt
    for y in range(12, SA):
        for x in range(SA):
            noise = ((x * 5 + y * 7) % 12) - 6
            r = max(0, min(255, 100 + noise))
            g = max(0, min(255, 75 + noise))
            b = max(0, min(255, 45 + noise))
            p[x,y] = rgb(r, g, b)
    # Grass top
    for y in range(6, 14):
        for x in range(SA):
            shade = 40 + ((x * 3) % 20)
            p[x,y] = rgb(20 + shade, 100 + shade, 20 + shade // 2)
    return img

# ================================================================
# 4. SCENE BACKGROUNDS (800x600)
# ================================================================
def make_bg_forest():
    """Forest background — 800x600"""
    img = Image.new("RGBA", (800, 600), (0,0,0,0))
    p = img.load()
    # Sky gradient
    for y in range(600):
        t = y / 600.0
        r = int(50 + 30 * t)
        g = int(80 + 60 * t)
        b = int(120 + 40 * t)
        for x in range(800):
            p[x,y] = rgb(r, g, b)
    # Ground
    for y in range(400, 600):
        t = (y - 400) / 200.0
        r = int(40 + 50 * t)
        g = int(100 + 60 * t)
        b = int(30 + 30 * t)
        for x in range(800):
            p[x,y] = rgb(r, g, b)
    # Trees (simple columns)
    for tx in [50, 150, 250, 400, 520, 650, 750]:
        tree_h = 150 + (tx * 17) % 100
        trunk_w = 10 + (tx * 13) % 8
        for y in range(300 - tree_h, 400):
            for x in range(tx - trunk_w, tx + trunk_w):
                if 0 <= x < 800 and 0 <= y < 600:
                    p[x,y] = rgb(60 + (y % 20), 40 + (y % 15), 20)
        # Foliage
        for y in range(290 - tree_h, 330 - tree_h // 2):
            for x in range(tx - 30, tx + 30):
                if 0 <= x < 800 and 0 <= y < 600:
                    dist = ((x - tx)**2 + (y - (290 - tree_h//2))**2) ** 0.5
                    if dist < 40:
                        shade = int(80 + 60 * (1.0 - dist / 40.0))
                        p[x,y] = rgb(20 + shade // 2, 80 + shade, 20 + shade // 3)
    return img

def make_bg_cave():
    """Cave background — 800x600"""
    img = Image.new("RGBA", (800, 600), (0,0,0,0))
    p = img.load()
    # Dark cave walls
    for y in range(600):
        for x in range(800):
            noise = ((x * 7 + y * 13) % 20) - 10
            r = max(0, min(255, 30 + noise))
            g = max(0, min(255, 25 + noise))
            b = max(0, min(255, 35 + noise))
            p[x,y] = rgb(r, g, b)
    # Stalactites
    for sx in range(0, 800, 40):
        height = 30 + (sx * 7) % 80
        for y in range(height):
            width = 8 + (y * 3) % 12
            for x in range(sx - width // 2, sx + width // 2):
                if 0 <= x < 800:
                    p[x, y] = rgb(60, 55, 50)
    # Glowing crystals
    for cx in [100, 300, 500, 700]:
        for y in range(200, 350):
            for x in range(cx - 8, cx + 8):
                if 0 <= x < 800 and 0 <= y < 600:
                    dist = ((x - cx)**2 + (y - 270)**2) ** 0.5
                    if dist < 10:
                        p[x,y] = rgb(100, 200, 255)
                    elif dist < 15:
                        p[x,y] = rgb(60, 150, 200)
        # Glow effect
        for radius in range(16, 40):
            alpha = max(0, 30 - radius)
            for angle in range(0, 360, 15):
                import math
                gx = cx + int(radius * math.cos(angle))
                gy = 270 + int(radius * math.sin(angle))
                if 0 <= gx < 800 and 0 <= gy < 600:
                    bg = p[gx, gy]
                    p[gx, gy] = rgb(
                        min(255, bg[0] + 20),
                        min(255, bg[1] + 40),
                        min(255, bg[2] + 60)
                    )
    return img

def make_bg_mountain():
    """Mountain background — 800x600"""
    img = Image.new("RGBA", (800, 600), (0,0,0,0))
    p = img.load()
    # Sky
    for y in range(400):
        t = y / 400.0
        r = int(60 + 100 * t)
        g = int(80 + 80 * t)
        b = int(130 + 50 * t)
        for x in range(800):
            p[x,y] = rgb(r, g, b)
    # Far mountains
    for mx, mh in [(100, 180), (300, 250), (500, 200), (700, 220)]:
        for y in range(400 - mh, 400):
            for x in range(mx - 120, mx + 120):
                if 0 <= x < 800:
                    dist = abs(x - mx)
                    height = mh * max(0, 1.0 - dist / 120.0)
                    if y >= 400 - height:
                        shade = int(80 + 40 * (1.0 - (y - (400 - height)) / height))
                        p[x,y] = rgb(shade, shade - 10, shade + 20)
    # Snow caps
    for mx, mh in [(300, 250), (700, 220)]:
        for y in range(400 - mh, 400 - mh + 20):
            for x in range(mx - 15, mx + 15):
                if 0 <= x < 800:
                    p[x,y] = rgb(200, 200, 220)
    # Ground
    for y in range(400, 600):
        t = (y - 400) / 200.0
        r = int(50 + 60 * t)
        g = int(70 + 50 * t)
        b = int(40 + 40 * t)
        for x in range(800):
            p[x,y] = rgb(r, g, b)
    return img

def make_bg_castle():
    """Castle background — 800x600"""
    img = Image.new("RGBA", (800, 600), (0,0,0,0))
    p = img.load()
    # Dark sky
    for y in range(600):
        t = y / 600.0
        r = int(20 + 30 * t)
        g = int(15 + 25 * t)
        b = int(40 + 30 * t)
        for x in range(800):
            p[x,y] = rgb(r, g, b)
    # Castle silhouette
    # Main keep
    for y in range(150, 420):
        for x in range(300, 500):
            if 0 <= x < 800:
                p[x,y] = rgb(50 + (y % 10), 45 + (y % 8), 55 + (y % 10))
    # Towers
    for tx, th in [(250, 250), (350, 300), (450, 280), (550, 240)]:
        tower_w = 30
        for y in range(420 - th, 420):
            for x in range(tx - tower_w, tx + tower_w):
                if 0 <= x < 800:
                    p[x,y] = rgb(55, 48, 60)
    # Battlements
    for bx in range(290, 510, 16):
        for y in range(140, 155):
            for x in range(bx, bx + 8):
                if 0 <= x < 800:
                    p[x,y] = rgb(60, 52, 65)
    # Windows (glowing)
    for wx, wy in [(330, 220), (370, 220), (410, 220), (450, 220),
                   (350, 280), (390, 280), (430, 280)]:
        for y in range(wy, wy + 16):
            for x in range(wx, wx + 8):
                if 0 <= x < 800:
                    p[x,y] = rgb(200, 180, 60)
    # Ground
    for y in range(420, 600):
        for x in range(800):
            p[x,y] = rgb(40, 50, 35)
    return img

# ================================================================
# 5. ITEM SPRITES (32x32)
# ================================================================
def make_item_coin():
    """Gold coin 32x32"""
    img = Image.new("RGBA", (32, 32), (0,0,0,0))
    p = img.load()
    for y in range(4, 28):
        for x in range(4, 28):
            dx, dy = x - 16, y - 16
            dist = (dx*dx + dy*dy) ** 0.5
            if dist < 12:
                shade = 1.0 - (dist / 12.0)
                p[x,y] = rgb(180 + int(75 * shade), 160 + int(40 * shade), 20 + int(20 * shade))
    # Dollar sign
    for y in range(10, 12):
        for x in [13, 14, 15, 16, 17, 18]:
            p[x,y] = rgb(200, 180, 60)
    for y in range(10, 24):
        for x in [14, 17]:
            p[x,y] = rgb(200, 180, 60)
    for y in range(20, 22):
        for x in [14, 15, 16, 17, 18]:
            p[x,y] = rgb(200, 180, 60)
    return img

def make_item_heart():
    """Heart powerup 32x32"""
    img = Image.new("RGBA", (32, 32), (0,0,0,0))
    p = img.load()
    for y in range(4, 28):
        for x in range(4, 28):
            # Heart shape formula
            dx = (x - 16) / 16.0 * 2
            dy = (y - 16) / 16.0 * 2
            heart = (dx*dx + dy*dy - 1)**3 - dx*dx * dy*dy*dy
            if heart < 0:
                shade = 1.0 + heart * 0.5
                r = int(200 + 55 * shade)
                g = int(40 + 30 * shade)
                b = int(40 + 30 * shade)
                p[x,y] = rgb(max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b)))
    return img

def make_item_star():
    """Star powerup 32x32"""
    img = Image.new("RGBA", (32, 32), (0,0,0,0))
    p = img.load()
    for y in range(2, 30):
        for x in range(2, 30):
            # Star shape via rotation
            dx, dy = x - 16, y - 16
            dist = (dx*dx + dy*dy) ** 0.5
            if dist < 14 and dist > 2:
                angle = (math.atan2(dy, dx) + 6.2832) % 6.2832
                # 5-pointed star
                star_angle = (angle * 5) % 6.2832
                star_r = 14 * 0.4
                if dist < star_r + 2:
                    shade = 1.0 - (dist / 14.0)
                    p[x,y] = rgb(200 + int(55 * shade), 180 + int(40 * shade), 20)
            elif dist <= 2:
                p[x,y] = rgb(255, 240, 100)
    return img

import math

# ================================================================
# GENERATE ALL SPRITES
# ================================================================
def main():
    print("🎮 RetroMania Arcade — Sprite Generator\n")
    print(f"Output: {ASSETS_DIR}\n")

    print("--- NINJA SPRITES (64x64) ---")
    save_png("ninja_idle.png", make_ninja_idle())
    save_png("ninja_walk1.png", make_ninja_walk1())
    save_png("ninja_walk2.png", make_ninja_walk2())
    save_png("ninja_jump.png", make_ninja_jump())
    save_png("ninja_attack.png", make_ninja_attack())

    print("\n--- ENEMY SPRITES ---")
    save_png("enemy_bouncer.png", make_enemy_bouncer())
    save_png("enemy_chaser.png", make_enemy_chaser())
    save_png("enemy_boss.png", make_enemy_boss())

    print("\n--- MAZE TILES (64x64) ---")
    save_png("tile_wall.png", make_tile_wall())
    save_png("tile_floor.png", make_tile_floor())
    save_png("tile_door.png", make_tile_door())
    save_png("tile_goal.png", make_tile_goal())
    save_png("tile_spike.png", make_tile_spike())
    save_png("tile_platform.png", make_tile_platform())

    print("\n--- SCENE BACKGROUNDS (800x600) ---")
    save_png("bg_forest.png", make_bg_forest())
    save_png("bg_cave.png", make_bg_cave())
    save_png("bg_mountain.png", make_bg_mountain())
    save_png("bg_castle.png", make_bg_castle())

    print("\n--- ITEMS (32x32) ---")
    save_png("item_coin.png", make_item_coin())
    save_png("item_heart.png", make_item_heart())
    save_png("item_star.png", make_item_star())

    print(f"\n✅ Generated {23} PNG sprites in assets/")
    print("Ready to compile and run!")

if __name__ == "__main__":
    main()
