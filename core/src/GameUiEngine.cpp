//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"
#include "Cursor.hpp"
#include "Settings.hpp"

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

    // NB: The original position of the window is treated as an offset.
    void Window::SetWindowScreenAlignment(VertAlignment vert, HoriAlignment hori)
    {
        float originalXOffset = rec.x;
        float originalYOffset = rec.y;

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
            window->Draw2D();
        }
    }

    void TextBox::UpdateDimensions()
    {
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

    void Window::UpdateChildren()
    {
        if (children.empty()) return;

        switch (tableAlignment)
        {
        case WindowTableAlignment::STACK_HORIZONTAL: {
            const float maxWidth = std::ceil(rec.width / children.size());
            for (int i = 0; i < children.size(); ++i)
            {
                const auto& table = children.at(i);
                table->parent = this;
                table->rec = rec;
                table->rec.width = maxWidth;
                table->rec.x = rec.x + (maxWidth * i);
                table->rec.height = rec.height; // Full height in horizontal mode
                if (!table->children.empty()) table->UpdateChildren();
            }
            break;
        }

        case WindowTableAlignment::STACK_VERTICAL: {
            const float maxHeight = std::ceil(rec.height / children.size());
            for (int i = 0; i < children.size(); ++i)
            {
                const auto& table = children.at(i);
                table->parent = this;
                table->rec = rec;
                table->rec.height = maxHeight;
                table->rec.y = rec.y + (maxHeight * i);
                table->rec.width = rec.width; // Full width in vertical mode
                if (!table->children.empty()) table->UpdateChildren();
            }
            break;
        }
        }
    }

    void Table::UpdateChildren()
    {
        const float rowHeight = std::ceil(rec.height / children.size());
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& row = children.at(i);
            row->parent = this;
            row->rec = rec;
            row->rec.height = rowHeight;
            row->rec.y = rec.y + (rowHeight * i);
            if (!row->children.empty()) row->UpdateChildren();
            // TODO: Add margin here
            // TODO: Add padding here
        }
    }

    void TableRow::UpdateChildren()
    {
        const float cellWidth = std::ceil(rec.width / children.size());
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& cell = children.at(i);
            cell->parent = this;
            cell->rec = rec;
            cell->rec.width = cellWidth;
            cell->rec.x = rec.x + (cellWidth * i);
            cell->UpdateChildren();
            // TODO: Add margin here
            // TODO: Add padding here
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
        Image _nPatchTexture, Vector2 pos, float w, float h, WindowTableAlignment _alignment)
    {
        windows.push_back(std::make_unique<Window>());
        auto& window = windows.back();
        window->tableAlignment = _alignment;
        window->settings = settings;
        window->mainNPatchTexture = LoadTextureFromImage(_nPatchTexture);
        window->rec = {pos.x, pos.y, w, h};
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

    [[nodiscard]] TableCell* TableRow::CreateTableCell()
    {
        children.push_back(std::make_unique<TableCell>());
        const auto& cell = children.back();
        cell->parent = this;
        UpdateChildren();
        return cell.get();
    }

    TextBox* TableCell::CreateTextbox(const std::string& _content)
    {
        children = std::make_unique<TextBox>();
        auto* textbox = dynamic_cast<TextBox*>(children.get());
        textbox->fontSize = 42;
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
        cursor->EnableContextSwitching();
        cursor->Enable();
        for (auto& window : windows)
        {
            if (mousePos.x >= window->rec.x && mousePos.x <= window->rec.x + window->rec.width &&
                mousePos.y >= window->rec.y && mousePos.y <= window->rec.y + window->rec.height)
            {
                cursor->DisableContextSwitching();
                cursor->Disable();
                break;
            }
        }

        // Handle input and update UI state here (e.g., button clicks, hover effects)
    }

    GameUIEngine::GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor)
        : settings(_settings), cursor(_cursor)
    {
        // TODO: CreateWindow should use a percentage of the screen as its positioning, as opposed to an absolute
        // pixel value
    }
} // namespace sage