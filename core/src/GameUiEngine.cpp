//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "slib.hpp"
#include "UserInput.hpp"

#include <ranges>
#include <sstream>

namespace sage
{

    void IdleState::Enter()
    {
        element->OnIdleStart();
    }

    void IdleState::Update()
    {
        auto mousePos = GetMousePosition();
        if (PointInsideRect(element->parent->rec, mousePos))
        {
            element->ChangeState(std::make_unique<HoverState>(element, engine));
        }
    }

    void IdleState::Exit()
    {
        element->OnIdleStop();
    }

    IdleState::IdleState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void HoverState::Enter()
    {
        element->OnHoverStart();
    }

    void HoverState::Exit()
    {
        // if we swap to predrag then this will be called...
        element->OnHoverStop();
    }

    void HoverState::Update()
    {
        // TODO: The easiest thing would be to have a separate state system for Window that is just hovered or not
        // and disables the cursor
        engine->gameData->cursor->DisableContextSwitching();
        engine->gameData->cursor->Disable();

        auto mousePos = GetMousePosition();
        if (!PointInsideRect(element->parent->rec, mousePos))
        {
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            element->OnClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable)
        {
            element->ChangeState(std::make_unique<DragDelayState>(element, engine));
            return;
        }

        // if mouse down on object, change to predragging

        element->HoverUpdate();
    }

    HoverState::HoverState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DragDelayState::Enter()
    {
        element->OnHoverStart();
        dragTimer.SetMaxTime(0.25f); // TODO: Do not use magic number
        dragTimer.SetAutoFinish(false);
    }

    void DragDelayState::Exit()
    {
        engine->hoveredDraggableCellElement.reset();
        element->OnHoverStop();
    }

    void DragDelayState::Update()
    {
        dragTimer.Update(GetFrameTime());
        if (!engine->ObjectBeingDragged() && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (dragTimer.HasExceededMaxTime())
            {
                element->ChangeState(std::make_unique<DragState>(element, engine));
                return;
            }
            if (!dragTimer.IsRunning())
            {
                engine->hoveredDraggableCellElement = element;
                dragTimer.Start();
            }
            if (engine->hoveredDraggableCellElement.has_value() &&
                engine->hoveredDraggableCellElement.value() != element)
            {
                element->ChangeState(std::make_unique<IdleState>(element, engine));
            }
        }
        else
        {
            element->OnClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
        }
    }

    DragDelayState::DragDelayState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DragState::Enter()
    {
        std::cout << "Dragging! \n";
        engine->draggedObject = element;
        element->OnDragStart();
    }

    void DragState::Exit()
    {
        std::cout << "Dropped! \n";
        auto cell = engine->GetCellUnderCursor();
        element->OnDrop(cell);
        engine->draggedObject.reset();
    }

    void DragState::Update()
    {
        // Determine if object is still being dragged
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            // Drop item
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }
        element->DragUpdate(); // Update drag
    }

    void DragState::Draw()
    {
        element->DragDraw();
    }

    DragState::DragState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    UIState::UIState(CellElement* _element, GameUIEngine* _engine) : element(_element), engine(_engine)
    {
    }

    void GameUIEngine::pruneWindows()
    {
        std::vector<unsigned int> toRemove;
        for (unsigned int i = 0; i < windows.size(); ++i)
        {
            auto& window = windows[i];
            if (window->markForRemoval)
            {
                toRemove.push_back(i);
            }
        }

        for (auto& i : toRemove)
        {
            windows.erase(windows.begin() + i);
        }
    }

    Window* GameUIEngine::CreateWindow(
        Texture _nPatchTexture,
        float x,
        float y,
        float _widthPercent,
        float _heightPercent,
        WindowTableAlignment _alignment)
    {
        windows.push_back(std::make_unique<Window>());
        const auto& window = windows.back();
        window->SetPosition(x, y);
        window->SetDimensionsPercent(_widthPercent, _heightPercent);
        window->tableAlignment = _alignment;
        window->settings = gameData->settings;
        window->tex = _nPatchTexture;
        // TODO: Shouldn't SetPosition/SetDimensions already do below?
        window->rec = {
            window->GetPosition().x,
            window->GetPosition().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

        // PlaceWindow(window.get(), window->GetPosition());

        entt::sink sink{gameData->userInput->onWindowUpdate};
        sink.connect<&Window::OnScreenSizeChange>(window.get());
        return window.get();
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(
        Texture _nPatchTexture,
        float _xOffsetPercent,
        float _yOffsetPercent,
        float _widthPercent,
        float _heightPercent,
        WindowTableAlignment _alignment)
    {
        windows.push_back(std::make_unique<WindowDocked>());
        auto* window = dynamic_cast<WindowDocked*>(windows.back().get());
        window->SetOffsetPercent(_xOffsetPercent, _yOffsetPercent);
        window->SetDimensionsPercent(_widthPercent, _heightPercent);
        window->tableAlignment = _alignment;
        window->settings = gameData->settings;
        window->tex = _nPatchTexture;
        window->rec = {
            window->GetOffset().x,
            window->GetOffset().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

        entt::sink sink{gameData->userInput->onWindowUpdate};
        sink.connect<&WindowDocked::OnScreenSizeChange>(window);

        return window;
    }

    void GameUIEngine::PlaceWindow(Window* window, Vector2 requestedPos) const
    {
        window->SetPosition(requestedPos.x, requestedPos.y);
        window->ClampToScreen();
        window->UpdateChildren();

        auto collision = GetWindowCollision(window);

        if (collision)
        {
            window->rec.x = collision->rec.x - window->rec.width;
            window->ClampToScreen();
            collision = GetWindowCollision(window);
            if (collision)
            {
                window->rec.x = collision->rec.x + collision->rec.width;
                window->ClampToScreen();
                collision = GetWindowCollision(window);
                if (collision)
                {
                    assert(0);
                }
            }
        }
    }

    bool GameUIEngine::ObjectBeingDragged() const
    {
        return draggedObject.has_value();
    }

    Window* GameUIEngine::GetWindowCollision(Window* toCheck) const
    {
        for (auto& window : windows)
        {
            if (window.get() == toCheck || window->hidden) continue;
            if (CheckCollisionRecs(window->rec, toCheck->rec))
            {
                return window.get();
            }
        }
        return nullptr;
    }

    CellElement* GameUIEngine::GetCellUnderCursor() const
    {
        Window* windowUnderCursor = nullptr;
        auto mousePos = GetMousePosition();
        for (auto& window : windows)
        {
            if (window->hidden) continue;
            if (PointInsideRect(window->rec, mousePos) && mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                windowUnderCursor = window.get();
            }
        }
        if (windowUnderCursor == nullptr) return nullptr;
        for (auto& table : windowUnderCursor->children)
        {
            for (auto& row : table->children)
            {
                for (auto& cell : row->children)
                {
                    auto element = cell->children.get();
                    if (PointInsideRect(cell->rec, mousePos))
                    {
                        return element;
                    }
                }
            }
        }
        return nullptr;
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
        auto it = std::find_if(windows.begin(), windows.end(), [clicked](const std::unique_ptr<Window>& ptr) {
            return ptr.get() == clicked;
        });
        std::rotate(it, it + 1, windows.end());
    }

    void GameUIEngine::DrawDebug2D() const
    {
        for (const auto& window : windows)
        {
            if (window->hidden) continue;
            window->DrawDebug2D();
        }
    }

    void GameUIEngine::Draw2D() const
    {
        for (const auto& window : windows)
        {
            if (window->hidden) continue;
            window->Draw2D();
        }

        if (draggedObject.has_value())
        {
            draggedObject.value()->state->Draw();
        }
    }

    /**
     *
     * @return Whether the window region is not obscured by another window
     */
    bool GameUIEngine::mouseInNonObscuredWindowRegion(Window* window, Vector2 mousePos) const
    {
        auto collision = GetWindowCollision(window);
        if (collision)
        {
            // check if window is lower
            auto windowIt =
                std::find_if(windows.begin(), windows.end(), [window](const std::unique_ptr<Window>& ptr) {
                    return ptr.get() == window;
                });

            auto colIt =
                std::find_if(windows.begin(), windows.end(), [collision](const std::unique_ptr<Window>& ptr) {
                    return ptr.get() == collision;
                });

            auto windowDist = std::distance(windows.begin(), windowIt);
            auto colDist = std::distance(windows.begin(), colIt);

            if (windowDist < colDist)
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

        for (auto& window : std::ranges::reverse_view(windows))
        {
            if (!window || window->hidden) continue;

            if (!PointInsideRect(window->rec, mousePos) || !mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                window->OnHoverStop();
                continue;
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                BringClickedWindowToFront(window.get());
            }

            gameData->cursor->Disable();
            gameData->cursor->DisableContextSwitching();

            window->OnHoverStart(); // TODO: Need to check if it was already being hovered?
            for (const auto& table : window->children)
            {
                for (const auto& row : table->children)
                {
                    for (const auto& cell : row->children)
                    {
                        auto element = cell->children.get();
                        element->state->Update();
                    }
                }
            }
        }
    }

    void GameUIEngine::Update()
    {
        if (draggedObject.has_value())
        {
            draggedObject.value()->state->Update();
        }
        else
        {
            gameData->cursor->Enable();
            gameData->cursor->EnableContextSwitching();
            pruneWindows();
            processWindows();
        }
    }

    GameUIEngine::GameUIEngine(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage