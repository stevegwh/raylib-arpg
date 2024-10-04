//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

namespace sage
{

    [[nodiscard]] Window* GameUIEngine::CreateWindow(Vector2 pos, float w, float h)
    {
        Window window;
        window.nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        window.tex = nPatchTexture;
        window.rec = {pos.x, pos.y, w, h};
        windows.push_back(window);
        return &windows.back();
    }

    [[nodiscard]] Table* Window::CreateTable()
    {
        Table table;
        table.parent = this;
        table.rec = Rectangle{rec.x, rec.y, rec.width, rec.height};
        table.tex = LoadTexture("resources/textures/panel_background.png"); // Example texture
        children.push_back(table);
        return &children.back();
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

    [[nodiscard]] TableRow* Table::CreateTableRow()
    {
        TableRow row;
        float percent = 100.0f / (children.size() + 1);
        row.width = FloatConstrained(percent, 0, rec.width);
        row.height = FloatConstrained(percent, 0, rec.height);
        children.push_back(row);

        return &children.back();
    }
    void Table::Draw2D()
    {
        for (auto& row : children)
        {
            row.Draw2D();
        }
    }

    [[nodiscard]] TableCell* TableRow::CreateTableCell(float width)
    {
        TableCell cell;
        float percent = 100.0f / (children.size() + 1);
        cell.width = FloatConstrained(percent, 0, rec.width);
        cell.height = FloatConstrained(percent, 0, rec.height);
        children.push_back(cell);
        return &children.back();
    }
    void TableRow::Draw2D()
    {
        for (auto& cell : children)
        {
            cell.Draw2D();
        }
    }

    TextBox* TableCell::CreateTextbox(const std::string& _content)
    {
        TextBox textbox;
        textbox.fontSize = 10; // Default font size
        textbox.content = _content;
        children.push_back(std::make_unique<TextBox>(textbox));
        return dynamic_cast<TextBox*>(children.back().get());
    }

    void TextBox::Draw2D()
    {
    }

    Button* TableCell::CreateButton(Texture _tex)
    {
        Button button;
        button.tex = _tex;
        children.push_back(std::make_unique<Button>(button));
        return dynamic_cast<Button*>(children.back().get());
    }

    void Button::Draw2D()
    {
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