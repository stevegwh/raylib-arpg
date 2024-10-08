//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"
#include "Cursor.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "UserInput.hpp"

#include "rlgl.h"

#include <cassert>
#include <sstream>

namespace sage
{
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

    void AbilitySlot::SetAbilityInfo()
    {
        if (const Ability* ability = playerAbilitySystem->GetAbility(slotNumber))
        {
            tex = LoadTexture(ability->iconPath.c_str()); // TODO: Replace with resource manager and asset id
        }
        else
        {
            tex.id = rlGetTextureIdDefault();
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

    void ImageBox::OnMouseStartHover()
    {
        RemoveShader();
        CellElement::OnMouseStartHover();
    }

    void ImageBox::OnMouseStopHover()
    {
        SetGrayscale();
        CellElement::OnMouseStopHover();
    }

    void ImageBox::UpdateDimensions()
    {
        float availableWidth = parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right);
        float availableHeight = parent->rec.height - (parent->GetPadding().up + parent->GetPadding().down);

        float originalRatio = static_cast<float>(tex.width) / tex.height;
        float finalWidth, finalHeight;

        // TODO: Should have OverflowBehaviour. I think SHRINK_TO_FIT and SHRINK_ROW_TO_FIT would be good
        // TODO: Should have ScaleBehaviour? DO_NOT_SCALE, STRETCH_TO_FIT, ZOOM_TO_FIT, STRETCH_MAINTAIN_RATIO.
        // Need better names...

        if (originalRatio > 1.0f) // Wider than tall
        {
            finalWidth = availableWidth;
            finalHeight = availableWidth / originalRatio;
        }
        else // Taller than wide
        {
            finalHeight = availableHeight;
            finalWidth = availableHeight * originalRatio;
        }

        // Scale down if the image is too big for either dimension
        if (finalWidth > availableWidth || finalHeight > availableHeight)
        {
            float widthRatio = availableWidth / finalWidth;
            float heightRatio = availableHeight / finalHeight;

            // Use the smaller ratio to ensure both dimensions fit
            float scaleFactor = std::min(widthRatio, heightRatio);

            finalWidth *= scaleFactor;
            finalHeight *= scaleFactor;
        }

        float horiOffset = 0; // Left
        float vertOffset = 0; // Top

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (availableHeight - finalHeight) / 2;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = availableHeight - finalHeight;
        }

        if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = availableWidth - finalWidth;
        }
        else if (horiAlignment == HoriAlignment::CENTER)
        {
            horiOffset = (availableWidth - finalWidth) / 2;
        }

        rec = Rectangle{
            parent->rec.x + parent->GetPadding().left + horiOffset,
            parent->rec.y + parent->GetPadding().up + vertOffset,
            finalWidth,
            finalHeight};

        tex.width = finalWidth;
        tex.height = finalHeight;
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

    void CloseButton::OnMouseClick()
    {
        parentWindow->hidden = true;
    }

    Dimensions Window::GetDimensions() const
    {
        return Dimensions{settings->screenWidth * widthPercent, settings->screenHeight * heightPercent};
    }

    /**
     *
     * @param _xPercent x position in percent of screen (1-100)
     * @param _yPercent y position in percent of screen (1-100)
     */
    void Window::SetDimensionsPercent(float _widthPercent, float _heightPercent)
    {
        widthPercent = _widthPercent / 100;
        heightPercent = _heightPercent / 100;
    }

    // TODO: Maybe make WindowFixed and WindowFloating and change how it handles position
    // Essentially, one relies on alignment and offsets and one just uses a raw position
    Vector2 Window::GetOffset() const
    {
        return Vector2{settings->screenWidth * xOffsetPercent, settings->screenHeight * yOffsetPercent};
    }

    /**
     *
     * @param _xOffsetPercent x position in percent of screen (1-100)
     * @param _yOffsetPercent y position in percent of screen (1-100)
     */
    void Window::SetOffsetPercent(float _xOffsetPercent, float _yOffsetPercent)
    {
        xOffsetPercent = _xOffsetPercent / 100;
        yOffsetPercent = _yOffsetPercent / 100;
    }

    // NB: The original position of the window is treated as an offset.
    void Window::SetAlignment(VertAlignment vert, HoriAlignment hori)
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

    void Window::OnScreenSizeChange()
    {
        rec = {GetOffset().x, GetOffset().y, GetDimensions().width, GetDimensions().height};
        SetAlignment(vertAlignment, horiAlignment);
        UpdateChildren();
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

    [[nodiscard]] Table* Window::CreateTable()
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
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
            const float maxWidth = std::ceil(availableWidth / children.size());
            for (int i = 0; i < children.size(); ++i)
            {
                const auto& table = children.at(i);
                table->parent = this;
                table->rec = rec;
                table->rec.width = maxWidth;
                table->rec.x = startX + (maxWidth * i);
                table->rec.height = availableHeight;
                table->rec.y = startY;
                if (!table->children.empty()) table->UpdateChildren();
            }
            break;
        }

        case WindowTableAlignment::STACK_VERTICAL: {
            const float maxHeight = std::ceil(availableHeight / children.size());
            for (int i = 0; i < children.size(); ++i)
            {
                const auto& table = children.at(i);
                table->parent = this;
                table->rec = rec;
                table->rec.height = maxHeight;
                table->rec.y = startY + (maxHeight * i);
                table->rec.width = availableWidth;
                table->rec.x = startX;
                if (!table->children.empty()) table->UpdateChildren();
            }
            break;
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
            DrawRectangle(row->rec.x, row->rec.y, row->rec.width, row->rec.height, col);
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

    [[nodiscard]] TableRow* Table::CreateTableRow()
    {
        children.push_back(std::make_unique<TableRow>());
        const auto& row = children.back();
        UpdateChildren();
        return row.get();
    }

    /**
     *
     * @param requestedHeight The desired height of the cell as a percent (0-100)
     * @return
     */
    [[nodiscard]] TableRow* Table::CreateTableRow(float requestedHeight)
    {
        assert(requestedHeight <= 100 && requestedHeight >= 0);
        children.push_back(std::make_unique<TableRow>());
        const auto& row = children.back();
        row->autoSize = false;
        row->requestedHeight = requestedHeight;
        UpdateChildren();
        return row.get();
    }

    [[nodiscard]] TableCell* TableRow::CreateTableCell()
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
    [[nodiscard]] TableCell* TableRow::CreateTableCell(float requestedWidth)
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
        PlayerAbilitySystem* _playerAbilitySystem, Window* _parentWindow, int _slotNumber)
    {
        children = std::make_unique<AbilitySlot>();
        auto* abilitySlot = dynamic_cast<AbilitySlot*>(children.get());
        abilitySlot->playerAbilitySystem = _playerAbilitySystem;
        abilitySlot->draggable = true;
        abilitySlot->canReceiveDragDrops = true;
        abilitySlot->slotNumber = _slotNumber;
        abilitySlot->parentWindow = _parentWindow;
        entt::sink sink{_parentWindow->onMouseStopHover};
        sink.connect<&AbilitySlot::OnMouseStopHover>(abilitySlot);
        abilitySlot->SetAbilityInfo();
        UpdateChildren();
        return abilitySlot;
    }

    TitleBar* TableCell::CreateTitleBar(Window* window, const std::string& _title, float fontSize)
    {
        children = std::make_unique<TitleBar>();
        auto* titleBar = dynamic_cast<TitleBar*>(children.get());
        titleBar->parentWindow = window;
        titleBar->draggable = true;
        titleBar->fontSize = fontSize;
        titleBar->overflowBehaviour = TextBox::OverflowBehaviour::SHRINK_TO_FIT;
        titleBar->content = _title;
        UpdateChildren();
        return titleBar;
    }

    CloseButton* TableCell::CreateCloseButton(Window* _parentWindow, Image _tex)
    {
        children = std::make_unique<CloseButton>();
        auto* closeButton = dynamic_cast<CloseButton*>(children.get());
        closeButton->parentWindow = _parentWindow;
        entt::sink sink{_parentWindow->onMouseStopHover};
        sink.connect<&ImageBox::OnMouseStopHover>(closeButton);
        closeButton->tex = LoadTextureFromImage(_tex);
        UpdateChildren();
        return closeButton;
    }

    TextBox* TableCell::CreateTextbox(
        const std::string& _content, float fontSize, TextBox::OverflowBehaviour overflowBehaviour)
    {
        children = std::make_unique<TextBox>();
        auto* textbox = dynamic_cast<TextBox*>(children.get());
        textbox->fontSize = fontSize;
        textbox->overflowBehaviour = overflowBehaviour;
        textbox->content = _content;
        UpdateChildren();
        return textbox;
    }

    ImageBox* TableCell::CreateImagebox(Window* _parentWindow, Image _tex)
    {
        children = std::make_unique<ImageBox>();
        auto* image = dynamic_cast<ImageBox*>(children.get());
        image->draggable = true;
        image->parentWindow = _parentWindow;
        entt::sink sink{_parentWindow->onMouseStopHover};
        sink.connect<&ImageBox::OnMouseStopHover>(image);
        image->tex = LoadTextureFromImage(_tex);
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
    }

    [[nodiscard]] Window* GameUIEngine::CreateWindow(
        Image _nPatchTexture,
        float _xOffsetPercent,
        float _yOffsetPercent,
        float _widthPercent,
        float _heightPercent,
        WindowTableAlignment _alignment)
    {
        windows.push_back(std::make_unique<Window>());
        auto& window = windows.back();
        window->SetOffsetPercent(_xOffsetPercent, _yOffsetPercent);
        window->SetDimensionsPercent(_widthPercent, _heightPercent);
        window->tableAlignment = _alignment;
        window->settings = settings;
        window->mainNPatchTexture = LoadTextureFromImage(_nPatchTexture);
        window->rec = {
            window->GetOffset().x,
            window->GetOffset().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

        entt::sink sink{userInput->onWindowUpdate};
        sink.connect<&Window::OnScreenSizeChange>(window.get());

        return window.get();
    }

    void GameUIEngine::DrawDebug2D()
    {
        for (auto& window : windows)
        {
            if (window->hidden) continue;
            window->DrawDebug2D();
        }
    }

    void GameUIEngine::Draw2D()
    {
        for (auto& window : windows)
        {
            if (window->hidden) continue;
            window->Draw2D();
        }

        if (draggedElement.has_value())
        {
            // TODO: Make dragged element a struct
            auto mousePos = GetMousePosition();
            DrawTexture(draggedElement.value()->tex, mousePos.x, mousePos.y, WHITE);
        }
    }

    void GameUIEngine::Update()
    {
        auto mousePos = GetMousePosition();

        // Reset cursor state if not dragging
        if (!draggedElement.has_value())
        {
            cursor->EnableContextSwitching();
            cursor->Enable();
        }

        // Reset drag timer on mouse release
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            draggedTimer = 0;
        }

        // Process all windows
        for (const auto& window : windows)
        {
            if (window->hidden) continue;

            // Handle window hover state
            bool isMouseInWindow = window->MouseInside(mousePos);
            if (!isMouseInWindow)
            {
                window->onMouseStopHover.publish();
                continue;
            }

            window->onMouseStartHover.publish();
            cursor->DisableContextSwitching();
            cursor->Disable();

            // Process all elements in window
            for (const auto& table : window->children)
                for (const auto& row : table->children)
                    for (const auto& cell : row->children)
                    {
                        auto& element = cell->children;
                        bool isMouseInElement = cell->MouseInside(mousePos);

                        // Handle element hover state
                        if (!isMouseInElement)
                        {
                            if (element->mouseHover)
                            {
                                cell->children->OnMouseStopHover();
                            }
                            continue;
                        }

                        element->OnMouseStartHover();

                        // Handle mouse interactions
                        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !draggedElement.has_value())
                        {
                            element->OnMouseClick();
                        }
                        else if (draggedElement.has_value() && IsMouseButtonUp(MOUSE_BUTTON_LEFT))
                        {
                            element->OnDragDropHere(draggedElement.value());
                        }
                        else if (
                            IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable &&
                            !draggedElement.has_value())
                        {
                            // Skip if already dragging something
                            if (draggedElement.has_value()) break;

                            // Skip if cursor changed hover while dragging
                            if (!hoveredDraggableElement.has_value() && draggedTimer > 0) continue;

                            // Reset hover if changed elements
                            if (hoveredDraggableElement.has_value() &&
                                hoveredDraggableElement.value() != element.get())
                            {
                                hoveredDraggableElement.reset();
                                break;
                            }

                            // Handle drag initiation
                            const auto currentTime = GetTime();
                            if (draggedTimer == 0)
                            {
                                hoveredDraggableElement = element.get();
                                draggedTimer = currentTime;
                            }
                            else if (currentTime > draggedTimer + draggedTimerThreshold)
                            {
                                draggedElement = element.get();
                                element->beingDragged = true;
                                draggedTimer = 0;
                            }
                        }
                        break; // We've found our element, stop processing
                    }
        }

        // Clean up drag states on mouse release
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            if (hoveredDraggableElement.has_value())
            {
                draggedTimer = 0;
                hoveredDraggableElement.reset();
            }

            if (draggedElement.has_value())
            {
                draggedElement.value()->beingDragged = false;
                draggedElement.reset();
            }
        }
    }

    GameUIEngine::GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor)
        : cursor(_cursor), userInput(_userInput), settings(_settings)
    {
    }
} // namespace sage