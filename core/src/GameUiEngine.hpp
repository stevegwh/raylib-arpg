//
// Created by steve on 02/10/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    struct Window;
    struct TableRow;
    struct Table;

    struct Settings;
    class UserInput;
    class Cursor;

    struct Padding
    {
        float up = 0;
        float down = 0;
        float left = 0;
        float right = 0;
    };

    struct Margin
    {
        float up = 0;
        float down = 0;
        float left = 0;
        float right = 0;
    };

    template <typename ChildName, typename ParentName>
    struct TableElement
    {
        ParentName* parent;
        Rectangle rec{};
        Padding padding;
        Margin margin;
        std::vector<ChildName> children;

        virtual void Draw2D(){

        };

        TableElement() = default;
        TableElement(const TableElement&) = default;
        TableElement(TableElement&&) noexcept = default;
        TableElement& operator=(const TableElement&) = default;
        TableElement& operator=(TableElement&&) noexcept = default;
        virtual ~TableElement() = default;
    };

    struct CellElement
    {
        Rectangle rec{};

        virtual void Draw2D(){

        };

        CellElement() = default;
        CellElement(const CellElement&) = default;
        CellElement(CellElement&&) noexcept = default;
        CellElement& operator=(const CellElement&) = default;
        CellElement& operator=(CellElement&&) noexcept = default;
        virtual ~CellElement() = default;
    };

    struct TextBox : public CellElement
    {
        float fontSize{};
        // font?
        // color?
        std::string content;
        // Text wrap, scroll bar etc?

        void Draw2D() override;
        TextBox() = default;
        TextBox(const TextBox&) = default;
        TextBox(TextBox&&) noexcept = default;
        TextBox& operator=(const TextBox&) = default;
        TextBox& operator=(TextBox&&) noexcept = default;
        ~TextBox() override = default;
    };

    struct Button : public CellElement
    {
        Texture tex{};
        entt::sigh<void(int)> onButtonStartHover;
        entt::sigh<void(int)> onButtonEndHover;
        entt::sigh<void(int)> onButtonPress;

        void Draw2D() override;
        Button() = default;
        Button(const Button&) = default;
        Button(Button&&) noexcept = default;
        Button& operator=(const Button&) = default;
        Button& operator=(Button&&) noexcept = default;
        ~Button() override = default;
    };

    struct TableCell : public TableElement<std::unique_ptr<CellElement>, TableRow>
    {
        Texture tex{};
        TextBox* CreateTextbox(const std::string& _content);
        Button* CreateButton(Texture _tex);
        void Draw2D() override;
        TableCell() = default;
        TableCell(const TableCell&) = delete; // Unique_ptr can't be copied
        TableCell(TableCell&&) noexcept = default;
        TableCell& operator=(const TableCell&) = delete;
        TableCell& operator=(TableCell&&) noexcept = default;
        ~TableCell() override = default;
    };

    struct TableRow : public TableElement<TableCell, Table>
    {
        Texture tex{};
        TableCell* CreateTableCell(Padding _padding, Margin _margin);
        TableCell* CreateTableCell();
        void Draw2D() override;
        TableRow() = default;
        TableRow(const TableRow&) = default;
        TableRow(TableRow&&) noexcept = default;
        TableRow& operator=(const TableRow&) = default;
        TableRow& operator=(TableRow&&) noexcept = default;
        ~TableRow() override = default;
    };

    struct Table : public TableElement<TableRow, Window>
    {
        Texture tex{};
        TableRow* CreateTableRow();
        void Draw2D() override;
        Table() = default;
        Table(const Table&) = default;
        Table(Table&&) noexcept = default;
        Table& operator=(const Table&) = default;
        Table& operator=(Table&&) noexcept = default;
        ~Table() override = default;
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
