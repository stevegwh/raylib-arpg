// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/PlayerState.hpp"
#include "systems/states/StateMachine.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <vector>

namespace sage
{
    class GameData;

    namespace playerstates
    {
        class DefaultState : public StateMachine
        {
            GameData* gameData;
            void onEnemyClick(entt::entity self, entt::entity target);
            void onFloorClick(entt::entity self);

          public:
            void Update(entt::entity entity) override;
            void Draw3D(entt::entity entity) override;
            void OnStateEnter(entt::entity entity) override;
            virtual ~DefaultState() = default;
            DefaultState(entt::registry* _registry, GameData* _gameData);
        };

        class MovingToTalkToNPCState : public StateMachine
        {
            GameData* gameData;
            // void onMoveCancel(entt::entity self);
            void onTargetReached(entt::entity self);

          public:
            void Update(entt::entity entity) override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~MovingToTalkToNPCState() = default;
            MovingToTalkToNPCState(entt::registry* _registry, GameData* gameData);
        };

        class MovingToAttackEnemyState : public StateMachine
        {
            GameData* gameData;
            // void onAttackCancel(entt::entity self);
            void onTargetReached(entt::entity self);
            // void onEnemyClick(entt::entity self, entt::entity target);

          public:
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~MovingToAttackEnemyState() = default;
            MovingToAttackEnemyState(entt::registry* _registry, GameData* gameData);
        };

        class CombatState : public StateMachine
        {
            GameData* gameData;
            void onTargetDeath(entt::entity self, entt::entity target);
            // void onAttackCancel(entt::entity self);
            bool checkInCombat(entt::entity entity);
            // void onEnemyClick(entt::entity self, entt::entity target);

          public:
            void Update(entt::entity entity) override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~CombatState() = default;
            CombatState(entt::registry* _registry, GameData* gameData);
        };
    } // namespace playerstates

    class PlayerStateController : public StateMachineController<
                                      PlayerStateController,
                                      PlayerState,
                                      PlayerStateEnum>
    {

      protected:
        StateMachine* GetSystem(PlayerStateEnum state) override;
        void OnComponentRemoved(entt::entity entity) override;
        void OnComponentAdded(entt::entity entity) override;

      public:
        std::unique_ptr<playerstates::DefaultState> defaultState;
        std::unique_ptr<playerstates::MovingToAttackEnemyState> approachingTargetState;
        std::unique_ptr<playerstates::CombatState> engagedInCombatState;

        PlayerStateController(entt::registry* _registry, GameData* gameData);
        void Update();
        void Draw3D();

        friend class StateMachineController; // Required for CRTP
    };
} // namespace sage