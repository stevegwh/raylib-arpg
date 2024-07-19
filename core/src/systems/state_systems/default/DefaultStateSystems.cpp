//
// Created by Steve Wheeler on 08/06/2024.
//

#include "DefaultStateSystems.hpp"

namespace sage
{
	void DefaultStateSystems::Update()
	{
		for (auto& system : systems)
		{
			system->Update();
		}
	}

	DefaultStateSystems::DefaultStateSystems(entt::registry* _registry,
	                                       ActorMovementSystem* _actorMovementSystem):
		waveMobDefaultSubSystem(std::make_unique<WaveMobDefaultStateSystem>(_registry,
		                                                                  _actorMovementSystem)),
		playerDefaultSubSystem(std::make_unique<PlayerDefaultStateSystem>(_registry,
		                                                                _actorMovementSystem))
	{
		systems.push_back(waveMobDefaultSubSystem.get());
		systems.push_back(playerDefaultSubSystem.get());
	}
} // sage
