#include "Ability.hpp"

namespace sage
{

    void Ability::ResetCooldown()
    {
        cooldownTimer.Reset();
    }

    bool Ability::IsActive()
    {
        if (ad.cursorBased)
        {
            return cooldownTimer.IsRunning() || state == AbilityStateEnum::CURSOR_SELECT;
        }
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
}; // namespace sage
