#include "AbilityFactory.hpp"

#include "abilities/AbilityIndicator.hpp"
#include "abilities/AbilityResourceManager.hpp"
#include "components/sgTransform.hpp"
#include "components/States.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/states/AbilityStateMachine.hpp"
#include <Serializer.hpp>

namespace sage
{
    void CreatePlayerAutoAttack(Ability& abilityComponent);
    void CreateRainOfFireAbility(Ability& abilityComponent);
    void CreateFloorFireAbility(Ability& abilityComponent);
    void CreateFireballAbility(Ability& abilityComponent);
    void CreateLightningBallAbility(Ability& abilityComponent);
    void CreateWavemobAutoAttackAbility(Ability& abilityComponenty);
    void CreateWhirlwindAbility(Ability& abilityComponent);

    entt::entity AbilityFactory::GetAbility(entt::entity caster, AbilityEnum abilityEnum)
    {
        if (!abilityMap.contains(caster))
        {
            std::cout << "WARNING [AbilityFactory]: Caster is not associated with any ability \n";
            return entt::null;
        }

        if (abilityMap[caster].contains(abilityEnum))
        {
            return abilityMap[caster][abilityEnum];
        }

        std::cout << "WARNING [AbilityFactory]: Caster does not have the following ability registered: "
                  << magic_enum::enum_name(abilityEnum) << std::endl;
        return entt::null;
    }

    entt::entity AbilityFactory::RegisterAbility(entt::entity caster, AbilityEnum abilityEnum)
    {
        entt::entity out = registry->create();
        registry->emplace<AbilityState>(out);
        registry->emplace<sgTransform>(out, out);
        abilityMap[caster].emplace(abilityEnum, out);

        auto& ability = registry->emplace<Ability>(out);

        switch (abilityEnum)
        {
        case AbilityEnum::PLAYER_AUTOATTACK:
            CreatePlayerAutoAttack(ability);
            break;
        case AbilityEnum::ENEMY_AUTOATTACK:
            CreateWavemobAutoAttackAbility(ability);
            break;
        case AbilityEnum::FIREBALL:
            CreateFireballAbility(ability);
            break;
        case AbilityEnum::LIGHTNINGBALL:
            CreateLightningBallAbility(ability);
            break;
        case AbilityEnum::RAINFOFIRE:
            CreateRainOfFireAbility(ability);
            break;
        case AbilityEnum::WHIRLWIND:
            CreateWhirlwindAbility(ability);
            break;
        default:
            break;
        }

        ability.self = out;
        ability.caster = caster;
        ability.cooldownTimer.SetMaxTime(ability.ad.base.cooldownDuration);
        ability.castTimer.SetMaxTime(ability.ad.base.castTime);

        ability.vfx = AbilityResourceManager::GetInstance().GetVisualFX(gameData, &ability);

        if (ability.ad.base.HasOptionalBehaviour(AbilityBehaviourOptional::INDICATOR))
        {
            ability.abilityIndicator =
                AbilityResourceManager::GetInstance().GetIndicator(ability.ad.indicator, gameData);
        }

        return out;
    }

    AbilityFactory::AbilityFactory(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

    // --------------------------------------------

    void createProjectile(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, GameData* data)
    {
        auto& ad = registry->get<Ability>(abilityEntity).ad;
        auto& projectileTrans = registry->get<sgTransform>(abilityEntity);
        auto& casterPos = registry->get<sgTransform>(caster).GetWorldPos();
        auto point = data->cursor->terrainCollision().point;

        if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CASTER))
        {
            projectileTrans.SetPosition(casterPos);
        }
        else if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CURSOR))
        {
            auto cursorPos = data->cursor->terrainCollision().point;
            projectileTrans.SetPosition(cursorPos);
        }

        data->actorMovementSystem->MoveToLocation(abilityEntity, point);
    }

    // --------------------------------------------

    void CreatePlayerAutoAttack(Ability& abilityComponent)
    {
        auto& ad = abilityComponent.ad;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.cooldownDuration = 1;
        ad.base.baseDamage = 10;
        ad.base.range = 5;
        ad.base.castTime = 0;
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_CASTER |
            AbilityBehaviour::MOVEMENT_HITSCAN | AbilityBehaviour::CAST_INSTANT | AbilityBehaviour::ATTACK_TARGET);
        ad.base.AddOptionalBehaviour(AbilityBehaviourOptional::REPEAT_AUTO);
        ad.base.targetType = AbilityTargetType::TARGET_ENEMY;

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animSpeed = 4;
        ad.animationParams.animationDelay = 0;
    }

    void CreateRainOfFireAbility(Ability& abilityComponent)
    {
        auto& ad = abilityComponent.ad;
        ad.base.cooldownDuration = 3;
        ad.base.range = 30;
        ad.base.baseDamage = 25;
        ad.base.radius = 30;
        ad.base.castTime = 0;
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
    }

    void CreateFloorFireAbility(Ability& abilityComponent)
    {
        // TODO: Implement
    }

    void CreateFireballAbility(Ability& abilityComponent)
    {
        auto& ad = abilityComponent.ad;
        ad.base.cooldownDuration = 1;
        ad.base.range = 30;
        ad.base.baseDamage = 50;
        ad.base.radius = 10;
        ad.base.castTime = 0;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_NONE |
            AbilityBehaviour::MOVEMENT_PROJECTILE | AbilityBehaviour::CAST_INSTANT |
            AbilityBehaviour::ATTACK_AOE_POINT);

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = "Fireball";
    }

    void CreateWavemobAutoAttackAbility(Ability& abilityComponent)
    {
        auto& ad = abilityComponent.ad;
        ad.base.cooldownDuration = 1;
        ad.base.range = 5;
        ad.base.baseDamage = 10;
        ad.base.castTime = 0;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_CASTER |
            AbilityBehaviour::MOVEMENT_HITSCAN | AbilityBehaviour::CAST_INSTANT | AbilityBehaviour::ATTACK_TARGET);
        ad.base.AddOptionalBehaviour(AbilityBehaviourOptional::REPEAT_AUTO);

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
    }

    void CreateLightningBallAbility(Ability& abilityComponent)
    {
        auto& ad = abilityComponent.ad;
        ad.base.cooldownDuration = 1;
        ad.base.range = 30;
        ad.base.baseDamage = 50;
        ad.base.radius = 10;
        ad.base.castTime = 0;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_NONE |
            AbilityBehaviour::MOVEMENT_PROJECTILE | AbilityBehaviour::CAST_INSTANT |
            AbilityBehaviour::ATTACK_AOE_POINT);

        ad.animationParams.animEnum = AnimationEnum::AUTOATTACK;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = "LightningBall";
    }

    void CreateWhirlwindAbility(Ability& abilityComponent)
    {
        auto& ad = abilityComponent.ad;
        ad.base.cooldownDuration = 0.15;
        ad.base.range = 15;
        ad.base.baseDamage = 10;
        ad.base.radius = 15;
        ad.base.castTime = 0;
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
    }

} // namespace sage