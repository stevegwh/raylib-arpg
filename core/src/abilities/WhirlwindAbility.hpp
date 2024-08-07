#pragma once

#include "Ability.hpp"

namespace sage
{
    struct WhirlwindAbility : public Ability
    {
        Timer windupTimer{};
        float whirlwindRadius = 15.0f;
        void Init(entt::entity self) override;
        void Execute(entt::entity self) override;
        void Update(entt::entity self) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry);
    };
} // namespace sage