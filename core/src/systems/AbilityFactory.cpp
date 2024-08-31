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
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.cooldownDuration = 1;
        ad.base.baseDamage = 10;
        ad.base.range = 5;
        ad.base.repeatable = true;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::ATTACK_TARGET;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animSpeed = 4;
        ad.animationParams.animationDelay = 0;

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        // ability.abilityIndicator = AbilityResourceManager::GetInstance().GetIndicator(ad.indicator, gameData);

        // Would much prefer emplacing the vfx with the above entity id, instead.
        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

    // RainOfFire factory function
    void CreateRainOfFireAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 30;
        ad.base.baseDamage = 25;
        ad.base.radius = 30;
        ad.base.element = AttackElement::FIRE;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CURSOR;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::ATTACK_AOE;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_STATIONARY;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = "RainOfFire";
        ad.indicator.indicatorKey = "CircularCursor";
        ad.requiresIndicator = true;

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
        ability.abilityIndicator = AbilityResourceManager::GetInstance().GetIndicator(ad.indicator, gameData);
    }

    // FloorFire factory function
    void CreateFloorFireAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 5;
        ad.base.baseDamage = 25;
        ad.base.radius = 30;
        ad.base.element = AttackElement::FIRE;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CURSOR;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::ATTACK_AOE;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_STATIONARY;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.75f;

        ad.vfx.name = "Fireball";
        ad.indicator.indicatorKey = "CircularCursor";

        ad.requiresIndicator = true;

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

    // Fireball factory function
    void CreateFireballAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 30;
        ad.base.baseDamage = 50;
        ad.base.radius = 10;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::ATTACK_AOE;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_PROJECTILE;

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

    // LightningBall factory function
    void CreateLightningBallAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::ATTACK_AOE;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_PROJECTILE;

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

    // WavemobAutoAttack factory function
    void CreateWavemobAutoAttackAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = true;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::ATTACK_TARGET;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        auto& ability = registry->emplace<Ability>(abilityEntity);
        ability.self = abilityEntity;
        ability.caster = caster;
        ability.ad = ad;
        ability.cooldownTimer.SetMaxTime(ad.base.cooldownDuration);
        ability.executionDelayTimer.SetMaxTime(ad.animationParams.animationDelay);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(ad.vfx, abilityEntity, gameData);
    }

    // WhirlwindAbility factory function
    void CreateWhirlwindAbility(
        entt::registry* registry, entt::entity caster, GameData* gameData, entt::entity& abilityEntity)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 0.15;
        ad.base.range = 15;
        ad.base.baseDamage = 10;
        ad.base.radius = 15;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::ATTACK_AOE;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;

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