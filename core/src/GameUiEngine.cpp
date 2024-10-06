//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"
#include "Cursor.hpp"
#include "Settings.hpp"
#include <cassert>

namespace sage
{
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

    void Window::Draw2D()
    {
        TableElement::Draw2D();
        for (auto& child : children)
        {
            child->Draw2D();
        }
    }

    void Table::Draw2D()
    {
        TableElement::Draw2D();
        std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& row = children[i];
            // DrawRectangle(row->rec.x, row->rec.y, row->rec.width, row->rec.height, colors[i]);
            row->Draw2D();
        }
    }

    void TableRow::Draw2D()
    {
        TableElement::Draw2D();
        std::vector colors = {RED, BLUE, YELLOW, WHITE, PINK};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& cell = children[i];
            Color col = colors[i];
            col.a = 150;
            // DrawRectangle(cell->rec.x, cell->rec.y, cell->rec.width, cell->rec.height, col);
            cell->Draw2D();
        }
    }

    void TableCell::Draw2D()
    {
        TableElement::Draw2D();
        if (children) // single element
        {
            children->Draw2D();
        }
    }

    void ImageBox::Draw2D()
    {
        DrawTexture(tex, rec.x, rec.y, WHITE);
    }

    void TextBox::Draw2D()
    {
        DrawTextEx(GetFontDefault(), content.c_str(), Vector2{rec.x, rec.y}, fontSize, fontSpacing, BLACK);
    }

    void GameUIEngine::Draw2D()
    {
        for (auto& window : windows)
        {
            if (window->hidden) continue;
            window->Draw2D();
        }
    }

    void TextBox::UpdateDimensions()
    {
        // TODO: Content should be a vector to allow for word wrap
        // Should break into new lines. if new line does not fit and cannot be split then reduce font size
        constexpr int MIN_FONT_SIZE = 4;

        float availableWidth = parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right);
        Vector2 textSize = MeasureTextEx(GetFontDefault(), content.c_str(), fontSize, fontSpacing);
        while (textSize.x > availableWidth && fontSize > MIN_FONT_SIZE)
        {
            fontSize -= 1;
            textSize = MeasureTextEx(GetFontDefault(), content.c_str(), fontSize, fontSpacing);
        }

        float horiOffset = 0; // Left
        float vertOffset = 0; // Top
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

    void ImageBox::UpdateDimensions()
    {
        float availableWidth = parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right);
        float availableHeight = parent->rec.height - (parent->GetPadding().up + parent->GetPadding().down);

        float originalRatio = static_cast<float>(tex.width) / tex.height;
        float finalWidth, finalHeight;

        // First attempt at fitting the image
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

    void Window::OnScreenSizeChange()
    {
        rec = {GetOffset().x, GetOffset().y, GetDimensions().width, GetDimensions().height};
        SetAlignment(vertAlignment, horiAlignment);
        UpdateChildren();
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

    void TableCell::UpdateChildren()
    {
        if (children)
        {
            children->parent = this;
            children->rec = rec;
            children->UpdateDimensions();
        }
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
        return window.get();
    }

    [[nodiscard]] Table* Window::CreateTable()
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
        UpdateChildren();
        return table.get();
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

    TextBox* TableCell::CreateTextbox(const std::string& _content)
    {
        children = std::make_unique<TextBox>();
        auto* textbox = dynamic_cast<TextBox*>(children.get());
        textbox->fontSize = 16;
        textbox->content = _content;
        UpdateChildren();
        return textbox;
    }

    ImageBox* TableCell::CreateImagebox(Image _tex)
    {
        children = std::make_unique<ImageBox>();
        auto* image = dynamic_cast<ImageBox*>(children.get());
        image->tex = LoadTextureFromImage(_tex);
        UpdateChildren();
        return image;
    }

    void GameUIEngine::Update()
    {
        auto mousePos = GetMousePosition();

        // if item is being dragged, do not enable below
        cursor->EnableContextSwitching();
        cursor->Enable();

        for (auto& window : windows)
        {
            if (window->hidden) continue;
            if (mousePos.x >= window->rec.x && mousePos.x <= window->rec.x + window->rec.width &&
                mousePos.y >= window->rec.y && mousePos.y <= window->rec.y + window->rec.height)
            {
                cursor->DisableContextSwitching();
                cursor->Disable();
                break;
            }
        }

        if (IsKeyDown(KEY_B))
        {
            for (auto& window : windows)
            {
                window->OnScreenSizeChange();
            }
        }

        // Get hovered or clicked element and interact with it
        // onMouseUp -> activate, onMouseDown -> add drag timer? then enable drag

        // Handle input and update UI state here (e.g., button clicks, hover effects)
    }

    GameUIEngine::GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor)
        : cursor(_cursor), settings(_settings)
    {
        // TODO: CreateWindow should use a percentage of the screen as its positioning, as opposed to an absolute
        // pixel value
    }
} // namespace sage