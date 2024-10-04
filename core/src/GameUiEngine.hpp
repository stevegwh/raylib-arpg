//
// Created by steve on 02/10/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    struct Settings;
    class UserInput;
    class Cursor;

    struct Table;
    struct TableRow;
    struct TableCell;
    struct UIElement;

    class FloatConstrained
    {
        float value = 0;
        float min = 0;
        float max = 100;

      public:
        FloatConstrained(float _value, float _min, float _max) : value(_value), min(_min), max(_max){};
        FloatConstrained() = default;

        [[nodiscard]] float GetValue() const
        {
            return value;
        }
    };

    struct Window
    {
        Rectangle rec;
        Texture tex;
        NPatchInfo nPatchInfo;
        entt::sigh<void(int)> onWindowStartHover;
        entt::sigh<void(int)> onWindowEndHover;
        std::vector<Table> children;

        Table* CreateTable();

        void Draw2D();
    };

    struct Table
    {
        Window* parent;
        Rectangle rec;
        FloatConstrained width;
        FloatConstrained height;
        Texture tex;
        std::vector<TableRow> children;
        TableRow* CreateTableRow();
        void Draw2D();
    };

    struct TableRow
    {
        Table* parent;
        Rectangle rec;
        FloatConstrained width;
        FloatConstrained height;
        Texture tex;
        std::vector<TableCell> children;
        TableCell* CreateTableCell(float width);
        void Draw2D();
    };

    struct UIElement
    {
        virtual void Draw2D() = 0;
    };

    struct TextBox : public UIElement
    {
        float fontSize;
        Rectangle rec;
        // font?
        // color?
        std::string content;
        // Text wrap, scroll bar etc?

        void Draw2D() override;
    };

    struct Button : public UIElement
    {
        Texture tex;
        Rectangle rec;
        entt::sigh<void(int)> onButtonStartHover;
        entt::sigh<void(int)> onButtonEndHover;
        entt::sigh<void(int)> onButtonPress;

        void Draw2D() override;
    };

    struct TableCell
    {
        TableRow* parent;
        Rectangle rec;
        FloatConstrained width;
        FloatConstrained height;
        float paddingLeft, paddingRight, paddingUp, paddingDown;
        float marginLeft, marginRight, marginUp, marginDown;
        Texture tex;
        TextBox* CreateTextbox(const std::string& _content);
        Button* CreateButton(Texture _tex);
        std::vector<std::unique_ptr<UIElement>> children;
        void Draw2D();
    };

    class GameUIEngine
    {
        Texture nPatchTexture;
        std::vector<Window> windows;
        int nextId;

      public:
        Window* CreateWindow(Vector2 pos, float w, float h);

        void Draw2D();
        void Update();

        GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor);
    };
} // namespace sage
