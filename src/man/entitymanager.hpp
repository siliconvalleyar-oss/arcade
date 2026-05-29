#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "componentstorage.hpp"
#include "../util/typealiases.hpp"
#include "../util/gamecontext.hpp"
#include "../cmp/entity.hpp"

namespace ECS{

	struct EntityManager_t : public GameContext_t
	{
		static constexpr std::size_t kNUMINITIALENTITIES { 1000 } ;	
		
		explicit EntityManager_t() { m_Entity.reserve(kNUMINITIALENTITIES); }
		
		Entity_t& createEntity(int32_t, int32_t, uint32_t, uint32_t, uint32_t);
	 	Entity_t& createEntity(int32_t, int32_t, const std::string&);

	 	const Vect_t<Entity_t>& getEntities() const override { return m_Entity; };
	 	      Vect_t<Entity_t>& getEntities()       override { return m_Entity; };
	 	const std::list<PhysicsComponent_t>& getPhysicsComponent() const override { return m_components.getPhysicsComponent(); };
          	  std::list<PhysicsComponent_t>& getPhysicsComponent()       override { return m_components.getPhysicsComponent(); };

	private:
	 	Vect_t<Entity_t> m_Entity{};
	 	ComponentStorage_t m_components;
	};

}

