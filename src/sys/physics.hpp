#pragma once
#include <cstdint>

namespace ECS {

 struct  GameContext_t;

 struct PhysicsSystem_t
    {
        explicit PhysicsSystem_t()=default;
        bool update(GameContext_t&) const;
        static constexpr int32_t kGRAVITY { 1 };
    };
}