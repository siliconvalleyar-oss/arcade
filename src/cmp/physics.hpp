#pragma once
#include <cstdint>

namespace ECS{

    struct PhysicsComponent_t
    {
        explicit PhysicsComponent_t() = default;

        int32_t x { 0 } , y { 0 };
        int32_t vx { 0 } , vy { 0 };
    };

}