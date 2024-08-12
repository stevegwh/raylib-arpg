#include "AbilityDefinitions.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "AbilityFunctions.hpp"

#include "raylib.h"

namespace sage
{

    AbilityData PlayerAutoAttack::initAbilityData(entt::registry* _registry)
    {
        static AbilityData playerAutoAbilityData{
            .baseData =
                {.element = AttackElement::PHYSICAL,
                 .cooldownDuration = 1,
                 .baseDamage = 10,
                 .range = 5,
                 .repeatable = true},
            .animationParams = {.animationDelay = 0}};

        playerAutoAbilityData.executeFunc =
            AbilityLibrary::GetInstance(_registry).GetAbility("PlayerAutoAttack");
        return playerAutoAbilityData;
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry)
        : Ability(_registry, initAbilityData(_registry))
    {
    }

    AbilityData RainOfFire::initAbilityData(entt::registry* _registry)
    {
        static AbilityData rainoffireAbilityData{
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

        rainoffireAbilityData.executeFunc =
            AbilityLibrary::GetInstance(_registry).GetAbility("RainOfFire");
        return rainoffireAbilityData;
    }

    RainOfFire::RainOfFire(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        NavigationGridSystem* _navigationGridSystem)
        : CursorAbility(
              _registry,
              _camera,
              _cursor,
              std::make_unique<TextureTerrainOverlay>(
                  _registry,
                  _navigationGridSystem,
                  "resources/textures/cursor/rainoffire_cursor.png",
                  Color{255, 215, 0, 255},
                  "resources/shaders/glsl330/bloom.fs"),
              initAbilityData(_registry))
    {
    }

    AbilityData WavemobAutoAttack::initAbilityData(entt::registry* _registry)
    {
        static AbilityData wavemobAutoAbilityData{
            .baseData = {
                .cooldownDuration = 1,
                .range = 5,
                .baseDamage = 10,
                .element = AttackElement::PHYSICAL,
                .repeatable = true}};

        wavemobAutoAbilityData.executeFunc =
            AbilityLibrary::GetInstance(_registry).GetAbility("WavemobAutoAttack");
        return wavemobAutoAbilityData;
    }

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry)
        : Ability(_registry, initAbilityData(_registry))
    {
    }

    AbilityData WhirlwindAbility::initAbilityData(entt::registry* _registry)
    {
        static AbilityData whirlwindAbilityData{
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

        whirlwindAbilityData.executeFunc =
            AbilityLibrary::GetInstance(_registry).GetAbility("Whirlwind");

        return whirlwindAbilityData;
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry)
        : Ability(_registry, initAbilityData(_registry))
    {
    }

} // namespace sage