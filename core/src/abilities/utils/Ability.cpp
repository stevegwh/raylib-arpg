#include "Ability.hpp"

#include "raylib.h"

namespace sage
{
    void Ability::IdleState::Update(entt::entity self)
    {
        ability->cooldownTimer.Update(GetFrameTime());
        if (ability->cooldownTimer.HasFinished() && ability->abilityData.repeatable)
        {
            ability->Init(self);
        }
    }

    void Ability::IdleState::Draw3D(entt::entity self)
    {
    }

    void Ability::AwaitingExecutionState::Update(entt::entity self)
    {
        ability->animationDelayTimer.Update(GetFrameTime());
        if (ability->animationDelayTimer.HasFinished())
        {
            // ability->OnAbilityExecute.publish(self);
            ability->Execute(self);
        }
    }

    void Ability::AwaitingExecutionState::OnEnter(entt::entity self)
    {
        ability->cooldownTimer.Start();
        ability->animationDelayTimer.Start();
    }
    void Ability::ChangeState(entt::entity self, AbilityState newState)
    {
        state->OnExit(self);
        state = states[newState].get();
        state->OnEnter(self);
    }

    void Ability::ResetCooldown()
    {
        cooldownTimer.Reset();
    }

    bool Ability::IsActive() const
    {
        return cooldownTimer.IsRunning();
    }

    float Ability::GetRemainingCooldownTime() const
    {
        return cooldownTimer.GetRemainingTime();
    }

    float Ability::GetCooldownDuration() const
    {
        return cooldownTimer.GetMaxTime();
    }

    bool Ability::CooldownReady() const
    {
        return cooldownTimer.HasFinished() || cooldownTimer.GetRemainingTime() <= 0;
    }

    void Ability::Update(entt::entity self)
    {
        if (vfx && vfx->active)
        {
            vfx->Update(GetFrameTime());
        }
    }

    void Ability::Draw3D(entt::entity self)
    {
        if (vfx && vfx->active)
        {
            vfx->Draw3D();
        }
    }

    void Ability::Init(entt::entity self)
    {
        Execute(self);
    }

    Ability::Ability(entt::registry* _registry, const AbilityData& _abilityData)
        : registry(_registry), abilityData(_abilityData)
    {
        cooldownTimer.SetMaxTime(abilityData.cooldownDuration);
        animationDelayTimer.SetMaxTime(abilityData.animationDelay);
    }
} // namespace sage