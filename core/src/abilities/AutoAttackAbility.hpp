#pragma once

#include "Ability.hpp"

namespace sage
{
    class AutoAttackAbility : public Ability
    {
      protected:
        AutoAttackAbility(entt::registry* _registry, AbilityData _abilityData);

      public:
        void Execute(entt::entity self) override;
        void Update(entt::entity self) override;
        void Init(entt::entity self) override;
        void Cancel();
        ~AutoAttackAbility() override = default;
    };
} // namespace sage