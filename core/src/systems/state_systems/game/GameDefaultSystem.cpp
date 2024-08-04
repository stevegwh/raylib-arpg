//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameDefaultSystem.hpp"
#include "components/states/GameStates.hpp"
#include <iostream>
#include <tuple>


namespace sage
{

	void GameDefaultSystem::OnTimerEnd()
	{
		std::cout << "Timer ended! \n";
		ChangeState<StateGameWaveattack, GameStates>(gameEntity);
	}
	
	void GameDefaultSystem::Update()
	{

	}
	
	void GameDefaultSystem::Draw3D()
	{

	}
	
	void GameDefaultSystem::OnStateEnter(entt::entity entity)
	{
    	timerId = timerManager->AddTimer(5.0f, &GameDefaultSystem::OnTimerEnd, this);
	}
	
	void GameDefaultSystem::OnStateExit(entt::entity entity)
	{
		timerManager->RemoveTimer(timerId);
	}

	GameDefaultSystem::GameDefaultSystem(entt::registry* _registry, entt::entity _gameEntity, TimerManager* _timerManager) :
			StateMachineSystem(_registry), gameEntity(_gameEntity), timerManager(_timerManager)
	{
	}
} // sage