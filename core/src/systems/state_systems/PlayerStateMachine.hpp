// Created by Steve Wheeler on 30/06/2024.

#pragma once

#include <entt/entt.hpp>

#include "components/states/PlayerStates.hpp"
#include "systems/BaseSystem.hpp"
#include "systems/state_systems/StateMachine.hpp"

#include <memory>
#include <vector>

namespace sage
{
    class ActorMovementSystem;
    class Cursor;
    class ControllableActorSystem;
    class NavigationGridSystem;
    class CollisionSystem;
    class TimerManager;
    struct AttackData;

    namespace playerstates
    {
        class DefaultState : public StateMachine<DefaultState, StatePlayerDefault>
        {
            ActorMovementSystem* actorMovementSystem;

          public:
            virtual ~DefaultState() = default;
            void Update() override;
            void Draw3D() override;
            void OnComponentAdded(entt::entity entity) override;
            void OnComponentRemoved(entt::entity entity) override;
            DefaultState(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem);
        };

        class InCombatState : public StateMachine<InCombatState, StatePlayerCombat>
        {
            ControllableActorSystem* controllableActorSystem;

            void startCombat(entt::entity self);
            [[nodiscard]] bool checkInCombat(entt::entity self);
            void onDeath(entt::entity self);
            void onTargetDeath(entt::entity self, entt::entity target);
            void onAttackCancel(entt::entity self);

          public:
            virtual ~InCombatState() = default;
            void OnEnemyClick(entt::entity self, entt::entity target);
            void OnHit(AttackData attackData);
            void Enable();
            void Disable();
            void Update() override;
            void Draw3D() override;
            void OnComponentAdded(entt::entity self) override;
            void OnComponentRemoved(entt::entity self) override;
            InCombatState(entt::registry* _registry, ControllableActorSystem* _controllableActorSystem,
                          CollisionSystem* _collisionSystem, TimerManager* _timerManager);
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
        std::unique_ptr<playerstates::InCombatState> inCombatState;

        PlayerStateController(entt::registry* _registry, Cursor* _cursor,
                              ControllableActorSystem* _controllableActorSystem,
                              ActorMovementSystem* _actorMovementSystem, CollisionSystem* _collisionSystem,
                              NavigationGridSystem* _navigationGridSystem, TimerManager* _timerManager);

        void Update();
        void Draw3D();
    };

} // namespace sage
