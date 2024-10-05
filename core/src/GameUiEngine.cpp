//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

namespace sage
{

    [[nodiscard]] Window* GameUIEngine::CreateWindow(Vector2 pos, float w, float h)
    {
        windows.push_back(std::make_unique<Window>());
        auto& window = windows.back();
        window->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        window->tex = nPatchTexture;
        window->rec = {pos.x, pos.y, w, h};
        return window.get();
    }

    [[nodiscard]] Table* Window::CreateTable()
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
        table->parent = this;
        // Table inherits window's dimensions and position
        table->rec = rec;
        // TODO: Add padding here
        // table.tex = LoadTexture("resources/textures/panel_background.png");

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

    Button* TableCell::CreateButton(Texture _tex)
    {
        auto button = std::make_unique<Button>();
        button->tex = _tex;

        // Position the button inside the cell, accounting for padding
        button->rec = {
            rec.x + padding.left,
            rec.y + padding.up,
            rec.width - (+padding.right),
            rec.height - (padding.up + padding.down)};
        children = std::move(button);
        return dynamic_cast<Button*>(children.get());
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
            children->UpdateRec();
        }
    }

    void TextBox::UpdateRec()
    {
        constexpr int MIN_FONT_SIZE = 4;
        float availableWidth = parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right);
        Vector2 textSize = MeasureTextEx(GetFontDefault(), content.c_str(), fontSize, fontSpacing);
        while (textSize.x > availableWidth && fontSize > MIN_FONT_SIZE)
        {
            fontSize -= 1;
            textSize = MeasureTextEx(GetFontDefault(), content.c_str(), fontSize, fontSpacing);
        }
        rec = {
            parent->rec.x + parent->GetPadding().left,
            parent->rec.y + parent->GetPadding().up,
            textSize.x,
            textSize.y};
    }

    void Button::UpdateRec()
    {
    }

    void Window::Draw2D() const
    {
        DrawTextureNPatch(tex, nPatchInfo, rec, {0.0f, 0.0f}, 0.0f,
                          WHITE); // Use {0.0f, 0.0f} for origin

        for (auto& child : children)
        {
            child->Draw2D();
        }
    }

    void Table::Draw2D()
    {
        std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& row = children[i];
            DrawRectangle(row->rec.x, row->rec.y, row->rec.width, row->rec.height, colors[i]);
            row->Draw2D();
        }
    }

    void TableRow::Draw2D()
    {
        std::vector colors = {RED, BLUE, YELLOW, WHITE, PINK};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& cell = children[i];
            Color col = colors[i];
            col.a = 150;
            DrawRectangle(cell->rec.x, cell->rec.y, cell->rec.width, cell->rec.height, col);
            cell->Draw2D();
        }
    }

    void TableCell::Draw2D()
    {
        if (children) // single element
        {
            children->Draw2D();
        }
    }

    void Button::Draw2D()
    {
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

    void GameUIEngine::Update()
    {
        // Handle input and update UI state here (e.g., button clicks, hover effects)
    }

    GameUIEngine::GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor)
        : nextId(1) // Initialize nextId to 1
    {
        nPatchTexture = LoadTexture("resources/textures/ninepatch_button.png");
    }
} // namespace sage