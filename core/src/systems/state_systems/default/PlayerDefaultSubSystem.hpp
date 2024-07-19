// Created by Steve Wheeler on 30/06/2024.

#pragma once

#include "entt/entt.hpp"

#include "systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"

namespace sage
{
	class PlayerDefaultSubSystem : public StateMachineSystem
	{
		ActorMovementSystem* actorMovementSystem;

	public:
		void Update() override;
		void Draw3D(entt::entity entity) override;
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;
		PlayerDefaultSubSystem(entt::registry* _registry,
			ActorMovementSystem* _actorMovementSystem);
	};
} // sage
