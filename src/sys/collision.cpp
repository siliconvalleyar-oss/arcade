#include "collision.hpp"
#include "../util/gamecontext.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

namespace ECS{

    bool CollisionSystem_t::checkPixelCollision(const Entity_t& a, const Entity_t& b)
    {
        if(a.phy == nullptr || b.phy == nullptr) return false;

        // Quick AABB reject
        if(a.phy->x >= b.phy->x + static_cast<int32_t>(b.w)) return false;
        if(a.phy->x + static_cast<int32_t>(a.w) <= b.phy->x) return false;
        if(a.phy->y >= b.phy->y + static_cast<int32_t>(b.h)) return false;
        if(a.phy->y + static_cast<int32_t>(a.h) <= b.phy->y) return false;

        // Calculate overlap rectangle in world space
        int32_t ox1 = std::max(a.phy->x, b.phy->x);
        int32_t oy1 = std::max(a.phy->y, b.phy->y);
        int32_t ox2 = std::min(a.phy->x + static_cast<int32_t>(a.w),
                               b.phy->x + static_cast<int32_t>(b.w));
        int32_t oy2 = std::min(a.phy->y + static_cast<int32_t>(a.h),
                               b.phy->y + static_cast<int32_t>(b.h));

        // Scan the overlap region checking alpha (bit 24-31)
        for(int32_t py = oy1; py < oy2; ++py) {
            // Row start in each sprite
            const uint32_t* aRow = a.sprite.data() + (py - a.phy->y) * a.w;
            const uint32_t* bRow = b.sprite.data() + (py - b.phy->y) * b.w;

            int32_t startX = ox1;
            int32_t endX   = ox2;
            int32_t aOff   = startX - a.phy->x;
            int32_t bOff   = startX - b.phy->x;

            for(int32_t px = startX; px < endX; ++px) {
                // Check alpha > 0 on both sprites
                if((aRow[aOff] & 0xFF000000) && (bRow[bOff] & 0xFF000000)) {
                    return true; // Non-transparent pixel overlap
                }
                aOff++;
                bOff++;
            }
        }
        return false;
    }

    bool CollisionSystem_t::update(GameContext_t& g) const
    {
        for(auto& e : g.getEntities()) {
            if(e.phy != nullptr) {
                // Left boundary
                if(e.phy->x < 0) {
                    e.phy->x = 0;
                    e.phy->vx = std::abs(e.phy->vx);
                }
                // Right boundary
                if(e.phy->x + static_cast<int32_t>(e.w) > static_cast<int32_t>(m_screenW)) {
                    e.phy->x = static_cast<int32_t>(m_screenW) - static_cast<int32_t>(e.w);
                    e.phy->vx = -std::abs(e.phy->vx);
                }
                // Top boundary
                if(e.phy->y < 0) {
                    e.phy->y = 0;
                    e.phy->vy = std::abs(e.phy->vy);
                }
                // Bottom boundary
                if(e.phy->y + static_cast<int32_t>(e.h) > static_cast<int32_t>(m_screenH)) {
                    e.phy->y = static_cast<int32_t>(m_screenH) - static_cast<int32_t>(e.h);
                    e.phy->vy = 0;
                }
            }
        }
        return true;
    }

}
