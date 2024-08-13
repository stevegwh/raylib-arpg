#include "Abilities.hpp"

#include "AbilityFunctions.hpp"

#include "Camera.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "particle/VisualFX.hpp"

#include <memory>

#include "raylib.h"

namespace sage
{

    AbilityData PlayerAutoAttack::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;
        ad.baseData.element = AttackElement::PHYSICAL;
        ad.baseData.cooldownDuration = 1;
        ad.baseData.baseDamage = 10;
        ad.baseData.range = 5;
        ad.baseData.repeatable = true;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animSpeed = 4;
        ad.animationParams.animationDelay = 0;

        ad.executeFunc = AbilityResourceManager::GetInstance(_registry).GetExecuteFunc(
            AbilityFunctionEnum::SingleTargetHit);

        return ad;
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry, Camera* _camera)
        : Ability(_registry, initAbilityData(_registry), _camera)
    {
    }

    AbilityData RainOfFire::initAbilityData(entt::registry* _registry, Cursor* cursor)
    {
        AbilityData ad;

        ad.baseData.cooldownDuration = 3;
        ad.baseData.range = 5;
        ad.baseData.baseDamage = 25;
        ad.baseData.element = AttackElement::FIRE;
        ad.baseData.repeatable = false;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.75f;

        ad.vfx.name = "RainOfFire";

        ad.cursor = cursor;

        // vfx = AbilityResourceManager::GetInstance(_registry).GetVisualFX(
        //     "RainOfFire", _camera);

        ad.executeFunc = AbilityResourceManager::GetInstance(_registry).GetExecuteFunc(
            AbilityFunctionEnum::MultihitRadiusFromCursor);
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
              initAbilityData(_registry, _cursor))
    {
        // assert(vfx != nullptr);
    }

    AbilityData WavemobAutoAttack::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.baseData.cooldownDuration = 1;
        ad.baseData.range = 5;
        ad.baseData.baseDamage = 10;
        ad.baseData.element = AttackElement::PHYSICAL;
        ad.baseData.repeatable = true;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        ad.executeFunc = AbilityResourceManager::GetInstance(_registry).GetExecuteFunc(
            AbilityFunctionEnum::SingleTargetHit);
        return ad;
    }

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry, Camera* _camera)
        : Ability(_registry, initAbilityData(_registry), _camera)
    {
    }

    AbilityData WhirlwindAbility::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.baseData.cooldownDuration = 3;
        ad.baseData.range = 5;
        ad.baseData.baseDamage = 25;
        ad.baseData.element = AttackElement::PHYSICAL;
        ad.baseData.repeatable = false;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.65f;

        ad.executeFunc = AbilityResourceManager::GetInstance(_registry).GetExecuteFunc(
            AbilityFunctionEnum::MultihitRadiusFromCaster);

        return ad;
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry, Camera* _camera)
        : Ability(_registry, initAbilityData(_registry), _camera)
    {
    }

} // namespace sage