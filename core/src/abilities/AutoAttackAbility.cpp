#include "AutoAttackAbility.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include "AbilityFunctions.hpp"

namespace sage
{
    void AutoAttackAbility::IdleState::Update(entt::entity self)
    {
        ability->cooldownTimer.Update(GetFrameTime());
        if (ability->cooldownTimer.HasFinished() && ability->abilityData.repeatable)
        {
            ability->Init(self);
        }
        // if (ability->vfx->active)
        // {
        //     ability->vfx->Update(GetFrameTime());
        // }
    }

    void AutoAttackAbility::IdleState::OnEnter(entt::entity self)
    {
        ability->cooldownTimer.Restart();
    }

    void AutoAttackAbility::IdleState::Draw3D(entt::entity self)
    {
        // if (ability->vfx->active)
        // {
        //     ability->vfx->Draw3D();
        // }
    }

    void AutoAttackAbility::AwaitingExecutionState::Update(entt::entity self)
    {
        ability->animationDelayTimer.Update(GetFrameTime());
        if (ability->animationDelayTimer.HasFinished())
        {
            ability->Execute(self);
        }
    }

    void AutoAttackAbility::AwaitingExecutionState::OnEnter(entt::entity self)
    {
        ability->cooldownTimer.Start();
        ability->animationDelayTimer.Start();
    }

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
    }

    AutoAttackAbility::AutoAttackAbility(
        entt::registry* _registry, AbilityData _abilityData)
        : Ability(_registry, _abilityData)
    {
        states[AbilityState::IDLE] = std::make_unique<IdleState>(this);
        states[AbilityState::AWAITING_EXECUTION] =
            std::make_unique<AwaitingExecutionState>(this);
        state = states[AbilityState::IDLE].get();
    }
} // namespace sage