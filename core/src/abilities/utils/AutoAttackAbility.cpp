#include "AutoAttackAbility.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include "AbilityFunctions.hpp"

namespace sage
{

    void AutoAttackAbility::Execute(entt::entity self)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);
        ChangeState(self, AbilityState::IDLE);
    }

    void AutoAttackAbility::Init(entt::entity self)
    {
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK, 4);
        ChangeState(self, AbilityState::AWAITING_EXECUTION);
    }

    void AutoAttackAbility::Cancel(entt::entity self)
    {
        cooldownTimer.Stop();
        animationDelayTimer.Stop();
        ChangeState(self, AbilityState::IDLE);
    }

    void AutoAttackAbility::Draw3D(entt::entity self)
    {
        state->Draw3D(self);
        Ability::Draw3D(self);
    }

    void AutoAttackAbility::Update(entt::entity self)
    {
        state->Update(self);
        Ability::Update(self);
    }

    void AutoAttackAbility::initStates()
    {
        states[AbilityState::IDLE] = std::make_unique<IdleState>(this);
        states[AbilityState::AWAITING_EXECUTION] =
            std::make_unique<AwaitingExecutionState>(this);
        state = states[AbilityState::IDLE].get();
    }

    AutoAttackAbility::AutoAttackAbility(
        entt::registry* _registry, AbilityData _abilityData)
        : Ability(_registry, _abilityData)
    {
        initStates();
    }
} // namespace sage