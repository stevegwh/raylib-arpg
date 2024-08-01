//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameDefaultSystem.hpp"
#include "components/states/GameStateComponents.hpp"
#include <iostream>

namespace sage
{

	void GameDefaultSystem::OnTimerEnd()
	{
		std::cout << "Timer ended! \n";
		ChangeState<StateGameWaveattack, StateComponents>(gameEntity);
	}
	
	void GameDefaultSystem::Update()
	{

	}
	
	void GameDefaultSystem::Draw3D()
	{

	}
	
	void GameDefaultSystem::OnStateEnter(entt::entity entity)
	{
		timerId = timerManager->AddTimer(5.0f, callback);
	}
	
	void GameDefaultSystem::OnStateExit(entt::entity entity)
	{
		timerManager->RemoveTimer(timerId);
	}

	GameDefaultSystem::GameDefaultSystem(entt::registry* _registry, entt::entity _gameEntity, TimerManager* _timerManager) :
			StateMachineSystem(_registry), gameEntity(_gameEntity), timerManager(_timerManager)
	{
		callback.connect<&GameDefaultSystem::OnTimerEnd>(this);
		registry->on_construct<StateGameDefault>().connect<&GameDefaultSystem::OnStateEnter>(this);
		registry->on_destroy<StateGameDefault>().connect<&GameDefaultSystem::OnStateExit>(this);
		// entt::delegate<void()> _callback, float _duration
	}
} // sage