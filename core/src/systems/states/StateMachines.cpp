#include "StateMachines.hpp"

#include "GameData.hpp"
#include "scenes/Scene.hpp"

namespace sage
{
    void StateMachines::Update()
    {
        gameModeStateMachine->Update();
        wavemobStatemachine->Update();
        playerStateMachine->Update();
        abilityStateMachine->Update();
    }

    void StateMachines::Draw3D()
    {
        wavemobStatemachine->Draw3D();
        playerStateMachine->Draw3D();
        // gameStateMachine->Draw3D();
        abilityStateMachine->Draw3D();
    }

    StateMachines::StateMachines(entt::registry* _registry, GameData* _gameData)
        : gameModeStateMachine(std::make_unique<GameModeStateController>(_registry, _gameData)),
          wavemobStatemachine(std::make_unique<WavemobStateController>(_registry, _gameData)),
          playerStateMachine(std::make_unique<PlayerStateController>(_registry, _gameData)),
          abilityStateMachine(std::make_unique<AbilityStateController>(_registry, _gameData))
    {
    }
} // namespace sage