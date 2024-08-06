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
    class ActorMovementSystem;
    class Cursor;
    class ControllableActorSystem;
    class NavigationGridSystem;
    class CollisionSystem;
    struct AttackData;

    namespace playerstates
    {
        class DefaultState : public StateMachine<DefaultState, StatePlayerDefault>
        {
            ActorMovementSystem* actorMovementSystem;

            void onEnemyClick(entt::entity self, entt::entity target);

          public:
            void Update() override;
            void Draw3D() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~DefaultState() = default;
            DefaultState(
                entt::registry* registry, ActorMovementSystem* actorMovementSystem);
        };

        class ApproachingTargetState
            : public StateMachine<ApproachingTargetState, StatePlayerApproachingTarget>
        {
            ControllableActorSystem* controllableActorSystem;

            void onAttackCancel(entt::entity self);
            void onTargetReached(entt::entity self);
            void onEnemyClick(entt::entity self, entt::entity target);

          public:
            void Update() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~ApproachingTargetState() = default;
            ApproachingTargetState(
                entt::registry* registry,
                ControllableActorSystem* controllableActorSystem);
        };

        class CombatState : public StateMachine<CombatState, StatePlayerCombat>
        {
            void onTargetDeath(entt::entity self, entt::entity target);
            void onAttackCancel(entt::entity self);
            bool checkInCombat(entt::entity entity);
            void onEnemyClick(entt::entity self, entt::entity target);

          public:
            void Update() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~CombatState() = default;

            CombatState(entt::registry* registry);
        };
    } // namespace playerstates

    class PlayerStateController
    {
      private:
        entt::registry* registry;
        Cursor* cursor;
        ControllableActorSystem* controllableActorSystem;
        std::vector<BaseSystem*> systems;

      public:
        std::unique_ptr<playerstates::DefaultState> defaultState;
        std::unique_ptr<playerstates::ApproachingTargetState> approachingTargetState;
        std::unique_ptr<playerstates::CombatState> engagedInCombatState;

        PlayerStateController(
            entt::registry* registry,
            Cursor* cursor,
            ControllableActorSystem* controllableActorSystem,
            ActorMovementSystem* actorMovementSystem,
            CollisionSystem* collisionSystem,
            NavigationGridSystem* navigationGridSystem);
        void Update();
        void Draw3D();
    };
} // namespace sage