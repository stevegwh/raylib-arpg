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
        windows.push_back(std::move(window));
        return &windows.back();
    }

    [[nodiscard]] Table* Window::CreateTable()
    {
        Table table;
        table.parent = this;
        // Table inherits window's dimensions and position
        table.rec = rec;
        table.tex = LoadTexture("resources/textures/panel_background.png");
        children.push_back(std::move(table));
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
        row.parent = this;

        float rowHeight = rec.height / (children.size() + 1);
        // Position the new row below existing rows
        float yPosition = rec.y + (children.size() * rowHeight);

        row.rec = {
            rec.x,     // x position same as table
            yPosition, // y position calculated based on row index
            rec.width, // full width of table
            rowHeight  // height divided equally
        };

        row.width = FloatConstrained(rec.width, 0, rec.width);
        row.height = FloatConstrained(rowHeight, 0, rec.height);

        // Reposition existing rows for even distribution
        for (size_t i = 0; i < children.size(); i++)
        {
            children[i].rec.y = rec.y + (i * rowHeight);
            children[i].rec.height = rowHeight;
            children[i].height = FloatConstrained(rowHeight, 0, rec.height);
        }

        children.push_back(std::move(row));
        return &children.back();
    }
    void Table::Draw2D()
    {
        for (auto& row : children)
        {
            row.Draw2D();
        }
    }

    [[nodiscard]] TableCell* TableRow::CreateTableCell(float requestedWidth)
    {
        TableCell cell;
        cell.parent = this;

        // Calculate available width in the row
        float totalRequestedWidth = 0;
        for (const auto& existingCell : children)
        {
            totalRequestedWidth += existingCell.width.GetValue();
        }
        totalRequestedWidth += requestedWidth;

        // Adjust requested width if it would exceed row width
        float scaleFactor = 1.0f;
        if (totalRequestedWidth > rec.width)
        {
            scaleFactor = rec.width / totalRequestedWidth;
            requestedWidth *= scaleFactor;

            // Also adjust existing cells
            for (auto& existingCell : children)
            {
                float newWidth = existingCell.width.GetValue() * scaleFactor;
                existingCell.width = FloatConstrained(newWidth, 0, rec.width);
                existingCell.rec.width = newWidth;
            }
        }

        // Calculate x position based on existing cells
        float xPosition = rec.x;
        for (const auto& existingCell : children)
        {
            xPosition += existingCell.rec.width;
        }

        // Set up the cell's rectangle
        cell.rec = {
            xPosition,      // Position after existing cells
            rec.y,          // Same y as parent row
            requestedWidth, // Scaled width
            rec.height      // Same height as parent row
        };

        cell.width = FloatConstrained(requestedWidth, 0, rec.width);
        cell.height = FloatConstrained(rec.height, 0, rec.height);

        // Set default padding and margins
        cell.paddingLeft = cell.paddingRight = 5; // Add some default padding
        cell.paddingUp = cell.paddingDown = 5;
        cell.marginLeft = cell.marginRight = cell.marginUp = cell.marginDown = 0;

        children.push_back(std::move(cell));
        return &children.back();
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
            c->Draw2D();
        }
    }

    TextBox* TableCell::CreateTextbox(const std::string& _content)
    {
        auto textbox = std::make_unique<TextBox>();
        textbox->fontSize = 20; // Increased font size for better visibility
        textbox->content = _content;

        // Calculate text dimensions
        Vector2 textSize = MeasureTextEx(GetFontDefault(), _content.c_str(), textbox->fontSize, 1);

        // Center the text in the cell
        float textX = rec.x + paddingLeft + (rec.width - paddingLeft - paddingRight - textSize.x) / 2;
        float textY = rec.y + paddingUp + (rec.height - paddingUp - paddingDown - textSize.y) / 2;

        textbox->rec = {textX, textY, textSize.x, textSize.y};

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
            rec.x + paddingLeft,
            rec.y + paddingUp,
            rec.width - (paddingLeft + paddingRight),
            rec.height - (paddingUp + paddingDown)};

        Button* ptr = button.get();
        children.push_back(std::move(button));
        return ptr;
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