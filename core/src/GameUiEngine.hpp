//
// Created by steve on 02/10/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <optional>
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

    enum class WindowTableAlignment
    {
        STACK_VERTICAL,
        STACK_HORIZONTAL
    };

    enum class HoriAlignment
    {
        LEFT,
        RIGHT,
        CENTER
    };

    enum class VertAlignment
    {
        TOP,
        MIDDLE,
        BOTTOM,
    };

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

    template <typename Child, typename Parent>
    struct TableElement
    {
      protected:
        Padding padding;
        Margin margin;

      public:
        Parent* parent;
        Rectangle rec{};
        Child children;

        std::optional<Texture> tex{};
        std::optional<NPatchInfo> nPatchInfo{};

        virtual void UpdateChildren() = 0;
        virtual void Draw2D()
        {
            if (tex.has_value())
            {
                if (nPatchInfo.has_value())
                {
                    DrawTextureNPatch(
                        tex.value(),
                        nPatchInfo.value(),
                        rec,
                        {0.0f, 0.0f},
                        0.0f,
                        WHITE); // Use {0.0f, 0.0f} for origin
                }
                else
                {
                    DrawTexture(tex.value(), rec.x, rec.y, WHITE);
                }
            }
        };

        void SetPaddingPixel(const Padding& _padding)
        {
            padding = _padding;
            UpdateChildren();
        }

        // Set padding in percent of parent
        void SetPaddingPercent(const Padding& _padding)
        {
            padding.up = rec.height * (_padding.up / 100);
            padding.down = rec.height * (_padding.down / 100);
            padding.left = rec.width * (_padding.left / 100);
            padding.right = rec.width * (_padding.right / 100);
            UpdateChildren();
        }

        // Returns pixel value of padding
        [[nodiscard]] const Padding& GetPadding() const
        {
            return padding;
        }

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
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;

        void SetVertAlignment(VertAlignment alignment);
        void SetHoriAlignment(HoriAlignment alignment);
        virtual void UpdateDimensions() = 0;
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

        void UpdateDimensions() override;
        void Draw2D() override;
        ~TextBox() override = default;
    };

    struct ImageBox final : public CellElement
    {
        Texture tex{};
        entt::sigh<void()> onStartHover;
        entt::sigh<void()> onEndHover;
        entt::sigh<void()> onPress;

        void UpdateDimensions() override;
        void Draw2D() override;
        ~ImageBox() override = default;
    };

    struct TableCell final : public TableElement<std::unique_ptr<CellElement>, TableRow>
    {
        TextBox* CreateTextbox(const std::string& _content);
        ImageBox* CreateImagebox(Image _tex);
        void UpdateChildren() override;
        void Draw2D() override;
        ~TableCell() override = default;
    };

    struct TableRow final : public TableElement<std::vector<std::unique_ptr<TableCell>>, Table>
    {
        TableCell* CreateTableCell();
        void UpdateChildren() override;
        void Draw2D() override;
        ~TableRow() override = default;
    };

    struct Table final : public TableElement<std::vector<std::unique_ptr<TableRow>>, Window>
    {
        TableRow* CreateTableRow();
        void UpdateChildren() override;
        void Draw2D() override;
        ~Table() override = default;
    };

    struct Window : TableElement<std::vector<std::unique_ptr<Table>>, void>
    {
        WindowTableAlignment tableAlignment = WindowTableAlignment::STACK_HORIZONTAL;
        Texture mainNPatchTexture; // npatch texture used by elements in window
        entt::sigh<void(int)> onWindowStartHover;
        entt::sigh<void(int)> onWindowEndHover;

        Table* CreateTable();

        void Draw2D() override;
        void UpdateChildren() override;
    };

    class GameUIEngine
    {
        std::vector<std::unique_ptr<Window>> windows;

      public:
        Window* CreateWindow(
            Image _nPatchTexture,
            Vector2 pos,
            float w,
            float h,
            WindowTableAlignment _alignment = WindowTableAlignment::STACK_HORIZONTAL);

        void Draw2D();
        void Update();

        GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor);
    };
} // namespace sage
