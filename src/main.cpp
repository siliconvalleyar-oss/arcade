extern "C" {
    #include "../lib/tinyPTC/src/tinyptc.h"
}
#include <cstdint>
#include <iostream>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstring>
#include "sys/rendersystem.hpp"
#include "sys/input.hpp"
#include "man/entitymanager.hpp"
#include "util/gamecontext.hpp"
#include "sys/collision.hpp"
#include "sys/physics.hpp"
#include "sys/audio.hpp"
#include "sys/scenedata.hpp"

constexpr uint32_t kSCRWIDTH     { 800 };
constexpr uint32_t kSCRHEIGHT    { 600 };
constexpr int32_t  kPLAYERSPEED  { 5 };
constexpr int32_t  kJUMPFORCE    { -16 };
constexpr int32_t  kTERMINALVEL  { 20 };
constexpr uint32_t kFRAMEDELAY_MS{ 16 };
constexpr uint32_t kINVINCIBILITY_FRAMES { 90 };
constexpr uint32_t kMAXLIVES     { 3 };
constexpr int32_t  kENEMYSPEED   { 2 };
constexpr uint32_t kTILESIZE     { 64 };

// HUD bar heights
constexpr uint32_t kHUDTOP       { 20 };
constexpr uint32_t kHUDBOTTOM    { 0 };

enum class GameState : uint8_t {
    TITLE,
    PLAYING,
    LEVEL_CLEAR,
    GAME_OVER,
    VICTORY
};

// Game globals
static uint32_t  g_currentLevel = 0;
static uint32_t  g_score = 0;
static uint32_t  g_lives = kMAXLIVES;
static GameState g_state = GameState::TITLE;
static uint32_t  g_invTimer = 0;
static uint32_t  g_coinCount = 0;
static uint32_t  g_levelClearTimer = 0;
static uint32_t  g_attackTimer = 0;
constexpr uint32_t kATTACK_FRAMES = 15;

// Current level tile data
static uint8_t g_levelTiles[64 * 12];

// ============================================================
// Level initialization
// ============================================================
static void buildLevel(uint32_t levelIdx)
{
    if(levelIdx >= ECS::kNUMLEVELS) return;
    buildLevelTiles(ECS::kLevels[levelIdx], g_levelTiles);
}

// ============================================================
// Tile access helpers
// ============================================================
static inline uint8_t getTile(int tx, int ty)
{
    if(tx < 0 || tx >= 64) return 1; // out of bounds = solid
    if(ty < 0 || ty >= 12) return (ty >= 12) ? 1 : 0; // below = solid, above = air
    return g_levelTiles[ty * 64 + tx];
}

static inline bool isSolid(int tx, int ty)
{
    uint8_t t = getTile(tx, ty);
    return (t == ECS::TILE_BRICK || t == ECS::TILE_GOAL);
}

// ============================================================
// Entity creation helpers
// ============================================================
struct Enemy_t {
    ECS::Entity_t* ent;
    float patrolWait;   // time before turning around
    int32_t homeX;      // original X position
};

static Enemy_t g_enemies[12];
static int g_numEnemies = 0;

static ECS::Entity_t* g_player = nullptr;

// ============================================================
// Ground / wall collision for platformer
// ============================================================
struct CollisionInfo {
    bool onGround;
    bool hitCeiling;
    bool hitLeftWall;
    bool hitRightWall;
};

static CollisionInfo resolvePlatformCollision(ECS::Entity_t& e)
{
    CollisionInfo info = { false, false, false, false };
    if(e.phy == nullptr) return info;

    int32_t& x = e.phy->x;
    int32_t& y = e.phy->y;
    int32_t& vx = e.phy->vx;
    int32_t& vy = e.phy->vy;
    int32_t w = static_cast<int32_t>(e.w);
    int32_t h = static_cast<int32_t>(e.h);
    int32_t ts = static_cast<int32_t>(kTILESIZE);

    // ===== HORIZONTAL =====
    x += vx;
    // Check left/right
    if(vx > 0) {
        int32_t rightX = x + w - 1;
        int32_t tx = rightX / ts;
        int32_t ty1 = y / ts;
        int32_t ty2 = (y + h - 1) / ts;
        for(int32_t ty = ty1; ty <= ty2; ++ty) {
            if(isSolid(tx, ty)) {
                x = tx * ts - w;
                vx = 0;
                info.hitRightWall = true;
                break;
            }
        }
    } else if(vx < 0) {
        int32_t leftX = x;
        int32_t tx = leftX / ts;
        int32_t ty1 = y / ts;
        int32_t ty2 = (y + h - 1) / ts;
        for(int32_t ty = ty1; ty <= ty2; ++ty) {
            if(isSolid(tx, ty)) {
                x = (tx + 1) * ts;
                vx = 0;
                info.hitLeftWall = true;
                break;
            }
        }
    }

    // ===== VERTICAL =====
    y += vy;
    if(vy > 0) { // Moving down
        int32_t bottomY = y + h - 1;
        int32_t ty = bottomY / ts;
        int32_t tx1 = x / ts;
        int32_t tx2 = (x + w - 1) / ts;
        for(int32_t tx = tx1; tx <= tx2; ++tx) {
            if(isSolid(tx, ty)) {
                y = ty * ts - h;
                vy = 0;
                info.onGround = true;
                break;
            }
        }
    } else if(vy < 0) { // Moving up
        int32_t topY = y;
        int32_t ty = topY / ts;
        int32_t tx1 = x / ts;
        int32_t tx2 = (x + w - 1) / ts;
        for(int32_t tx = tx1; tx <= tx2; ++tx) {
            if(isSolid(tx, ty)) {
                y = (ty + 1) * ts;
                vy = 0;
                info.hitCeiling = true;
                break;
            }
        }
    }

    return info;
}

// ============================================================
// Check if entity is standing on ground
// ============================================================
static bool checkOnGround(const ECS::Entity_t& e)
{
    if(e.phy == nullptr) return false;
    int32_t checkY = e.phy->y + static_cast<int32_t>(e.h);
    int32_t ty = checkY / static_cast<int32_t>(kTILESIZE);
    int32_t tx1 = e.phy->x / static_cast<int32_t>(kTILESIZE);
    int32_t tx2 = (e.phy->x + static_cast<int32_t>(e.w) - 1) / static_cast<int32_t>(kTILESIZE);
    // Check wall tiles
    for(int32_t tx = tx1; tx <= tx2; ++tx) {
        if(isSolid(tx, ty)) return true;
    }
    return false;
}

// ============================================================
// Coin tile check (collect coins embedded in tile data)
// ============================================================
static bool collectCoinAt(const ECS::Entity_t& e)
{
    if(e.phy == nullptr) return false;
    int32_t x = e.phy->x + static_cast<int32_t>(e.w) / 2;
    int32_t y = e.phy->y + static_cast<int32_t>(e.h) / 2;
    int32_t tx = x / static_cast<int32_t>(kTILESIZE);
    int32_t ty = y / static_cast<int32_t>(kTILESIZE);
    if(tx < 0 || tx >= 64 || ty < 0 || ty >= 12) return false;
    if(g_levelTiles[ty * 64 + tx] == ECS::TILE_COIN) {
        g_levelTiles[ty * 64 + tx] = ECS::TILE_AIR; // Remove coin
        return true;
    }
    return false;
}

// ============================================================
// Check if entity is at the goal
// ============================================================
static bool checkGoal(const ECS::Entity_t& e)
{
    if(e.phy == nullptr) return false;
    int32_t cx = e.phy->x + static_cast<int32_t>(e.w) / 2;
    int32_t cy = e.phy->y + static_cast<int32_t>(e.h) / 2;
    int32_t tx = cx / static_cast<int32_t>(kTILESIZE);
    int32_t ty = cy / static_cast<int32_t>(kTILESIZE);
    if(tx < 0 || tx >= 64 || ty < 0 || ty >= 12) return false;
    return (g_levelTiles[ty * 64 + tx] == ECS::TILE_GOAL);
}

// ============================================================
// Pit death check
// ============================================================
static bool checkPitDeath(const ECS::Entity_t& e)
{
    if(e.phy == nullptr) return false;
    return (e.phy->y > static_cast<int32_t>(12 * kTILESIZE) + 100);
}

// ============================================================
// Spawn entities for current level
// ============================================================
static void spawnEntities(ECS::EntityManager_t& em, uint32_t levelIdx)
{
    if(levelIdx >= ECS::kNUMLEVELS) return;
    const auto& level = ECS::kLevels[levelIdx];

    // Player
    int32_t px = level.spawnX * static_cast<int32_t>(kTILESIZE);
    int32_t py = level.spawnY * static_cast<int32_t>(kTILESIZE);
    g_player = &em.createEntity(px, py, "assets/ninja_idle.png");
    g_player->type = ECS::EntityType::PLAYER;
    g_player->w = 48;
    g_player->h = 48;
    // Load animation frames
    g_player->animFrames.clear();
    {
        ECS::Entity_t f1("assets/ninja_idle.png");
        ECS::Entity_t f2("assets/ninja_walk1.png");
        ECS::Entity_t f3("assets/ninja_walk2.png");
        ECS::Entity_t f4("assets/ninja_jump.png");
        ECS::Entity_t f5("assets/ninja_attack.png");
        if(!f1.sprite.empty()) g_player->animFrames.push_back(f1.sprite);
        if(!f2.sprite.empty()) g_player->animFrames.push_back(f2.sprite);
        if(!f3.sprite.empty()) g_player->animFrames.push_back(f3.sprite);
        if(!f4.sprite.empty()) g_player->animFrames.push_back(f4.sprite);
        if(!f5.sprite.empty()) g_player->animFrames.push_back(f5.sprite);
    }
    g_player->animated = true;
    if(!g_player->animFrames.empty()) g_player->sprite = g_player->animFrames[0];

    // Enemies
    g_numEnemies = 0;
    int ei = 0;
    for(int i = 0; i < level.numEnemies && g_numEnemies < 12; ++i) {
        int ex = level.enemies[ei++] * static_cast<int32_t>(kTILESIZE);
        int ey = level.enemies[ei++] * static_cast<int32_t>(kTILESIZE);
        auto& e = em.createEntity(ex, ey, (i < level.numBouncers) ? "assets/enemy_bouncer.png" : "assets/enemy_chaser.png");
        e.type = (i < level.numBouncers) ? ECS::EntityType::BOUNCER : ECS::EntityType::CHASER;
        e.w = 44;
        e.h = 44;
        e.phy->vx = (std::rand() % 2 == 0) ? kENEMYSPEED : -kENEMYSPEED;
        e.phy->vy = 0;

        Enemy_t& en = g_enemies[g_numEnemies++];
        en.ent = &e;
        en.patrolWait = 0.0f;
        en.homeX = ex;
    }

    // Player spawn reset
    g_player->phy->x = px;
    g_player->phy->y = py;
    g_player->phy->vx = 0;
    g_player->phy->vy = 0;
}

// ============================================================
// Main
// ============================================================
int main(void)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    try {
        ECS::EntityManager_t entityMan;
        g_player = nullptr;
        for(int i = 0; i < 12; ++i) g_enemies[i].ent = nullptr;
        g_numEnemies = 0;
        g_currentLevel = 0;
        g_score = 0;
        g_lives = kMAXLIVES;
        g_state = GameState::TITLE;
        g_invTimer = 0;
        g_coinCount = 0;
        g_levelClearTimer = 0;
        g_attackTimer = 0;

        ECS::RenderSystem_t render{ kSCRWIDTH, kSCRHEIGHT };
        ECS::AudioSystem_t audio;

        ECS::initInput();

        // Load maze tile sprites (for rendering the tile map)
        render.loadMazeTiles();

        // Build first level
        buildLevel(0);
        spawnEntities(entityMan, 0);
        if(ECS::kNUMLEVELS > 0) {
            render.loadSceneBG(ECS::kLevels[0].bgFile);
            render.setUseSceneBG(true);
        }

        audio.startMusic();

        bool running = true;
        uint32_t animTimer = 0;
        uint32_t animFrame = 0;

        while(running)
        {
            auto startTime = std::chrono::steady_clock::now();
            auto& input = ECS::getInputState();

            // ======== TITLE SCREEN ========
            if(g_state == GameState::TITLE) {
                static uint32_t titleTimer = 0;
                titleTimer++;
                // Auto-start after ~1.5 seconds for screenshot capture
                if(input.space || input.up || input.restart || titleTimer > 100) {
                    g_state = GameState::PLAYING;
                    titleTimer = 0;
                    g_currentLevel = 0;
                    g_score = 0;
                    g_lives = kMAXLIVES;
                    g_coinCount = 0;
                    g_invTimer = 0;
                    buildLevel(0);
                    entityMan = ECS::EntityManager_t();
                    spawnEntities(entityMan, 0);
                    if(ECS::kNUMLEVELS > 0) {
                        render.loadSceneBG(ECS::kLevels[0].bgFile);
                        render.setUseSceneBG(true);
                    }
                    audio.playJump();
                }
            }

            // ======== PLAYING ========
            else if(g_state == GameState::PLAYING) {

                // --- Input ---
                if(g_player != nullptr && g_player->phy != nullptr) {
                    if(input.left)  g_player->phy->vx = -kPLAYERSPEED;
                    else if(input.right) g_player->phy->vx = kPLAYERSPEED;
                    else g_player->phy->vx = 0;

                    bool onGround = checkOnGround(*g_player);
                    if((input.space || input.up) && onGround) {
                        g_player->phy->vy = kJUMPFORCE;
                        audio.playJump();
                    }
                }

                // --- Gravity ---
                if(g_player != nullptr && g_player->phy != nullptr) {
                    g_player->phy->vy += 1; // gravity
                    if(g_player->phy->vy > kTERMINALVEL) g_player->phy->vy = kTERMINALVEL;
                }

                // --- Enemy AI ---
                for(int i = 0; i < g_numEnemies; ++i) {
                    auto* e = g_enemies[i].ent;
                    if(e == nullptr || e->phy == nullptr) continue;

                    if(e->type == ECS::EntityType::BOUNCER) {
                        // Bouncer: hop and change direction
                        e->phy->vy += 1;
                        if(e->phy->vy > 6) e->phy->vy = 6;
                        bool onG = checkOnGround(*e);
                        if(onG) {
                            e->phy->vy = -8; // hop
                            // Random direction change
                            if(std::rand() % 60 == 0) e->phy->vx = -e->phy->vx;
                        }
                        // Clamp horizontal speed
                        if(e->phy->vx > kENEMYSPEED) e->phy->vx = kENEMYSPEED;
                        if(e->phy->vx < -kENEMYSPEED) e->phy->vx = -kENEMYSPEED;
                    } else {
                        // Chaser: patrol back and forth
                        // Check wall ahead
                        int32_t checkX = (e->phy->vx > 0)
                            ? e->phy->x + static_cast<int32_t>(e->w) + 4
                            : e->phy->x - 4;
                        int32_t checkY = e->phy->y + static_cast<int32_t>(e->h) / 2;
                        int32_t tx = checkX / static_cast<int32_t>(kTILESIZE);
                        int32_t ty = checkY / static_cast<int32_t>(kTILESIZE);
                        if(isSolid(tx, ty)) {
                            e->phy->vx = -e->phy->vx;
                        }

                        // Simple gravity
                        e->phy->vy += 1;
                        if(e->phy->vy > 8) e->phy->vy = 8;
                    }
                }

                // --- Physics & Collision ---
                if(g_player != nullptr && g_player->phy != nullptr) {
                    // Player horizontal
                    g_player->phy->x += g_player->phy->vx;
                    // Check wall collisions for player
                    auto& p = *g_player;
                    int32_t ts = static_cast<int32_t>(kTILESIZE);
                    if(p.phy->vx > 0) {
                        int32_t rightX = p.phy->x + static_cast<int32_t>(p.w) - 1;
                        int32_t tx = rightX / ts;
                        int32_t ty1 = p.phy->y / ts;
                        int32_t ty2 = (p.phy->y + static_cast<int32_t>(p.h) - 1) / ts;
                        for(int32_t ty = ty1; ty <= ty2; ++ty) {
                            if(isSolid(tx, ty)) {
                                p.phy->x = tx * ts - static_cast<int32_t>(p.w);
                                break;
                            }
                        }
                    } else if(p.phy->vx < 0) {
                        int32_t leftX = p.phy->x;
                        int32_t tx = leftX / ts;
                        int32_t ty1 = p.phy->y / ts;
                        int32_t ty2 = (p.phy->y + static_cast<int32_t>(p.h) - 1) / ts;
                        for(int32_t ty = ty1; ty <= ty2; ++ty) {
                            if(isSolid(tx, ty)) {
                                p.phy->x = (tx + 1) * ts;
                                break;
                            }
                        }
                    }

                    // Player vertical
                    p.phy->y += p.phy->vy;
                    if(p.phy->vy > 0) {
                        int32_t bottomY = p.phy->y + static_cast<int32_t>(p.h) - 1;
                        int32_t ty = bottomY / ts;
                        int32_t tx1 = p.phy->x / ts;
                        int32_t tx2 = (p.phy->x + static_cast<int32_t>(p.w) - 1) / ts;
                        for(int32_t tx = tx1; tx <= tx2; ++tx) {
                            if(isSolid(tx, ty)) {
                                p.phy->y = ty * ts - static_cast<int32_t>(p.h);
                                p.phy->vy = 0;
                                break;
                            }
                        }
                    } else if(p.phy->vy < 0) {
                        int32_t topY = p.phy->y;
                        int32_t ty = topY / ts;
                        int32_t tx1 = p.phy->x / ts;
                        int32_t tx2 = (p.phy->x + static_cast<int32_t>(p.w) - 1) / ts;
                        for(int32_t tx = tx1; tx <= tx2; ++tx) {
                            if(isSolid(tx, ty)) {
                                p.phy->y = (ty + 1) * ts;
                                p.phy->vy = 0;
                                break;
                            }
                        }
                    }
                }

                // Enemy physics + collision
                for(int i = 0; i < g_numEnemies; ++i) {
                    auto* e = g_enemies[i].ent;
                    if(e == nullptr || e->phy == nullptr) continue;
                    if(e->phy) resolvePlatformCollision(*e);
                }

                // --- Coin collection ---
                if(g_player != nullptr && g_player->phy != nullptr) {
                    if(collectCoinAt(*g_player)) {
                        g_score += 100;
                        g_coinCount++;
                        audio.playScore();
                        render.emitParticles(
                            g_player->phy->x + static_cast<int32_t>(g_player->w) / 2,
                            g_player->phy->y + static_cast<int32_t>(g_player->h) / 2 - 20,
                            6, 0x00FFD700, 1.0f, 3.0f, 15.0f, 1);
                    }
                }

                // --- Enemy-player collision (pixel) ---
                if(g_player != nullptr && g_player->phy != nullptr && g_invTimer == 0) {
                    for(int i = 0; i < g_numEnemies; ++i) {
                        auto* e = g_enemies[i].ent;
                        if(e == nullptr || e->phy == nullptr) continue;
                        if(!ECS::CollisionSystem_t::checkPixelCollision(*g_player, *e)) continue;

                        // Check if player is stomping (falling from above)
                        bool stomp = (g_player->phy->vy > 0 &&
                            g_player->phy->y + static_cast<int32_t>(g_player->h) - 8 <= e->phy->y);

                        if(stomp) {
                            // Kill enemy
                            g_score += 200;
                            audio.playScore();
                            render.emitParticles(
                                e->phy->x + static_cast<int32_t>(e->w) / 2,
                                e->phy->y + static_cast<int32_t>(e->h) / 2,
                                10, 0x00FFAA44, 1.0f, 4.0f, 20.0f, 1);
                            e->phy = nullptr;
                            g_player->phy->vy = -10; // Bounce up
                        } else {
                            // Player hit
                            if(g_lives > 0) g_lives--;
                            audio.playDeath();
                            render.emitParticles(
                                g_player->phy->x + static_cast<int32_t>(g_player->w) / 2,
                                g_player->phy->y + static_cast<int32_t>(g_player->h) / 2,
                                15, 0x00FF6644, 1.5f, 5.0f, 25.0f, 2);

                            if(g_lives == 0) {
                                g_state = GameState::GAME_OVER;
                                audio.playGameOver();
                                break;
                            }

                            // Respawn at level start
                            const auto& lv = ECS::kLevels[g_currentLevel];
                            g_player->phy->x = lv.spawnX * static_cast<int32_t>(kTILESIZE);
                            g_player->phy->y = lv.spawnY * static_cast<int32_t>(kTILESIZE);
                            g_player->phy->vx = 0;
                            g_player->phy->vy = 0;
                            g_invTimer = kINVINCIBILITY_FRAMES;
                            break;
                        }
                    }
                }

                // --- Invincibility timer ---
                if(g_invTimer > 0) g_invTimer--;

                // --- Pit death ---
                if(g_player != nullptr && g_player->phy != nullptr && checkPitDeath(*g_player)) {
                    if(g_lives > 0) g_lives--;
                    audio.playDeath();
                    if(g_lives == 0) {
                        g_state = GameState::GAME_OVER;
                        audio.playGameOver();
                    } else {
                        const auto& lv = ECS::kLevels[g_currentLevel];
                        g_player->phy->x = lv.spawnX * static_cast<int32_t>(kTILESIZE);
                        g_player->phy->y = lv.spawnY * static_cast<int32_t>(kTILESIZE);
                        g_player->phy->vx = 0;
                        g_player->phy->vy = 0;
                        g_invTimer = kINVINCIBILITY_FRAMES;
                    }
                }

                // --- Goal check ---
                if(g_player != nullptr && g_player->phy != nullptr && checkGoal(*g_player)) {
                    audio.playScore();
                    g_state = GameState::LEVEL_CLEAR;
                    g_levelClearTimer = 0;

                    // Big celebration particles
                    render.emitParticles(
                        g_player->phy->x + static_cast<int32_t>(g_player->w) / 2,
                        g_player->phy->y - 20,
                        30, 0x00FFDD00, 2.0f, 6.0f, 40.0f, 2);
                    render.emitParticles(
                        g_player->phy->x + static_cast<int32_t>(g_player->w) / 2,
                        g_player->phy->y - 20,
                        20, 0x00FF4444, 1.0f, 4.0f, 30.0f, 1);
                }

                // --- Player animation ---
                if(g_player != nullptr && g_player->animFrames.size() >= 4) {
                    bool onG = checkOnGround(*g_player);
                    if(!onG) {
                        g_player->sprite = g_player->animFrames[3]; // jump
                    } else if(g_player->phy->vx != 0) {
                        animTimer++;
                        if(animTimer >= 8) {
                            animTimer = 0;
                            animFrame = (animFrame == 0) ? 1 : 0;
                        }
                        g_player->sprite = g_player->animFrames[animFrame + 1];
                    } else {
                        g_player->sprite = g_player->animFrames[0]; // idle
                        animTimer = 0;
                        animFrame = 0;
                    }

                    // Attack animation (with cooldown)
                    if(g_attackTimer > 0) g_attackTimer--;
                    if(input.attack && g_attackTimer == 0) {
                        g_attackTimer = kATTACK_FRAMES;
                    }
                    if(g_attackTimer > 0 && g_player->animFrames.size() >= 5) {
                        g_player->sprite = g_player->animFrames[4]; // attack frame
                        if(g_attackTimer == kATTACK_FRAMES - 1) {
                            render.emitParticles(
                                g_player->phy->x + static_cast<int32_t>(g_player->w),
                                g_player->phy->y + static_cast<int32_t>(g_player->h) / 2,
                                5, 0x00AACCFF, 2.0f, 6.0f, 15.0f, 2);
                        }
                    }
                }
            }

            // ======== LEVEL CLEAR ========
            else if(g_state == GameState::LEVEL_CLEAR) {
                g_levelClearTimer++;
                if((input.space || input.up) && g_levelClearTimer > 30) {
                    g_currentLevel++;
                    if(g_currentLevel >= ECS::kNUMLEVELS) {
                        g_state = GameState::VICTORY;
                    } else {
                        buildLevel(g_currentLevel);
                        entityMan = ECS::EntityManager_t();
                        spawnEntities(entityMan, g_currentLevel);
                        if(g_currentLevel < ECS::kNUMLEVELS) {
                            render.loadSceneBG(ECS::kLevels[g_currentLevel].bgFile);
                            render.setUseSceneBG(true);
                        }
                        g_state = GameState::PLAYING;
                        audio.playJump();
                    }
                }
            }

            // ======== GAME OVER ========
            else if(g_state == GameState::GAME_OVER) {
                if(input.restart) {
                    audio.stopMusic();
                    render.clearParticles();
                    g_currentLevel = 0;
                    g_score = 0;
                    g_lives = kMAXLIVES;
                    g_coinCount = 0;
                    g_invTimer = 0;
                    g_player = nullptr;
                    for(int i = 0; i < 12; ++i) g_enemies[i].ent = nullptr;
                    g_numEnemies = 0;
                    buildLevel(0);
                    entityMan = ECS::EntityManager_t();
                    spawnEntities(entityMan, 0);
                    if(ECS::kNUMLEVELS > 0) {
                        render.loadSceneBG(ECS::kLevels[0].bgFile);
                        render.setUseSceneBG(true);
                    }
                    g_state = GameState::PLAYING;
                    audio.startMusic();
                }
            }

            // ======== VICTORY ========
            else if(g_state == GameState::VICTORY) {
                if(input.restart) {
                    audio.stopMusic();
                    render.clearParticles();
                    g_currentLevel = 0;
                    g_score = 0;
                    g_lives = kMAXLIVES;
                    g_coinCount = 0;
                    g_invTimer = 0;
                    g_player = nullptr;
                    for(int i = 0; i < 12; ++i) g_enemies[i].ent = nullptr;
                    g_numEnemies = 0;
                    buildLevel(0);
                    entityMan = ECS::EntityManager_t();
                    spawnEntities(entityMan, 0);
                    if(ECS::kNUMLEVELS > 0) {
                        render.loadSceneBG(ECS::kLevels[0].bgFile);
                        render.setUseSceneBG(true);
                    }
                    g_state = GameState::PLAYING;
                    audio.startMusic();
                }
            }

            // ======== CAMERA ========
            int32_t camX = 0, camY = 0;
            if(g_player != nullptr && g_player->phy != nullptr) {
                int32_t worldW = 64 * static_cast<int32_t>(kTILESIZE);
                int32_t worldH = 12 * static_cast<int32_t>(kTILESIZE);
                camX = g_player->phy->x + static_cast<int32_t>(g_player->w) / 2
                     - static_cast<int32_t>(kSCRWIDTH) / 2;
                camY = static_cast<int32_t>(worldH) - static_cast<int32_t>(kSCRHEIGHT);
                camX = std::max(0, std::min(camX, worldW - static_cast<int32_t>(kSCRWIDTH)));
                camY = std::max(0, camY);
            }
            render.setCamera(camX, camY);

            // ======== RENDER ========
            int32_t playerVx = (g_player != nullptr && g_player->phy != nullptr)
                             ? g_player->phy->vx : 0;
            render.beginFrame(playerVx, camX, camY);

            // Draw maze tiles (level blocks)
            render.drawMaze(g_levelTiles, 64, 12, kTILESIZE, camX, camY);

            // Draw entities (player, enemies)
            render.drawAllEntities(entityMan.getEntities());

            // Draw particles
            render.drawParticles();

            // ======== HUD ========
            {
                auto* fb = render.getFramebuffer();
                uint32_t w = kSCRWIDTH;

                // Top bar
                if(g_state != GameState::TITLE) {
                    for(uint32_t x = 0; x < w; ++x) {
                        for(uint32_t y = 0; y < kHUDTOP; ++y) {
                            fb[y * w + x] = (g_invTimer > 0) ? 0x00443322 : 0x00222222;
                        }
                    }

                    // Score
                    ECS::BitmapFont_t::drawString(fb, w, 10, 4, "SCORE", 0x00CCCCCC);
                    ECS::BitmapFont_t::drawInt(fb, w, 80, 4, static_cast<int32_t>(g_score), 0x00FFFFFF);

                    // Coins
                    ECS::BitmapFont_t::drawString(fb, w, 180, 4, "COINS", 0x00CCCCCC);
                    ECS::BitmapFont_t::drawInt(fb, w, 240, 4, static_cast<int32_t>(g_coinCount), 0x00FFD700);

                    // Level name
                    if(g_currentLevel < ECS::kNUMLEVELS) {
                        const char* name = ECS::kLevels[g_currentLevel].name;
                        int len = 0;
                        while(name[len]) len++;
                        int textW = len * 6;
                        int sx = (static_cast<int32_t>(w) - textW) / 2;
                        ECS::BitmapFont_t::drawString(fb, w, sx, 4, name, 0x00FFDD88);
                    }

                    // Lives as hearts
                    ECS::BitmapFont_t::drawString(fb, w, w - 180, 4, "LIVES", 0x00CCCCCC);
                    for(uint32_t i = 0; i < g_lives; ++i) {
                        // Draw heart symbol as 2-byte 'heart'
                        ECS::BitmapFont_t::drawChar(fb, w,
                            w - 110 + static_cast<int32_t>(i) * 22, 4, '<', 0x00FF4444);
                        ECS::BitmapFont_t::drawChar(fb, w,
                            w - 105 + static_cast<int32_t>(i) * 22, 4, '3', 0x00FF4444);
                    }

                    if(g_invTimer > 0) {
                        ECS::BitmapFont_t::drawString(fb, w, w / 2 - 65, 4, "INVINCIBLE", 0x00FFAA00);
                    }
                }
            }

            // ======== STATE OVERLAYS ========
            if(g_state == GameState::TITLE) {
                render.drawTitle();
            } else if(g_state == GameState::GAME_OVER) {
                render.drawGameOver();
            } else if(g_state == GameState::LEVEL_CLEAR) {
                render.drawLevelClear();
            } else if(g_state == GameState::VICTORY) {
                render.drawVictory(g_score, g_coinCount);
            }

            running = render.endFrame();

            // Update particles AFTER drawing
            render.updateParticles();

            // Frame rate limiting
            auto elapsed = std::chrono::steady_clock::now() - startTime;
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            if(elapsedMs < kFRAMEDELAY_MS) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(kFRAMEDELAY_MS - elapsedMs)
                );
            }
        }

        audio.stopMusic();
    }
    catch(const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch(...) {
        std::cout << "Program terminated." << std::endl;
    }

    return 0;
}
