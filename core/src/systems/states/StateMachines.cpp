#include "StateMachines.hpp"

#include "GameData.hpp"
#include "scenes/Scene.hpp"

#include "GameStateMachine.hpp"
#include "PlayerStateMachine.hpp"
#include "WavemobStateMachine.hpp"

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

    StateMachines::~StateMachines()
    {
        delete wavemobStatemachine;
        delete playerStateMachine;
        delete gameStateMachine;
    }

    StateMachines::StateMachines(entt::registry* _registry, GameData* _gameData)
        : wavemobStatemachine(new WavemobStateController(_registry, _gameData)),
          playerStateMachine(new PlayerStateController(_registry, _gameData)),
          gameStateMachine(new GameStateController(_registry, _gameData))
    {
    }
} // namespace sage