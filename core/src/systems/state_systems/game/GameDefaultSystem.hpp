//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "TimerManager.hpp"

#include "components/states/GameStates.hpp"
#include "systems/state_systems/StateMachine.hpp"

namespace sage
{  

	class GameDefaultSystem : public StateMachine<GameDefaultSystem, StateGameDefault>
	{
		entt::entity gameEntity;
		int timerId = -1;
		TimerManager* timerManager;
		void OnTimerEnd();
		// Countdown to next wave
		// Swap state when timer reached
		
	public:
		void Update() override;
		void Draw3D() override;
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;
		GameDefaultSystem(entt::registry* _registry, entt::entity _gameEntity, TimerManager* _timerManager);
	};

} // sage
