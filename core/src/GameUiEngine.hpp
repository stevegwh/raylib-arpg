//
// Created by steve on 02/10/2024.
//

#pragma once

#include "GameUiElements.hpp"

#include "raylib.h"
#include "Timer.hpp"
#include <iostream>
#include <optional>
#include <vector>

namespace sage
{
    class GameUIEngine;
    class UIState;

    struct Settings;
    class UserInput;
    class Cursor;

    class UIState
    {
      protected:
        UIElement* element{};
        GameUIEngine* engine{};

      public:
        virtual void Update() = 0;
        virtual void Draw() = 0;
        virtual void Enter() = 0;
        virtual void Exit() = 0;
        virtual ~UIState() = default;
        explicit UIState(UIElement* _element, GameUIEngine* _engine);
    };

    class IdleState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~IdleState() override;                                          // OnExit
        explicit IdleState(UIElement* _element, GameUIEngine* _engine); // OnEnter
    };

    class HoveredState : public UIState
    {
        Timer dragTimer;

      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~HoveredState() override;
        explicit HoveredState(UIElement* _element, GameUIEngine* _engine);
    };

    class DraggingState : public UIState
    {
      public:
        Rectangle originalPosition{};
        Vector2 mouseOffset{};
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~DraggingState() override;                                          // OnExit
        explicit DraggingState(UIElement* _element, GameUIEngine* _engine); // OnEnter
    };

    class DroppingState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~DroppingState() override;                                          // OnExit
        explicit DroppingState(UIElement* _element, GameUIEngine* _engine); // OnEnter
    };

    class GameUIEngine
    {
        std::vector<std::unique_ptr<Window>> windows;
        std::vector<std::unique_ptr<Window>> delayedWindows;
        UserInput* userInput;
        Settings* settings;

        std::optional<UIElement*> draggedObject;
        std::optional<CellElement*> hoveredDraggableCellElement;

        void pruneWindows();
        void processWindows() const;

      public:
        Cursor* cursor;

        Window* CreateWindow(
            Texture _nPatchTexture,
            float x,
            float y,
            float _widthPercent,
            float _heightPercent,
            WindowTableAlignment _alignment = WindowTableAlignment::STACK_HORIZONTAL);

        WindowDocked* CreateWindowDocked(
            Texture _nPatchTexture,
            float _xOffsetPercent,
            float _yOffsetPercent,
            float _widthPercent,
            float _heightPercent,
            WindowTableAlignment _alignment = WindowTableAlignment::STACK_HORIZONTAL);

        [[nodiscard]] CellElement* GetCellUnderCursor() const;
        void DrawDebug2D() const;
        void Draw2D() const;
        void Update();

        GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor);
        friend class UIState;
        friend class DroppingState;
        friend class DraggingState;
        friend class HoveredState;
    };
} // namespace sage
