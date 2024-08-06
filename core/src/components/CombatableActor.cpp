//
// Created by Steve Wheeler on 04/06/2024.
//

#include "CombatableActor.hpp"

namespace sage
{
    void CombatableActor::EnemyClicked(entt::entity enemy)
    {
        assert(enemy != entt::null);
        onEnemyClicked.publish(self, enemy);
    }

    void CombatableActor::AttackCancelled()
    {
        onAttackCancelled.publish(self);
    }

    void CombatableActor::TargetDeath(entt::entity _target)
    {
        assert(_target != entt::null);
        onTargetDeath.publish(self, _target);
    }

    CombatableActor::CombatableActor(entt::entity _self) : self(_self)
    {
    }
} // namespace sage
