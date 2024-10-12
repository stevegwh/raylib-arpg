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

    void IdleState::Enter()
    {
    }

    void IdleState::Exit()
    {
    }

    void IdleState::Update()
    {
        auto mousePos = GetMousePosition();
        if (MouseInside(element->rec, mousePos))
        {
            element->ChangeState(std::make_unique<HoveredState>(element, engine));
        }
    }

    void IdleState::Draw()
    {
    }

    IdleState::~IdleState()
    {
    }

    IdleState::IdleState(UIElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void HoveredState::Enter()
    {
        element->OnMouseStartHover();
        dragTimer.SetMaxTime(0.25f); // TODO: Do not use magic number
        dragTimer.SetAutoFinish(false);
    }

    void HoveredState::Exit()
    {
        // if we swap to predrag then this will be called...
        element->OnMouseStopHover();
    }

    void HoveredState::Update()
    {
        engine->cursor->DisableContextSwitching();
        engine->cursor->Disable();
        dragTimer.Update(GetFrameTime());
        element->OnMouseContinueHover();

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
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable && dragTimer.HasExceededMaxTime())
        {
            element->ChangeState(std::make_unique<DraggingState>(element, engine));
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable && !dragTimer.IsRunning())
        {
            dragTimer.Start();
        }

        // // Skip if already dragging something
        // if (draggedElement.has_value()) return;
        //
        // // If we haven't started timing for this element yet
        // if (!hoveredDraggableElement.has_value())
        // {
        //     hoveredDraggableElement = element;
        //     dragTimer.SetMaxTime(draggedTimerThreshold);
        //     dragTimer.Start();
        // }
        // // If the cursor has changed drag target, reset
        // else if (hoveredDraggableElement.value() != element)
        // {
        //     hoveredDraggableElement = element;
        //     dragTimer.Restart();
        // }
        // // Check if we've held long enough to start dragging
        // else if (dragTimer.HasExceededMaxTime())
        // {
        //     const Vector2 offset = {
        //         static_cast<float>(settings->screenWidth * 0.005),
        //         static_cast<float>(settings->screenHeight * 0.005)};
        //
        //     if (const auto titleBar = dynamic_cast<TitleBar*>(element))
        //     {
        //         draggedElement = std::make_unique<DraggedWindow>(titleBar->parent->GetWindow());
        //         draggedElement.value()->mouseOffset = {
        //             mousePos.x - window->rec.x - offset.x, mousePos.y - window->rec.y - offset.y};
        //     }
        //     else
        //     {
        //         draggedElement = std::make_unique<DraggedCellElement>(element);
        //         draggedElement.value()->mouseOffset = {
        //             mousePos.x - element->rec.x - offset.x, mousePos.y - element->rec.y - offset.y};
        //     }
        //
        //     dragTimer.Reset();
        // }
    }

    void HoveredState::Draw()
    {
    }

    HoveredState::~HoveredState()
    {
    }

    HoveredState::HoveredState(UIElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DraggingState::Enter()
    {
        element->beingDragged = true;
        element->OnMouseStartDrag();
        // element->hidden = true;
        // hide element
        auto mousePos = GetMousePosition();
        mouseOffset = {
            static_cast<float>(engine->settings->screenWidth * 0.005),
            static_cast<float>(engine->settings->screenHeight * 0.005)};

        // if (const auto titleBar = dynamic_cast<TitleBar*>(element))
        // {
        //     engine->draggedElement = std::make_unique<DraggedWindow>(titleBar->parent->GetWindow());
        //     engine->draggedElement.value()->mouseOffset = {
        //         mousePos.x - GetWindow()->rec.x - mouseOffset.x, mousePos.y - window->rec.y - mouseOffset.y};
        // }
        // else
        // {
        //     engine->draggedElement = std::make_unique<DraggedCellElement>(element);
        //     engine->draggedElement.value()->mouseOffset = {
        //         mousePos.x - element->rec.x - mouseOffset.x, mousePos.y - element->rec.y - mouseOffset.y};
        // }
    }

    void DraggingState::Exit()
    {
        element->beingDragged = false;
        // element->hidden = false;
    }

    void DraggingState::Update()
    {
        // Determine if object is still being dragged
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            element->ChangeState(std::make_unique<DroppingState>(element, engine));
            return;
        }
        element->OnMouseContinueDrag(); // Update drag
    }

    void DraggingState::Draw()
    {
    }

    DraggingState::~DraggingState()
    {
    }

    DraggingState::DraggingState(UIElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DroppingState::Enter()
    {
        auto cell = engine->GetCellUnderCursor();
        element->OnDropped(cell);
        element->ChangeState(std::make_unique<IdleState>(element, engine));
    }

    void DroppingState::Exit()
    {
        engine->hoveredDraggableElement.reset();
        engine->draggedElement.reset();
    }

    void DroppingState::Update()
    {
    }

    void DroppingState::Draw()
    {
    }

    DroppingState::~DroppingState()
    {
    }

    DroppingState::DroppingState(UIElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    UIState::UIState(UIElement* _element, GameUIEngine* _engine) : element(_element), engine(_engine)
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
        windows.push_back(std::make_unique<Window>(this));
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
        windows.push_back(std::make_unique<WindowDocked>(this));
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

    CellElement* GameUIEngine::GetCellUnderCursor() const
    {
        return nullptr;
    }

    void DraggedWindow::Update()
    {
        auto mousePos = GetMousePosition();
        element->SetPosition(mousePos.x - mouseOffset.x, mousePos.y - mouseOffset.y);
        element->UpdateChildren();
    }

    UIElement* DraggedWindow::GetElement()
    {
        return element;
    }

    void DraggedWindow::Draw()
    {
    }

    void DraggedWindow::OnDrop()
    {
    }

    DraggedWindow::DraggedWindow(Window* _window) : element(_window)
    {
    }

    UIElement* DraggedCellElement::GetElement()
    {
        return element;
    }

    void DraggedCellElement::Draw()
    {
        auto mousePos = GetMousePosition();
        DrawTexture(element->tex, mousePos.x - mouseOffset.x, mousePos.y - mouseOffset.y, WHITE);
    }

    void DraggedCellElement::Update()
    {
    }

    void DraggedCellElement::OnDrop()
    {
        // TODO: Should identify whether it has been dropped on something already
    }

    DraggedCellElement::DraggedCellElement(CellElement* _element) : element(_element)
    {
        element->beingDragged = true;
    }

    DraggedCellElement::~DraggedCellElement()
    {
        element->beingDragged = false;
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

        if (draggedElement.has_value())
        {
            draggedElement.value()->Draw();
        }
    }

    // void GameUIEngine::processCell(
    //     CellElement* element, Window* window, bool& elementFound, const Vector2& mousePos)
    // {
    //     // element->state->Update();
    //
    //     // // Handle element hover state
    //     // if (!MouseInside(element->parent->rec, mousePos))
    //     // {
    //     //     if (element->mouseHover)
    //     //     {
    //     //         element->OnMouseStopHover();
    //     //     }
    //     //     return;
    //     // }
    //     //
    //     // elementFound = true; // We found an element under the mouse
    //     //
    //     // bool elementDropped = draggedElement.has_value() && IsMouseButtonUp(MOUSE_BUTTON_LEFT);
    //     // bool mouseButtonClicked = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    //     // auto mouseDownOnDraggableElement = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable;
    //     //
    //     // // Handle mouse interactions
    //     // if (elementDropped)
    //     // {
    //     //     element->OnDragDropHere(draggedElement.value()->GetElement());
    //     //     return;
    //     // }
    //     //
    //     // // No item being dragged
    //     // if (!element->mouseHover)
    //     // {
    //     //     element->OnMouseStartHover();
    //     // }
    //     //
    //     // if (mouseButtonClicked)
    //     // {
    //     //     element->OnMouseStopHover();
    //     //     element->OnMouseClick();
    //     // }
    //     // else if (mouseDownOnDraggableElement)
    //     // {
    //     //     // Skip if already dragging something
    //     //     if (draggedElement.has_value()) return;
    //     //
    //     //     // If we haven't started timing for this element yet
    //     //     if (!hoveredDraggableElement.has_value())
    //     //     {
    //     //         hoveredDraggableElement = element;
    //     //         dragTimer.SetMaxTime(draggedTimerThreshold);
    //     //         dragTimer.Start();
    //     //     }
    //     //     // If the cursor has changed drag target, reset
    //     //     else if (hoveredDraggableElement.value() != element)
    //     //     {
    //     //         hoveredDraggableElement = element;
    //     //         dragTimer.Restart();
    //     //     }
    //     //     // Check if we've held long enough to start dragging
    //     //     else if (dragTimer.HasExceededMaxTime())
    //     //     {
    //     //         const Vector2 offset = {
    //     //             static_cast<float>(settings->screenWidth * 0.005),
    //     //             static_cast<float>(settings->screenHeight * 0.005)};
    //     //
    //     //         if (const auto titleBar = dynamic_cast<TitleBar*>(element))
    //     //         {
    //     //             draggedElement = std::make_unique<DraggedWindow>(titleBar->parent->GetWindow());
    //     //             draggedElement.value()->mouseOffset = {
    //     //                 mousePos.x - window->rec.x - offset.x, mousePos.y - window->rec.y - offset.y};
    //     //         }
    //     //         else
    //     //         {
    //     //             draggedElement = std::make_unique<DraggedCellElement>(element);
    //     //             draggedElement.value()->mouseOffset = {
    //     //                 mousePos.x - element->rec.x - offset.x, mousePos.y - element->rec.y - offset.y};
    //     //         }
    //     //
    //     //         dragTimer.Reset();
    //     //     }
    //     // }
    //     // else if (element->mouseHover)
    //     // {
    //     //     element->OnMouseContinueHover();
    //     //     // Reset timer if we're not holding the mouse button
    //     //     if (dragTimer.IsRunning())
    //     //     {
    //     //         dragTimer.Reset();
    //     //         hoveredDraggableElement.reset();
    //     //     }
    //     // }
    // }

    void GameUIEngine::processWindows(const Vector2& mousePos)
    {
        const auto windowCount = windows.size();
        bool elementFound = false;
        for (const auto& window : windows)
        {
            // Make sure we do not process newly added windows this cycle
            if (windows.size() > windowCount) break;
            if (window->hidden) continue;

            window->state->Update();

            // // Handle window hover state
            // if (!MouseInside(window->rec, mousePos))
            // {
            //     window->OnMouseStopHover();
            //     continue;
            // }
            //
            // cursor->DisableContextSwitching();
            // cursor->Disable();
            //
            // if (!window->mouseHover)
            // {
            //     window->OnMouseStartHover();
            // }
            //
            // for (const auto& table : window->children)
            // {
            //     for (const auto& row : table->children)
            //     {
            //         for (const auto& cell : row->children)
            //         {
            //             auto element = cell->children.get();
            //
            //             // Already found a hovered element, loop continues to remove previous hover states
            //             if (elementFound)
            //             {
            //                 if (element->mouseHover)
            //                 {
            //                     element->OnMouseStopHover();
            //                 }
            //                 return;
            //             }
            //
            //             processCell(element, window.get(), elementFound, mousePos);
            //         }
            //     }
            // }
        }
    }

    void GameUIEngine::cleanUpDragState()
    {
        if (hoveredDraggableElement.has_value())
        {
            dragTimer.Reset();
            hoveredDraggableElement.reset();
        }

        if (draggedElement.has_value())
        {
            draggedElement.reset();
        }
    }

    void GameUIEngine::Update()
    {
        const float dt = GetFrameTime();
        dragTimer.Update(dt);

        pruneWindows();
        const auto mousePos = GetMousePosition();

        // Reset cursor state if not dragging
        if (!draggedElement.has_value())
        {
            cursor->EnableContextSwitching();
            cursor->Enable();
        }
        else
        {
            draggedElement.value()->Update();
        }

        processWindows(mousePos);

        // Clean up drag states on mouse release
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            cleanUpDragState();
        }
    }

    GameUIEngine::GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor)
        : cursor(_cursor), userInput(_userInput), settings(_settings)
    {
        dragTimer.SetMaxTime(draggedTimerThreshold);
        dragTimer.SetAutoFinish(false);
    }
} // namespace sage