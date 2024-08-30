//
// Created by Steve Wheeler on 21/07/2024.
//

#include "CursorAbility.hpp"

#include "AbilityData.hpp"
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
        std::unique_ptr<AbilityIndicator> abilityIndicator;
        bool cursorActive = false;

        void enableCursor()
        {
            abilityIndicator->Init(gameData->cursor->terrainCollision().point);
            abilityIndicator->Enable(true);
            gameData->cursor->Disable();
            gameData->cursor->Hide();
        }

        void disableCursor()
        {
            gameData->cursor->Enable();
            gameData->cursor->Show();
            abilityIndicator->Enable(false);
        }

        void toggleCursor()
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

      public:
        entt::sigh<void(entt::entity)> onConfirm;
        void Update() override
        {
            abilityIndicator->Update(gameData->cursor->terrainCollision().point);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                onConfirm.publish(caster);
            }
        }

        void OnEnter() override
        {
            enableCursor();
            cursorActive = true;
        }

        void OnExit() override
        {
            if (cursorActive)
            {
                disableCursor();
                cursorActive = false;
            }
        }

        CursorSelectState(
            entt::registry* _registry,
            entt::entity _caster,
            entt::entity _abilityEntity,
            GameData* _gameData,
            Timer& cooldownTimer,
            Timer& animationDelayTimer,
            std::unique_ptr<AbilityIndicator> _abilityIndicator)
            : AbilityState(_registry, _caster, _abilityEntity, _gameData, cooldownTimer, animationDelayTimer),
              abilityIndicator(std::move(_abilityIndicator))
        {
        }
    };

    // --------------------------------------------

    bool CursorAbility::IsActive()
    {
        const auto current = states[AbilityStateEnum::CURSOR_SELECT].get();
        return AbilityStateMachine::IsActive() || state == current;
    }

    void CursorAbility::Init()
    {
        if (state == states[AbilityStateEnum::CURSOR_SELECT].get())
        {
            ChangeState(AbilityStateEnum::IDLE);
        }
        else
        {
            ChangeState(AbilityStateEnum::CURSOR_SELECT);
        }
    }

    void CursorAbility::confirm()
    {
        AbilityStateMachine::Init();
    }

    CursorAbility::~CursorAbility()
    {
    }

    CursorAbility::CursorAbility(
        entt::registry* _registry, entt::entity _caster, entt::entity _abilityEntity, GameData* _gameData)
        : AbilityStateMachine(_registry, _caster, _abilityEntity, _gameData)
    {
        auto& abilityData = registry->get<AbilityData>(abilityEntity);
        auto cursorState = std::make_unique<CursorSelectState>(
            _registry,
            _caster,
            _abilityEntity,
            _gameData,
            cooldownTimer,
            animationDelayTimer,
            AbilityResourceManager::GetInstance().GetIndicator(abilityData.indicator, _gameData));
        entt::sink onConfirmSink{cursorState->onConfirm};
        onConfirmSink.connect<&CursorAbility::confirm>(this);
        states[AbilityStateEnum::CURSOR_SELECT] = std::move(cursorState);
    }
} // namespace sage
