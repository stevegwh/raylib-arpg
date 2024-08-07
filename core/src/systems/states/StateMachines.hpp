#pragma once

#include "GameStateMachine.hpp"
#include "PlayerStateMachine.hpp"
#include "WavemobStateMachine.hpp"

#include <memory>

namespace sage
{
    class GameData; // forward declaration
    class StateMachines
    {

      public:
        // Systems
        std::unique_ptr<WavemobStateController> wavemobStatemachine;
        std::unique_ptr<GameStateController> gameStateMachine;
        std::unique_ptr<PlayerStateController> playerStateMachine;
        void Update();
        void Draw3D();
        StateMachines(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage