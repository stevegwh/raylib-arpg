//
// Created by Steve Wheeler on 21/07/2024.
//

#include "CursorAbility.hpp"

#include "Camera.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"

#include "AbilityFunctions.hpp"

#include "raylib.h"

namespace sage
{

    void CursorAbility::CursorSelectState::enableCursor()
    {
        ability->spellCursor->Init(ability->cursor->terrainCollision().point);
        ability->spellCursor->Enable(true);
        ability->cursor->Disable();
        ability->cursor->Hide();
    }

    void CursorAbility::CursorSelectState::disableCursor()
    {
        ability->cursor->Enable();
        ability->cursor->Show();
        ability->spellCursor->Enable(false);
    }

    void CursorAbility::CursorSelectState::Update(entt::entity self)
    {
        ability->spellCursor->Update(ability->cursor->terrainCollision().point);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            ability->confirm(self);
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

    void CursorAbility::Init(entt::entity self)
    {
        if (state == states[AbilityState::CURSOR_SELECT].get())
        {
            ChangeState(self, AbilityState::IDLE);
        }
        else
        {
            ChangeState(self, AbilityState::CURSOR_SELECT);
        }
    }

    void CursorAbility::Execute(entt::entity self)
    {
        vfx->InitSystem(cursor->collision().point);
        Hit360AroundPoint(
            registry, self, abilityData, cursor->collision().point, whirlwindRadius);

        ChangeState(self, AbilityState::IDLE);
    }

    void CursorAbility::confirm(entt::entity self)
    {
        ChangeState(self, AbilityState::AWAITING_EXECUTION);
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
    }

    void CursorAbility::Draw3D(entt::entity self)
    {
        state->Draw3D(self);
        Ability::Draw3D(self);
    }

    void CursorAbility::Update(entt::entity self)
    {
        state->Update(self);
        Ability::Update(self);
    }

    void CursorAbility::initStates()
    {
        states[AbilityState::IDLE] = std::make_unique<Ability::IdleState>(this);
        states[AbilityState::CURSOR_SELECT] = std::make_unique<CursorSelectState>(this);
        states[AbilityState::AWAITING_EXECUTION] =
            std::make_unique<Ability::AwaitingExecutionState>(this);
        state = states[AbilityState::IDLE].get();
    }

    CursorAbility::CursorAbility(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        std::unique_ptr<TextureTerrainOverlay> _spellCursor,
        AbilityData _abilityData)
        : Ability(_registry, _abilityData), cursor(_cursor)
    {
        initStates();
        vfx = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
        spellCursor = std::move(_spellCursor);
    }
} // namespace sage
