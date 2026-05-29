#pragma once
#include <list>
#include "../cmp/physics.hpp"

namespace ECS 
{

struct PhysicsComponent_t;

    struct ComponentStorage_t
    {
        explicit ComponentStorage_t() = default;
        ComponentStorage_t(const ComponentStorage_t&) = delete;
        ComponentStorage_t(ComponentStorage_t&&) = default;
        ComponentStorage_t& operator=(const ComponentStorage_t&) = delete;
        ComponentStorage_t& operator=(ComponentStorage_t&&) = default;
        
        PhysicsComponent_t& createPhysicsComponent();

        std::list<PhysicsComponent_t>& getPhysicsComponent() { return m_physicsComponents; };
        const std::list<PhysicsComponent_t>& getPhysicsComponent() const { return m_physicsComponents; };

    private:
        std::list<PhysicsComponent_t> m_physicsComponents{};
    };

}