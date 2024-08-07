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
        .range = 5};

    void WhirlwindAbility::Init(entt::entity self)
    {
        if (GetRemainingCooldownTime() > 0)
        {
            return;
        }

        // std::cout << "Whirlwind ability used \n";
        shouldCast = true;
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, 3, true);

        cooldownTimer.Start();
        animationDelayTimer.Start();
    }

    void WhirlwindAbility::Execute(entt::entity self)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(
            registry, self, abilityData, actorTransform.position(), whirlwindRadius);
        shouldCast = false;
    }

    void WhirlwindAbility::Update(entt::entity self)
    {
        cooldownTimer.Update(GetFrameTime());
        animationDelayTimer.Update(GetFrameTime());
        if (shouldCast && animationDelayTimer.HasFinished())
        {
            Execute(self);
            animationDelayTimer.Reset();
        }
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry)
        : Ability(_registry, _abilityData)
    {
        animationDelayTimer.SetMaxTime(WINDUP);
        abilityData.element = AttackElement::PHYSICAL;
        abilityData.baseDamage = DAMAGE;
        abilityData.cooldownDuration = COOLDOWN;
        abilityData.range = 5;
    }
} // namespace sage
