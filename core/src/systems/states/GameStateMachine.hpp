//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "components/states/States.hpp"
#include "systems/states/StateMachine.hpp"
#include <Timer.hpp>

#include <entt/entt.hpp>

#include <memory>
#include <vector>

namespace sage
{
    class GameData;

    namespace gamestates
    {
        class DefaultState : public StateMachine
        {
            entt::entity gameEntity;
            Timer timer{};
            void OnTimerEnd();

          public:
            void Update(entt::entity entity) override;
            void Draw3D(entt::entity entity) override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            DefaultState(entt::registry* _registry, entt::entity _gameEntity);
        };

        class WaveState : public StateMachine
        {
            GameData* gameData;
            void initWave();
            void OnTimerEnd();

          public:
            void Update(entt::entity entity) override;
            void Draw3D(entt::entity entity) override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            WaveState(
                entt::registry* _registry, GameData* _gameData, entt::entity _gameEntity);
        };
    } // namespace gamestates

    class GameStateController
        : public StateMachineController<GameStateController, GameState, GameStateEnum>
    {
        entt::entity gameEntity;

      protected:
        StateMachine* GetSystem(GameStateEnum state) override;

      public:
        std::unique_ptr<gamestates::DefaultState> defaultState;
        std::unique_ptr<gamestates::WaveState> waveState;

        GameStateController(entt::registry* _registry, GameData* gameData);
        void Update();
        void Draw3D();

        friend class StateMachineController; // Required for CRTP
    };

} // namespace sage
