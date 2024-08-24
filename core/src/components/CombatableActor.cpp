//
// Created by Steve Wheeler on 04/06/2024.
//

#include "CombatableActor.hpp"

namespace sage
{

    CombatableActor::CombatableActor(entt::entity _self) : self(_self)
    //   onAttackCancelledBridge(std::make_unique<EntityEventBridge<entt::entity>>(_self)),
    //   onTargetDeathBridge(std::make_unique<EntityEventBridge<entt::entity>>(_self))
    {
    }
} // namespace sage
