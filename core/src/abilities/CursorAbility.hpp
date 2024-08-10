#pragma once

#include "Ability.hpp"

#include <memory>

namespace sage
{
    class Cursor;
    class Camera;

    class CursorAbility : public Ability
    {
        Cursor* cursor;
        float whirlwindRadius = 50.0f;

        class IdleState : public State
        {
            CursorAbility* ability;

          public:
            void Update(entt::entity self) override;
            void Draw3D(entt::entity self) override;
            IdleState(CursorAbility* _ability) : ability(_ability)
            {
            }
        };

        class CursorSelectState : public State
        {
            CursorAbility* ability;
            bool cursorActive = false;
            void enableCursor();
            void disableCursor();
            void toggleCursor(entt::entity self);

          public:
            void Update(entt::entity self) override;
            void OnEnter(entt::entity self) override;
            void OnExit(entt::entity self) override;
            CursorSelectState(CursorAbility* _ability) : ability(_ability)
            {
            }
        };

        class AwaitingExecutionState : public State
        {
            CursorAbility* ability;

          public:
            void Update(entt::entity self) override;
            void OnEnter(entt::entity self) override;
            AwaitingExecutionState(CursorAbility* _ability) : ability(_ability)
            {
            }
        };

        void initStates();

      protected:
        virtual void confirm(entt::entity self);
        CursorAbility(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            std::unique_ptr<TextureTerrainOverlay> _spellCursor,
            AbilityData _abilityData);

      public:
        void Init(entt::entity self) override;
        void Execute(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void Update(entt::entity self) override;
        ~CursorAbility() override = default;
    };
} // namespace sage