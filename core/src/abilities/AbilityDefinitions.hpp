#pragma once

#include "utils/AutoAttackAbility.hpp"
#include "utils/CursorAbility.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class NavigationGridSystem;
    class Camera;

    struct PlayerAutoAttack : public AutoAttackAbility
    {
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry, Cursor* _cursor);
    };

    class RainOfFire : public CursorAbility
    {
      public:
        RainOfFire(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            NavigationGridSystem* _navigationGridSystem);
    };

    struct WavemobAutoAttack : public AutoAttackAbility
    {
        ~WavemobAutoAttack() override = default;
        WavemobAutoAttack(entt::registry* _registry, Cursor* _cursor);
    };

    struct WhirlwindAbility : public AutoAttackAbility
    {
        float whirlwindRadius = 15.0f;
        void Execute(entt::entity self) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry, Cursor* _cursor);
    };
} // namespace sage