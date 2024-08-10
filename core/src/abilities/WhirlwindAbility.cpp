//
// Created by Steve Wheeler on 21/07/2024.
//

#include "WhirlwindAbility.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "AbilityFunctions.hpp"

#include "raylib.h"

static constexpr float COOLDOWN = 3.0f;
static constexpr int DAMAGE = 25;
static constexpr float WINDUP = 0.65f;

namespace sage
{
    static constexpr AbilityData _abilityData{
        .element = AttackElement::PHYSICAL,
        .cooldownDuration = COOLDOWN,
        .baseDamage = DAMAGE,
        .range = 5,
        .animationDelay = WINDUP,
        .repeatable = false};

    void WhirlwindAbility::Execute(entt::entity self)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(
            registry, self, abilityData, actorTransform.position(), whirlwindRadius);
        ChangeState(self, AbilityState::IDLE);
    }

    void WhirlwindAbility::Init(entt::entity self)
    {
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
        ChangeState(self, AbilityState::AWAITING_EXECUTION);
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry)
        : AutoAttackAbility(_registry, _abilityData)
    {
    }
} // namespace sage
