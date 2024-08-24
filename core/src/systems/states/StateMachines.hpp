#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData; // forward declaration
    class Scene;    // forward declaration
    class WavemobStateController;
    class GameStateController;
    class PlayerStateController;

    class StateMachines
    {
        // Systems
        WavemobStateController* wavemobStatemachine;
        GameStateController* gameStateMachine;
        PlayerStateController* playerStateMachine;

      public:
        void Update();
        void Draw3D();
        ~StateMachines();
        StateMachines(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage