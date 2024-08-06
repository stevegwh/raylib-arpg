//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "components/states/GameStates.hpp"
#include "systems/states/StateMachine.hpp"
#include <Timer.hpp>

#include <entt/entt.hpp>

#include <memory>
#include <vector>

namespace sage
{
    class GameData;
    class BaseSystem;

    namespace gamestates
    {
        class DefaultState : public StateMachine<DefaultState, StateGameDefault>
        {
            entt::entity gameEntity;
            Timer timer{};
            void OnTimerEnd();

          public:
            void Update() override;
            void Draw3D() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            DefaultState(entt::registry* _registry, entt::entity _gameEntity);
        };

        class WaveState : public StateMachine<WaveState, StateGameWave>
        {
            GameData* gameData;
            void initWave();
            void OnTimerEnd();

          public:
            void Update() override;
            void Draw3D() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            WaveState(
                entt::registry* _registry, GameData* _gameData, entt::entity _gameEntity);
        };
    } // namespace gamestates

    class GameStateController
    {
        entt::entity gameEntity;
        std::vector<BaseSystem*> systems;

      public:
        std::unique_ptr<gamestates::DefaultState> defaultState;
        std::unique_ptr<gamestates::WaveState> waveState;
        void Update();
        GameStateController(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
