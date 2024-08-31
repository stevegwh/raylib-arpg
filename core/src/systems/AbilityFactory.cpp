#include "AbilityFactory.hpp"

#include "abilities/AbilityIndicator.hpp"
#include "abilities/AbilityResourceManager.hpp"
#include "components/sgTransform.hpp"
#include "components/States.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "systems/states/AbilityStateMachine.hpp"
#include <Serializer.hpp>

#include "raymath.h"
#include <memory>

namespace sage
{
    void CreatePlayerAutoAttack(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity);
    void CreateRainOfFireAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity);
    void CreateFloorFireAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity);
    void CreateFireballAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity);
    void CreateLightningBallAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity);
    void CreateWavemobAutoAttackAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity);
    void CreateWhirlwindAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity);

    entt::entity AbilityFactory::GetAbility(entt::entity entity, AbilityEnum abilityEnum)
    {
        if (!abilityMap.contains(entity))
        {
            return entt::null;
        }

        if (abilityMap[entity].contains(abilityEnum))
        {
            return abilityMap[entity][abilityEnum];
        }

        return entt::null;
    }

    entt::entity AbilityFactory::RegisterAbility(entt::entity caster, AbilityEnum abilityEnum)
    {
        entt::entity out = registry->create();
        registry->emplace<AbilityState>(out);
        registry->emplace<sgTransform>(out, out);
        abilityMap[caster].emplace(abilityEnum, out);

        switch (abilityEnum)
        {
        case AbilityEnum::PLAYER_AUTOATTACK:
            CreatePlayerAutoAttack(registry, caster, gameData, out);
            break;
        case AbilityEnum::ENEMY_AUTOATTACK:
            CreateWavemobAutoAttackAbility(registry, caster, gameData, out);
            break;
        case AbilityEnum::FIREBALL:
            CreateFireballAbility(registry, caster, gameData, out);
            break;
        case AbilityEnum::LIGHTNINGBALL:
            CreateLightningBallAbility(registry, caster, gameData, out);
            break;
        case AbilityEnum::RAINFOFIRE:
            CreateRainOfFireAbility(registry, caster, gameData, out);
            break;
        case AbilityEnum::WHIRLWIND:
            CreateWhirlwindAbility(registry, caster, gameData, out);
            break;
        default:
            break;
        }

        return out;
    }

    AbilityFactory::AbilityFactory(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

    // --------------------------------------------

    void CreatePlayerAutoAttack(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.cooldownDuration = 1;
        ad.base.baseDamage = 10;
        ad.base.range = 5;
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_CASTER |
            AbilityBehaviour::MOVEMENT_HITSCAN | AbilityBehaviour::CAST_INSTANT | AbilityBehaviour::ATTACK_TARGET);
        ad.base.AddOptionalBehaviour(AbilityBehaviourOptional::REPEAT_AUTO);
        ad.base.targetType = AbilityTargetType::TARGET_ENEMY;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animSpeed = 4;
        ad.animationParams.animationDelay = 0;

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

    void CreateRainOfFireAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 30;
        ad.base.baseDamage = 25;
        ad.base.radius = 30;
        ad.base.AddElement(AbilityElement::FIRE);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CURSOR | AbilityBehaviour::FOLLOW_NONE |
            AbilityBehaviour::MOVEMENT_STATIONARY | AbilityBehaviour::CAST_INSTANT |
            AbilityBehaviour::ATTACK_AOE_POINT);
        ad.base.AddOptionalBehaviour(AbilityBehaviourOptional::INDICATOR);

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = "RainOfFire";
        ad.indicator.indicatorKey = "CircularCursor";

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
        ability.abilityIndicator = AbilityResourceManager::GetInstance().GetIndicator(ad.indicator, gameData);
    }

    void CreateFireballAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 30;
        ad.base.baseDamage = 50;
        ad.base.radius = 10;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_NONE |
            AbilityBehaviour::MOVEMENT_PROJECTILE | AbilityBehaviour::CAST_INSTANT |
            AbilityBehaviour::ATTACK_AOE_TARGET);

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = "Fireball";

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

    void CreateWavemobAutoAttackAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_CASTER |
            AbilityBehaviour::MOVEMENT_HITSCAN | AbilityBehaviour::CAST_INSTANT | AbilityBehaviour::ATTACK_TARGET);
        ad.base.AddOptionalBehaviour(AbilityBehaviourOptional::REPEAT_AUTO);

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

    void CreateLightningBallAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.AddElement(AbilityElement::LIGHTNING);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_NONE |
            AbilityBehaviour::MOVEMENT_PROJECTILE | AbilityBehaviour::CAST_INSTANT |
            AbilityBehaviour::ATTACK_AOE_TARGET);
        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        ad.vfx.name = "LightningBall";

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

    void CreateWhirlwindAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 0.15;
        ad.base.range = 15;
        ad.base.baseDamage = 10;
        ad.base.radius = 15;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_CASTER |
            AbilityBehaviour::MOVEMENT_STATIONARY | AbilityBehaviour::CAST_INSTANT |
            AbilityBehaviour::ATTACK_AOE_POINT);

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 5;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = "360SwordSlash";

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

} // namespace sage