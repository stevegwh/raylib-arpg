// Created by Steve Wheeler on 30/06/2024.
#pragma once
#include "components/States.hpp"
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
        class DefaultState : public StateMachine
        {
            GameData* gameData;

          public:
            void OnDeath(entt::entity entity);
            void OnHit(AttackData attackData);
            void Update(entt::entity entity) override;
            void Draw3D(entt::entity entity) override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~DefaultState() = default;
            DefaultState(entt::registry* registry, GameData* _gameData);
        };

        class TargetOutOfRangeState : public StateMachine
        {
            GameData* gameData;

            void onTargetReached(entt::entity self);
            bool isTargetOutOfSight(entt::entity self);

          public:
            void Update(entt::entity entity) override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~TargetOutOfRangeState() = default;
            TargetOutOfRangeState(entt::registry* registry, GameData* _gameData);
        };

        class CombatState : public StateMachine
        {
          private:
            void onTargetDeath(entt::entity self, entt::entity target);
            bool checkInCombat(entt::entity self);

          public:
            void Update(entt::entity entity) override;
            void OnStateEnter(entt::entity self) override;
            void OnStateExit(entt::entity self) override;
            virtual ~CombatState() = default;
            CombatState(entt::registry* _registry);
        };

        class DyingState : public StateMachine
        {
            GameData* gameData;
            void destroyEntity(entt::entity self);

          public:
            void Update(entt::entity entity) override;
            void OnStateEnter(entt::entity self) override;
            void OnStateExit(entt::entity self) override;
            virtual ~DyingState() = default;
            DyingState(entt::registry* _registry, GameData* _gameData);
        };
    } // namespace enemystates

    class WavemobStateController : public StateMachineController<
                                       WavemobStateController,
                                       WavemobState,
                                       WavemobStateEnum>
    {

      protected:
        StateMachine* GetSystem(WavemobStateEnum state) override;

      public:
        std::unique_ptr<enemystates::DefaultState> defaultState;
        std::unique_ptr<enemystates::TargetOutOfRangeState> targetOutOfRangeState;
        std::unique_ptr<enemystates::CombatState> combatState;
        std::unique_ptr<enemystates::DyingState> dyingState;

        WavemobStateController(entt::registry* _registry, GameData* gameData);
        void Update();
        void Draw3D();

        friend class StateMachineController; // Required for CRTP
    };
} // namespace sage