#pragma once

#include "utils/Ability.hpp"
#include "utils/CursorAbility.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class NavigationGridSystem;
    class Camera;

    struct PlayerAutoAttack : public Ability
    {
        void Execute(entt::entity self) override;
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry);
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

    struct WavemobAutoAttack : public Ability
    {
        void Execute(entt::entity self) override;
        ~WavemobAutoAttack() override = default;
        WavemobAutoAttack(entt::registry* _registry);
    };

    struct WhirlwindAbility : public Ability
    {
        float whirlwindRadius = 15.0f;
        void Execute(entt::entity self) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry);
    };
} // namespace sage