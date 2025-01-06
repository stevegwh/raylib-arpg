#include "Ability.hpp"

namespace sage
{

    void Ability::ResetCooldown()
    {
        cooldownTimer.Reset();
    }

    bool Ability::IsActive()
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

    Ability::Ability()
        : startCast(std::make_unique<Event<entt::entity>>()),
          cancelCast(std::make_unique<Event<entt::entity>>()),
          castFailed(std::make_unique<Event<entt::entity, AbilityCastFail>>())
    {
    }

}; // namespace sage
