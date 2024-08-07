// Created by Steve Wheeler on 30/06/2024.
#pragma once
#include "components/states/EnemyStates.hpp"
#include "systems/BaseSystem.hpp"
#include "systems/states/StateMachine.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <vector>

namespace sage
{
    class GameData;
    struct AttackData;

    namespace enemystates
    {
        class DefaultState : public StateMachine<DefaultState, StateEnemyDefault>
        {
            GameData* gameData;

          public:
            void OnHit(AttackData attackData);
            void Update() override;
            void Draw3D() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~DefaultState() = default;
            DefaultState(entt::registry* registry, GameData* _gameData);
        };

        class TargetOutOfRangeState
            : public StateMachine<TargetOutOfRangeState, StateEnemyTargetOutOfRange>
        {
            GameData* gameData;

            void onTargetReached(entt::entity self);
            bool isTargetOutOfSight(entt::entity self);

          public:
            void Update() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~TargetOutOfRangeState() = default;
            TargetOutOfRangeState(entt::registry* registry, GameData* _gameData);
        };

        class CombatState : public StateMachine<CombatState, StateEnemyCombat>
        {
          private:
            void onTargetDeath(entt::entity self, entt::entity target);
            bool checkInCombat(entt::entity self);

          public:
            void Update() override;
            void OnStateEnter(entt::entity self) override;
            void OnStateExit(entt::entity self) override;
            virtual ~CombatState() = default;
            CombatState(entt::registry* _registry);
        };

        class DyingState : public StateMachine<DyingState, StateEnemyDying>
        {
            GameData* gameData;
            void destroyEntity(entt::entity self);

          public:
            void Update() override;
            void OnStateEnter(entt::entity self) override;
            void OnStateExit(entt::entity self) override;
            virtual ~DyingState() = default;
            DyingState(entt::registry* _registry, GameData* _gameData);
        };
    } // namespace enemystates

    class WavemobStateController
    {
      private:
        entt::registry* registry;
        std::vector<BaseSystem*> systems;

      public:
        std::unique_ptr<enemystates::DefaultState> defaultState;
        std::unique_ptr<enemystates::TargetOutOfRangeState> targetOutOfRangeState;
        std::unique_ptr<enemystates::CombatState> engagedInCombatState;
        std::unique_ptr<enemystates::DyingState> dyingState;

        WavemobStateController(entt::registry* registry, GameData* _gameData);
        void Update();
        void Draw3D();
    };
} // namespace sage