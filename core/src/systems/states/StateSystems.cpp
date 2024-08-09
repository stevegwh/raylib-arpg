#include "StateSystems.hpp"

#include "GameData.hpp"

namespace sage
{
    void StateSystems::Update()
    {
        wavemobStatemachine->Update();
        gameStateMachine->Update();
        playerStateMachine->Update();
    }

    void StateSystems::Draw3D()
    {
        wavemobStatemachine->Draw3D();
        playerStateMachine->Draw3D();
        // gameStateMachine->Draw3D();
    }

    StateSystems::StateSystems(entt::registry* _registry, GameData* _gameData)
    {
        wavemobStatemachine =
            std::make_unique<WavemobStateController>(_registry, _gameData);
        playerStateMachine =
            std::make_unique<PlayerStateController>(_registry, _gameData);

        gameStateMachine = std::make_unique<GameStateController>(_registry, _gameData);
    }
} // namespace sage