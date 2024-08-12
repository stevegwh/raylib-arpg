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
        // TODO: Fairly sure the issue is that below you are overwriting defaults, causing
        // weird behaviour
        static AbilityData playerAutoAbilityData;
        playerAutoAbilityData.baseData.element = AttackElement::PHYSICAL;
        playerAutoAbilityData.baseData.cooldownDuration = 1;
        playerAutoAbilityData.baseData.baseDamage = 10;
        playerAutoAbilityData.baseData.range = 5;
        playerAutoAbilityData.baseData.repeatable = true;

        playerAutoAbilityData.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        playerAutoAbilityData.animationParams.animSpeed = 4;
        playerAutoAbilityData.animationParams.animationDelay = 0;

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
        static AbilityData rainoffireAbilityData;

        rainoffireAbilityData.baseData.cooldownDuration = 3;
        rainoffireAbilityData.baseData.range = 5;
        rainoffireAbilityData.baseData.baseDamage = 25;
        rainoffireAbilityData.baseData.element = AttackElement::FIRE;
        rainoffireAbilityData.baseData.repeatable = false;

        rainoffireAbilityData.animationParams.animEnum = AnimationEnum::SPIN;
        rainoffireAbilityData.animationParams.animSpeed = 1;
        rainoffireAbilityData.animationParams.oneShot = true;
        rainoffireAbilityData.animationParams.animationDelay = 0.75f;

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
        static AbilityData wavemobAutoAbilityData;

        wavemobAutoAbilityData.baseData.cooldownDuration = 1;
        wavemobAutoAbilityData.baseData.range = 5;
        wavemobAutoAbilityData.baseData.baseDamage = 10;
        wavemobAutoAbilityData.baseData.element = AttackElement::PHYSICAL;
        wavemobAutoAbilityData.baseData.repeatable = true;

        wavemobAutoAbilityData.animationParams.animEnum = AnimationEnum::AUTOATTACK;

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
        static AbilityData whirlwindAbilityData;

        whirlwindAbilityData.baseData.cooldownDuration = 3;
        whirlwindAbilityData.baseData.range = 5;
        whirlwindAbilityData.baseData.baseDamage = 25;
        whirlwindAbilityData.baseData.element = AttackElement::PHYSICAL;
        whirlwindAbilityData.baseData.repeatable = false;

        whirlwindAbilityData.animationParams.animEnum = AnimationEnum::SPIN;
        whirlwindAbilityData.animationParams.animSpeed = 1;
        whirlwindAbilityData.animationParams.oneShot = true;
        whirlwindAbilityData.animationParams.animationDelay = 0.65f;

        whirlwindAbilityData.executeFunc =
            AbilityLibrary::GetInstance(_registry).GetAbility("Whirlwind");

        return whirlwindAbilityData;
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry)
        : Ability(_registry, initAbilityData(_registry))
    {
    }

} // namespace sage