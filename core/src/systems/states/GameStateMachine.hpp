//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"
#include <Timer.hpp>

#include <entt/entt.hpp>

#include <vector>

namespace sage
{
    class GameData;

    class GameStateController : public StateMachineController<GameStateController, GameState, GameStateEnum>
    {
        entt::entity gameEntity;

        class DefaultState;
        class WaveState;

        DefaultState* defaultState;
        WaveState* waveState;

      protected:
        StateMachine* GetSystem(GameStateEnum state) override;

      public:
        GameStateController(entt::registry* _registry, GameData* gameData);
        void Update();
        void Draw3D();

        friend class StateMachineController; // Required for CRTP
    };

} // namespace sage
