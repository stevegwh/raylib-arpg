#pragma once

#include "utils/AutoAttackAbility.hpp"

namespace sage
{
    struct WhirlwindAbility : public AutoAttackAbility
    {
        float whirlwindRadius = 15.0f;
        void Execute(entt::entity self) override;
        void Init(entt::entity self) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry);
    };
} // namespace sage