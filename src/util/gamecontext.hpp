#pragma once
#include <list>
#include "../cmp/entity.hpp"
#include "typealiases.hpp"

namespace ECS{

    struct GameContext_t
    {
        virtual ~GameContext_t() = default;
        virtual         Vect_t<Entity_t>&                  getEntities() = 0;
        virtual const   Vect_t<Entity_t>&                  getEntities() const = 0;
        virtual         std::list<PhysicsComponent_t>&     getPhysicsComponent() = 0;
        virtual const   std::list<PhysicsComponent_t>&     getPhysicsComponent() const = 0;
    };

}