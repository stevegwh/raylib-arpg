// Created by Steve Wheeler on 30/06/2024.
#pragma once
#include "components/states/PlayerStates.hpp"
#include "systems/BaseSystem.hpp"
#include "systems/states/StateMachine.hpp"
#include <entt/entt.hpp>
#include <memory>
#include <vector>

namespace sage
{
    class GameData;

    namespace playerstates
    {
        class DefaultState : public StateMachine<DefaultState, StatePlayerDefault>
        {
            GameData* gameData;
            void onEnemyClick(entt::entity self, entt::entity target);
            void onFloorClick(entt::entity self);

          public:
            void Update() override;
            void Draw3D() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~DefaultState() = default;
            DefaultState(entt::registry* _registry, GameData* _gameData);
        };

        class ApproachingTargetState
            : public StateMachine<ApproachingTargetState, StatePlayerApproachingTarget>
        {
            GameData* gameData;
            void onAttackCancel(entt::entity self);
            void onTargetReached(entt::entity self);
            void onEnemyClick(entt::entity self, entt::entity target);

          public:
            void Update() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~ApproachingTargetState() = default;
            ApproachingTargetState(entt::registry* _registry, GameData* gameData);
        };

        class CombatState : public StateMachine<CombatState, StatePlayerCombat>
        {
            GameData* gameData;
            void onTargetDeath(entt::entity self, entt::entity target);
            void onAttackCancel(entt::entity self);
            bool checkInCombat(entt::entity entity);
            void onEnemyClick(entt::entity self, entt::entity target);

          public:
            void Update() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~CombatState() = default;
            CombatState(entt::registry* _registry, GameData* gameData);
        };
    } // namespace playerstates

    class PlayerStateController
    {
      private:
        std::vector<BaseSystem*> systems;

      public:
        std::unique_ptr<playerstates::DefaultState> defaultState;
        std::unique_ptr<playerstates::ApproachingTargetState> approachingTargetState;
        std::unique_ptr<playerstates::CombatState> engagedInCombatState;

        PlayerStateController(entt::registry* _registry, GameData* gameData);
        void Update();
        void Draw3D();
    };
} // namespace sage