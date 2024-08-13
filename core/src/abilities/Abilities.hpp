#pragma once

#include "Ability.hpp"
#include "CursorAbility.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class NavigationGridSystem;
    class Camera;

    class PlayerAutoAttack : public Ability
    {
        AbilityData initAbilityData(entt::registry* registry);

      public:
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry, Camera* _camera);
    };

    class RainOfFire : public CursorAbility
    {
        AbilityData initAbilityData(entt::registry* registry, Cursor* cursor);

      public:
        RainOfFire(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            NavigationGridSystem* _navigationGridSystem);
    };

    class WavemobAutoAttack : public Ability
    {
        AbilityData initAbilityData(entt::registry* registry);

      public:
        ~WavemobAutoAttack() override = default;
        WavemobAutoAttack(entt::registry* _registry, Camera* _camera);
    };

    class WhirlwindAbility : public Ability
    {
        AbilityData initAbilityData(entt::registry* registry);

      public:
        float whirlwindRadius = 15.0f;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry, Camera* _camera);
    };
} // namespace sage