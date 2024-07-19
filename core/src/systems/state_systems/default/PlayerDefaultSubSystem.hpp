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
		void Update();
		void Draw3D();
		void OnStateEnter(entt::entity entity);
		void OnStateExit(entt::entity entity);
		PlayerDefaultSubSystem(entt::registry* _registry,
			ActorMovementSystem* _actorMovementSystem);
	};
} // sage
