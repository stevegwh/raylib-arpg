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
        float whirlwindRadius = 50.0f;

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