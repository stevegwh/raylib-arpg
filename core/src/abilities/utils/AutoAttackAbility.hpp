#pragma once

#include "Ability.hpp"

namespace sage
{
    class AutoAttackAbility : public Ability
    {
      protected:
        AutoAttackAbility(
            entt::registry* _registry, AbilityData _abilityData, Cursor* _cursor);

      public:
        void Execute(entt::entity self) override;
        void Init(entt::entity self) override;
        ~AutoAttackAbility() override = default;
    };
} // namespace sage