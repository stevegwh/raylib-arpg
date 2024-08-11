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
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
    }

    void CursorAbility::initStates()
    {
    }

    CursorAbility::CursorAbility(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        std::unique_ptr<TextureTerrainOverlay> _spellCursor,
        AbilityData _abilityData)
        : Ability(_registry, _abilityData, _cursor)
    {
        initStates();
        auto cursorState = std::make_unique<Ability::CursorSelectState>(this, _cursor);
        {
            entt::sink sink{cursorState->onConfirm};
            sink.connect<&CursorAbility::confirm>(this);
        }

        states[AbilityStateEnum::IDLE] = std::make_unique<Ability::IdleState>(this);
        states[AbilityStateEnum::CURSOR_SELECT] = std::move(cursorState);
        states[AbilityStateEnum::AWAITING_EXECUTION] =
            std::make_unique<Ability::AwaitingExecutionState>(this);
        state = states[AbilityStateEnum::IDLE].get();

        vfx = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
        spellCursor = std::move(_spellCursor);
    }
} // namespace sage
