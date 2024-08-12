#include "Abilities.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "AbilityFunctions.hpp"

#include "raylib.h"

namespace sage
{

    AbilityData PlayerAutoAttack::initAbilityData(entt::registry* _registry)
    {
        static AbilityData ad;
        ad.baseData.element = AttackElement::PHYSICAL;
        ad.baseData.cooldownDuration = 1;
        ad.baseData.baseDamage = 10;
        ad.baseData.range = 5;
        ad.baseData.repeatable = true;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animSpeed = 4;
        ad.animationParams.animationDelay = 0;

        ad.executeFunc =
            AbilityLibrary::GetInstance(_registry).GetAbility("SingleTargetHit");

        return ad;
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry)
        : Ability(_registry, initAbilityData(_registry))
    {
    }

    AbilityData RainOfFire::initAbilityData(entt::registry* _registry)
    {
        static AbilityData ad;

        ad.baseData.cooldownDuration = 3;
        ad.baseData.range = 5;
        ad.baseData.baseDamage = 25;
        ad.baseData.element = AttackElement::FIRE;
        ad.baseData.repeatable = false;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.75f;

        ad.executeFunc = AbilityLibrary::GetInstance(_registry).GetAbility("RainOfFire");
        return ad;
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
        static AbilityData ad;

        ad.baseData.cooldownDuration = 1;
        ad.baseData.range = 5;
        ad.baseData.baseDamage = 10;
        ad.baseData.element = AttackElement::PHYSICAL;
        ad.baseData.repeatable = true;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        ad.executeFunc =
            AbilityLibrary::GetInstance(_registry).GetAbility("SingleTargetHit");
        return ad;
    }

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry)
        : Ability(_registry, initAbilityData(_registry))
    {
    }

    AbilityData WhirlwindAbility::initAbilityData(entt::registry* _registry)
    {
        static AbilityData ad;

        ad.baseData.cooldownDuration = 3;
        ad.baseData.range = 5;
        ad.baseData.baseDamage = 25;
        ad.baseData.element = AttackElement::PHYSICAL;
        ad.baseData.repeatable = false;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.65f;

        ad.executeFunc = AbilityLibrary::GetInstance(_registry).GetAbility("Whirlwind");

        return ad;
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry)
        : Ability(_registry, initAbilityData(_registry))
    {
    }

} // namespace sage