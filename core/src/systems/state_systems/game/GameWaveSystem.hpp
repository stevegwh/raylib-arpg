//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "TimerManager.hpp"

#include "components/states/GameStates.hpp"
#include "systems/state_systems/StateMachineSystem.hpp"

namespace sage
{

	class GameWaveSystem : public StateMachineSystem<GameWaveSystem, StateGameWaveattack>
	{
		TimerManager* timerManager;
		void OnTimerEnd();
		entt::delegate<void()> callback;
		// Countdown to next wave
		// Swap state when timer reached

	public:
		GameWaveSystem(entt::registry* _registry, entt::entity _gameEntity, TimerManager* _timerManager);
		void Update() override;
		void Draw3D() override;
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;
	};

} // sage
