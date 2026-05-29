#pragma once 
#include <cstdint>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include "../../lib/picoPNG/src/picopng.hpp"
#include "physics.hpp"
#include "../util/typealiases.hpp"

namespace ECS{

    enum class EntityType : uint8_t {
        PLAYER,
        BOUNCER,
        CHASER,
        BOSS,
        ITEM_COIN,
        ITEM_HEART,
        ITEM_STAR,
        TILE_WALL,
        TILE_FLOOR,
        TILE_GOAL,
        NONE
    };

    struct Entity_t
    {
        explicit Entity_t(uint32_t _w , uint32_t _h)
        : w(_w) , h(_h)
        {
            sprite.resize(w*h);	
        }

        explicit Entity_t(const std::string& filename)
        {
            loadPNG(filename);
        }

        void loadPNG(const std::string& filename)
        {
            std::vector<unsigned char> pixels{};
            unsigned long dw{0}, dh{0};
            std::ifstream file(filename, std::ios::binary);
            if(!file.is_open()) {
                w = 32; h = 32;
                sprite.resize(w * h, 0x00FF00FF); // Magenta = missing texture
                return;
            }
            std::vector<unsigned char> filevec(
                std::istreambuf_iterator<char>{file},
                std::istreambuf_iterator<char>{}
            );

            if(filevec.empty()) {
                w = 32; h = 32;
                sprite.resize(w * h, 0x00FF00FF);
                return;
            }

            int result = decodePNG(pixels, dw, dh, filevec.data(), filevec.size());
            if(result != 0 || dw == 0 || dh == 0) {
                w = 32; h = 32;
                sprite.resize(w * h, 0x00FF00FF);
                return;
            }

            w = static_cast<uint32_t>(dw);
            h = static_cast<uint32_t>(dh);
            sprite.reserve(pixels.size() / 4);
            for(auto p = pixels.begin(); p != pixels.end(); p += 4) {
                // PNG decode gives RGBA bytes: [R, G, B, A]
                // Build 0xAARRGGBB format for X11 little-endian:
                //   pixel = (A << 24) | (R << 16) | (G << 8) | B
                // On LE memory: [B, G, R, A] → X11 reads Blue=B, Green=G, Red=R
                uint32_t pixel = 
                    static_cast<uint32_t>(*(p+0)) << 16   // R -> bits 23:16
                |	static_cast<uint32_t>(*(p+1)) << 8    // G -> bits 15:8
                |	static_cast<uint32_t>(*(p+2))         // B -> bits 7:0
                |	static_cast<uint32_t>(*(p+3)) << 24;  // A -> bits 31:24
                sprite.push_back(pixel);
            }
        }

        // Animation support: load multiple frames into animFrames
        void loadAnimation(const std::vector<std::string>& frameFiles)
        {
            animFrames.clear();
            for(const auto& f : frameFiles) {
                Entity_t tmp(f);
                animFrames.push_back(std::move(tmp.sprite));
            }
            if(!animFrames.empty()) {
                animated = true;
                // Use first frame dimensions
                sprite = animFrames[0];
                // Recalculate w,h from first frame
                // (They were set by loading the first PNG; but we loaded all, so frames share size)
            }
        }

        // Update animation frame
        void advanceAnimation()
        {
            if(!animated || animFrames.size() <= 1) return;
            animTimer++;
            if(animTimer >= animSpeed) {
                animTimer = 0;
                currentFrame = (currentFrame + 1) % animFrames.size();
                sprite = animFrames[currentFrame];
            }
        }

        // Set a specific animation frame
        void setAnimFrame(uint32_t frame)
        {
            if(!animated || animFrames.empty()) return;
            currentFrame = frame % animFrames.size();
            sprite = animFrames[currentFrame];
        }

        PhysicsComponent_t* phy { nullptr };
        uint32_t w { 0 } , h { 0 };
        std::vector<uint32_t> sprite{};
        EntityID_t entityID { ++nextID };
        static EntityID_t nextID;

        // Animation state
        std::vector<std::vector<uint32_t>> animFrames;
        uint32_t currentFrame { 0 };
        uint32_t animTimer { 0 };
        uint32_t animSpeed { 10 };
        bool animated { false };

        // Entity type for game logic
        EntityType type { EntityType::NONE };
    };
}
