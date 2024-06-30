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
	                                       StateMachineSystem* _stateMachineSystem,
	                                       ActorMovementSystem* _actorMovementSystem):
		BaseSystem<StateMachineComponent>(_registry),
		stateMachineSystem(_stateMachineSystem),
		waveMobDefaultSubSystem(std::make_unique<WaveMobDefaultSubSystem>(_registry,
		                                                                  _stateMachineSystem,
		                                                                  _actorMovementSystem)),
		playerDefaultSubSystem(std::make_unique<PlayerDefaultSubSystem>(_registry,
		                                                                _stateMachineSystem,
		                                                                _actorMovementSystem))
	{
	}
} // sage
