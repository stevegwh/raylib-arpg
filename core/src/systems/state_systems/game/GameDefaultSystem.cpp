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
		ChangeState<StateGameWaveattack, GameStates>(gameEntity);
	}
	
	void GameDefaultSystem::Update()
	{

	}
	
	void GameDefaultSystem::Draw3D()
	{

	}
	
	void GameDefaultSystem::OnComponentAdded(entt::entity entity)
	{
    	timerId = timerManager->AddTimerOneshot(5.0f, &GameDefaultSystem::OnTimerEnd, this);
	}
	
	void GameDefaultSystem::OnComponentRemoved(entt::entity entity)
	{
	}

	GameDefaultSystem::GameDefaultSystem(entt::registry* _registry, entt::entity _gameEntity, TimerManager* _timerManager) :
			StateMachine(_registry), gameEntity(_gameEntity), timerManager(_timerManager)
	{
	}
} // sage