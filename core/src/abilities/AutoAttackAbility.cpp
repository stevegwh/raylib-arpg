#include "AutoAttackAbility.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include "AbilityFunctions.hpp"

namespace sage
{

    void AutoAttackAbility::Execute(entt::entity self)
    {
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK, 4);

        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);
    }

    void AutoAttackAbility::Init(entt::entity self)
    {
        cooldownTimer.Start();
    }

    void AutoAttackAbility::Cancel()
    {
        cooldownTimer.Stop();
    }

    void AutoAttackAbility::Update(entt::entity self)
    {
        cooldownTimer.Update(GetFrameTime());
        if (cooldownTimer.HasFinished())
        {
            Execute(self);
            cooldownTimer.Restart();
        }
    }

    AutoAttackAbility::AutoAttackAbility(
        entt::registry* _registry, AbilityData _abilityData)
        : Ability(_registry, _abilityData)
    {
    }
} // namespace sage