#!/usr/bin/env python3
"""Capture a proper screenshot of the RetroMania Arcade game window.

This script:
1. Finds the game window position using xwininfo
2. Takes a full-screen XWD screenshot
3. Crops to the exact game window (or 800x600 centered if window not found)
4. Analyzes pixel colors to verify game rendering
5. Saves as PNG
"""
import subprocess
import time
import os
import signal
import struct
import sys
from PIL import Image
from collections import Counter

PROJECT = "/home/optimus/Documentos/freebuff/retroman/application_v1.27"
BINARY = os.path.join(PROJECT, "bin", "app")
OUTPUT = os.path.join(PROJECT, "..", "..", "game_screenshot_final.png")

os.chdir(PROJECT)
env = os.environ.copy()
env["DISPLAY"] = ":0"

# Kill previous instances
subprocess.run(["pkill", "-9", "-f", "bin/app"], capture_output=True)
time.sleep(0.5)

# Start the game (auto-starts after ~1.5s on title screen)
proc = subprocess.Popen(
    [BINARY],
    stdout=subprocess.DEVNULL,
    stderr=subprocess.DEVNULL,
    env=env,
    preexec_fn=lambda: os.setpgrp()
)
print(f"Game started PID: {proc.pid}")
time.sleep(3.5)  # Wait for title screen -> auto-start into PLAYING

# Try to find the game window
window_rect = None
try:
    result = subprocess.run(
        ["xwininfo", "-root", "-tree"],
        capture_output=True, text=True, env=env, timeout=5
    )
    for line in result.stdout.split('\n'):
        if 'RetroMania' in line:
            print(f"Found window: {line.strip()}")
            # Parse: "0x1234567 (800x600+1+33)  ..."
            import re
            m = re.search(r'(\d+)x(\d+)\+(\d+)\+(\d+)', line)
            if m:
                w, h, x, y = int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4))
                window_rect = (x, y, x + w, y + h)
                print(f"Window rect: {window_rect}")
except Exception as e:
    print(f"xwininfo error: {e}")

# Take full screenshot
result = subprocess.run(
    ["xwd", "-root", "-out", "/tmp/game_cap.xwd"],
    capture_output=True, text=True, env=env, timeout=5
)
print(f"xwd exit: {result.returncode}")

# Kill the game
try:
    os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
except Exception as e:
    print(f"killpg error: {e}")
    try:
        proc.kill()
    except:
        pass
try:
    proc.wait(timeout=3)
except:
    pass
print("Game terminated")

# Check file
if not os.path.exists("/tmp/game_cap.xwd"):
    print("ERROR: Screenshot file not created!")
    sys.exit(1)

size = os.path.getsize("/tmp/game_cap.xwd")
print(f"Screenshot file size: {size} bytes")

# Parse XWD and convert to PNG
with open("/tmp/game_cap.xwd", "rb") as f:
    data = f.read()

endian = ">" if struct.unpack_from(">H", data, 0)[0] == 0 else "<"
fields = struct.unpack_from(endian + "I"*25, data, 0)
full_w, full_h, bpp = fields[4], fields[5], fields[11]
print(f"Full screen: {full_w}x{full_h}, bpp={bpp}")

# Parse pixels (32bpp TrueColor)
pixel_offset = fields[0]
name_end = data.find(b"\x00", pixel_offset)
if name_end > pixel_offset:
    pixel_offset = (name_end + 4) & ~3
bpl = fields[12]

pixels = bytearray()
for y in range(full_h):
    row_start = pixel_offset + y * bpl
    for x in range(full_w):
        px_off = row_start + x * 4
        if px_off + 3 <= len(data):
            if endian == ">":
                # MSB first: ARGB
                pixels.extend([data[px_off+1], data[px_off+2], data[px_off+3]])
            else:
                # LSB first: BGRA
                pixels.extend([data[px_off+2], data[px_off+1], data[px_off+0]])
        else:
            pixels.extend([0, 0, 0])

full_img = Image.frombytes("RGB", (full_w, full_h), bytes(pixels))

# Crop to game window
if window_rect:
    game = full_img.crop(window_rect)
    print(f"Cropped to game window at {window_rect}")
else:
    # Center crop as fallback (assumes game window is centered)
    cx, cy = full_w // 2, full_h // 2
    game = full_img.crop((cx - 400, cy - 300, cx + 400, cy + 300))
    print(f"Fallback: center crop to 800x600")

print(f"Game image size: {game.size}")

# Detailed pixel analysis
pixel_list = list(game.getdata())
color_counts = Counter(pixel_list)
total = game.size[0] * game.size[1]
unique = len(color_counts)

print(f"\n{'='*60}")
print(f"GAME WINDOW PIXEL ANALYSIS ({game.size[0]}x{game.size[1]})")
print(f"{'='*60}")
print(f"Total pixels: {total}")
print(f"Unique colors: {unique}")

# Count black pixels
black = sum(1 for p in pixel_list if p[0] < 10 and p[1] < 10 and p[2] < 10)
print(f"Near-black pixels (<10): {black} ({black*100/total:.1f}%)")

# Count sky blue pixels
sky_blue = sum(1 for p in pixel_list if abs(p[0] - 135) < 20 and abs(p[1] - 206) < 20 and abs(p[2] - 235) < 20)
print(f"Sky blue (background): {sky_blue}px ({sky_blue*100/total:.1f}%)")

print(f"\nTop 20 colors:")
for color, count in color_counts.most_common(20):
    pct = count / total * 100
    print(f"  RGB{color}: {count:7d}px ({pct:5.1f}%)")

# Check for specific game element colors
check_colors = {
    "Sky blue (bg)": (135, 206, 235),
    "Title yellow": (255, 221, 0),     # 0xFFDD00
    "White text": (204, 204, 204),      # 0xCCCCCC
    "Ninja blue": (55, 55, 75),         # ninja gi
    "Enemy red": (200, 50, 50),         # bouncer
    "Coin gold": (255, 215, 0),         # 0xFFD700
    "Wall brown": (74, 70, 60),         # 0x4A463C
    "Door brown": (120, 80, 40),        # 0x785028
    "Goal purple": (60, 20, 100),       # 0x3C1464
}
print(f"\nElement color search (exact):")
for name, color in check_colors.items():
    nearby = sum(1 for p in pixel_list 
                 if abs(p[0] - color[0]) < 10 
                 and abs(p[1] - color[1]) < 10 
                 and abs(p[2] - color[2]) < 10)
    if nearby > 10:
        print(f"  ✅ {name}: {nearby}px near RGB{color}")

# Save
game.save(OUTPUT)
print(f"\n✅ Screenshot saved: {OUTPUT}")
