//
// Created by Steve Wheeler on 04/06/2024.
//

#include "CombatableActor.hpp"

namespace sage
{
    void CombatableActor::EnemyClicked(entt::entity enemy)
    {
        target = enemy;
        onEnemyClicked.publish(self, enemy);
    
    }
    
    void CombatableActor::AttackCancelled()
    {
        onAttackCancelled.publish(self);
    }
    
    void CombatableActor::TargetDeath(entt::entity _target)
    {
        onTargetDeath.publish(self, _target);
        target = entt::null;
    }
} // sage
