//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "systems/ActorMovementSystem.hpp"
#include "WaveMobDefaultStateSystem.hpp"
#include "PlayerDefaultStateSystem.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace sage
{
	class DefaultStateSystems
	{
		std::vector<StateMachineSystem*> systems;
	public:
		
		std::unique_ptr<WaveMobDefaultStateSystem> waveMobDefaultSubSystem;
		std::unique_ptr<PlayerDefaultStateSystem> playerDefaultSubSystem;
		void Update();
		DefaultStateSystems(entt::registry* _registry,
		                   ActorMovementSystem* _actorMovementSystem);
	};
} // sage
