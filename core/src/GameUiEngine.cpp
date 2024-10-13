//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"
#include "Cursor.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"

#include <sstream>

namespace sage
{
    bool MouseInside(Rectangle rec, Vector2 mousePos)
    {
        return mousePos.x >= rec.x && mousePos.x <= rec.x + rec.width && mousePos.y >= rec.y &&
               mousePos.y <= rec.y + rec.height;
    }

    void IdleState::Update()
    {
        auto mousePos = GetMousePosition();
        if (MouseInside(element->rec, mousePos))
        {
            element->ChangeState(std::make_unique<HoveredState>(element, engine));
        }
    }

    IdleState::IdleState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void HoveredState::Enter()
    {
        element->OnMouseStartHover();
    }

    void HoveredState::Exit()
    {
        // if we swap to predrag then this will be called...
        element->OnMouseStopHover();
    }

    void HoveredState::Update()
    {
        // TODO: The easiest thing would be to have a separate state system for Window that is just hovered or not
        // and disables the cursor
        engine->cursor->DisableContextSwitching();
        engine->cursor->Disable();

        auto mousePos = GetMousePosition();
        if (!MouseInside(element->rec, mousePos))
        {
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            element->OnMouseClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable)
        {
            element->ChangeState(std::make_unique<PreDraggingState>(element, engine));
            return;
        }

        // if mouse down on object, change to predragging

        element->MouseHoverUpdate();
    }

    HoveredState::HoveredState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void PreDraggingState::Enter()
    {
        element->OnMouseStartHover();
        dragTimer.SetMaxTime(0.25f); // TODO: Do not use magic number
        dragTimer.SetAutoFinish(false);
    }

    void PreDraggingState::Exit()
    {
        engine->hoveredDraggableCellElement.reset();
        element->OnMouseStopHover();
    }

    void PreDraggingState::Update()
    {
        dragTimer.Update(GetFrameTime());
        if (!engine->ObjectBeingDragged() && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (dragTimer.HasExceededMaxTime())
            {
                element->ChangeState(std::make_unique<DraggingState>(element, engine));
                return;
            }
            if (!dragTimer.IsRunning())
            {
                engine->hoveredDraggableCellElement = element;
                dragTimer.Start();
                // Set hovered object
            }
            if (engine->hoveredDraggableCellElement.has_value() &&
                engine->hoveredDraggableCellElement.value() != element)
            {
                element->ChangeState(std::make_unique<IdleState>(element, engine));
            }
        }
        else
        {
            element->OnMouseClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
        }
    }

    PreDraggingState::PreDraggingState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DraggingState::Enter()
    {
        std::cout << "Dragging! \n";
        engine->draggedObject = element;
        element->OnMouseStartDrag();
    }

    void DraggingState::Exit()
    {
        std::cout << "Dropped! \n";
        // TODO: This only gets called when window is being hovered?
        auto cell = engine->GetCellUnderCursor();
        element->OnDropped(cell);
        engine->draggedObject.reset();
    }

    void DraggingState::Update()
    {
        // Determine if object is still being dragged
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            // Drop item
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }
        element->MouseDragUpdate(); // Update drag
    }

    void DraggingState::Draw()
    {
        element->MouseDragDraw();
    }

    DraggingState::DraggingState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    UIState::UIState(CellElement* _element, GameUIEngine* _engine) : element(_element), engine(_engine)
    {
    }

    void GameUIEngine::pruneWindows()
    {
        for (auto it = windows.begin(); it != windows.end();)
        {
            auto& window = *it;
            if (window->markForRemoval)
            {
                windows.erase(it);
            }
            else
            {
                ++it;
            }
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
        window->settings = settings;
        window->tex = _nPatchTexture;
        window->rec = {
            window->GetPosition().x,
            window->GetPosition().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

        entt::sink sink{userInput->onWindowUpdate};
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
        window->settings = settings;
        window->tex = _nPatchTexture;
        window->rec = {
            window->GetOffset().x,
            window->GetOffset().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

        entt::sink sink{userInput->onWindowUpdate};
        sink.connect<&WindowDocked::OnScreenSizeChange>(window);

        return window;
    }

    bool GameUIEngine::ObjectBeingDragged() const
    {
        return draggedObject.has_value();
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
                    if (MouseInside(element->rec, mousePos))
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
                window->OnMouseStopHover();
                return;
            }

            cursor->Disable();
            cursor->DisableContextSwitching();

            window->OnMouseStartHover(); // TODO: Need to check if it was already being hovered?
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
            cursor->Enable();
            cursor->EnableContextSwitching();
            pruneWindows();
            processWindows();
        }
    }

    GameUIEngine::GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor)
        : cursor(_cursor), userInput(_userInput), settings(_settings)
    {
    }
} // namespace sage