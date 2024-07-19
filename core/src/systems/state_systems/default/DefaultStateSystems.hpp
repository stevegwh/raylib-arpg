//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "systems/ActorMovementSystem.hpp"
#include "WaveMobDefaultSubSystem.hpp"
#include "PlayerDefaultSubSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{
	class DefaultStateSystems
	{
		std::unique_ptr<WaveMobDefaultSubSystem> waveMobDefaultSubSystem;
		std::unique_ptr<PlayerDefaultSubSystem> playerDefaultSubSystem;

	public:
		void Update();
		DefaultStateSystems(entt::registry* _registry,
		                   ActorMovementSystem* _actorMovementSystem);
	};
} // sage
