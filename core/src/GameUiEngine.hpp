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

    struct Dimensions
    {
        float width;
        float height;
    };

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

        // Save original width/height request and calculate each time UpdateChildren is called.
        // This would make it very easy to increase/decrease scale if you increase the screen size (full screen
        // etc).

      public:
        Parent* parent;
        Rectangle rec{};
        Child children;

        std::optional<Texture> tex{};
        std::optional<NPatchInfo> nPatchInfo{};

        virtual void UpdateChildren() = 0;
        virtual void DrawDebug2D() = 0;
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

        bool MouseInside(Vector2 mousePos)
        {
            return mousePos.x >= rec.x && mousePos.x <= rec.x + rec.width && mousePos.y >= rec.y &&
                   mousePos.y <= rec.y + rec.height;
        }

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
        Texture tex{};
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;
        bool mouseHover = false;

        void SetVertAlignment(VertAlignment alignment);
        void SetHoriAlignment(HoriAlignment alignment);

        entt::sigh<void()> onStartHover;
        entt::sigh<void()> onEndHover;
        entt::sigh<void()> onMouseClicked;

        bool draggable = false;
        bool beingDragged = false;
        entt::sigh<void()> onDragStart;
        entt::sigh<void()> onDragEnd;

        virtual void OnMouseStartHover()
        {
            mouseHover = true;
            onStartHover.publish();
        };

        virtual void OnMouseStopHover()
        {
            mouseHover = false;
            onEndHover.publish();
        };

        virtual void OnMouseClick()
        {
            onMouseClicked.publish();
        };

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
        enum class OverflowBehaviour
        {
            SHRINK_TO_FIT,
            WORD_WRAP
        };
        OverflowBehaviour overflowBehaviour = OverflowBehaviour::SHRINK_TO_FIT;
        float fontSize = 12;
        float fontSpacing = 2;
        Font font = GetFontDefault();
        // color?
        std::string content;

        void SetOverflowBehaviour(OverflowBehaviour _behaviour);
        void UpdateDimensions() override;
        void Draw2D() override;
        ~TextBox() override = default;
    };

    struct ImageBox final : public CellElement
    {
        std::optional<Shader> shader;
        Window* parentWindow;

        void OnMouseStartHover() override;
        void OnMouseStopHover() override;
        void OnMouseClick() override;

        void SetGrayscale();
        void RemoveShader();
        void UpdateDimensions() override;
        void Draw2D() override;
        ~ImageBox() override = default;
    };

    struct TableCell final : public TableElement<std::unique_ptr<CellElement>, TableRow>
    {
        float requestedWidth{};
        bool autoSize = true;
        TextBox* CreateTextbox(
            const std::string& _content,
            float fontSize = 16,
            TextBox::OverflowBehaviour overflowBehaviour = TextBox::OverflowBehaviour::SHRINK_TO_FIT);
        ImageBox* CreateImagebox(Window* _parentWindow, Image _tex);
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~TableCell() override = default;
    };

    struct TableRow final : public TableElement<std::vector<std::unique_ptr<TableCell>>, Table>
    {
        float requestedHeight{};
        bool autoSize = true;
        TableCell* CreateTableCell();
        TableCell* CreateTableCell(float _requestedWidth);
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~TableRow() override = default;
    };

    struct Table final : public TableElement<std::vector<std::unique_ptr<TableRow>>, Window>
    {
        TableRow* CreateTableRow();
        TableRow* CreateTableRow(float _requestedHeight);
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~Table() override = default;
    };

    struct Window : TableElement<std::vector<std::unique_ptr<Table>>, void>
    {
        bool hidden = false;
        const Settings* settings; // for screen width/height

        entt::sigh<void()> onMouseStartHover;
        entt::sigh<void()> onMouseStopHover;

        float xOffsetPercent = 0;
        float yOffsetPercent = 0;
        float widthPercent = 0;
        float heightPercent = 0;
        WindowTableAlignment tableAlignment = WindowTableAlignment::STACK_HORIZONTAL;
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;

        Texture mainNPatchTexture; // npatch texture used by elements in window

        [[nodiscard]] Dimensions GetDimensions() const;
        void SetDimensionsPercent(float _widthPercent, float _heightPercent);
        [[nodiscard]] Vector2 GetOffset() const;
        void SetOffsetPercent(float _xOffsetPercent, float _yOffsetPercent);
        void SetAlignment(VertAlignment vert, HoriAlignment hori);
        Table* CreateTable();
        void OnScreenSizeChange();
        void DrawDebug2D() override;
        void Draw2D() override;
        void UpdateChildren() override;
    };

    class GameUIEngine
    {
        std::vector<std::unique_ptr<Window>> windows;
        Cursor* cursor;
        UserInput* userInput;
        Settings* settings;

        std::optional<CellElement*> draggedElement{};
        double draggedTimer = 0;
        float draggedTimerThreshold = 0.25f;

      public:
        Window* CreateWindow(
            Image _nPatchTexture,
            float _xOffsetPercent,
            float _yOffsetPercent,
            float _widthPercent,
            float _heightPercent,
            WindowTableAlignment _alignment = WindowTableAlignment::STACK_HORIZONTAL);

        void DrawDebug2D();
        void Draw2D();
        void Update();

        GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor);
    };
} // namespace sage
