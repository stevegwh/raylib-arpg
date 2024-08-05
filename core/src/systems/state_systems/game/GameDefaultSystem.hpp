//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "components/states/GameStates.hpp"
#include "systems/state_systems/StateMachine.hpp"
#include <Timer.hpp>

namespace sage
{  

	class GameDefaultSystem : public StateMachine<GameDefaultSystem, StateGameDefault>
	{
		entt::entity gameEntity;
		Timer timer{};
		void OnTimerEnd();
		
	public:
		void Update() override;
		void Draw3D() override;
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;
		GameDefaultSystem(entt::registry* _registry, entt::entity _gameEntity);
	};

} // sage
