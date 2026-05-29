#!/usr/bin/env python3
"""Capture RetroMania Arcade gameplay — presses Space to start, then captures.

This script uses xdotool if available, or just captures the title screen.
"""
import subprocess
import time
import os
import signal
import struct
import re
import sys
from PIL import Image
from collections import Counter

PROJECT = "/home/optimus/Documentos/freebuff/retroman/application_v1.27"
BINARY = os.path.join(PROJECT, "bin", "app")
OUTPUT = os.path.join(PROJECT, "..", "..", "game_gameplay.png")

os.chdir(PROJECT)
env = os.environ.copy()
env["DISPLAY"] = ":0"

# Kill previous instances
subprocess.run(["pkill", "-9", "-f", "bin/app"], capture_output=True)
time.sleep(0.5)

# Start the game
proc = subprocess.Popen(
    [BINARY],
    stdout=subprocess.DEVNULL,
    stderr=subprocess.DEVNULL,
    env=env,
    preexec_fn=lambda: os.setpgrp()
)
print(f"Game started PID: {proc.pid}")
time.sleep(2)

# Try to press Space to start the game
for key_cmd in [["xdotool", "key", "space"], ["xdotool", "key", "Space"]]:
    try:
        result = subprocess.run(key_cmd, capture_output=True, text=True, env=env, timeout=3)
        if result.returncode == 0:
            print(f"Pressed key with: {' '.join(key_cmd)}")
            break
        else:
            print(f"Key command failed: {result.stderr}")
    except FileNotFoundError:
        print("xdotool not found, trying next key command")
    except Exception as e:
        print(f"Key command error: {e}")

# Also try via window
try:
    result = subprocess.run(
        ["xdotool", "search", "--name", "RetroMania"],
        capture_output=True, text=True, env=env, timeout=3
    )
    if result.stdout.strip():
        wid = result.stdout.strip().split('\n')[0]
        print(f"Found game window ID: {wid}")
        subprocess.run(["xdotool", "windowactivate", wid], capture_output=True, env=env, timeout=3)
        time.sleep(0.3)
        subprocess.run(["xdotool", "key", "space"], capture_output=True, env=env, timeout=3)
        print("Pressed Space via window focus")
except Exception as e:
    print(f"xdotool window search error: {e}")

time.sleep(1.5)  # Wait for gameplay to start

# Find the window position
window_rect = None
try:
    result = subprocess.run(
        ["xwininfo", "-root", "-tree"],
        capture_output=True, text=True, env=env, timeout=5
    )
    for line in result.stdout.split('\n'):
        if 'RetroMania' in line:
            print(f"Window: {line.strip()}")
            m = re.search(r'(\d+)x(\d+)\+(\d+)\+(\d+)', line)
            if m:
                w, h, x, y = int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4))
                window_rect = (x, y, x + w, y + h)
                print(f"Window rect: {window_rect}")
except Exception as e:
    print(f"xwininfo error: {e}")

# Take screenshot
result = subprocess.run(
    ["xwd", "-root", "-out", "/tmp/game_playing.xwd"],
    capture_output=True, text=True, env=env, timeout=5
)
print(f"xwd exit: {result.returncode}")

# Kill the game
try:
    os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
except:
    try:
        proc.kill()
    except:
        pass
try:
    proc.wait(timeout=3)
except:
    pass

# Parse XWD
with open("/tmp/game_playing.xwd", "rb") as f:
    data = f.read()

endian = ">" if struct.unpack_from(">H", data, 0)[0] == 0 else "<"
fields = struct.unpack_from(endian + "I"*25, data, 0)
full_w, full_h, bpp = fields[4], fields[5], fields[11]
print(f"Full screen: {full_w}x{full_h}")

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
                pixels.extend([data[px_off+1], data[px_off+2], data[px_off+3]])
            else:
                pixels.extend([data[px_off+2], data[px_off+1], data[px_off+0]])
        else:
            pixels.extend([0, 0, 0])

full_img = Image.frombytes("RGB", (full_w, full_h), bytes(pixels))

# Save full screenshot for reference
full_img.save("/tmp/game_full_ref.png")
print(f"Full screenshot saved: {full_img.size}")

# Crop to game window
if window_rect:
    game = full_img.crop(window_rect)
    print(f"Cropped to: {window_rect}")
else:
    cx, cy = full_w // 2, full_h // 2
    game = full_img.crop((cx - 400, cy - 300, cx + 400, cy + 300))

pixel_list = list(game.getdata())
color_counts = Counter(pixel_list)
total = game.size[0] * game.size[1]
unique = len(color_counts)

print(f"\n{'='*60}")
print(f"GAMEPLAY SCREENSHOT ANALYSIS ({game.size[0]}x{game.size[1]})")
print(f"{'='*60}")
print(f"Total pixels: {total}")
print(f"Unique colors: {unique}")

black = sum(1 for p in pixel_list if p[0] < 10 and p[1] < 10 and p[2] < 10)
print(f"Black pixels (<10): {black} ({black*100/total:.1f}%)")

print(f"\nTop 30 colors:")
for color, count in color_counts.most_common(30):
    pct = count / total * 100
    print(f"  {str(color):20s} {count:7d}px ({pct:5.1f}%)")

# Check for game elements
checks = {
    "Sky blue": (135, 206, 235),
    "Title yellow": (255, 221, 0),
    "Ninja body": (55, 55, 75),
    "Ninja skin": (255, 200, 155),
    "Enemy red body": (200, 50, 50),
    "Coin gold": (255, 215, 0),
    "Wall brown": (74, 70, 60),
    "Floor brown": (110, 95, 70),
    "Dark overlay": (67, 103, 117),  # Sky blue / 2
    "HUD dark": (34, 34, 34),
}
print(f"\nElement color search:")
for name, color in checks.items():
    nearby = sum(1 for p in pixel_list
                 if abs(p[0] - color[0]) < 10
                 and abs(p[1] - color[1]) < 10
                 and abs(p[2] - color[2]) < 10)
    if nearby > 50:
        print(f"  ✅ {name}: {nearby}px near {color}")

game.save(OUTPUT)
print(f"\n✅ Saved: {OUTPUT}")
