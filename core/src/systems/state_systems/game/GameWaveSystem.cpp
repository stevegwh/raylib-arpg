//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameData.hpp"
#include "GameWaveSystem.hpp"
#include "GameObjectFactory.hpp"
#include <iostream>

namespace sage
{
	void GameWaveSystem::initWave()
	{
		// TODO: This is far, far too slow. Need to buffer the enemies models or share mesh data
		auto enemy2 = GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 10.0f}, "Enemy");
		auto enemy3 = GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 20.0f}, "Enemy");
		auto enemy4 = GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 30.0f}, "Enemy");
		auto enemy5 = GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 40.0f}, "Enemy");
	}
	
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
		initWave();
	}
	
	void GameWaveSystem::OnStateExit(entt::entity entity)
	{

	}
	
	GameWaveSystem::GameWaveSystem(
			entt::registry* _registry,
			GameData* _gameData,
			entt::entity _gameEntity, 
			TimerManager* _timerManager) :
			StateMachineSystem(_registry),
			gameData(_gameData)
	{
	}
} // sage