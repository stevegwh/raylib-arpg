//
// Created by Steve Wheeler on 23/02/2024.
//

#include "WorldSystem.hpp"


namespace sage
{
	WorldSystem::WorldSystem(entt::registry* _registry, entt::entity rootNodeId) :
		BaseSystem(_registry), root(rootNodeId)
	{
	}
}
