#pragma once
#include <cstdint>
#include "..//cmp/entity.hpp"

namespace ECS{

struct GameContext_t;

    struct CollisionSystem_t
    {
        explicit CollisionSystem_t(uint32_t w, uint32_t h)
            : m_screenW(w), m_screenH(h) {}
        bool update(GameContext_t&) const;

        // Per-pixel AABB + alpha collision between two entities
        static bool checkPixelCollision(const Entity_t& a, const Entity_t& b);

    private:
        uint32_t m_screenW { 0 };
        uint32_t m_screenH { 0 };
    };

}
