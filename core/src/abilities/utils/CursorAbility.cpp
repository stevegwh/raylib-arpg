//
// Created by Steve Wheeler on 21/07/2024.
//

#include "CursorAbility.hpp"

#include "Camera.hpp"
#include "components/Animation.hpp"

#include "AbilityFunctions.hpp"

namespace sage
{
    void CursorAbility::Init(entt::entity self)
    {
        if (state == states[AbilityStateEnum::CURSOR_SELECT].get())
        {
            ChangeState(self, AbilityStateEnum::IDLE);
        }
        else
        {
            ChangeState(self, AbilityStateEnum::CURSOR_SELECT);
        }
    }

    void CursorAbility::Execute(entt::entity self)
    {
        vfx->InitSystem(cursor->collision().point);
        Hit360AroundPoint(
            registry, self, abilityData, cursor->collision().point, whirlwindRadius);

        ChangeState(self, AbilityStateEnum::IDLE);
    }

    void CursorAbility::confirm(entt::entity self)
    {
        ChangeState(self, AbilityStateEnum::AWAITING_EXECUTION);
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByParams(abilityData.animationParams);
    }

    CursorAbility::CursorAbility(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        std::unique_ptr<TextureTerrainOverlay> _spellCursor,
        AbilityData _abilityData)
        : Ability(_registry, _abilityData, _cursor)
    {
        auto idleState = dynamic_cast<IdleState*>(states[AbilityStateEnum::IDLE].get());
        entt::sink onRestartTriggeredSink{idleState->onRestartTriggered};
        onRestartTriggeredSink.connect<&CursorAbility::Init>(this);

        auto awaitingExecutionState =
            std::make_unique<AwaitingExecutionState>(cooldownTimer, animationDelayTimer);
        entt::sink onExecuteSink{awaitingExecutionState->onExecute};
        onExecuteSink.connect<&CursorAbility::Execute>(this);
        states[AbilityStateEnum::AWAITING_EXECUTION] = std::move(awaitingExecutionState);

        auto cursorState = std::make_unique<CursorSelectState>(
            cooldownTimer, animationDelayTimer, cursor, std::move(_spellCursor));
        entt::sink onConfirmSink{cursorState->onConfirm};
        onConfirmSink.connect<&CursorAbility::confirm>(this);
        states[AbilityStateEnum::CURSOR_SELECT] = std::move(cursorState);

        vfx = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
    }
} // namespace sage
