//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

#include "Camera.hpp"
#include "components/CombatableActor.hpp"
#include "components/ItemComponent.hpp"
#include "components/Renderable.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameUiFactory.hpp"
#include "slib.hpp"
#include "systems/InventorySystem.hpp"
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

        element->HoverUpdate();
    }

    HoverState::HoverState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DragDelayState::Enter()
    {
        element->OnHoverStart();
        dragTimer.SetMaxTime(element->dragDelayTime);
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
        engine->draggedObject = element;
        element->OnDragStart();
    }

    void DragState::Exit()
    {
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

        if (tooltipWindow && tooltipWindow->markForRemoval)
        {
            tooltipWindow.reset();
        }
    }

    Window* GameUIEngine::CreateTooltipWindow(
        const Texture& _nPatchTexture,
        const float x,
        const float y,
        const float _widthPercent,
        const float _heightPercent,
        const WindowTableAlignment _alignment)
    {
        return CreateWindow(_nPatchTexture, x, y, _widthPercent, _heightPercent, _alignment, true);
    }

    Window* GameUIEngine::CreateWindow(
        Texture _nPatchTexture,
        const float x,
        const float y,
        const float _widthPercent,
        const float _heightPercent,
        const WindowTableAlignment _alignment,
        const bool tooltip)
    {

        auto window = std::make_unique<Window>();

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
        if (tooltip)
        {
            tooltipWindow = std::move(window);
            return tooltipWindow.get();
        }
        windows.push_back(std::move(window));
        return windows.back().get();
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(
        Texture _nPatchTexture,
        const float _xOffsetPercent,
        const float _yOffsetPercent,
        const float _widthPercent,
        const float _heightPercent,
        const WindowTableAlignment _alignment)
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

        if (auto collision = GetWindowCollision(window))
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
        const auto mousePos = GetMousePosition();
        for (auto& window : windows)
        {
            if (window->hidden) continue;
            if (PointInsideRect(window->rec, mousePos) && mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                windowUnderCursor = window.get();
            }
        }
        if (windowUnderCursor == nullptr) return nullptr;
        for (const auto& table : windowUnderCursor->children)
        {
            for (const auto& row : table->children)
            {
                for (const auto& cell : row->children)
                {
                    const auto element = cell->children.get();
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
        const auto it =
            std::find_if(windows.begin(), windows.end(), [clicked](const std::unique_ptr<Window>& ptr) {
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

        if (tooltipWindow)
        {
            tooltipWindow->Draw2D();
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
        if (auto collision = GetWindowCollision(window))
        {
            // check if window is lower
            auto windowIt =
                std::find_if(windows.begin(), windows.end(), [window](const std::unique_ptr<Window>& ptr) {
                    return ptr.get() == window;
                });

            const auto colIt =
                std::find_if(windows.begin(), windows.end(), [collision](const std::unique_ptr<Window>& ptr) {
                    return ptr.get() == collision;
                });

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
                        const auto element = cell->children.get();
                        element->state->Update();
                    }
                }
            }
        }
    }

    void GameUIEngine::onWorldItemHover(entt::entity entity) const
    {
        if (!gameData->inventorySystem->CheckWorldItemRange()) return;
        auto& item = registry->get<ItemComponent>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        GameUiFactory::CreateWorldTooltip(gameData->uiEngine.get(), item.name, pos);
    }

    void GameUIEngine::onWorldCombatableHover(entt::entity entity) const
    {
        auto& renderable = registry->get<Renderable>(entity);
        auto& combatable = registry->get<CombatableActor>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        // Create a name tooltip
        GameUiFactory::CreateCombatableTooltip(gameData->uiEngine.get(), renderable.name, combatable, pos);
    }

    void GameUIEngine::onNPCHover(entt::entity entity)
    {
        auto& renderable = registry->get<Renderable>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        // Create a name tooltip
        GameUiFactory::CreateWorldTooltip(gameData->uiEngine.get(), renderable.name, pos);
    }

    void GameUIEngine::onStopWorldHover() const
    {
        if (tooltipWindow)
        {
            tooltipWindow->markForRemoval = true;
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
        entt::sink sink{_gameData->cursor->onCombatableHover};
        sink.connect<&GameUIEngine::onWorldCombatableHover>(this);
        entt::sink sink2{_gameData->cursor->onItemHover};
        sink2.connect<&GameUIEngine::onWorldItemHover>(this);
        entt::sink sink3{_gameData->cursor->onStopHover};
        sink3.connect<&GameUIEngine::onStopWorldHover>(this);
        entt::sink sink4{_gameData->cursor->onNPCHover};
        sink4.connect<&GameUIEngine::onNPCHover>(this);
    }
} // namespace sage