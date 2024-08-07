#include "StateMachines.hpp"

#include "GameData.hpp"

namespace sage
{
    void StateMachines::Update()
    {
        wavemobStatemachine->Update();
        gameStateMachine->Update();
        playerStateMachine->Update();
    }

    void StateMachines::Draw3D()
    {
        wavemobStatemachine->Draw3D();
        playerStateMachine->Draw3D();
        // gameStateMachine->Draw3D();
    }

    StateMachines::StateMachines(entt::registry* _registry, GameData* _gameData)
    {

        wavemobStatemachine =
            std::make_unique<WavemobStateController>(_registry, _gameData);
        playerStateMachine =
            std::make_unique<PlayerStateController>(_registry, _gameData);

        gameStateMachine = std::make_unique<GameStateController>(_registry, _gameData);
    }
} // namespace sage