#pragma once

#include "AutoAttackAbility.hpp"

namespace sage
{
    struct PlayerAutoAttack : public AutoAttackAbility
    {
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry);
    };
} // namespace sage