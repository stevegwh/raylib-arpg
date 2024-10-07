#include "AbilityFactory.hpp"

#include "abilities/AbilityFunctions.hpp"
#include "abilities/AbilityIndicator.hpp"
#include "abilities/vfx/FireballVFX.hpp"
#include "abilities/vfx/FloorFireVFX.hpp"
#include "abilities/vfx/LightningBallVFX.hpp"
#include "abilities/vfx/RainOfFireVFX.hpp"
#include "abilities/vfx/VisualFX.hpp"
#include "abilities/vfx/WhirlwindVFX.hpp"
#include "components/Ability.hpp"
#include "components/sgTransform.hpp"
#include "components/States.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/states/AbilityStateMachine.hpp"
#include <Serializer.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace sage
{
    void CreatePlayerAutoAttack(Ability& abilityComponent);
    void CreateRainOfFireAbility(Ability& abilityComponent);
    void CreateFloorFireAbility(Ability& abilityComponent);
    void CreateFireballAbility(Ability& abilityComponent);
    void CreateLightningBallAbility(Ability& abilityComponent);
    void CreateWavemobAutoAttackAbility(Ability& abilityComponenty);
    void CreateWhirlwindAbility(Ability& abilityComponent);
    std::unique_ptr<AbilityIndicator> GetIndicator(AbilityData::IndicatorData data, GameData* _gameData);
    std::unique_ptr<VisualFX> GetVisualFX(GameData* _gameData, Ability* _ability);

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

        ability.vfx = GetVisualFX(gameData, &ability);

        if (ability.ad.base.HasOptionalBehaviour(AbilityBehaviourOptional::INDICATOR))
        {
            ability.abilityIndicator = GetIndicator(ability.ad.indicator, gameData);
        }

        return out;
    }

    AbilityFactory::AbilityFactory(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

    // Helper functions
    // --------------------------------------------

    std::unique_ptr<AbilityIndicator> GetIndicator(AbilityData::IndicatorData data, GameData* _gameData)
    {
        if (data.indicatorKey == AbilityIndicatorEnum::NONE) return nullptr;

        std::unique_ptr<AbilityIndicator> obj;

        if (data.indicatorKey == AbilityIndicatorEnum::CIRCULAR_MAGIC_CURSOR)
        {
            obj = std::make_unique<AbilityIndicator>(
                _gameData->registry, _gameData->navigationGridSystem.get(), AssetID::IMG_RAINOFFIRE_CURSOR);
        }
        else
        {
            return nullptr;
        }
        return std::move(obj);
    }

    std::unique_ptr<VisualFX> GetVisualFX(GameData* _gameData, Ability* _ability)
    {
        if (_ability->ad.vfx.name == AbilityVFXEnum::NONE) return nullptr;

        std::unique_ptr<VisualFX> obj;

        if (_ability->ad.vfx.name == AbilityVFXEnum::RAINOFFIRE)
        {
            obj = std::make_unique<RainOfFireVFX>(_gameData, _ability);
        }
        else if (_ability->ad.vfx.name == AbilityVFXEnum::FLOORFIRE)
        {
            obj = std::make_unique<FloorFireVFX>(_gameData, _ability);
        }
        else if (_ability->ad.vfx.name == AbilityVFXEnum::WHIRLWIND)
        {
            obj = std::make_unique<WhirlwindVFX>(_gameData, _ability);
        }
        else if (_ability->ad.vfx.name == AbilityVFXEnum::LIGHTNINGBALL)
        {
            obj = std::make_unique<LightningBallVFX>(_gameData, _ability);
        }
        else if (_ability->ad.vfx.name == AbilityVFXEnum::FIREBALL)
        {
            obj = std::make_unique<FireballVFX>(_gameData, _ability);
        }
        else
        {
            return nullptr;
        }

        return std::move(obj);
    }

    void createProjectile(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, GameData* data)
    {
        auto& ad = registry->get<Ability>(abilityEntity).ad;
        auto& projectileTrans = registry->get<sgTransform>(abilityEntity);
        auto& casterPos = registry->get<sgTransform>(caster).GetWorldPos();
        auto point = data->cursor->getFirstNaviCollision().point;

        if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CASTER))
        {
            projectileTrans.SetPosition(casterPos);
        }
        else if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CURSOR))
        {
            auto cursorPos = data->cursor->getFirstNaviCollision().point;
            projectileTrans.SetPosition(cursorPos);
        }

        data->actorMovementSystem->MoveToLocation(abilityEntity, point);
    }

    // --------------------------------------------

    void CreatePlayerAutoAttack(Ability& abilityComponent)
    {
        auto& ad = abilityComponent.ad;
        ad.base.AddElement(AbilityElement::PHYSICAL);
        ad.base.cooldownDuration = 0.75;
        ad.base.baseDamage = 25;
        ad.base.range = 5;
        ad.base.castTime = 0;
        ad.base.AddBehaviour(
            AbilityBehaviour::SPAWN_AT_CASTER | AbilityBehaviour::FOLLOW_CASTER |
            AbilityBehaviour::MOVEMENT_HITSCAN | AbilityBehaviour::CAST_INSTANT | AbilityBehaviour::ATTACK_TARGET);
        ad.base.AddOptionalBehaviour(AbilityBehaviourOptional::REPEAT_AUTO);
        ad.base.targetType = AbilityTargetType::TARGET_ENEMY;

        ad.animationParams.animEnum = AnimationEnum::SLASH;
        ad.animationParams.animSpeed = 3;
        ad.animationParams.animationDelay = 0;
    }

    void CreateRainOfFireAbility(Ability& abilityComponent)
    {
        abilityComponent.name = "Rain of Fire";
        abilityComponent.description = "Hits all enemies around the attacker.";
        abilityComponent.iconPath = "resources/icons/abilities/rain_of_fire.png";
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

        ad.animationParams.animEnum = AnimationEnum::SPELLCAST_UP;
        ad.animationParams.animSpeed = 3;
        ad.animationParams.oneShot = true;
        ad.animationParams.animationDelay = 0;

        ad.vfx.name = AbilityVFXEnum::RAINOFFIRE;
        ad.indicator.indicatorKey = AbilityIndicatorEnum::CIRCULAR_MAGIC_CURSOR;
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

        ad.animationParams.animEnum = AnimationEnum::SPELLCAST_FWD;
        ad.animationParams.animationDelay = 0;
        ad.animationParams.oneShot = true;

        ad.vfx.name = AbilityVFXEnum::FIREBALL;
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
        abilityComponent.name = "Lighting Ball";
        abilityComponent.description = "Hits all enemies around the attacker.";
        abilityComponent.iconPath = "resources/icons/abilities/lightning_ball.png";
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

        ad.animationParams.animEnum = AnimationEnum::SPELLCAST_FWD;
        ad.animationParams.animationDelay = 1; // Animation delay not a thing
        ad.animationParams.animSpeed = 4;
        ad.animationParams.oneShot = true;

        ad.vfx.name = AbilityVFXEnum::LIGHTNINGBALL;
    }

    void CreateWhirlwindAbility(Ability& abilityComponent)
    {
        abilityComponent.name = "Whirlwind";
        abilityComponent.description = "Hits all enemies around the attacker.";
        abilityComponent.iconPath = "resources/icons/abilities/whirlwind.png";
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

        ad.vfx.name = AbilityVFXEnum::WHIRLWIND;
    }

} // namespace sage