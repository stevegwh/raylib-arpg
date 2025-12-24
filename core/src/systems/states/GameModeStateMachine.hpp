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

    class GameModeStateMachine : public StateMachine<GameState, GameStateEnum>
    {
        entt::entity gameEntity;

        class DefaultState;
        class WaveState;
        class CombatState;

      public:
        void Update();
        void Draw3D();
        void StartCombat();

        GameModeStateMachine(entt::registry* _registry, Systems* sys);

        friend class StateMachine; // Required for CRTP
    };

} // namespace sage
