//
// Created by Steve Wheeler on 23/02/2024.
//

#pragma once

#include "../components/WorldObject.hpp"
#include "BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
	class WorldSystem : public BaseSystem<WorldObject>
	{
		entt::entity root;

	public:
		WorldSystem(entt::registry* _registry, entt::entity rootNodeId);
	};
}
