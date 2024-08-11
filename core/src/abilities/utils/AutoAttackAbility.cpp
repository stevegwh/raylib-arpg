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
        ChangeState(self, AbilityStateEnum::IDLE);
    }

    void AutoAttackAbility::Init(entt::entity self)
    {
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK, 4);
        ChangeState(self, AbilityStateEnum::AWAITING_EXECUTION);
    }

    AutoAttackAbility::AutoAttackAbility(
        entt::registry* _registry, AbilityData _abilityData, Cursor* _cursor)
        : Ability(_registry, _abilityData, _cursor)
    {

        auto idleState = dynamic_cast<IdleState*>(states[AbilityStateEnum::IDLE].get());
        entt::sink onRestartTriggeredSink{idleState->onRestartTriggered};
        onRestartTriggeredSink.connect<&AutoAttackAbility::Init>(this);

        auto awaitingExecutionState =
            std::make_unique<AwaitingExecutionState>(cooldownTimer, animationDelayTimer);
        entt::sink onExecuteSink{awaitingExecutionState->onExecute};
        onExecuteSink.connect<&AutoAttackAbility::Execute>(this);
        states[AbilityStateEnum::AWAITING_EXECUTION] = std::move(awaitingExecutionState);
    }
} // namespace sage