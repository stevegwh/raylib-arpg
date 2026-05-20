//
// GameUIEngine implementation. See GameUIEngine.hpp.
//

#include "GameUIEngine.hpp"

#include "../Cursor.hpp"
#include "../EngineSystems.hpp"
#include "../Settings.hpp"
#include "../UserInput.hpp"
#include "../slib.hpp"

#include "raylib.h"

#include <algorithm>
#include <ranges>

namespace sage
{
    void GameUIEngine::pruneWindows()
    {
        if (tooltipWindow && tooltipWindow->IsMarkedForRemoval())
        {
            tooltipWindow.reset();
        }

        windows.erase(
            std::ranges::remove_if(windows, [](const auto& window) { return window->IsMarkedForRemoval(); })
                .begin(),
            windows.end());
    }

    void GameUIEngine::CreateErrorMessage(const std::string& msg)
    {
        errorMessage.emplace(settings, msg);
    }

    TooltipWindow* GameUIEngine::CreateTooltipWindow(std::unique_ptr<TooltipWindow> _tooltipWindow)
    {
        tooltipWindow = std::move(_tooltipWindow);
        tooltipWindow->windowUpdateSub = userInput->onWindowUpdate.Subscribe(
            [this](Vector2 prev, Vector2 current) { tooltipWindow->OnWindowUpdate(prev, current); });

        tooltipWindow->InitLayout();
        // FinalizeLayout called externally
        return tooltipWindow.get();
    }

    Window* GameUIEngine::CreateWindow(std::unique_ptr<Window> _window)
    {
        windows.push_back(std::move(_window));
        auto* window = windows.back().get();
        window->windowUpdateSub = userInput->onWindowUpdate.Subscribe(
            [window](Vector2 prev, Vector2 current) { window->OnWindowUpdate(prev, current); });
        window->InitLayout();
        return window;
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(std::unique_ptr<WindowDocked> _windowDocked)
    {
        windows.push_back(std::move(_windowDocked));
        auto* window = dynamic_cast<WindowDocked*>(windows.back().get());
        window->windowUpdateSub = userInput->onWindowUpdate.Subscribe(
            [window](Vector2 prev, Vector2 current) { window->OnWindowUpdate(prev, current); });
        window->InitLayout();
        return window;
    }

    bool GameUIEngine::ObjectBeingDragged() const
    {
        return draggedObject.has_value();
    }

    Window* GameUIEngine::GetWindowCollision(const Window* toCheck) const
    {
        for (auto& window : windows)
        {
            if (window.get() == toCheck || window->IsHidden()) continue;
            if (CheckCollisionRecs(window->rec, toCheck->rec))
            {
                return window.get();
            }
        }
        return nullptr;
    }

    CellElement* GameUIEngine::GetCellUnderCursor() const
    {
        const auto mousePos = GetMousePosition();
        for (auto windowIt = windows.rbegin(); windowIt != windows.rend(); ++windowIt)
        {
            auto& window = *windowIt;
            if (window->IsHidden()) continue;

            if (!window->CapturesCursor(mousePos)) continue;

            for (const auto& child : window->children)
            {
                if (child->CapturesCursor(mousePos))
                {
                    if (auto childElement = child->GetCellUnderCursor())
                    {
                        return childElement;
                    }
                }
            }
        }

        for (auto windowIt = windows.rbegin(); windowIt != windows.rend(); ++windowIt)
        {
            auto& window = *windowIt;
            if (window->IsHidden()) continue;
            if (!PointInsideRect(window->rec, mousePos)) continue;
            if (!mouseInNonObscuredWindowRegion(window.get(), mousePos)) continue;

            for (const auto& child : window->children)
            {
                if (auto childElement = child->GetCellUnderCursor())
                {
                    return childElement;
                }
            }
            return nullptr;
        }
        return nullptr;
    }

    bool GameUIEngine::IsMouseOverWindow() const
    {
        const auto mousePos = GetMousePosition();
        for (const auto& window : windows)
        {
            if (!window || window->IsHidden()) continue;
            if (PointInsideRect(window->rec, mousePos) && mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                return true;
            }
            if (window->CapturesCursor(mousePos))
            {
                return true;
            }
        }

        return tooltipWindow && !tooltipWindow->hidden && PointInsideRect(tooltipWindow->rec, mousePos);
    }

    Rectangle GameUIEngine::GetOverlap(Rectangle rec1, Rectangle rec2)
    {
        float x1 = std::max(rec1.x, rec2.x);
        float y1 = std::max(rec1.y, rec2.y);
        float x2 = std::min(rec1.x + rec1.width, rec2.x + rec2.width);
        float y2 = std::min(rec1.y + rec1.height, rec2.y + rec2.height);

        if (x1 < x2 && y1 < y2)
        {
            Rectangle overlap;
            overlap.x = x1;
            overlap.y = y1;
            overlap.width = x2 - x1;
            overlap.height = y2 - y1;
            return overlap;
        }
        return Rectangle{0, 0, 0, 0};
    }

    void GameUIEngine::BringClickedWindowToFront(Window* clicked)
    {
        const auto it = std::ranges::find_if(
            windows, [clicked](const std::unique_ptr<Window>& ptr) { return ptr.get() == clicked; });
        std::rotate(it, it + 1, windows.end());
    }

    void GameUIEngine::DrawDebug2D() const
    {
        for (const auto& window : windows)
        {
            if (window->IsHidden()) continue;
            window->DrawDebug2D();
        }
    }

    void GameUIEngine::Draw2D() const
    {
        overlayDrawQueue.clear();

        for (const auto& window : windows)
        {
            if (window->IsHidden()) continue;
            window->Draw2D();
        }

        for (const auto& drawOverlay : overlayDrawQueue)
        {
            if (drawOverlay) drawOverlay();
        }
        overlayDrawQueue.clear();

        if (tooltipWindow && !tooltipWindow->hidden)
        {
            tooltipWindow->Draw2D();
        }

        if (draggedObject.has_value())
        {
            drawUIState(*draggedObject.value());
        }

        if (errorMessage.has_value())
        {
            errorMessage->Draw2D();
        }
    }

    void GameUIEngine::QueueOverlayDraw(std::function<void()> draw) const
    {
        overlayDrawQueue.push_back(std::move(draw));
    }

    /**
     *
     * @return Whether the window region is not obscured by another window
     */
    bool GameUIEngine::mouseInNonObscuredWindowRegion(Window* window, Vector2 mousePos) const
    {
        if (auto collision = GetWindowCollision(window))
        {
            // check if window is lower
            auto windowIt = std::ranges::find_if(
                windows, [window](const std::unique_ptr<Window>& ptr) { return ptr.get() == window; });

            const auto colIt = std::ranges::find_if(
                windows, [collision](const std::unique_ptr<Window>& ptr) { return ptr.get() == collision; });

            const auto windowDist = std::distance(windows.begin(), windowIt);

            if (auto colDist = std::distance(windows.begin(), colIt); windowDist < colDist)
            {
                auto rec = GetOverlap(window->rec, collision->rec);
                if (PointInsideRect(rec, mousePos))
                {
                    // this part of the window is being obscured by another
                    return false;
                }
            }
        }
        return true;
    }

    void GameUIEngine::processWindows()
    {
        const auto mousePos = GetMousePosition();

        for (auto& window : windows)
        {
            if (!window || window->IsMarkedForRemoval() || window->IsHidden()) continue;

            const bool insideWindow = PointInsideRect(window->rec, mousePos);
            const bool capturesCursor = window->CapturesCursor(mousePos);
            if ((!insideWindow && !capturesCursor) ||
                (insideWindow && !mouseInNonObscuredWindowRegion(window.get(), mousePos)))
            {
                window->OnHoverStop();
                continue;
            }

            if (insideWindow && (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
            {
                BringClickedWindowToFront(window.get());
            }

            cursor->Disable();
            cursor->DisableContextSwitching();

            window->OnHoverStart(); // TODO: Need to check if it was already being hovered?
            window->Update();
        }
    }

    void GameUIEngine::Update()
    {
        // Snapshot raylib input once per frame so the state machine doesn't pull
        // from globals; everyone reads via Input() for the rest of the pass.
        currentInput = InputSnapshot::Capture();

        if (draggedObject.has_value())
        {
            updateUIState(*draggedObject.value(), *this, currentInput);
        }
        else
        {
            cursor->Enable();
            cursor->EnableContextSwitching();
            processWindows();
            pruneWindows();
        }

        if (errorMessage.has_value() && errorMessage->Finished())
        {
            errorMessage.reset();
        }
    }

    GameUIEngine::GameUIEngine(entt::registry* _registry, const EngineSystems* _sys)
        : registry(_registry),
          userInput(_sys->userInput.get()),
          cursor(_sys->cursor.get()),
          settings(_sys->settings)
    {
    }
} // namespace sage
