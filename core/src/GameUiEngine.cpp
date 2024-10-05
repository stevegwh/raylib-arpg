//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

namespace sage
{

    [[nodiscard]] Window* GameUIEngine::CreateWindow(Vector2 pos, float w, float h)
    {
        windows.emplace_back();
        auto& window = windows.back();
        window.nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        window.tex = nPatchTexture;
        window.rec = {pos.x, pos.y, w, h};
        return &windows.back();
    }

    [[nodiscard]] Table* Window::CreateTable()
    {
        children.emplace_back();
        auto& table = children.back();
        table.parent = this;
        // Table inherits window's dimensions and position
        table.rec = rec;
        // TODO: Add padding here
        // table.tex = LoadTexture("resources/textures/panel_background.png");
        return &children.back();
    }

    [[nodiscard]] TableRow* Table::CreateTableRow()
    {
        children.emplace_back();
        auto& row = children.back();
        row.parent = this;
        row.rec = rec;
        UpdateChildren();

        // TODO: Add padding here
        return &children.back();
    }

    [[nodiscard]] TableCell* TableRow::CreateTableCell(Padding _padding, Margin _margin)
    {
        children.emplace_back();
        auto& cell = children.back();
        cell.padding = _padding;
        cell.margin = _margin;
        cell.parent = this;
        cell.rec = rec;

        UpdateChildren();

        return &children.back();
    }

    [[nodiscard]] TableCell* TableRow::CreateTableCell()
    {
        return CreateTableCell({}, {});
    }

    TextBox* TableCell::CreateTextbox(const std::string& _content)
    {
        auto textbox = std::make_unique<TextBox>();
        textbox->fontSize = 10;
        textbox->content = _content;
        textbox->parent = this;
        textbox->UpdateRec();
        TextBox* ptr = textbox.get();
        children.push_back(std::move(textbox));
        return ptr;
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

        Button* ptr = button.get();
        children.push_back(std::move(button));
        return ptr;
    }

    void Table::UpdateChildren()
    {
        float rowHeight = rec.height / children.size();
        for (int i = 0; i < children.size(); ++i)
        {
            auto& row = children.at(i);
            row.rec.height = rowHeight;
            row.rec.y = rec.y + (rowHeight * i);
            row.UpdateChildren();
            // TODO: Add margin here
        }
    }

    void TableRow::UpdateChildren()
    {
        float cellWidth = rec.width / children.size();
        for (int i = 0; i < children.size(); ++i)
        {
            auto& cell = children.at(i);
            cell.rec.width = cellWidth;
            cell.rec.x = rec.x + (cellWidth * i);
            cell.UpdateChildren();
        }
    }

    void TableCell::UpdateChildren()
    {
        for (auto& element : children)
        {
            element->UpdateRec();
        }
    }

    void TextBox::UpdateRec()
    {
        // Calculate text dimensions
        Vector2 textSize = MeasureTextEx(GetFontDefault(), content.c_str(), fontSize, 1);
        rec = {parent->rec.x + parent->padding.left, rec.y + parent->padding.up, textSize.x, textSize.y};
    }

    void Window::Draw2D()
    {
        DrawTextureNPatch(tex, nPatchInfo, rec, {0.0f, 0.0f}, 0.0f,
                          WHITE); // Use {0.0f, 0.0f} for origin

        for (auto& child : children)
        {
            child.Draw2D();
        }
    }

    void Table::Draw2D()
    {
        for (auto& row : children)
        {
            row.Draw2D();
        }
    }

    void TableRow::Draw2D()
    {
        for (auto& cell : children)
        {
            cell.Draw2D();
        }
    }

    void TableCell::Draw2D()
    {
        for (auto& c : children)
        {
            DrawRectangle(rec.x, rec.y, rec.width, rec.height, BLUE);
            c->Draw2D();
        }
    }

    void Button::Draw2D()
    {
    }

    void TextBox::Draw2D()
    {
        DrawText(content.c_str(), rec.x, rec.y, fontSize, BLACK);
    }

    void GameUIEngine::Draw2D()
    {
        for (auto& window : windows)
        {
            window.Draw2D();
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