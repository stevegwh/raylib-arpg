#pragma once

#include "AutoAttackAbility.hpp"

#include <entt/entt.hpp>

namespace sage
{
    struct WavemobAutoAttack : public AutoAttackAbility
    {
        ~WavemobAutoAttack() override = default;
        WavemobAutoAttack(entt::registry* _registry);
    };
} // namespace sage