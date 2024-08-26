//
// Created by Steve Wheeler on 21/07/2024.
//

#include "CursorAbility.hpp"

#include "AbilityFunctions.hpp"
#include "AbilityIndicator.hpp"
#include "AbilityResourceManager.hpp"
#include "AbilityState.hpp"
#include "components/Animation.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "TextureTerrainOverlay.hpp"
#include "vfx/VisualFX.hpp"

namespace sage
{
    // --------------------------------------------

    class CursorAbility::CursorSelectState : public AbilityState
    {
        Cursor* cursor;
        std::unique_ptr<AbilityIndicator> abilityIndicator;
        bool cursorActive = false;
        void enableCursor();
        void disableCursor();
        void toggleCursor(entt::entity self);

      public:
        entt::sigh<void(entt::entity)> onConfirm;
        void Update(entt::entity self) override;
        void OnEnter(entt::entity self) override;
        void OnExit(entt::entity self) override;

        CursorSelectState(
            Timer& _coolDownTimer,
            Timer& _animationDelayTimer,
            Cursor* _cursor,
            std::unique_ptr<AbilityIndicator> _abilityIndicator)
            : AbilityState(_coolDownTimer, _animationDelayTimer),
              cursor(_cursor),
              abilityIndicator(std::move(_abilityIndicator))
        {
        }
    };

    void CursorAbility::CursorSelectState::enableCursor()
    {
        abilityIndicator->Init(cursor->terrainCollision().point);
        abilityIndicator->Enable(true);
        cursor->Disable();
        cursor->Hide();
    }

    void CursorAbility::CursorSelectState::disableCursor()
    {
        cursor->Enable();
        cursor->Show();
        abilityIndicator->Enable(false);
    }

    void CursorAbility::CursorSelectState::Update(entt::entity self)
    {
        abilityIndicator->Update(cursor->terrainCollision().point);
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

    void CursorAbility::confirm(entt::entity self)
    {
        // if (vfx)
        // {
        //     vfx->InitSystem(cursor->terrainCollision().point); // TODO: Add a target parameter in abilityData
        // }
        Ability::Init(self);
    }

    CursorAbility::~CursorAbility()
    {
    }

    CursorAbility::CursorAbility(entt::registry* _registry, AbilityData _abilityData, GameData* _gameData)
        : Ability(_registry, _abilityData, _gameData), cursor(_gameData->cursor.get())
    {
        auto cursorState = std::make_unique<CursorSelectState>(
            cooldownTimer,
            animationDelayTimer,
            _gameData->cursor.get(),
            AbilityResourceManager::GetInstance().GetIndicator(abilityData.indicator, _gameData));
        entt::sink onConfirmSink{cursorState->onConfirm};
        onConfirmSink.connect<&CursorAbility::confirm>(this);
        states[AbilityStateEnum::CURSOR_SELECT] = std::move(cursorState);
    }
} // namespace sage
