#pragma once

#include "Ability.hpp"

#include <memory>

namespace sage
{
    class AbilityIndicator;
    class Cursor;

    class CursorAbility : public Ability
    {
        class CursorSelectState;
        Cursor* cursor;

      protected:
        virtual void confirm(entt::entity self);
        CursorAbility(entt::registry* _registry, AbilityData _abilityData, GameData* _gameData);

      public:
        bool IsActive() override;
        void Init(entt::entity self) override;
        // void Execute(entt::entity self) override;
        ~CursorAbility() override;
    };
} // namespace sage