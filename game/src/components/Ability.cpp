#include "Ability.hpp"

#include "abilities/vfx/FireballVFX.hpp"
#include "abilities/vfx/FloorFireVFX.hpp"
#include "abilities/vfx/LightningBallVFX.hpp"
#include "abilities/vfx/RainOfFireVFX.hpp"
#include "abilities/vfx/VisualFX.hpp"
#include "abilities/vfx/WhirlwindVFX.hpp"
#include "AbilityFactory.hpp"
#include "Systems.hpp"

namespace lq
{

    VisualFX* Ability::GetVfx(entt::registry* registry) const
    {
        auto vfxEnum = registry->get<AbilityData>(self).vfx.name;
        if (vfxEnum == AbilityVFXEnum::NONE) return nullptr;

        if (vfxEnum == AbilityVFXEnum::RAINOFFIRE && registry->any_of<RainOfFireVFX>(self))
        {
            return &registry->get<RainOfFireVFX>(self);
        }
        else if (vfxEnum == AbilityVFXEnum::FLOORFIRE && registry->any_of<FloorFireVFX>(self))
        {
            return &registry->get<FloorFireVFX>(self);
        }
        else if (vfxEnum == AbilityVFXEnum::WHIRLWIND && registry->any_of<WhirlwindVFX>(self))
        {
            return &registry->get<WhirlwindVFX>(self);
        }
        else if (vfxEnum == AbilityVFXEnum::LIGHTNINGBALL && registry->any_of<LightningBallVFX>(self))
        {
            return &registry->get<LightningBallVFX>(self);
        }
        else if (vfxEnum == AbilityVFXEnum::FIREBALL && registry->any_of<FireballVFX>(self))
        {
            return &registry->get<FireballVFX>(self);
        }

        return nullptr;
    }

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

}; // namespace lq
