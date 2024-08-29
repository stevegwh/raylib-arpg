#include "AbilityState.hpp"

namespace sage
{
    void AbilityState::Update()
    {
    }

    void AbilityState::Draw3D()
    {
    }

    void AbilityState::OnEnter()
    {
    }

    void AbilityState::OnExit()
    {
    }

    AbilityState::~AbilityState()
    {
    }

    AbilityState::AbilityState(entt::entity _self, Timer& cooldownTimer, Timer& animationDelayTimer)
        : self(_self), cooldownTimer(cooldownTimer), animationDelayTimer(animationDelayTimer)
    {
    }
}; // namespace sage