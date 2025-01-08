//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"
#include <Timer.hpp>

#include "entt/entt.hpp"

#include <vector>

namespace sage
{
    class Systems;

    class GameModeStateController : public StateMachineController<GameState, GameStateEnum>
    {
        entt::entity gameEntity;

        class DefaultState;
        class WaveState;
        class CombatState;

      public:
        void Update();
        void Draw3D();
        void StartCombat();

        GameModeStateController(entt::registry* _registry, Systems* sys);

        friend class StateMachineController; // Required for CRTP
    };

} // namespace sage
