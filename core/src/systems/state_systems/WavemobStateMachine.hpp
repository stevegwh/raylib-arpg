// Created by Steve Wheeler on 30/06/2024.
#pragma once
#include "components/states/EnemyStates.hpp"
#include "systems/BaseSystem.hpp"
#include "systems/state_systems/StateMachine.hpp"

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

    namespace enemystates
    {
        class DefaultState : public StateMachine<DefaultState, StateEnemyDefault>
        {
            ActorMovementSystem* actorMovementSystem;
          public:
            virtual ~DefaultState() = default;
            void Update() override;
            void Draw3D() override;
            void OnHit(AttackData attackData);
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            DefaultState(entt::registry* registry, ActorMovementSystem* actorMovementSystem);
        };

        class TargetOutOfRangeState : public StateMachine<TargetOutOfRangeState, StateEnemyTargetOutOfRange>
        {
            ControllableActorSystem* controllableActorSystem;
            ActorMovementSystem* actorMovementSystem;
            CollisionSystem* collisionSystem;

          public:
            void OnReachedTarget(entt::entity self);
            bool isTargetOutOfSight(entt::entity self);
            void Update() override;
            void OnStateEnter(entt::entity entity) override;
            void OnStateExit(entt::entity entity) override;
            virtual ~TargetOutOfRangeState() = default;
            TargetOutOfRangeState(entt::registry* registry, 
            ControllableActorSystem* controllableActorSystem,
            ActorMovementSystem* actorMovementSystem,
            CollisionSystem* collisionSystem);
        };

        class CombatState : public StateMachine<CombatState, StateEnemyCombat>
        {
          public:
            virtual ~CombatState() = default;
            void Update() override;
            void OnStateEnter(entt::entity self) override;
            void OnStateExit(entt::entity self) override;
            void onTargetDeath(entt::entity self, entt::entity target);
            bool checkInCombat(entt::entity self);
            CombatState(entt::registry* _registry);
        };

        class DyingState : public StateMachine<DyingState, StateEnemyDying>
        {
			ActorMovementSystem* actorMovementSystem;
			void destroyEntity(entt::entity self);
          public:
            virtual ~DyingState() = default;
            void Update() override;
            void OnStateEnter(entt::entity self) override;
            void OnStateExit(entt::entity self) override;
            DyingState(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem);
        };
    } // namespace enemystates

    class WavemobStateController
    {
      private:
        entt::registry* registry;
        Cursor* cursor;
        ControllableActorSystem* controllableActorSystem;
        std::vector<BaseSystem*> systems;

      public:
        std::unique_ptr<enemystates::DefaultState> defaultState;
        std::unique_ptr<enemystates::TargetOutOfRangeState> targetOutOfRangeState;
        std::unique_ptr<enemystates::CombatState> engagedInCombatState;
		std::unique_ptr<enemystates::DyingState> dyingState;

        WavemobStateController(entt::registry* registry, Cursor* cursor,
                              ControllableActorSystem* controllableActorSystem,
                              ActorMovementSystem* actorMovementSystem, CollisionSystem* collisionSystem,
                              NavigationGridSystem* navigationGridSystem);
        void Update();
        void Draw3D();
    };
} // namespace sage