#include <iostream>
#include <memory>
#include "entitymanager.hpp"
#include "../util/gamecontext.hpp"

namespace ECS 
{
	Entity_t& EntityManager_t::createEntity(int32_t x, int32_t y, const std::string& filename)
	{
		auto& e = m_Entity.emplace_back(filename);
		auto& ph = m_components.createPhysicsComponent();
		e.phy = &ph;
		ph.x = x;
		ph.y = y;
		return e;
	}

	Entity_t& EntityManager_t::createEntity(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color)
	{
		auto& e = m_Entity.emplace_back(w, h);
		auto& ph = m_components.createPhysicsComponent();
		e.phy = &ph;
		ph.x = x;
		ph.y = y;
		// Fill sprite with solid color
		for(auto& p : e.sprite) {
			p = color;
		}
		return e;
	}
}
