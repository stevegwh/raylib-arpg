#pragma once

#include "AbilityStateMachine.hpp"

#include <memory>

namespace sage
{
    class AbilityIndicator;
    class Cursor;

    class CursorAbility : public AbilityStateMachine
    {
        class CursorSelectState;
        Cursor* cursor;

      protected:
        virtual void confirm();
        CursorAbility(
            entt::registry* _registry, entt::entity _self, AbilityData _abilityData, GameData* _gameData);

      public:
        bool IsActive() override;
        void Init() override;
        // void Execute(entt::entity self) override;
        ~CursorAbility() override;
    };
} // namespace sage