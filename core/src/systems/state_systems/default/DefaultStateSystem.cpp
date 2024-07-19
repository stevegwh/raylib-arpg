//
// Created by Steve Wheeler on 08/06/2024.
//

#include "DefaultStateSystem.hpp"

namespace sage
{
	void DefaultStateSystem::Update()
	{
		waveMobDefaultSubSystem->Update();
	}

	DefaultStateSystem::DefaultStateSystem(entt::registry* _registry,
	                                       ActorMovementSystem* _actorMovementSystem):
		waveMobDefaultSubSystem(std::make_unique<WaveMobDefaultSubSystem>(_registry,
		                                                                  _actorMovementSystem)),
		playerDefaultSubSystem(std::make_unique<PlayerDefaultSubSystem>(_registry,
		                                                                _actorMovementSystem))
	{
	}
} // sage
