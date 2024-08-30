#pragma once

#include "AbilityStateMachine.hpp"

namespace sage
{
    class AbilityIndicator;

    class CursorAbility : public AbilityStateMachine
    {
        class CursorSelectState;

      protected:
        virtual void confirm();
        CursorAbility(
            entt::registry* _registry, entt::entity _caster, entt::entity _abilityEntity, GameData* _gameData);

      public:
        bool IsActive() override;
        void Init() override;
        // void Execute(entt::entity caster) override;
        ~CursorAbility() override;
    };
} // namespace sage