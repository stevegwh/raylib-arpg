//
// Created by steve on 02/10/2024.
//

#pragma once

#include "GameUiElements.hpp"

#include "raylib.h"
#include "Timer.hpp"

#include <entt/entt.hpp>
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

    class DraggedObject
    {
      protected:
      public:
        Vector2 mouseOffset{};
        virtual UIElement* GetElement() = 0;
        virtual ~DraggedObject() = default;
        virtual void Update() = 0;
        virtual void Draw() = 0;
        virtual void OnDrop(){};
    };

    class DraggedWindow : public DraggedObject
    {
        Window* element{};

      public:
        UIElement* GetElement() override;
        void Update() override;
        void Draw() override;
        void OnDrop() override;
        explicit DraggedWindow(Window* _window);
    };

    class DraggedCellElement : public DraggedObject
    {
        CellElement* element{};

      public:
        UIElement* GetElement() override;
        void Update() override;
        void Draw() override;
        void OnDrop() override;
        explicit DraggedCellElement(CellElement* _element);
        ~DraggedCellElement() override;
    };

    class UIState
    {
      protected:
        UIElement* element{};

      public:
        virtual void Update() = 0;
        virtual void Draw() = 0;
        virtual void Enter() = 0;
        virtual void Exit() = 0;
        virtual ~UIState() = default;
        explicit UIState(UIElement* _element);
    };

    class IdleState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~IdleState() override;                   // OnExit
        explicit IdleState(UIElement* _element); // OnEnter
    };

    class HoveredState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~HoveredState() override;
        explicit HoveredState(UIElement* _element);
    };

    class PreDraggingState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~PreDraggingState() override;                   // OnExit
        explicit PreDraggingState(UIElement* _element); // OnEnter
    };

    class DraggingState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~DraggingState() override;                   // OnExit
        explicit DraggingState(UIElement* _element); // OnEnter
    };

    class DroppingState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~DroppingState() override;                   // OnExit
        explicit DroppingState(UIElement* _element); // OnEnter
    };

    class GameUIEngine
    {
        Timer dragTimer;
        static constexpr float draggedTimerThreshold = 0.25f;
        std::vector<std::unique_ptr<Window>> windows;
        std::vector<std::unique_ptr<Window>> delayedWindows;
        UserInput* userInput;
        Settings* settings;

        std::optional<std::unique_ptr<DraggedObject>> draggedElement;
        std::optional<CellElement*> hoveredDraggableElement{};

        void clearAllHover(unsigned int start, unsigned int end);
        void pruneWindows();
        void processCell(CellElement* cell, Window* window, bool& elementFound, const Vector2& mousePos);
        void processWindows(const Vector2& mousePos);
        void cleanUpDragState();

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

        void DrawDebug2D() const;
        void Draw2D() const;
        void Update();

        GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor);
    };
} // namespace sage
