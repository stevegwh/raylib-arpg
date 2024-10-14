//
// Created by steve on 02/10/2024.
//

#include "GameUiElements.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "Cursor.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "rlgl.h"
#include <cassert>
#include <sstream>

namespace sage
{

    void UIElement::OnHoverStart()
    {
    }

    void UIElement::OnHoverStop()
    {
    }

    void CellElement::SetVertAlignment(VertAlignment alignment)
    {
        vertAlignment = alignment;
        UpdateDimensions();
    }

    void CellElement::SetHoriAlignment(HoriAlignment alignment)
    {
        horiAlignment = alignment;
        UpdateDimensions();
    }

    void CellElement::OnMouseClick()
    {
        onMouseClicked.publish();
    }

    void CellElement::MouseHoverUpdate()
    {
    }

    void CellElement::OnMouseStartDrag()
    {
        beingDragged = true;
    };

    // TODO: confusing name.
    // In this case, "DroppedElement" is the (potential) cell that this element has been dropped onto
    void CellElement::OnDropped(CellElement* droppedElement)
    {
        beingDragged = false;
        if (droppedElement && droppedElement->canReceiveDragDrops)
        {
            droppedElement->OnDragDropHere(this);
        }
        // Not dropped on a cell, what to do?
    }

    void CellElement::OnDragDropHere(CellElement* droppedElement)
    {
        if (!canReceiveDragDrops) return;

        std::cout << "Reached here \n";
    }

    void CellElement::ChangeState(std::unique_ptr<UIState> newState)
    {
        if (newState == state) return;

        state->Exit();
        state = std::move(newState);
        state->Enter();
    }

    CellElement::CellElement(GameUIEngine* _engine)
        : engine(_engine), state(std::make_unique<IdleState>(this, engine)){};

    void TextBox::SetOverflowBehaviour(OverflowBehaviour _behaviour)
    {
        overflowBehaviour = _behaviour;
        UpdateDimensions();
    }

    void TextBox::UpdateDimensions()
    {
        constexpr int MIN_FONT_SIZE = 4;
        float availableWidth = parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right);

        if (overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {
            Vector2 textSize = MeasureTextEx(font, content.c_str(), fontSize, fontSpacing);
            while (textSize.x > availableWidth && fontSize > MIN_FONT_SIZE)
            {
                fontSize -= 1;
                textSize = MeasureTextEx(font, content.c_str(), fontSize, fontSpacing);
            }
        }
        else if (overflowBehaviour == OverflowBehaviour::WORD_WRAP)
        {
            std::string wrappedText;
            std::string currentLine;
            std::istringstream words(content);
            std::string word;

            // Process each word
            while (words >> word)
            {
                // Create temporary line with new word
                std::string testLine = currentLine;
                if (!testLine.empty()) testLine += " "; // Add space between words
                testLine += word;

                // Measure the line with the new word
                Vector2 lineSize = MeasureTextEx(font, testLine.c_str(), fontSize, fontSpacing);

                if (lineSize.x <= availableWidth)
                {
                    // Word fits, add it to current line
                    currentLine = testLine;
                }
                else
                {
                    // Word doesn't fit, start new line
                    if (!wrappedText.empty()) wrappedText += "\n";
                    wrappedText += currentLine;
                    currentLine = word;
                }
            }

            // Add the last line
            if (!currentLine.empty())
            {
                if (!wrappedText.empty()) wrappedText += "\n";
                wrappedText += currentLine;
            }

            // Update the content with wrapped text
            content = wrappedText;
        }

        // Measure final text size
        Vector2 textSize = MeasureTextEx(font, content.c_str(), fontSize, fontSpacing);

        float horiOffset = 0;
        float vertOffset = 0;
        float availableHeight = parent->rec.height - (parent->GetPadding().up + parent->GetPadding().down);

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (availableHeight - textSize.y) / 2;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = availableHeight - textSize.y;
        }

        if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = availableWidth - textSize.x;
        }
        else if (horiAlignment == HoriAlignment::CENTER)
        {
            horiOffset = (availableWidth - textSize.x) / 2;
        }
        else if (horiAlignment == HoriAlignment::WINDOW_CENTER)
        {
            auto window = parent->GetWindow();
            float windowAvailableWidth =
                window->rec.width - (window->GetPadding().left + window->GetPadding().right);
            horiOffset = (windowAvailableWidth - textSize.x) / 2;
        }

        rec = {
            parent->rec.x + parent->GetPadding().left + horiOffset,
            parent->rec.y + parent->GetPadding().up + vertOffset,
            textSize.x,
            textSize.y};
    }

    void TextBox::Draw2D()
    {
        DrawTextEx(font, content.c_str(), Vector2{rec.x, rec.y}, fontSize, fontSpacing, BLACK);
    }

    TextBox::TextBox(GameUIEngine* _engine) : CellElement(_engine){};

    void TitleBar::OnMouseStartDrag()
    {
        draggedWindow = parent->GetWindow();
        auto mousePos = GetMousePosition();
        Vector2 offset = {
            static_cast<float>(engine->settings->screenWidth * 0.005),
            static_cast<float>(engine->settings->screenHeight * 0.005)};
        dragOffset = {
            mousePos.x - draggedWindow.value()->rec.x - offset.x,
            mousePos.y - draggedWindow.value()->rec.y - offset.y};
    }

    void TitleBar::MouseDragUpdate()
    {
        auto mousePos = GetMousePosition();
        draggedWindow.value()->SetPosition(mousePos.x - dragOffset.x, mousePos.y - dragOffset.y);
        draggedWindow.value()->UpdateChildren();
    }

    void TitleBar::OnDropped(CellElement* droppedElement)
    {
        draggedWindow.reset();
        dragOffset = {0, 0};
    }

    TitleBar::TitleBar(GameUIEngine* _engine) : TextBox(_engine){};

    void ImageBox::SetOverflowBehaviour(OverflowBehaviour _behaviour)
    {
        overflowBehaviour = _behaviour;
        UpdateDimensions();
    }

    void ImageBox::SetGrayscale()
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/grayscale.fs");
    }

    void ImageBox::RemoveShader()
    {
        shader.reset();
    }

    void ImageBox::OnMouseClick()
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/bloom.fs");
        CellElement::OnMouseClick();
    }

    void ImageBox::OnHoverStart()
    {
        hoverTimer = GetTime();
        RemoveShader();
        CellElement::OnHoverStart();
    }

    void ImageBox::OnHoverStop()
    {
        hoverTimer = 0;
        if (tooltipWindow.has_value())
        {
            tooltipWindow.value()->Remove();
            tooltipWindow.reset();
        }
        SetGrayscale();
        CellElement::OnHoverStop();
    }

    Dimensions ImageBox::calculateAvailableSpace() const
    {
        return {
            parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right),
            parent->rec.height - (parent->GetPadding().up + parent->GetPadding().down)};
    }

    float ImageBox::calculateAspectRatio() const
    {
        return static_cast<float>(tex.width) / tex.height;
    }

    Dimensions ImageBox::calculateInitialDimensions(const Dimensions& space) const
    {
        float originalRatio = calculateAspectRatio();

        if (originalRatio > 1.0f)
        { // Wider than tall
            return {space.width, space.width / originalRatio};
        }
        else
        { // Taller than wide
            return {space.height * originalRatio, space.height};
        }
    }

    Vector2 ImageBox::calculateAlignmentOffset(const Dimensions& dimensions, const Dimensions& space) const
    {
        float vertOffset = 0;
        float horiOffset = 0;

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (space.height - dimensions.height) / 2;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = space.height - dimensions.height;
        }

        if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = space.width - dimensions.width;
        }
        else if (horiAlignment == HoriAlignment::CENTER)
        {
            horiOffset = (space.width - dimensions.width) / 2;
        }

        return {horiOffset, vertOffset};
    }

    void ImageBox::updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space)
    {
        rec = Rectangle{
            parent->rec.x + parent->GetPadding().left + offset.x,
            parent->rec.y + parent->GetPadding().up + offset.y,
            dimensions.width,
            dimensions.height};

        tex.width = dimensions.width;
        tex.height = dimensions.height;
    }

    void ImageBox::shrinkRowToFit() const
    {
        auto& row = parent->parent->children;

        // First pass: calculate the required scale factor for each cell
        float minScaleFactor = 1.0f;

        for (auto& cell : row)
        {
            auto* imageBox = dynamic_cast<ImageBox*>(cell->children.get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            // Calculate scale factor needed for this image
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            // Keep track of the smallest scale factor needed
            minScaleFactor = std::min(minScaleFactor, scaleFactor);
        }

        // Second pass: apply the minimum scale factor to all cells
        for (auto& cell : row)
        {
            auto* imageBox = dynamic_cast<ImageBox*>(cell->children.get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            // Apply uniform scaling
            dimensions.width *= minScaleFactor;
            dimensions.height *= minScaleFactor;

            auto offset = imageBox->calculateAlignmentOffset(dimensions, space);
            imageBox->updateRectangle(dimensions, offset, space);
        }
    }

    Dimensions ImageBox::handleOverflow(const Dimensions& dimensions, const Dimensions& space) const
    {
        if (dimensions.width <= space.width && dimensions.height <= space.height)
        {
            return dimensions;
        }

        if (overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            return {dimensions.width * scaleFactor, dimensions.height * scaleFactor};
        }

        return dimensions; // Return original dimensions if no scaling needed
    }

    void ImageBox::OnMouseStartDrag()
    {
        auto mousePos = GetMousePosition();
        Vector2 offset = {
            static_cast<float>(engine->settings->screenWidth * 0.005),
            static_cast<float>(engine->settings->screenHeight * 0.005)};
        dragOffset = {rec.x - mousePos.x + offset.x, rec.y - mousePos.y + offset.y};
        CellElement::OnMouseStartDrag();
    }

    void ImageBox::OnDropped(CellElement* droppedElement)
    {
        dragOffset = {0, 0};
        CellElement::OnDropped(droppedElement);
    }

    void ImageBox::MouseDragDraw()
    {
        auto mousePos = GetMousePosition();
        DrawTexture(tex, mousePos.x + dragOffset.x, mousePos.y + dragOffset.y, WHITE);
    }

    void ImageBox::UpdateDimensions()
    {
        if (overflowBehaviour == OverflowBehaviour::SHRINK_ROW_TO_FIT)
        {
            shrinkRowToFit();
            return;
        }
        auto space = calculateAvailableSpace();

        auto dimensions = calculateInitialDimensions(space);
        dimensions = handleOverflow(dimensions, space);
        auto offset = calculateAlignmentOffset(dimensions, space);

        updateRectangle(dimensions, offset, space);
    }

    void ImageBox::Draw2D()
    {
        if (shader.has_value())
        {
            BeginShaderMode(shader.value());
        }
        DrawTexture(tex, rec.x, rec.y, WHITE);
        if (shader.has_value())
        {
            EndShaderMode();
        }
    }

    ImageBox::ImageBox(GameUIEngine* _engine) : CellElement(_engine){};

    void AbilitySlot::SetAbilityInfo()
    {
        if (const Ability* ability = playerAbilitySystem->GetAbility(slotNumber))
        {
            tex = LoadTexture(ability->iconPath.c_str()); // TODO: Replace with resource manager and asset id
        }
        else
        {
            tex.id = rlGetTextureIdDefault();
            tex.width = 0;
            tex.height = 0;
            // tex = LoadTexture("resources/icons/abilities/default.png"); // TODO: Replace with AssetID (or use
            // rlGetDefaultTexture) Set default
        }
    }

    void AbilitySlot::OnDragDropHere(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<AbilitySlot*>(droppedElement))
        {
            playerAbilitySystem->SwapAbility(slotNumber, dropped->slotNumber);
            dropped->SetAbilityInfo();
            SetAbilityInfo();
            dropped->UpdateDimensions();
            UpdateDimensions();
        }
    }

    void AbilitySlot::MouseHoverUpdate()
    {
        ImageBox::MouseHoverUpdate();
        if (tooltipWindow.has_value() || GetTime() < hoverTimer + hoverTimerThreshold) return;
        if (auto* ability = playerAbilitySystem->GetAbility(slotNumber))
        {
            const Vector2 mousePos = GetMousePosition();
            const float offsetX = mousePos.x - rec.x;
            const float offsetY = mousePos.y - rec.y - rec.height / 2;
            tooltipWindow =
                GameUiFactory::CreateAbilityToolTip(engine, *ability, {rec.x + offsetX, rec.y + offsetY});
        }
    }

    void AbilitySlot::OnMouseClick()
    {
        playerAbilitySystem->PressAbility(slotNumber);
        ImageBox::OnMouseClick();
    }

    AbilitySlot::AbilitySlot(GameUIEngine* _engine) : ImageBox(_engine){};

    void InventorySlot::SetItemInfo()
    {
        auto& inventory = registry->get<InventoryComponent>(controllableActorSystem->GetControlledActor());
        auto itemId = inventory.GetItem(row, col);
        if (itemId != entt::null)
        {
            auto& item = registry->get<ItemComponent>(itemId);
            tex = ResourceManager::GetInstance().TextureLoad(item.icon);
        }
        else
        {
            tex.id = rlGetTextureIdDefault();
            tex.width = 0;
            tex.height = 0;
            // tex = LoadTexture("resources/icons/abilities/default.png"); // TODO: Replace with AssetID (or use
            // rlGetDefaultTexture) Set default
        }
    }

    void InventorySlot::OnDragDropHere(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            auto& inventory = registry->get<InventoryComponent>(controllableActorSystem->GetControlledActor());
            inventory.SwapItems(row, col, dropped->row, dropped->col);
            dropped->SetItemInfo();
            SetItemInfo();
            dropped->UpdateDimensions();
            UpdateDimensions();
        }
    }

    void InventorySlot::MouseHoverUpdate()
    {
        ImageBox::MouseHoverUpdate();
        if (tooltipWindow.has_value() || GetTime() < hoverTimer + hoverTimerThreshold) return;
        auto& inventory = registry->get<InventoryComponent>(controllableActorSystem->GetControlledActor());
        auto itemId = inventory.GetItem(row, col);
        if (itemId != entt::null)
        {
            auto& item = registry->get<ItemComponent>(itemId);
            const Vector2 mousePos = GetMousePosition();
            const float offsetX = mousePos.x - rec.x;
            const float offsetY = mousePos.y - rec.y - rec.height / 2;
            tooltipWindow = GameUiFactory::CreateItemTooltip(engine, item, {rec.x + offsetX, rec.y + offsetY});
        }
    }

    InventorySlot::InventorySlot(GameUIEngine* _engine) : ImageBox(_engine){};

    void CloseButton::OnMouseClick()
    {
        parent->GetWindow()->hidden = true;
    }

    CloseButton::CloseButton(GameUIEngine* _engine) : ImageBox(_engine){};

    void WindowDocked::OnScreenSizeChange()
    {
        rec = {GetOffset().x, GetOffset().y, GetDimensions().width, GetDimensions().height};
        SetAlignment(vertAlignment, horiAlignment);
        UpdateChildren();
    }

    Vector2 WindowDocked::GetOffset() const
    {
        return Vector2{settings->screenWidth * xOffsetPercent, settings->screenHeight * yOffsetPercent};
    }

    /**
     *
     * @param _xOffsetPercent x offset from docked position in percent of screen (1-100)
     * @param _yOffsetPercent y offset from docked position in percent of screen (1-100)
     */
    void WindowDocked::SetOffsetPercent(float _xOffsetPercent, float _yOffsetPercent)
    {
        xOffsetPercent = _xOffsetPercent / 100;
        yOffsetPercent = _yOffsetPercent / 100;
    }

    void WindowDocked::SetAlignment(VertAlignment vert, HoriAlignment hori)
    {
        vertAlignment = vert;
        horiAlignment = hori;
        float originalXOffset = GetOffset().x;
        float originalYOffset = GetOffset().y;

        float xOffset = 0;
        float yOffset = 0;

        // Calculate horizontal position
        switch (hori)
        {
        case HoriAlignment::LEFT:
            xOffset = 0;
            break;

        case HoriAlignment::CENTER:
            xOffset = (settings->screenWidth - rec.width) / 2;
            break;

        case HoriAlignment::RIGHT:
            xOffset = settings->screenWidth - rec.width;
            break;
        }

        // Calculate vertical position
        switch (vert)
        {
        case VertAlignment::TOP:
            yOffset = 0;
            break;

        case VertAlignment::MIDDLE:
            yOffset = (settings->screenHeight - rec.height) / 2;
            break;

        case VertAlignment::BOTTOM:
            yOffset = settings->screenHeight - rec.height;
            break;
        }

        rec.x = xOffset + originalXOffset;
        rec.y = yOffset + originalYOffset;

        UpdateChildren();
    }

    void Window::Remove()
    {
        hidden = true;
        markForRemoval = true;
    }

    void Window::OnScreenSizeChange()
    {
        // TODO: Could do something fancier with changing the x/y based on the new width/height
        // This would be "easy" to do as OnScreenSizeChange is reacting to an event that passes width/height as the
        // parameters (which we currently ignore). Alternatively, we have a pointer to settings, anyway.
        rec = {GetPosition().x, GetPosition().y, GetDimensions().width, GetDimensions().height};
        UpdateChildren();
    }

    Dimensions Window::GetDimensions() const
    {
        return Dimensions{settings->screenWidth * widthPercent, settings->screenHeight * heightPercent};
    }

    /**
     *
     * @param _widthPercent Width as percent of screen (1-100)
     * @param _heightPercent Height as percent of screen (1-100)
     */
    void Window::SetDimensionsPercent(float _widthPercent, float _heightPercent)
    {
        widthPercent = _widthPercent / 100;
        heightPercent = _heightPercent / 100;
    }

    void Window::SetPosition(float x, float y)
    {
        rec.x = x;
        rec.y = y;
    }

    Vector2 Window::GetPosition() const
    {
        return {rec.x, rec.y};
    }

    void Window::OnHoverStart()
    {
        UIElement::OnHoverStart();
    }

    void Window::OnHoverStop()
    {
        UIElement::OnHoverStop();
        for (auto& table : children)
        {
            for (auto& row : table->children)
            {
                for (auto& cell : row->children)
                {
                    auto& element = cell->children;
                    // Allow elements being dragged to continue being active outside of window's bounds
                    if (element->beingDragged) continue;
                    element->ChangeState(std::make_unique<IdleState>(element.get(), element->engine));
                }
            }
        }
    }

    void Window::DrawDebug2D()
    {
        for (auto& child : children)
        {
            child->DrawDebug2D();
        }
    }

    void Window::Draw2D()
    {
        TableElement::Draw2D();
        for (auto& child : children)
        {
            child->Draw2D();
        }
    }

    TableGrid* Window::CreateTableGrid(int rows, int cols, float cellSpacing)
    {
        children.push_back(std::make_unique<TableGrid>());
        const auto& table = dynamic_cast<TableGrid*>(children.back().get());
        table->parent = this;
        table->cellSpacing = cellSpacing;
        // Create rows and cells with initial autoSize = true
        for (int i = 0; i < rows; ++i)
        {
            TableRow* row = table->CreateTableRow();
            for (int j = 0; j < cols; ++j)
            {
                row->CreateTableCell();
            }
        }
        UpdateChildren();
        return table;
    }

    Table* Window::CreateTable()
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
        table->parent = this;
        UpdateChildren();
        return table.get();
    }

    Table* Window::CreateTable(float requestedWidthOrHeight)
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
        table->parent = this;
        table->autoSize = false;
        if (tableAlignment == WindowTableAlignment::STACK_VERTICAL)
        {
            table->requestedHeight = requestedWidthOrHeight;
        }
        else if (tableAlignment == WindowTableAlignment::STACK_HORIZONTAL)
        {
            table->requestedWidth = requestedWidthOrHeight;
        }
        UpdateChildren();
        return table.get();
    }

    void Window::UpdateChildren()
    {
        if (children.empty()) return;

        // Account for window padding
        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);
        float startX = rec.x + GetPadding().left;
        float startY = rec.y + GetPadding().up;

        switch (tableAlignment)
        {
        case WindowTableAlignment::STACK_HORIZONTAL: {
            float totalRequestedPercent = 0.0f;
            int autoSizeCount = 0;

            // First pass: Calculate total of percentage-based widths
            for (const auto& table : children)
            {
                if (table->autoSize)
                {
                    autoSizeCount++;
                }
                else
                {
                    totalRequestedPercent += table->requestedWidth;
                }
            }

            if (totalRequestedPercent > 100.0f)
            {
                totalRequestedPercent = 100.0f;
            }

            float remainingPercent = 100.0f - totalRequestedPercent;
            float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

            // Second pass: Update each table
            float currentX = startX;
            for (const auto& table : children)
            {
                table->parent = this;
                table->rec = rec;

                float tableWidth;
                if (table->autoSize)
                {
                    tableWidth = std::ceil(availableWidth * (autoSizePercent / 100.0f));
                }
                else
                {
                    tableWidth = std::ceil(availableWidth * (table->requestedWidth / 100.0f));
                }

                table->rec.width = tableWidth;
                table->rec.x = currentX;
                table->rec.height = availableHeight;
                table->rec.y = startY;

                if (!table->children.empty()) table->UpdateChildren();

                currentX += tableWidth;
            }
            break;
        }

        case WindowTableAlignment::STACK_VERTICAL: {
            float totalRequestedPercent = 0.0f;
            int autoSizeCount = 0;

            // First pass: Calculate total of percentage-based heights
            for (const auto& table : children)
            {
                if (table->autoSize)
                {
                    autoSizeCount++;
                }
                else
                {
                    totalRequestedPercent += table->requestedHeight;
                }
            }

            if (totalRequestedPercent > 100.0f)
            {
                totalRequestedPercent = 100.0f;
            }

            float remainingPercent = 100.0f - totalRequestedPercent;
            float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

            // Second pass: Update each table
            float currentY = startY;
            for (const auto& table : children)
            {
                table->parent = this;
                table->rec = rec;

                float tableHeight;
                if (table->autoSize)
                {
                    tableHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
                }
                else
                {
                    tableHeight = std::ceil(availableHeight * (table->requestedHeight / 100.0f));
                }

                table->rec.height = tableHeight;
                table->rec.y = currentY;
                table->rec.width = availableWidth;
                table->rec.x = startX;

                if (!table->children.empty()) table->UpdateChildren();

                currentY += tableHeight;
            }
            break;
        }
        }
    }

    void TableGrid::UpdateChildren()
    {
        if (children.empty()) return;
        // 1. Get number of columns
        int cols = children[0]->children.size();

        // 2. Calculate available space
        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);

        // 3. Find the maximum power-of-two cell size that fits
        int maxCellSize = 1 << static_cast<int>(std::floor(
                              std::log2(std::min(availableWidth / cols, availableHeight / children.size()))));

        // 4. Calculate actual cell size with spacing
        float cellSize = (std::min(availableWidth / cols, availableHeight / children.size()) - cellSpacing) /
                         maxCellSize * maxCellSize;

        // 5. Calculate total grid dimensions
        float gridWidth = cellSize * cols + cellSpacing * (cols - 1); // Account for spacing between columns
        float gridHeight =
            cellSize * children.size() + cellSpacing * (children.size() - 1); // Account for spacing between rows

        // 6. Calculate starting position to center the grid
        float startX = rec.x + (availableWidth - gridWidth) / 2.0f;
        float startY = rec.y + (availableHeight - gridHeight) / 2.0f;

        // 7. Apply dimensions to rows and cells
        float currentY = startY;
        for (const auto& row : children)
        {
            row->rec.height = cellSize;
            row->rec.y = currentY;
            row->rec.x = startX;
            row->rec.width = gridWidth;

            float currentX = row->rec.x;
            for (const auto& cell : row->children)
            {
                cell->rec.width = cellSize;
                cell->rec.x = currentX;
                cell->rec.y = currentY;
                cell->rec.height = cellSize;
                currentX += cellSize + cellSpacing; // Add spacing after each cell
            }
            currentY += cellSize + cellSpacing; // Add spacing after each row
        }

        for (auto& row : children)
        {
            for (auto& cell : row->children)
            {
                if (cell->children)
                {
                    cell->children->UpdateDimensions();
                }
            }
        }
    }

    void Table::DrawDebug2D()
    {
        std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& row = children[i];
            Color col = colors[i];
            col.a = 150;
            // DrawRectangle(row->rec.x, row->rec.y, row->rec.width, row->rec.height, col);
            row->DrawDebug2D();
        }
    }

    void Table::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& row : children)
        {
            row->Draw2D();
        }
    }

    void Table::UpdateChildren()
    {
        // Account for table padding
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);
        float startY = rec.y + GetPadding().up;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based heights
        for (const auto& row : children)
        {
            if (row->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += row->requestedHeight;
            }
        }

        if (totalRequestedPercent > 100.0f)
        {
            totalRequestedPercent = 100.0f;
        }

        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each row
        float currentY = startY;
        for (const auto& row : children)
        {
            row->parent = this;
            row->rec = rec;

            float rowHeight;
            if (row->autoSize)
            {
                rowHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
            }
            else
            {
                rowHeight = std::ceil(availableHeight * (row->requestedHeight / 100.0f));
            }

            // Set row dimensions accounting for table padding
            row->rec.height = rowHeight;
            row->rec.y = currentY;
            row->rec.x = rec.x + GetPadding().left;
            row->rec.width = rec.width - (GetPadding().left + GetPadding().right);

            if (!row->children.empty())
            {
                row->UpdateChildren();
            }

            currentY += rowHeight;
        }
    }

    TableRow* Table::CreateTableRow()
    {
        children.push_back(std::make_unique<TableRow>());
        const auto& row = children.back();
        row->parent = this;
        UpdateChildren();
        return row.get();
    }

    /**
     *
     * @param requestedHeight The desired height of the cell as a percent (0-100)
     * @return
     */
    TableRow* Table::CreateTableRow(float requestedHeight)
    {
        assert(requestedHeight <= 100 && requestedHeight >= 0);
        children.push_back(std::make_unique<TableRow>());
        const auto& row = children.back();
        row->parent = this;
        row->autoSize = false;
        row->requestedHeight = requestedHeight;
        UpdateChildren();
        return row.get();
    }

    TableCell* TableRow::CreateTableCell()
    {
        children.push_back(std::make_unique<TableCell>());
        const auto& cell = children.back();
        cell->parent = this;
        UpdateChildren();
        return cell.get();
    }

    /**
     *
     * @param requestedWidth The desired width of the cell as a percent (0-100)
     * @return
     */
    TableCell* TableRow::CreateTableCell(float requestedWidth)
    {
        assert(requestedWidth <= 100 && requestedWidth >= 0);
        children.push_back(std::make_unique<TableCell>());
        const auto& cell = children.back();
        cell->parent = this;
        cell->autoSize = false;
        cell->requestedWidth = requestedWidth;
        UpdateChildren();
        return cell.get();
    }

    void TableRow::DrawDebug2D()
    {
        std::vector colors = {RED, BLUE, YELLOW, WHITE, PINK};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& cell = children[i];
            Color col = colors[i];
            col.a = 100;
            DrawRectangle(cell->rec.x, cell->rec.y, cell->rec.width, cell->rec.height, col);
            cell->DrawDebug2D();
        }
    }

    void TableRow::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& cell : children)
        {
            cell->Draw2D();
        }
    }

    void TableRow::UpdateChildren()
    {
        // Account for row padding
        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float startX = rec.x + GetPadding().left;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based widths
        for (const auto& cell : children)
        {
            if (cell->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += cell->requestedWidth;
            }
        }

        if (totalRequestedPercent > 100.0f)
        {
            totalRequestedPercent = 100.0f;
        }

        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each cell
        float currentX = startX;
        for (const auto& cell : children)
        {
            cell->parent = this;
            cell->rec = rec;

            float cellWidth;
            if (cell->autoSize)
            {
                cellWidth = std::ceil(availableWidth * (autoSizePercent / 100.0f));
            }
            else
            {
                cellWidth = std::ceil(availableWidth * (cell->requestedWidth / 100.0f));
            }

            // Set cell dimensions accounting for row padding
            cell->rec.width = cellWidth;
            cell->rec.x = currentX;
            cell->rec.y = rec.y + GetPadding().up;
            cell->rec.height = rec.height - (GetPadding().up + GetPadding().down);

            cell->UpdateChildren();

            currentX += cellWidth;
        }
    }

    AbilitySlot* TableCell::CreateAbilitySlot(
        GameUIEngine* engine, PlayerAbilitySystem* _playerAbilitySystem, int _slotNumber)
    {
        children = std::make_unique<AbilitySlot>(engine);
        auto* abilitySlot = dynamic_cast<AbilitySlot*>(children.get());
        abilitySlot->SetGrayscale();
        abilitySlot->playerAbilitySystem = _playerAbilitySystem;
        abilitySlot->draggable = true;
        abilitySlot->canReceiveDragDrops = true;
        abilitySlot->slotNumber = _slotNumber;
        abilitySlot->SetAbilityInfo();
        UpdateChildren();
        return abilitySlot;
    }

    InventorySlot* TableCell::CreateInventorySlot(
        entt::registry* _registry,
        GameUIEngine* engine,
        ControllableActorSystem* _controllableActorSystem,
        unsigned int row,
        unsigned int col)
    {
        children = std::make_unique<InventorySlot>(engine);
        auto* slot = dynamic_cast<InventorySlot*>(children.get());
        slot->registry = _registry;
        slot->controllableActorSystem = _controllableActorSystem;
        slot->draggable = true;
        slot->canReceiveDragDrops = true;
        slot->row = row;
        slot->col = col;
        slot->SetItemInfo();
        UpdateChildren();
        return slot;
    }

    TitleBar* TableCell::CreateTitleBar(GameUIEngine* engine, const std::string& _title, float fontSize)
    {
        children = std::make_unique<TitleBar>(engine);
        auto* titleBar = dynamic_cast<TitleBar*>(children.get());
        titleBar->draggable = true;
        titleBar->fontSize = fontSize;
        titleBar->overflowBehaviour = TextBox::OverflowBehaviour::SHRINK_TO_FIT;
        titleBar->content = _title;
        UpdateChildren();
        return titleBar;
    }

    CloseButton* TableCell::CreateCloseButton(GameUIEngine* engine, Texture _tex)
    {
        children = std::make_unique<CloseButton>(engine);
        auto* closeButton = dynamic_cast<CloseButton*>(children.get());
        closeButton->SetGrayscale();
        closeButton->tex = _tex;
        UpdateChildren();
        return closeButton;
    }

    TextBox* TableCell::CreateTextbox(
        GameUIEngine* engine,
        const std::string& _content,
        float fontSize,
        TextBox::OverflowBehaviour overflowBehaviour)
    {
        children = std::make_unique<TextBox>(engine);
        auto* textbox = dynamic_cast<TextBox*>(children.get());
        textbox->fontSize = fontSize;
        textbox->overflowBehaviour = overflowBehaviour;
        textbox->content = _content;
        UpdateChildren();
        return textbox;
    }

    ImageBox* TableCell::CreateImagebox(GameUIEngine* engine, Texture _tex)
    {
        children = std::make_unique<ImageBox>(engine);
        auto* image = dynamic_cast<ImageBox*>(children.get());
        image->draggable = true;
        image->tex = _tex;
        UpdateChildren();
        return image;
    }

    void TableCell::UpdateChildren()
    {
        auto& element = children;
        if (element)
        {
            element->parent = this;
            element->rec = rec;
            element->UpdateDimensions();
        }
    }

    void TableCell::Draw2D()
    {
        TableElement::Draw2D();
        auto& element = children;
        if (element && !element->beingDragged) // hide if dragged
        {
            element->Draw2D();
        }
    }

    void TableCell::DrawDebug2D()
    {
        Color col = BLACK;
        col.a = 50;
        // DrawRectangle(children->rec.x, children->rec.y, children->rec.width, children->rec.height, col);
    }
} // namespace sage