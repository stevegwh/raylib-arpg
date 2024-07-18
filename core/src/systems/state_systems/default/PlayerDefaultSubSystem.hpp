// Created by Steve Wheeler on 30/06/2024.

#pragma once

#include "entt/entt.hpp"

#include "systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"

namespace sage
{
	class PlayerDefaultSubSystem
	{
		entt::registry* registry;
		StateMachineSystem* stateMachineSystem;
		ActorMovementSystem* actorMovementSystem;

	public:
		void Update();
		void OnStateAdded(entt::entity entity) const;
		void OnStateRemoved(entt::entity entity) const;
		PlayerDefaultSubSystem(entt::registry* _registry,
		                       StateMachineSystem* _stateMachineSystem,
		                       ActorMovementSystem* _actorMovementSystem);
	};
} // sage
