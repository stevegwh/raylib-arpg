//
// GameUIEngine: top-level UI manager. Owns the window list, the tooltip window,
// drag/hover tracking, and orchestrates per-frame Update / Draw2D.
//

#pragma once

#include "UIBase.hpp"
#include "UIWindow.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sage
{
    class EngineSystems;
    class UserInput;
    class Cursor;
    struct Settings;

    class GameUIEngine
    {

      protected:
        std::optional<ErrorMessage> errorMessage;
        std::vector<std::unique_ptr<Window>> windows;
        std::unique_ptr<TooltipWindow> tooltipWindow;
        std::optional<CellElement*> draggedObject;
        std::optional<CellElement*> hoveredDraggableCellElement;
        InputSnapshot currentInput;

        void pruneWindows();
        void processWindows();

        [[nodiscard]] bool mouseInNonObscuredWindowRegion(Window* window, Vector2 mousePos) const;

      public:
        entt::registry* registry;
        UserInput* userInput;
        Cursor* cursor;
        Settings* settings;
        GameUIEngine(entt::registry* _registry, const EngineSystems* _sys);
        virtual ~GameUIEngine() = default;

        void BringClickedWindowToFront(Window* clicked);
        void CreateErrorMessage(const std::string& msg);
        TooltipWindow* CreateTooltipWindow(std::unique_ptr<TooltipWindow> _tooltipWindow);
        Window* CreateWindow(std::unique_ptr<Window> _window);
        WindowDocked* CreateWindowDocked(std::unique_ptr<WindowDocked> _windowDocked);

        [[nodiscard]] static Rectangle GetOverlap(Rectangle rec1, Rectangle rec2);
        [[nodiscard]] bool ObjectBeingDragged() const;
        [[nodiscard]] Window* GetWindowCollision(const Window* toCheck) const;
        [[nodiscard]] CellElement* GetCellUnderCursor() const;
        [[nodiscard]] bool IsMouseOverWindow() const;
        void DrawDebug2D() const;
        void Draw2D() const;
        void Update();

        // The current frame's input snapshot. Captured at the top of Update().
        [[nodiscard]] const InputSnapshot& Input() const { return currentInput; }

        // Drag/hover-lock accessors used by the state machine. Replaces the friend
        // declarations the state classes used to need to mutate the protected fields.
        void SetDraggedObject(CellElement* el) { draggedObject = el; }
        void ClearDraggedObject() { draggedObject.reset(); }
        void ClaimHoveredDraggableLock(CellElement* el) { hoveredDraggableCellElement = el; }
        void ReleaseHoveredDraggableLock() { hoveredDraggableCellElement.reset(); }
        [[nodiscard]] std::optional<CellElement*> GetHoveredDraggableLock() const
        {
            return hoveredDraggableCellElement;
        }
    };
} // namespace sage
