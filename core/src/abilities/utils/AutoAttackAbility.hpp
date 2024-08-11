#pragma once

#include "Ability.hpp"

namespace sage
{
    class AutoAttackAbility : public Ability
    {
        class IdleState : public State
        {
            AutoAttackAbility* ability;

          public:
            void Update(entt::entity self) override;
            void Draw3D(entt::entity self) override;
            IdleState(AutoAttackAbility* _ability) : ability(_ability)
            {
            }
        };

        class AwaitingExecutionState : public State
        {
            AutoAttackAbility* ability;

          public:
            void OnEnter(entt::entity self) override;
            void Update(entt::entity self) override;
            AwaitingExecutionState(AutoAttackAbility* _ability) : ability(_ability)
            {
            }
        };

        void initStates();

      protected:
        AutoAttackAbility(entt::registry* _registry, AbilityData _abilityData);

      public:
        void Execute(entt::entity self) override;
        void Update(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void Init(entt::entity self) override;
        void Cancel(entt::entity self);
        ~AutoAttackAbility() override = default;
    };
} // namespace sage