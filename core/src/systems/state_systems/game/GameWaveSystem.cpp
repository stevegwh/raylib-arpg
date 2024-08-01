//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameWaveSystem.hpp"
#include "components/states/StateGameWaveattack.hpp"
#include <iostream>

namespace sage
{
	void GameWaveSystem::Update()
	{

	}
	
	void GameWaveSystem::Draw3D()
	{

	}
	
	void GameWaveSystem::OnStateEnter(entt::entity entity)
	{
		// Create waves here (enemies etc)
		std::cout << "Wave state entered! \n";
	}
	
	void GameWaveSystem::OnStateExit(entt::entity entity)
	{

	}
	
	GameWaveSystem::GameWaveSystem(entt::registry* _registry, entt::entity _gameEntity, TimerManager* _timerManager) :
			StateMachineSystem(_registry)
	{
		registry->on_construct<StateGameWaveattack>().connect<&GameWaveSystem::OnStateEnter>(this);
		registry->on_destroy<StateGameWaveattack>().connect<&GameWaveSystem::OnStateExit>(this);
	}
} // sage