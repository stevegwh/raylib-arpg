//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "systems/state_systems/StateMachine.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "components/states/EnemyStates.hpp"

namespace sage
{
	class WaveMobDefaultStateSystem : public StateMachine<WaveMobDefaultStateSystem, StateEnemyDefault>
	{
		ActorMovementSystem* actorMovementSystem;

	public:
		void OnComponentAdded(entt::entity entity) override;
		void OnComponentRemoved(entt::entity entity) override;
		void Update() override;
		void Draw3D() override;
		
		WaveMobDefaultStateSystem(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem);
	};
} // sage
