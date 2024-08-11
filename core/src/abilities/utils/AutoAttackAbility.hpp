#pragma once

#include "Ability.hpp"

namespace sage
{
    // TODO: If you could make these event-based, you could just make these state
    // classes generic

    class AutoAttackAbility : public Ability
    {
        void initStates();

      protected:
        AutoAttackAbility(entt::registry* _registry, AbilityData _abilityData);

      public:
        void Execute(entt::entity self) override;
        void Init(entt::entity self) override;
        void Cancel(entt::entity self);
        ~AutoAttackAbility() override = default;
    };
} // namespace sage