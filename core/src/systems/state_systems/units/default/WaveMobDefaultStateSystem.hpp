//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "systems/state_systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "components/states/StateEnemyDefault.hpp"

namespace sage
{
	class WaveMobDefaultStateSystem : public StateMachineSystem<WaveMobDefaultStateSystem, StateEnemyDefault>
	{
		ActorMovementSystem* actorMovementSystem;

	public:
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;
		void Update() override;
		void Draw3D() override;
		
		WaveMobDefaultStateSystem(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem);
	};
} // sage
