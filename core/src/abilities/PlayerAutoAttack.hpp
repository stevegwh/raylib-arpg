#pragma once

#include "utils/AutoAttackAbility.hpp"

namespace sage
{
    struct PlayerAutoAttack : public AutoAttackAbility
    {
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry, Cursor* _cursor);
    };
} // namespace sage