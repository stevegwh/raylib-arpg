//
// Created by Steve Wheeler on 21/07/2024.
//

#include "CursorAbility.hpp"

#include "Camera.hpp"
#include "components/Animation.hpp"

#include "AbilityFunctions.hpp"

namespace sage
{
    // --------------------------------------------

    void CursorAbility::CursorSelectState::enableCursor()
    {
        spellCursor->Init(cursor->terrainCollision().point);
        spellCursor->Enable(true);
        cursor->Disable();
        cursor->Hide();
    }

    void CursorAbility::CursorSelectState::disableCursor()
    {
        cursor->Enable();
        cursor->Show();
        spellCursor->Enable(false);
    }

    void CursorAbility::CursorSelectState::Update(entt::entity self)
    {
        spellCursor->Update(cursor->terrainCollision().point);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            onConfirm.publish(self);
        }
    }

    void CursorAbility::CursorSelectState::toggleCursor(entt::entity self)
    {
        if (cursorActive)
        {
            disableCursor();
            cursorActive = false;
        }
        else
        {
            enableCursor();
            cursorActive = true;
        }
    }

    void CursorAbility::CursorSelectState::OnExit(entt::entity self)
    {
        if (cursorActive)
        {
            disableCursor();
            cursorActive = false;
        }
    }

    void CursorAbility::CursorSelectState::OnEnter(entt::entity self)
    {
        enableCursor();
        cursorActive = true;
    }

    // --------------------------------------------

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
        : Ability(_registry, _abilityData), cursor(_cursor)
    {

        auto cursorState = std::make_unique<CursorSelectState>(
            cooldownTimer, animationDelayTimer, _cursor, std::move(_spellCursor));
        entt::sink onConfirmSink{cursorState->onConfirm};
        onConfirmSink.connect<&CursorAbility::confirm>(this);
        states[AbilityStateEnum::CURSOR_SELECT] = std::move(cursorState);

        vfx = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
    }
} // namespace sage
