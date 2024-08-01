//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameStateSystem.hpp"
#include "GameDefaultSystem.hpp"

namespace sage
{
	void GameStateSystem::Update()
	{
		for (auto& system : systems)
		{
			system->Update();
		}
	}
	GameStateSystem::GameStateSystem(entt::registry* _registry, TimerManager* _timerManager)
	{
		gameEntity = _registry->create();
		defaultSystem = std::make_unique<GameDefaultSystem>(_registry, gameEntity, _timerManager);
		waveSystem = std::make_unique<GameWaveSystem>(_registry, gameEntity, _timerManager);
		systems.push_back(defaultSystem.get());
		systems.push_back(waveSystem.get());
		// TODO: Seems like a hacky way to do this
		defaultSystem->ChangeState<StateGameDefault, GameStates>(gameEntity);
	}
} // sage