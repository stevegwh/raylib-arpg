#pragma once

#include "AbilityStateMachine.hpp"
#include "GameModeStateMachine.hpp"
#include "PlayerStateMachine.hpp"
#include "WavemobStateMachine.hpp"

#include <entt/entt.hpp>
#include <memory>

namespace sage
{
    class GameData; // forward declaration
    class Scene;    // forward declaration

    class StateMachines
    {
      public:
        // Systems
        std::unique_ptr<GameModeStateController> gameModeStateMachine;
        std::unique_ptr<WavemobStateController> wavemobStatemachine;
        std::unique_ptr<PlayerStateController> playerStateMachine;
        std::unique_ptr<AbilityStateController> abilityStateMachine;
        void Update();
        void Draw3D();
        StateMachines(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage