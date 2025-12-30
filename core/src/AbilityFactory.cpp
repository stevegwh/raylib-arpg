#include "AbilityFactory.hpp"

#include "abilities/AbilityData.hpp"
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
#include "Serializer.hpp"
#include "Systems.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/states/AbilityStateMachine.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace sage
{
    void CreatePlayerAutoAttack(entt::registry* registry, entt::entity abilityEntity);
    void CreateRainOfFireAbility(entt::registry* registry, entt::entity abilityEntity);
    void CreateFloorFireAbility(entt::registry* registry, entt::entity abilityEntity);
    void CreateFireballAbility(entt::registry* registry, entt::entity abilityEntity);
    void CreateLightningBallAbility(entt::registry* registry, entt::entity abilityEntity);
    void CreateWavemobAutoAttackAbility(entt::registry* registry, entt::entity abilityEntity);
    void CreateWhirlwindAbility(entt::registry* registry, entt::entity abilityEntity);
    std::unique_ptr<AbilityIndicator> GetIndicator(AbilityData::IndicatorData data, Systems* _sys);
    void AttachVisualFX(Systems* _sys, entt::entity abilityEntity);

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
        static const std::unordered_map<AbilityEnum, std::function<void(entt::registry*, entt::entity)>>
            abilityDataCreators = {
                {AbilityEnum::PLAYER_AUTOATTACK, CreatePlayerAutoAttack},
                {AbilityEnum::ENEMY_AUTOATTACK, CreateWavemobAutoAttackAbility},
                {AbilityEnum::FIREBALL, CreateFireballAbility},
                {AbilityEnum::LIGHTNINGBALL, CreateLightningBallAbility},
                {AbilityEnum::RAINFOFIRE, CreateRainOfFireAbility},
                {AbilityEnum::WHIRLWIND, CreateWhirlwindAbility}};
        // Attaches AbilityData to the out entity
        abilityDataCreators.at(abilityEnum)(registry, out);
        auto& data = registry->get<AbilityData>(out);
        ability.self = out;
        ability.caster = caster;
        ability.cooldownTimer.SetMaxTime(data.base.cooldownDuration);
        ability.castTimer.SetMaxTime(data.base.castTime);
        AttachVisualFX(sys, out);
        if (data.base.HasOptionalBehaviour(AbilityBehaviourOptional::INDICATOR))
        {
            ability.abilityIndicator = GetIndicator(data.indicator, sys);
        }
        return out;
    }

    AbilityFactory::AbilityFactory(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
    }

    // Helper functions
    // --------------------------------------------

    std::unique_ptr<AbilityIndicator> GetIndicator(AbilityData::IndicatorData data, Systems* _sys)
    {
        if (data.indicatorKey == AbilityIndicatorEnum::NONE) return nullptr;

        std::unique_ptr<AbilityIndicator> obj;

        if (data.indicatorKey == AbilityIndicatorEnum::CIRCULAR_MAGIC_CURSOR)
        {
            obj = std::make_unique<AbilityIndicator>(
                _sys->registry, _sys->navigationGridSystem.get(), "indicator_rainoffire");
        }
        else
        {
            return nullptr;
        }
        return std::move(obj);
    }

    void AttachVisualFX(Systems* _sys, entt::entity abilityEntity)
    {
        const auto& ad = _sys->registry->get<AbilityData>(abilityEntity);
        if (ad.vfx.name == AbilityVFXEnum::NONE) return;

        auto* _ability = &_sys->registry->get<Ability>(abilityEntity);

        if (ad.vfx.name == AbilityVFXEnum::RAINOFFIRE)
        {
            _sys->registry->emplace<RainOfFireVFX>(abilityEntity, _sys, _ability);
        }
        else if (ad.vfx.name == AbilityVFXEnum::FLOORFIRE)
        {
            _sys->registry->emplace<FloorFireVFX>(abilityEntity, _sys, _ability);
        }
        else if (ad.vfx.name == AbilityVFXEnum::WHIRLWIND)
        {
            _sys->registry->emplace<WhirlwindVFX>(abilityEntity, _sys, _ability);
        }
        else if (ad.vfx.name == AbilityVFXEnum::LIGHTNINGBALL)
        {
            _sys->registry->emplace<LightningBallVFX>(abilityEntity, _sys, _ability);
        }
        else if (ad.vfx.name == AbilityVFXEnum::FIREBALL)
        {
            _sys->registry->emplace<FireballVFX>(abilityEntity, _sys, _ability);
        }
    }

    void createProjectile(entt::registry* registry, entt::entity caster, entt::entity abilityEntity, Systems* data)
    {
        auto& ad = registry->get<AbilityData>(abilityEntity);
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

    void CreatePlayerAutoAttack(entt::registry* registry, entt::entity abilityEntity)
    {
        auto& ad = registry->emplace<AbilityData>(abilityEntity);
        ad.name = "AutoAttack";
        ad.description = "Hits the enemy for damage every so often.";
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

    void CreateRainOfFireAbility(entt::registry* registry, entt::entity abilityEntity)
    {
        auto& ad = registry->emplace<AbilityData>(abilityEntity);
        auto& abilityComponent = registry->get<Ability>(abilityEntity);
        ad.name = "Rain of Fire";
        ad.description = "Hits all enemies around the attacker.";
        // abilityComponent.iconPath = "resources/icons/abilities/rain_of_fire.png";
        abilityComponent.icon = "icon_rain_of_fire";
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

    void CreateFloorFireAbility(entt::registry* registry, entt::entity abilityEntity)
    {
        // TODO: Implement
    }

    void CreateFireballAbility(entt::registry* registry, entt::entity abilityEntity)
    {
        auto& ad = registry->emplace<AbilityData>(abilityEntity);
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

    void CreateWavemobAutoAttackAbility(entt::registry* registry, entt::entity abilityEntity)
    {
        auto& ad = registry->emplace<AbilityData>(abilityEntity);
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

    void CreateLightningBallAbility(entt::registry* registry, entt::entity abilityEntity)
    {
        auto& ad = registry->emplace<AbilityData>(abilityEntity);
        auto& abilityComponent = registry->get<Ability>(abilityEntity);
        ad.name = "Lightning Ball";
        ad.description = "Hits all enemies around the attacker.";
        // abilityComponent.iconPath = "resources/icons/abilities/lightning_ball.png";
        abilityComponent.icon = "icon_lightning_ball";
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

    void CreateWhirlwindAbility(entt::registry* registry, entt::entity abilityEntity)
    {
        auto& ad = registry->emplace<AbilityData>(abilityEntity);
        auto& abilityComponent = registry->get<Ability>(abilityEntity);
        ad.name = "Whirlwind";
        ad.description = "Hits all enemies around the attacker.";
        // abilityComponent.iconPath = "resources/icons/abilities/whirlwind.png";
        abilityComponent.icon = "icon_whirlwind";
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