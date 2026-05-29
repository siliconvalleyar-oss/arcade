#include "componentstorage.hpp"

namespace ECS{

     PhysicsComponent_t& ComponentStorage_t::createPhysicsComponent() {
         return m_physicsComponents.emplace_back();
     }

}
