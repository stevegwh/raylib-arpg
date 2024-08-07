#pragma once

#include "Ability.hpp"

#include <entt/entt.hpp>

namespace sage
{
    struct WavemobAutoAttack : public Ability
    {
        void Execute(entt::entity self) override;
        void Update(entt::entity self) override;
        void Init(entt::entity self) override;
        void Cancel();
        ~WavemobAutoAttack() override = default;
        WavemobAutoAttack(entt::registry* _registry);
    };
} // namespace sage