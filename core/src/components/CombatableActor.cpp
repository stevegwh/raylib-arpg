//
// Created by Steve Wheeler on 04/06/2024.
//

#include "CombatableActor.hpp"

namespace sage
{

    CombatableActor::CombatableActor(entt::entity _self)
        : self(_self),
          onAttackCancelled(std::make_unique<EntityEventBridge<entt::entity>>(_self)),
          onTargetDeath(std::make_unique<EntityEventBridge<entt::entity>>(_self))
    {
        // onAttackCancelled->SetOutSignal(onAttackCancelledSig);
        // onTargetDeath->SetOutSignal(onTargetDeathSig);
    }
} // namespace sage
