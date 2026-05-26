//
// Layout container hierarchy: TableElement (abstract base), TableCell, TableRow,
// Table, and the *Grid variants. Builds the tree that Window owns.
//

#pragma once

#include "UIBase.hpp"

#include "raylib.h"

#include <memory>
#include <optional>
#include <vector>

namespace sage
{
    class TextBox;
    class Checkbox;
    class DropdownList;
    class TitleBar;
    class ImageBox;
    class GameWindowButton;
    class CloseButton;
    class TableGrid;
    class TableCell;
    class TableRow;
    class Table;
    class Window;
    struct Settings;

    class TableElement : public UIElement
    {
      protected:
        struct UnscaledDimensions
        {
            Rectangle rec{};
            Padding padding{};
            Vector2 scalePosOffset{}; // The offset between the old and new screen size
        };

        TextureStretchMode textureStretchMode = TextureStretchMode::NONE;
        virtual void Reset();

      public:
        Padding padding;
        UnscaledDimensions unscaledDimensions{};
        TableElement* parent{};
        std::vector<std::unique_ptr<TableElement>> children;
        std::optional<std::unique_ptr<CellElement>> element;
        std::optional<Texture> tex{};
        std::optional<NPatchInfo> nPatchInfo{};

        virtual CellElement* GetCellUnderCursor(Vector2 mousePos);
        [[nodiscard]] virtual bool CapturesCursor(Vector2 point) const;
        void OnHoverStop() override;
        virtual void Update();
        virtual void ScaleContents(Settings* _settings);
        virtual void SetPos(float x, float y);
        void SetDimensions(float w, float h);
        void SetTexture(const Texture& _tex, TextureStretchMode _stretchMode);
        void UpdateTextureDimensions();
        virtual void FinalizeLayout();
        virtual void InitLayout() = 0;
        virtual void DrawDebug2D();
        virtual void Draw2D();
        [[nodiscard]] Window* GetWindow();

        TableElement(TableElement* _parent, float x, float y, float width, float height, Padding _padding);
        TableElement(TableElement* _parent, Padding _padding);

        TableElement(const TableElement&) = delete;
        TableElement(TableElement&&) noexcept = default;
        TableElement& operator=(const TableElement&) = delete;
        TableElement& operator=(TableElement&&) noexcept = default;
        ~TableElement() override = default;
    };

    class TableCell : public TableElement
    {
      protected:
        float requestedWidth{};
        bool autoSize = true;

      public:
        // TODO: use polymorphism for any duplicates
        TextBox* CreateTextbox(std::unique_ptr<TextBox> _textBox, const std::string& _content);
        Checkbox* CreateCheckbox(std::unique_ptr<Checkbox> _checkbox);
        DropdownList* CreateDropdownList(std::unique_ptr<DropdownList> _dropdown);
        TitleBar* CreateTitleBar(std::unique_ptr<TitleBar> _titleBar, const std::string& _title);
        ImageBox* CreateImagebox(std::unique_ptr<ImageBox> _imageBox);
        CloseButton* CreateCloseButton(std::unique_ptr<CloseButton> _closeButton);
        GameWindowButton* CreateGameWindowButton(std::unique_ptr<GameWindowButton> _button);
        TableGrid* CreateTableGrid(int rows, int cols, float cellSpacing = 0, Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void InitLayout() override;
        ~TableCell() override = default;
        explicit TableCell(TableRow* _parent, Padding _padding = {0, 0, 0, 0});
        friend class TableRow;
    };

    class TableRow : public TableElement
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableCell* CreateTableCell(Padding _padding = {0, 0, 0, 0});
        TableCell* CreateTableCell(float _requestedWidth, Padding _padding = {0, 0, 0, 0});
        void InitLayout() override;
        ~TableRow() override = default;
        explicit TableRow(Table* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Table;
    };

    class TableRowGrid final : public TableRow
    {
        float cellSpacing = 0;

      public:
        void InitLayout() override;
        explicit TableRowGrid(Table* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Table;
    };

    class Table : public TableElement
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableRowGrid* CreateTableRowGrid(int cols, float cellSpacing, Padding _padding);
        TableRow* CreateTableRow(Padding _padding = {0, 0, 0, 0});
        TableRow* CreateTableRow(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void InitLayout() override;
        ~Table() override = default;
        explicit Table(Window* _parent, Padding _padding = {0, 0, 0, 0});
        explicit Table(TableCell* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Window;
        friend class TableCell;
    };

    class TableGrid final : public Table
    {
        float cellSpacing = 0;

      public:
        void InitLayout() override;
        explicit TableGrid(Window* _parent, Padding _padding = {0, 0, 0, 0});
        explicit TableGrid(TableCell* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Window;
        friend class TableCell;
    };

    // Per-child input to distributeAlong: percentage-based or auto.
    struct SizeRequest
    {
        bool autoSize;
        float requestedPercent;
    };

    // Two-pass percentage size distribution along one axis. Auto-sized children
    // share the leftover percent equally; explicit-percent children take their
    // requested share. Returned sizes are ceil-rounded.
    std::vector<float> distributeAlong(float availableSize, const std::vector<SizeRequest>& requests);
} // namespace sage
