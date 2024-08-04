//
// Created by Steve on 05/06/24.
//

#pragma once

#include "abilities/PlayerAutoAttack.hpp"
#include "components/CombatableActor.hpp"
#include "components/states/PlayerStates.hpp"
#include "systems/state_systems/StateMachineSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "TimerManager.hpp"

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
    class PlayerCombatStateSystem : public StateMachineSystem<PlayerCombatStateSystem, StatePlayerCombat>
    {
        ControllableActorSystem* controllableActorSystem;
        std::unique_ptr<PlayerAutoAttack> autoAttackAbility;
		
        void startCombat(entt::entity self);
        [[nodiscard]] bool checkInCombat(entt::entity self);
        void onDeath(entt::entity self);
        void onTargetDeath(entt::entity self, entt::entity target);
        void onAttackCancel(entt::entity self);

      public:
        void OnEnemyClick(entt::entity self, entt::entity target);
        void Update() override;
        void Draw3D() override;
        void OnHit(AttackData attackData);
        void Enable();
        void Disable();
        void OnStateEnter(entt::entity entity) override;
        void OnStateExit(entt::entity entity) override;

        PlayerCombatStateSystem(entt::registry* _registry, ControllableActorSystem* _controllableActorSystem,
                                CollisionSystem* _collisionSystem, TimerManager* _timerManager);
    };
} // namespace sage
