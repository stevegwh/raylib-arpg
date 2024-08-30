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

    entt::entity PlayerAutoAttack::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.cooldownDuration = 1;
        ad.base.baseDamage = 10;
        ad.base.range = 5;
        ad.base.repeatable = true;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::HIT_TARGETED_UNIT;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animSpeed = 4;
        ad.animationParams.animationDelay = 0;

        // serializer::SaveAbilityData(ad, "resources/player_auto_attack.json");

        // TODO: Why can't we initialise vfx here?

        auto entity = _registry->create();
        _registry->emplace<AbilityData>(entity, ad);
        return entity;
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry, entt::entity _self, GameData* _gameData)
        : AbilityStateMachine(_registry, _self, initAbilityData(_registry), _gameData)
    {
    }

    entt::entity RainOfFire::initAbilityData(entt::registry* _registry, Cursor* cursor)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 5;
        ad.base.baseDamage = 25;
        ad.base.element = AttackElement::FIRE;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CURSOR;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::HIT_ALL_IN_RADIUS;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_STATIONARY;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.75f;

        ad.vfx.name = "RainOfFire";
        ad.indicator.indicatorKey = "CircularCursor";

        // vfx = AbilityResourceManager::GetInstance(_registry).GetVisualFX(
        //     "RainOfFire", _camera);

        // serializer::SaveAbilityData(ad, "resources/player_rainoffire.json");

        // serializer::LoadAbilityData(ad, "resources/player_rainoffire.json");
        ad.cursor = cursor;

        // ad.executeFunc = AbilityResourceManager::GetInstance().GetExecuteFunc(
        //     AbilityResourceManager::GetInstance().StringToExecuteFuncEnum(ad.base.executeFuncName));

        auto entity = _registry->create();
        _registry->emplace<AbilityData>(entity, ad);
        return entity;
    }

    RainOfFire::RainOfFire(entt::registry* _registry, entt::entity _self, GameData* _gameData)
        : CursorAbility(_registry, _self, initAbilityData(_registry, _gameData->cursor.get()), _gameData)
    {
        // assert(vfx != nullptr);
    }

    entt::entity FloorFire::initAbilityData(entt::registry* _registry, Cursor* cursor)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 3;
        ad.base.range = 5;
        ad.base.baseDamage = 25;
        ad.base.element = AttackElement::FIRE;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CURSOR;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::HIT_ALL_IN_RADIUS;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_STATIONARY;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 1;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0.75f;

        ad.vfx.name = "Fireball";
        ad.indicator.indicatorKey = "CircularCursor";

        // vfx = AbilityResourceManager::GetInstance(_registry).GetVisualFX(
        //     "RainOfFire", _camera);

        // serializer::SaveAbilityData(ad, "resources/player_floorfire.json");

        // serializer::LoadAbilityData(ad, "resources/player_rainoffire.json");

        ad.cursor = cursor;
        // ad.executeFunc = AbilityResourceManager::GetInstance().GetExecuteFunc(
        //     AbilityResourceManager::GetInstance().StringToExecuteFuncEnum(ad.base.executeFuncName));

        auto entity = _registry->create();
        _registry->emplace<AbilityData>(entity, ad);
        return entity;
    }

    FloorFire::FloorFire(entt::registry* _registry, entt::entity _self, GameData* _gameData)
        : CursorAbility(_registry, _self, initAbilityData(_registry, _gameData->cursor.get()), _gameData)
    {
        // assert(vfx != nullptr);
    }

    entt::entity Fireball::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 15;
        ad.base.baseDamage = 50;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::HIT_ALL_IN_RADIUS;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_PROJECTILE;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        ad.vfx.name = "Fireball";

        // serializer::SaveAbilityData(ad, "resources/wavemob_auto_attack.json");

        // ad.executeFunc =
        //     AbilityResourceManager::GetInstance().GetExecuteFunc(AbilityFunctionEnum::SingleTargetHit);
        auto entity = _registry->create();
        _registry->emplace<AbilityData>(entity, ad);
        return entity;
    }

    Fireball::Fireball(entt::registry* _registry, entt::entity _self, GameData* _gameData)
        : AbilityStateMachine(_registry, _self, initAbilityData(_registry), _gameData)
    {
    }

    entt::entity LightningBall::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::HIT_ALL_IN_RADIUS;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::DETACHED_PROJECTILE;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        ad.vfx.name = "LightningBall";

        // serializer::SaveAbilityData(ad, "resources/wavemob_auto_attack.json");

        // ad.executeFunc =
        //     AbilityResourceManager::GetInstance().GetExecuteFunc(AbilityFunctionEnum::SingleTargetHit);
        auto entity = _registry->create();
        _registry->emplace<AbilityData>(entity, ad);
        return entity;
    }

    LightningBall::LightningBall(entt::registry* _registry, entt::entity _self, GameData* _gameData)
        : AbilityStateMachine(_registry, _self, initAbilityData(_registry), _gameData)
    {
    }

    entt::entity WavemobAutoAttack::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = true;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::HIT_TARGETED_UNIT;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;

        // serializer::SaveAbilityData(ad, "resources/wavemob_auto_attack.json");

        // ad.executeFunc =
        //     AbilityResourceManager::GetInstance().GetExecuteFunc(AbilityFunctionEnum::SingleTargetHit);
        auto entity = _registry->create();
        _registry->emplace<AbilityData>(entity, ad);
        return entity;
    }

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry, entt::entity _self, GameData* _gameData)
        : AbilityStateMachine(_registry, _self, initAbilityData(_registry), _gameData)
    {
    }

    entt::entity WhirlwindAbility::initAbilityData(entt::registry* _registry)
    {
        AbilityData ad;

        ad.base.cooldownDuration = 0.15;
        ad.base.range = 15;
        ad.base.baseDamage = 10;
        ad.base.element = AttackElement::PHYSICAL;
        ad.base.repeatable = false;
        ad.base.spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
        ad.base.behaviourOnHit = AbilityBehaviourOnHit::HIT_ALL_IN_RADIUS;
        ad.base.behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;

        ad.animationParams.animEnum = AnimationEnum::SPIN;
        ad.animationParams.animSpeed = 5;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = "360SwordSlash";

        // serializer::SaveAbilityData(ad, "resources/whirlwind.json");

        // ad.executeFunc =
        //     AbilityResourceManager::GetInstance().GetExecuteFunc(AbilityFunctionEnum::MultihitRadiusFromCaster);

        auto entity = _registry->create();
        _registry->emplace<AbilityData>(entity, ad);
        return entity;
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry, entt::entity _self, GameData* _gameData)
        : AbilityStateMachine(_registry, _self, initAbilityData(_registry), _gameData)
    {
    }

} // namespace sage