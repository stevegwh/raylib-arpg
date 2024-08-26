#include "Abilities.hpp"

#include "AbilityFunctions.hpp"
#include "AbilityIndicator.hpp"
#include "AbilityResourceManager.hpp"

#include "GameData.hpp"

#include "Camera.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "vfx/VisualFX.hpp"

#include <memory>

#include "raylib.h"

#include <Serializer.hpp>

namespace sage
{

    AbilityData PlayerAutoAttack::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.cooldownDuration = 1;
        ad.base.baseDamage = 10;
        ad.base.range = 5;
        ad.base.repeatable = true;
        ad.base.executeFuncName = "SingleTargetHit";

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animSpeed = 4;
        ad.animationParams.animationDelay = 0;

        serializer::SaveAbilityData(ad, "resources/player_auto_attack.json");

        ad.executeFunc =
            AbilityResourceManager::GetInstance().GetExecuteFunc(AbilityFunctionEnum::SingleTargetHit);

        return ad;
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry, GameData* _gameData)
        : Ability(_registry, initAbilityData(_registry), _gameData)
    {
    }

    AbilityData RainOfFire::initAbilityData(entt::registry* _registry, Cursor* cursor)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 5;
        ad.base.baseDamage = 25;
        ad.base.element = AttackElement::FIRE;
        ad.base.repeatable = false;
        ad.base.executeFuncName = "MultihitRadiusFromCursor";

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.75f;

        ad.vfx.name = "RainOfFire";
        ad.indicator.indicatorKey = "CircularCursor";

        // vfx = AbilityResourceManager::GetInstance(_registry).GetVisualFX(
        //     "RainOfFire", _camera);

        serializer::SaveAbilityData(ad, "resources/player_rainoffire.json");

        // serializer::LoadAbilityData(ad, "resources/player_rainoffire.json");
        ad.cursor = cursor;

        ad.executeFunc = AbilityResourceManager::GetInstance().GetExecuteFunc(
            AbilityResourceManager::GetInstance().StringToExecuteFuncEnum(ad.base.executeFuncName));

        return ad;
    }

    RainOfFire::RainOfFire(entt::registry* _registry, GameData* _gameData)
        : CursorAbility(_registry, initAbilityData(_registry, _gameData->cursor.get()), _gameData)
    {
        // assert(vfx != nullptr);
    }

    AbilityData FloorFire::initAbilityData(entt::registry* _registry, Cursor* cursor)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 5;
        ad.base.baseDamage = 25;
        ad.base.element = AttackElement::FIRE;
        ad.base.repeatable = false;
        ad.base.executeFuncName = "MultihitRadiusFromCursor";

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.75f;

        ad.vfx.name = "FloorFire";
        ad.indicator.indicatorKey = "CircularCursor";

        // vfx = AbilityResourceManager::GetInstance(_registry).GetVisualFX(
        //     "RainOfFire", _camera);

        serializer::SaveAbilityData(ad, "resources/player_floorfire.json");

        // serializer::LoadAbilityData(ad, "resources/player_rainoffire.json");

        ad.cursor = cursor;
        ad.executeFunc = AbilityResourceManager::GetInstance().GetExecuteFunc(
            AbilityResourceManager::GetInstance().StringToExecuteFuncEnum(ad.base.executeFuncName));

        return ad;
    }

    FloorFire::FloorFire(entt::registry* _registry, GameData* _gameData)
        : CursorAbility(_registry, initAbilityData(_registry, _gameData->cursor.get()), _gameData)
    {
        // assert(vfx != nullptr);
    }

    AbilityData WavemobAutoAttack::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = true;
        ad.base.executeFuncName = "SingleTargetHit";

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        serializer::SaveAbilityData(ad, "resources/wavemob_auto_attack.json");

        ad.executeFunc =
            AbilityResourceManager::GetInstance().GetExecuteFunc(AbilityFunctionEnum::SingleTargetHit);
        return ad;
    }

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry, GameData* _gameData)
        : Ability(_registry, initAbilityData(_registry), _gameData)
    {
    }

    AbilityData WhirlwindAbility::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 5;
        ad.base.baseDamage = 25;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = false;
        ad.base.executeFuncName = "MultihitRadiusFromCaster";

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.65f;

        ad.vfx.name = "360SwordSlash";

        serializer::SaveAbilityData(ad, "resources/whirlwind.json");

        ad.executeFunc =
            AbilityResourceManager::GetInstance().GetExecuteFunc(AbilityFunctionEnum::MultihitRadiusFromCaster);

        return ad;
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry, GameData* _gameData)
        : Ability(_registry, initAbilityData(_registry), _gameData)
    {
    }

} // namespace sage