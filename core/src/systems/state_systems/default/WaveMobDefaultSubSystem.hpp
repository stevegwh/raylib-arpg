//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"

namespace sage
{
	class WaveMobDefaultSubSystem : public StateMachineSystem
	{
		ActorMovementSystem* actorMovementSystem;

	public:
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;
		void Update() override;
		void Draw3D(entt::entity entity) override;
		WaveMobDefaultSubSystem(entt::registry* _registry,
		                        ActorMovementSystem* _actorMovementSystem);
	};
} // sage
