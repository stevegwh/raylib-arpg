//
// Created by Steve Wheeler on 04/06/2024.
//

#include "CombatableActor.hpp"

namespace sage
{

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
        onAttackCancelledb = std::make_unique<EntityEventBridge<entt::entity>>(_self);
    }
} // namespace sage
