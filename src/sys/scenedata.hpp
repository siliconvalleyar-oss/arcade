#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

namespace ECS {

// Tile types for platformer
enum Tile : uint8_t {
    TILE_AIR    = 0,
    TILE_BRICK  = 1,
    TILE_GOAL   = 3,
    TILE_COIN   = 6,  // Coin tile (collected on touch)
};

// Compact level description
struct LevelDesc {
    const char* name;
    const char* bgFile;
    int         width;    // in tiles
    int         height;   // in tiles
    const uint8_t* ground;  // height per column (0=top, height-1=bottom, 255=pit)
    // Platforms: pairs of (x, y) where x=column, y=row of leftmost tile
    int         numPlats;
    const uint16_t* platforms; // each pair = (x, y)
    int         platWidths; // width of each platform
    const uint8_t* platW;    // width of each platform
    // Enemies: pairs of (x, y)
    int         numEnemies;
    const uint16_t* enemies;
    int         numBouncers; // first N enemies are bouncers
    // Coins: pairs of (x, y)
    int         numCoins;
    const uint16_t* coins;
    // Player spawn
    int         spawnX, spawnY;
    // Goal position
    int         goalX, goalY;
};

// Build a tile array from LevelDesc
inline void buildLevelTiles(const LevelDesc& desc, uint8_t* tiles)
{
    int w = desc.width;
    int h = desc.height;
    std::memset(tiles, TILE_AIR, w * h);

    // Ground
    for(int x = 0; x < w && x < 200; ++x) {
        int gy = desc.ground[x];
        if(gy >= 0 && gy < h) {
            tiles[gy * w + x] = TILE_BRICK;
        }
    }

    // Platforms
    int pi = 0;
    for(int p = 0; p < desc.numPlats; ++p) {
        int px = desc.platforms[pi++];
        int py = desc.platforms[pi++];
        int pw = desc.platW ? static_cast<int>(desc.platW[p]) : 3;
        for(int xi = 0; xi < pw && px + xi < w; ++xi) {
            tiles[py * w + (px + xi)] = TILE_BRICK;
        }
    }

    // Coins (stored as tiles for reference)
    int ci = 0;
    for(int c = 0; c < desc.numCoins; ++c) {
        int cx = desc.coins[ci++];
        int cy = desc.coins[ci++];
        if(cx < w && cy < h) {
            tiles[cy * w + cx] = TILE_COIN;
        }
    }

    // Goal
    if(desc.goalX >= 0 && desc.goalX < w && desc.goalY >= 0 && desc.goalY < h) {
        tiles[desc.goalY * w + desc.goalX] = TILE_GOAL;
    }
}

// ============================================================
// Level 1: Forest Meadow (64 tiles wide, 12 tiles tall)
// ============================================================
static const uint8_t kForestGround[64] = {
    11,11,11,11,11,11,11,11, 11,11,11,11,11,11,11,11,
    11,11,11,11,11,11,11,11, 11,11,11,11,11,11,11,11,
    11,11,11,11,10,11,10,11, 11,11,11,11,11,11,11,11,
    11,11,11,11,11,11,11,11, 11,11,11,11,11,11,11,11,
};
// Platform data: (x, y) pairs
static const uint16_t kForestPlats[] = {
    8,9,   20,8,  32,9,  44,8,  50,7
};
static const uint8_t kForestPlatW[] = { 3, 4, 3, 3, 3 };
// Enemy data: (x, y) pairs
static const uint16_t kForestEnemies[] = {
    9,10,  22,10,  38,10,  52,9
};
// Coin data: (x, y) pairs
static const uint16_t kForestCoins[] = {
    5,8,   6,8,   7,8,   15,8,  16,8,  17,8,
    25,6,  26,6,  27,6,  35,8,  36,8,  37,8,
    45,6,  46,6,  47,6,  55,8,  56,8,
};

// ============================================================
// Level 2: Cavern Depths (64 tiles wide, 12 tiles tall)
// ============================================================
static const uint8_t kCaveGround[64] = {
    11,11,11,11,11,10,11,11, 11,11,11,10,11,11,11,11,
    11,11,11,11,11,11,11,11, 10,11,11,11,11,11,10,11,
    11,11,11,11,11,11,11,11, 11,11,11,11,10,11,11,11,
    11,10,11,11,11,11,10,11, 11,11,11,11,11,11,11,11,
};
static const uint16_t kCavePlats[] = {
    4,8,   14,8,  24,7,  34,8,  44,7,  54,8
};
static const uint8_t kCavePlatW[] = { 5, 4, 5, 4, 4, 4 };
static const uint16_t kCaveEnemies[] = {
    8,10,  18,10,  28,9,  40,10,  50,9,  58,10
};
static const uint16_t kCaveCoins[] = {
    5,6,   6,6,   7,6,   15,6,  16,6,  17,6,
    26,5,  27,5,  28,5,  35,6,  36,6,  37,6,
    45,5,  46,5,  47,5,  55,6,  56,6,  57,6,
};

// ============================================================
// Level 3: Sky Peaks (64 tiles wide, 12 tiles tall)
// ============================================================
static const uint8_t kMountainGround[64] = {
    11,11,11,11,11,11,11,11, 11,11,11,11,11,11,11,11,
    11,11,11,11,11,11,11,11, 11,11,11,10,11,11,11,11,
    11,11,10,11,11,11,11,11, 11,11,11,11,11,11,11,11,
    11,11,11,11,11,11,11,11, 10,11,11,11,11,11,10,11,
};
static const uint16_t kMountainPlats[] = {
    3,7,   12,6,  22,8,  30,6,  40,7,  48,5,  56,7
};
static const uint8_t kMountainPlatW[] = { 4, 6, 4, 5, 4, 5, 4 };
static const uint16_t kMountainEnemies[] = {
    7,9,   16,9,  26,9,  34,9,  44,8,  52,9,  60,9
};
static const uint16_t kMountainCoins[] = {
    4,5,   5,5,   6,5,   14,4,  15,4,  16,4,
    24,6,  25,6,  26,6,  32,5,  33,5,  34,5,
    42,5,  43,5,  44,5,  50,4,  51,4,  52,4,
};

// ============================================================
// Level 4: Castle Fortress (64 tiles wide, 12 tiles tall)
// ============================================================
static const uint8_t kCastleGround[64] = {
    11,11,11,11,11,11,10,11, 11,11,11,11,11,11,11,11,
    11,11,11,11,10,11,11,11, 11,11,10,11,11,11,11,11,
    11,11,11,11,11,11,11,11, 11,11,11,11,11,11,11,11,
    11,11,11,11,11,11,11,11, 11,11,11,10,11,10,11,11,
};
static const uint16_t kCastlePlats[] = {
    3,8,   12,7,  20,8,  28,6,  36,7,  44,6,  52,7,  58,6
};
static const uint8_t kCastlePlatW[] = { 4, 5, 4, 5, 4, 5, 4, 4 };
static const uint16_t kCastleEnemies[] = {
    6,10,  15,10,  24,10,  32,9,  40,10,  48,9,  55,10,  60,10
};
static const uint16_t kCastleCoins[] = {
    4,6,   5,6,   6,6,   14,5,  15,5,  16,5,
    22,6,  23,6,  24,6,   30,5,  31,5,  32,5,
    38,5,  39,5,  40,5,   46,4,  47,4,  48,4,
    54,5,  55,5,  56,5,   60,4,  61,4,
};

// All levels
static const LevelDesc kLevels[] = {
    {
        "FOREST MEADOW",
        "assets/bg_forest.png",
        64, 12,
        kForestGround,
        5, kForestPlats, 5, kForestPlatW,
        4, kForestEnemies, 2,  // first 2 are bouncers
        17, kForestCoins,
        2, 10,  // spawn
        61, 10, // goal
    },
    {
        "CAVERN DEPTHS",
        "assets/bg_cave.png",
        64, 12,
        kCaveGround,
        6, kCavePlats, 6, kCavePlatW,
        6, kCaveEnemies, 3,
        18, kCaveCoins,
        2, 10,
        61, 10,
    },
    {
        "SKY PEAKS",
        "assets/bg_mountain.png",
        64, 12,
        kMountainGround,
        7, kMountainPlats, 7, kMountainPlatW,
        7, kMountainEnemies, 3,
        18, kMountainCoins,
        2, 10,
        61, 10,
    },
    {
        "CASTLE FORTRESS",
        "assets/bg_castle.png",
        64, 12,
        kCastleGround,
        8, kCastlePlats, 8, kCastlePlatW,
        8, kCastleEnemies, 4,
        22, kCastleCoins,
        2, 10,
        61, 10,
    },
};

static constexpr uint32_t kNUMLEVELS = sizeof(kLevels) / sizeof(kLevels[0]);

} // namespace ECS
