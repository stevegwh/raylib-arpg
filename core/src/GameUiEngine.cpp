//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "UserInput.hpp"

#include <sstream>

namespace sage
{
    bool MouseInside(Rectangle rec, Vector2 mousePos)
    {
        return mousePos.x >= rec.x && mousePos.x <= rec.x + rec.width && mousePos.y >= rec.y &&
               mousePos.y <= rec.y + rec.height;
    }

    void IdleState::Enter()
    {
        element->OnIdleStart();
    }

    void IdleState::Update()
    {
        auto mousePos = GetMousePosition();
        if (MouseInside(element->parent->rec, mousePos))
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
        if (!MouseInside(element->parent->rec, mousePos))
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
        window->rec = {
            window->GetPosition().x,
            window->GetPosition().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

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

    bool GameUIEngine::ObjectBeingDragged() const
    {
        return draggedObject.has_value();
    }

    Window* GameUIEngine::GetWindowCollision(Window* toCheck) const
    {
        for (auto& window : windows)
        {
            if (window.get() == toCheck) continue;
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
            if (MouseInside(window->rec, mousePos))
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
                    if (MouseInside(cell->rec, mousePos))
                    {
                        return element;
                    }
                }
            }
        }
        return nullptr;
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

    void GameUIEngine::processWindows() const
    {
        const auto mousePos = GetMousePosition();
        const auto windowCount = windows.size();
        for (const auto& window : windows)
        {
            // Make sure we do not process newly added windows this cycle
            if (windows.size() > windowCount) break;
            if (window->hidden) continue;

            if (!MouseInside(window->rec, mousePos))
            {
                window->OnHoverStop();
                continue;
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