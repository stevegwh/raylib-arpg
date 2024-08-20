#include "AbilityState.hpp"

namespace sage
{
    void AbilityState::Update(entt::entity self)
    {
    }

    void AbilityState::Draw3D(entt::entity self)
    {
    }

    void AbilityState::OnEnter(entt::entity self)
    {
    }

    void AbilityState::OnExit(entt::entity self)
    {
    }

    AbilityState::~AbilityState()
    {
    }

    AbilityState::AbilityState(Timer& cooldownTimer, Timer& animationDelayTimer)
        : cooldownTimer(cooldownTimer), animationDelayTimer(animationDelayTimer)
    {
    }
}; // namespace sage