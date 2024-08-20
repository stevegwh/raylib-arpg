#pragma once

#include "Ability.hpp"
#include "AbilityIndicator.hpp"
#include "Cursor.hpp"
#include "TextureTerrainOverlay.hpp"

#include <memory>

namespace sage
{

    class CursorAbility : public Ability
    {
        Cursor* cursor;
        float whirlwindRadius = 50.0f;

        class CursorSelectState : public AbilityState
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

      protected:
        virtual void confirm(entt::entity self);
        CursorAbility(
            entt::registry* _registry,
            AbilityData _abilityData,
            GameData* _gameData,
            std::unique_ptr<AbilityIndicator> _abilityIndicator);

      public:
        void Init(entt::entity self) override;
        // void Execute(entt::entity self) override;
        ~CursorAbility() override = default;
    };
} // namespace sage