# RetroMania Arcade — Ninja Platformer

## Overview
Side-scrolling platformer built with C++17 and tinyPTC (X11 framebuffer). 800x600 resolution, ECS architecture, procedural audio.

## Project Structure
```
arcade/
├── Makefile              # Build system (g++, C++17, -lX11 -lXext)
├── assets/               # PNG sprites (57 files)
│   ├── ninja_*.png       # Player frames (idle, walk1, walk2, jump, attack)
│   ├── enemy_*.png       # Enemies (bouncer, chaser, boss)
│   ├── tile_*.png        # Tiles (wall, floor, door, goal, spike, platform)
│   ├── bg_*.png          # Backgrounds (forest, cave, mountain, castle)
│   └── item_*.png        # Items (coin, heart, star)
├── src/
│   ├── main.cpp          # Game loop, level logic, input handling, HUD
│   ├── cmp/              # Components (entity, physics, component)
│   ├── man/              # Managers (entitymanager, componentstorage)
│   ├── sys/              # Systems (rendersystem, input, physics, collision, audio)
│   │   └── scenedata.hpp # Level definitions (4 levels), tile map builder
│   └── util/             # Utilities (gamecontext, typealiases)
├── lib/
│   ├── tinyPTC/          # X11 framebuffer library
│   └── picoPNG/          # PNG decoding library
├── tools/
│   ├── generate_sprites.py    # Generate all 23+ PNG assets via Pillow
│   ├── capture_screenshot.py  # Capture screenshot of running game
│   ├── capture_gameplay.py    # Capture gameplay screenshot
│   └── improve_sprites.py     # Sprite enhancement tool
└── scripts/
    └── install_deps.sh   # Install dependencies
```

## Build & Run
```bash
make              # Build release
make run          # Build and run
make debug        # Debug build (adds -g -DDEBUG -O0)
make clean        # Clean build artifacts
./bin/GameAracedeApp  # Run directly
```

## Gameplay
- **Goal**: Reach the flag (TILE_GOAL) in each level, survive 4 levels
- **Score**: Coins (+100), Stomp enemies (+200)
- **Lives**: 3 (hearts in HUD), lose one on enemy contact or pit death
- **Invincibility**: 90 frames after taking damage
- **Attack**: Press F for sword slash (visual + particles)
- **Enemies**:
  - Bouncer: Red slime, hops and changes direction randomly
  - Chaser: Blue bat, patrols back-and-forth, bounces off walls

## Levels (64 tiles × 12 tiles)
| # | Name            | BG File             | Enemies | Bouncers | Coins |
|---|-----------------|---------------------|---------|----------|-------|
| 1 | FOREST MEADOW   | assets/bg_forest.png | 4      | 2        | 17    |
| 2 | CAVERN DEPTHS   | assets/bg_cave.png   | 6      | 3        | 18    |
| 3 | SKY PEAKS       | assets/bg_mountain.png| 7     | 3        | 18    |
| 4 | CASTLE FORTRESS | assets/bg_castle.png | 8      | 4        | 22    |

## Controls
| Key              | Action         |
|------------------|----------------|
| A / Left Arrow   | Move left      |
| D / Right Arrow  | Move right     |
| Space / W / Up   | Jump           |
| F                | Attack (sword) |
| R                | Restart        |
| ESC              | Quit           |

## Game States
- **TITLE**: Auto-advances after ~1.5s or press Space. Shows controls.
- **PLAYING**: Active gameplay with physics, collision, enemies, coins.
- **LEVEL_CLEAR**: Press Space to advance. Shows celebration particles.
- **GAME_OVER**: Press R to restart from level 1.
- **VICTORY**: After beating all 4 levels. Shows final score + coins. Press R to restart.

## Architecture (ECS)

### Components (`src/cmp/`)
- `Entity_t`: Position, size, sprite (PNG-loaded pixel data), animation frames, type
- `PhysicsComponent_t`: x, y, vx, vy
- `Component_t`: Base component with EntityID

### Entity Manager (`src/man/`)
- `EntityManager_t`: Owns entities vector + physics components list
- Creates entities at spawn positions with PNG sprites

### Systems (`src/sys/`)
- **RenderSystem**: Framebuffer management, camera, maze tiles, entities, particles, HUD, background parallax, title/gameover/victory screens
- **InputSystem**: X11 key press/release callbacks via tinyPTC
- **PhysicsSystem**: Gravity, velocity updates
- **CollisionSystem**: Per-pixel AABB + alpha collision detection
- **AudioSystem**: Procedural PCM (sine, noise, sweep) piped to `aplay`

### Main Game Loop (`src/main.cpp`)
1. Input handling (player movement, jump, attack)
2. Gravity + terminal velocity
3. Enemy AI (bouncer hop/chaser patrol)
4. Physics & tile collision resolution
5. Coin collection (tile-based)
6. Enemy-player collision (stomp vs damage)
7. Pit death check
8. Goal check
9. Animation state machine (idle/walk/jump/attack)
10. Camera follows player horizontally
11. Render: BG → Maze → Entities → Particles → HUD → State overlays
12. Frame rate cap at ~62.5 fps (16ms)

## Tile System
- `TILE_AIR=0`, `TILE_BRICK=1`, `TILE_GOAL=3`, `TILE_COIN=6`
- Ground per column (index 11 in data = bottommost tile row)
- Platforms defined as (x, y, width) tuples
- 64×12 grid, each tile 64×64 pixels

## Audio
- Jump: Ascending sweep (400→800 Hz, 120ms)
- Death: Descending sweep (600→80 Hz) + noise
- Score: Two-tone chime (880 Hz + 1100 Hz)
- Game Over: Descending tone sequence (440→330→220 Hz)
- Music: 140 BPM looping beat (kick, hi-hat, bass line)

## Sprite Generation
```bash
python3 tools/generate_sprites.py   # Regenerate all PNG assets
```
Requires: Python 3 + Pillow (`pip install Pillow`)

## Dependencies
- g++ with C++17 support
- X11 and Xext development libraries
- alsa-utils (for `aplay` audio playback)
- Python 3 + Pillow (for sprite generation)
- xdotool, xwd, xwininfo (for screenshot tools)

## Commit History
- `70d51d5` — "Arcade Game" (initial)
- `87298e5` — "sources" (current)
