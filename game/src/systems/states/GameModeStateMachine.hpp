//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachine.hpp"
#include "engine/Timer.hpp"

#include "entt/entt.hpp"

#include <vector>

namespace lq
{
    class Systems;

    class GameModeStateMachine : public sage::StateMachine<GameState, GameStateEnum>
    {
        entt::entity gameEntity;

        class DefaultState;
        class WaveState;
        class CombatState;

      public:
        void Update();
        void Draw3D();
        void StartCombat();

        GameModeStateMachine(entt::registry* _registry);

        friend class StateMachine; // Required for CRTP
    };

} // namespace lq
