// Created by Steve Wheeler on 30/06/2024.

#pragma once

#include "entt/entt.hpp"

#include "systems/state_systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "components/states/PlayerStates.hpp"

namespace sage
{
	class PlayerDefaultStateSystem : public StateMachine<PlayerDefaultStateSystem, StatePlayerDefault>
	{
		ActorMovementSystem* actorMovementSystem;

	public:
		void Update() override;
		void Draw3D() override;
		void OnComponentAdded(entt::entity entity) override;
		void OnComponentRemoved(entt::entity entity) override;
		PlayerDefaultStateSystem(entt::registry* _registry,
			ActorMovementSystem* _actorMovementSystem);
	};
} // sage
