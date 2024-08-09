#pragma once

#include "Ability.hpp"

#include "particle/RainOfFireVFX.hpp"
#include "TextureTerrainOverlay.hpp"

#include <memory>

namespace sage
{
    class NavigationGridSystem;
    class Cursor;
    class Camera;

    class RainOfFireAbility : public Ability
    {
        Timer animationDelayTimer{};
        std::unique_ptr<RainOfFireVFX> vfx;
        Cursor* cursor;
        std::unique_ptr<TextureTerrainOverlay> spellCursor;
        float whirlwindRadius = 50.0f;

        class IdleState : public State
        {
            RainOfFireAbility* ability;

          public:
            void Update(entt::entity self) override;
            void Draw3D(entt::entity self) override;
            IdleState(RainOfFireAbility* _ability) : ability(_ability)
            {
            }
        };

        class CursorSelectState : public State
        {
            RainOfFireAbility* ability;
            bool cursorActive = false;
            void enableCursor();
            void disableCursor();
            void toggleCursor(entt::entity self);

          public:
            void Update(entt::entity self) override;
            void OnEnter(entt::entity self) override;
            void OnExit(entt::entity self) override;
            CursorSelectState(RainOfFireAbility* _ability) : ability(_ability)
            {
            }
        };

        class AwaitingExecutionState : public State
        {
            RainOfFireAbility* ability;

          public:
            void Update(entt::entity self) override;
            void OnEnter(entt::entity self) override;
            AwaitingExecutionState(RainOfFireAbility* _ability) : ability(_ability)
            {
            }
        };

      protected:
        virtual void confirm(entt::entity self);

      public:
        void Init(entt::entity self) override;
        void Execute(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void Update(entt::entity self) override;
        ~RainOfFireAbility() override = default;
        RainOfFireAbility(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            NavigationGridSystem* _navigationGridSystem);
    };
} // namespace sage