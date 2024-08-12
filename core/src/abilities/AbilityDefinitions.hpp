#pragma once

#include "utils/Ability.hpp"
#include "utils/CursorAbility.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class NavigationGridSystem;
    class Camera;

    class PlayerAutoAttack : public Ability
    {
        static constexpr AbilityData playerAutoAbilityData{
            .baseData =
                {.element = AttackElement::PHYSICAL,
                 .cooldownDuration = 1,
                 .baseDamage = 10,
                 .range = 5,
                 .repeatable = true},
            .animationParams = {.animationDelay = 0}};

      public:
        void Execute(entt::entity self) override;
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry);
    };

    class RainOfFire : public CursorAbility
    {
        static constexpr AbilityData rainoffireAbilityData{
            .baseData =
                {.cooldownDuration = 3,
                 .range = 5,
                 .baseDamage = 25,
                 .element = AttackElement::FIRE,
                 .repeatable = false},
            .animationParams = {
                .animEnum = AnimationEnum::SPIN,
                .animSpeed = 1,
                .oneShot = true,
                .animationDelay = 0.75f}};

      public:
        RainOfFire(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            NavigationGridSystem* _navigationGridSystem);
    };

    class WavemobAutoAttack : public Ability
    {

        static constexpr AbilityData wavemobAutoAbilityData{
            .baseData = {
                .cooldownDuration = 1,
                .range = 5,
                .baseDamage = 10,
                .element = AttackElement::PHYSICAL,
                .repeatable = true}};

      public:
        void Execute(entt::entity self) override;
        ~WavemobAutoAttack() override = default;
        WavemobAutoAttack(entt::registry* _registry);
    };

    class WhirlwindAbility : public Ability
    {
        static constexpr AbilityData whirlwindAbilityData{
            .baseData =
                {.cooldownDuration = 3,
                 .range = 5,
                 .baseDamage = 25,
                 .element = AttackElement::PHYSICAL,
                 .repeatable = false},
            .animationParams = {
                .animEnum = AnimationEnum::SPIN,
                .animSpeed = 1,
                .oneShot = true,
                .animationDelay = 0.65f}};

      public:
        float whirlwindRadius = 15.0f;
        void Execute(entt::entity self) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry);
    };
} // namespace sage