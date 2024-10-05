//
// Created by steve on 02/10/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    struct TableCell;
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

        virtual void UpdateChildren() = 0;
        virtual void Draw2D() = 0;

        TableElement() = default;
        TableElement(const TableElement&) = default;
        TableElement(TableElement&&) noexcept = default;
        TableElement& operator=(const TableElement&) = default;
        TableElement& operator=(TableElement&&) noexcept = default;
        virtual ~TableElement() = default;
    };

    struct CellElement
    {
        TableCell* parent{};
        Rectangle rec{};

        virtual void UpdateRec() = 0;
        virtual void Draw2D() = 0;

        CellElement() = default;
        CellElement(const CellElement&) = default;
        CellElement(CellElement&&) noexcept = default;
        CellElement& operator=(const CellElement&) = default;
        CellElement& operator=(CellElement&&) noexcept = default;
        virtual ~CellElement() = default;
    };

    struct TextBox final : public CellElement
    {
        float fontSize{};
        float fontSpacing = 2;
        // font?
        // color?
        std::string content;
        // Text wrap, scroll bar etc?

        void UpdateRec() override;
        void Draw2D() override;
        ~TextBox() override = default;
    };

    struct Button final : public CellElement
    {
        Texture tex{};
        entt::sigh<void(int)> onButtonStartHover;
        entt::sigh<void(int)> onButtonEndHover;
        entt::sigh<void(int)> onButtonPress;

        void UpdateRec() override;
        void Draw2D() override;
        ~Button() override = default;
    };

    struct TableCell
    {
        TableRow* parent;
        Rectangle rec{};
        std::unique_ptr<CellElement> child;
        Padding padding;
        Margin margin;
        Texture tex{};
        void UpdateChild();
        TextBox* CreateTextbox(const std::string& _content);
        Button* CreateButton(Texture _tex);
        void Draw2D();
        ~TableCell() = default;
    };

    struct TableRow final : public TableElement<std::unique_ptr<TableCell>, Table>
    {
        Texture tex{};
        void UpdateChildren() override;
        TableCell* CreateTableCell(Padding _padding, Margin _margin);
        TableCell* CreateTableCell();
        void Draw2D() override;
        ~TableRow() override = default;
    };

    struct Table final : public TableElement<std::unique_ptr<TableRow>, Window>
    {
        Texture tex{};
        void UpdateChildren() override;
        TableRow* CreateTableRow();
        void Draw2D() override;
        ~Table() override = default;
    };

    struct Window
    {
        Rectangle rec;
        Texture tex;
        NPatchInfo nPatchInfo;
        entt::sigh<void(int)> onWindowStartHover;
        entt::sigh<void(int)> onWindowEndHover;
        std::vector<std::unique_ptr<Table>> children;

        Table* CreateTable();

        void Draw2D() const;
    };

    class GameUIEngine
    {
        Texture nPatchTexture;
        std::vector<std::unique_ptr<Window>> windows;
        int nextId;

      public:
        Window* CreateWindow(Vector2 pos, float w, float h);

        void Draw2D();
        void Update();

        GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor);
    };
} // namespace sage
