//
// Layout container hierarchy implementation. See UILayout.hpp.
//

#include "UILayout.hpp"

#include "../GameUiEngine.hpp" // for full Window / TextBox / ImageBox / etc.
#include "../Settings.hpp"
#include "../slib.hpp"
#include "UIElements.hpp"

#include "raylib.h"

#include <cassert>
#include <cmath>
#include <queue>

namespace sage
{
    namespace
    {
        // Cleaned-up replacement for the previous reinterpret_cast<T*>(child.get()) idiom
        // — children are inserted via type-specific Create* methods so the static_cast is
        // sound, but we keep a debug-only dynamic_cast assert to catch regressions.
        template <typename T>
        T* downcast(TableElement* e)
        {
            assert(dynamic_cast<T*>(e) != nullptr);
            return static_cast<T*>(e);
        }
    } // namespace

    std::vector<float> distributeAlong(float availableSize, const std::vector<SizeRequest>& requests)
    {
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;
        for (const auto& r : requests)
        {
            if (r.autoSize)
                ++autoSizeCount;
            else
                totalRequestedPercent += r.requestedPercent;
        }
        totalRequestedPercent = std::min(totalRequestedPercent, 100.0f);
        const float remainingPercent = 100.0f - totalRequestedPercent;
        const float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        std::vector<float> out;
        out.reserve(requests.size());
        for (const auto& r : requests)
        {
            const float pct = r.autoSize ? autoSizePercent : r.requestedPercent;
            out.push_back(std::ceil(availableSize * (pct / 100.0f)));
        }
        return out;
    }

    void Table::InitLayout()
    {
        const float availableHeight = rec.height - (padding.up + padding.down);
        const float startY = rec.y + padding.up;

        std::vector<SizeRequest> requests;
        requests.reserve(children.size());
        for (const auto& r : children)
        {
            auto* row = downcast<TableRow>(r.get());
            requests.push_back({row->autoSize, row->requestedHeight});
        }
        auto sizes = distributeAlong(availableHeight, requests);

        float currentY = startY;
        for (size_t i = 0; i < children.size(); ++i)
        {
            auto* row = downcast<TableRow>(children[i].get());
            row->parent = this;
            row->rec = rec;

            const float rowHeight = sizes[i];

            // Set row dimensions accounting for table padding
            row->rec.height = rowHeight;
            row->rec.y = currentY;
            row->rec.x = rec.x + padding.left;
            row->rec.width = rec.width - (padding.left + padding.right);

            UpdateTextureDimensions();

            if (!row->children.empty())
            {
                row->InitLayout();
            }

            currentY += rowHeight;
        }
    }

    void TableRowGrid::InitLayout()
    {
        if (children.empty()) return;

        const unsigned int cols = children.size();
        const float availableWidth = rec.width - (padding.left + padding.right);
        const float availableHeight = rec.height - (padding.up + padding.down);
        const int maxCellSize =
            1 << static_cast<int>(std::floor(std::log2(std::min(availableWidth / cols, availableHeight))));
        const float cellSize =
            (std::min(availableWidth / cols, availableHeight) - cellSpacing) / maxCellSize * maxCellSize;
        const float gridWidth = cellSize * cols + cellSpacing * (cols - 1); // Account for spacing between columns
        const float startX = rec.x + (availableWidth - gridWidth) / 2.0f;
        const float startY = rec.y + (availableHeight - cellSize) / 2.0f;

        float currentX = startX;
        for (const auto& cell : children)
        {
            cell->SetPos(currentX, startY);
            cell->SetDimensions(cellSize, cellSize);
            currentX += cellSize + cellSpacing; // Add spacing after each cell

            if (cell->element.has_value())
            {
                cell->element.value()->UpdateDimensions();
            }
            // TODO: end here or?
        }
    }

    void TableGrid::InitLayout()
    {
        if (children.empty()) return;

        const unsigned int cols = children[0]->children.size();
        const float availableWidth = rec.width - (padding.left + padding.right);
        const float availableHeight = rec.height - (padding.up + padding.down);
        const int maxCellSize =
            1 << static_cast<int>(
                std::floor(std::log2(std::min(availableWidth / cols, availableHeight / children.size()))));
        const float cellSize = (std::min(availableWidth / cols, availableHeight / children.size()) - cellSpacing) /
                               maxCellSize * maxCellSize;
        const float gridWidth = cellSize * cols + cellSpacing * (cols - 1); // Account for spacing between columns
        const float gridHeight =
            cellSize * children.size() + cellSpacing * (children.size() - 1); // Account for spacing between rows
        const float startX = rec.x + (availableWidth - gridWidth) / 2.0f;
        const float startY = rec.y + (availableHeight - gridHeight) / 2.0f;
        float currentY = startY;
        for (const auto& row : children)
        {
            row->SetPos(startX, currentY);
            row->SetDimensions(gridWidth, cellSize);

            float currentX = row->GetRec().x;
            for (const auto& cell : row->children)
            {
                cell->SetPos(currentX, currentY);
                cell->SetDimensions(cellSize, cellSize);
                currentX += cellSize + cellSpacing; // Add spacing after each cell
            }
            currentY += cellSize + cellSpacing; // Add spacing after each row
        }

        for (const auto& row : children)
        {
            for (const auto& cell : row->children)
            {
                if (cell->element.has_value())
                {
                    cell->element.value()->UpdateDimensions();
                }
                // TODO: End here?
            }
        }
    }

    void TableRow::InitLayout()
    {
        const float availableWidth = rec.width - (padding.left + padding.right);
        const float startX = rec.x + padding.left;

        std::vector<SizeRequest> requests;
        requests.reserve(children.size());
        for (const auto& c : children)
        {
            auto* cell = downcast<TableCell>(c.get());
            requests.push_back({cell->autoSize, cell->requestedWidth});
        }
        auto sizes = distributeAlong(availableWidth, requests);

        float currentX = startX;
        for (size_t i = 0; i < children.size(); ++i)
        {
            auto* cell = downcast<TableCell>(children[i].get());
            cell->parent = this;
            cell->rec = rec;

            const float cellWidth = sizes[i];

            // Set cell dimensions accounting for row padding
            cell->rec.width = cellWidth;
            cell->rec.x = currentX;
            cell->rec.y = rec.y + padding.up;
            cell->rec.height = rec.height - (padding.up + padding.down);

            UpdateTextureDimensions();

            cell->InitLayout();

            cell->unscaledDimensions.rec = rec;
            cell->unscaledDimensions.padding = padding;

            currentX += cellWidth;
        }
    }

    void TableCell::InitLayout()
    {
        if (element.has_value())
        {
            if (element.value())
            {
                element.value()->parent = this;
                element.value()->rec = rec;
                element.value()->UpdateDimensions();
            }
            return;
        }

        // Same vertical-distribution algorithm as Window::InitLayout / Table::InitLayout.
        const float availableHeight = rec.height - (padding.up + padding.down);
        const float startY = rec.y + padding.up;
        const float availableWidth = rec.width - (padding.left + padding.right);
        const float startX = rec.x + padding.left;

        std::vector<SizeRequest> requests;
        requests.reserve(children.size());
        for (const auto& p : children)
        {
            auto* table = downcast<Table>(p.get());
            requests.push_back({table->autoSize, table->requestedHeight});
        }
        auto sizes = distributeAlong(availableHeight, requests);

        float currentY = startY;
        for (size_t i = 0; i < children.size(); ++i)
        {
            auto* table = downcast<Table>(children[i].get());
            table->parent = this;
            table->rec = rec;

            const float panelHeight = sizes[i];
            table->rec.height = panelHeight;
            table->rec.y = currentY;
            table->rec.width = availableWidth;
            table->rec.x = startX;

            UpdateTextureDimensions();

            if (!table->children.empty()) table->InitLayout();

            currentY += panelHeight;
        }
    }

    void TableElement::Reset()
    {
        rec = unscaledDimensions.rec;
        padding = unscaledDimensions.padding;
        for (auto& child : children)
        {
            child->Reset();
        }
    }

    CellElement* TableElement::GetCellUnderCursor()
    {
        const auto& mousePos = GetMousePosition();
        if (element.has_value())
        {
            if (element.value()->CapturesCursor(mousePos) || PointInsideRect(rec, mousePos))
            {
                return element.value().get();
            }
            return nullptr;
        }
        for (const auto& child : children)
        {
            if (child->CapturesCursor(mousePos))
            {
                if (auto childCell = child->GetCellUnderCursor())
                {
                    return childCell;
                }
            }
        }
        for (const auto& child : children)
        {
            if (auto childCell = child->GetCellUnderCursor())
            {
                return childCell;
            }
        }
        return nullptr;
    }

    bool TableElement::CapturesCursor(Vector2 point) const
    {
        if (element.has_value())
        {
            return element.value() && element.value()->CapturesCursor(point);
        }

        for (const auto& child : children)
        {
            if (child->CapturesCursor(point))
            {
                return true;
            }
        }
        return false;
    }

    void TableElement::Update()
    {
        assert(!(element.has_value() && !children.empty()));
        if (element.has_value())
        {
            auto* el = element.value().get();
            updateUIState(*el, *el->engine, el->engine->Input());
            return;
        }
        for (const auto& child : children)
        {
            child->Update();
        }
    }

    void TableElement::OnHoverStop()
    {
        if (element.has_value())
        {
            if (!element.value()) return;
            if (element.value()->beingDragged) return;
            auto* el = element.value().get();
            transitionTo(*el, *el->engine, IdleState{});
        }
        else
        {
            for (const auto& child : children)
            {
                child->OnHoverStop();
            }
        }
    }

    void TableElement::ScaleContents(Settings* _settings)
    {
        {
            auto posScaled = _settings->ScalePos({rec.x, rec.y});

            rec = {
                posScaled.x,
                posScaled.y,
                _settings->ScaleValueMaintainRatio(rec.width),
                _settings->ScaleValueMaintainRatio(rec.height)};

            padding = {
                _settings->ScaleValueMaintainRatio(padding.up),
                _settings->ScaleValueMaintainRatio(padding.down),
                _settings->ScaleValueMaintainRatio(padding.left),
                _settings->ScaleValueMaintainRatio(padding.right)};

            UpdateTextureDimensions();

            if (element.has_value())
            {
                element.value()->UpdateDimensions();
            }
            else
            {
                for (const auto& child : children)
                {
                    child->ScaleContents(_settings);
                }
            }
        }
    }

    void TableElement::SetPos(float x, float y)
    {

        rec = {x, y, rec.width, rec.height};
    }

    void TableElement::SetDimensions(float w, float h)
    {
        rec = {rec.x, rec.y, w, h};
    }

    void TableElement::SetTexture(const Texture& _tex, TextureStretchMode _stretchMode)
    {
        tex = _tex;
        textureStretchMode = _stretchMode;
        UpdateTextureDimensions();
    }

    void TableElement::UpdateTextureDimensions()
    {
        if (!tex.has_value()) return;
        if (textureStretchMode == TextureStretchMode::STRETCH)
        {
            tex->width = rec.width;
            tex->height = rec.height;
        }
        else if (textureStretchMode == TextureStretchMode::FILL)
        {
            if (tex->width > tex->height)
            {
                tex->width = rec.width;
            }
            else
            {
                tex->height = rec.height;
            }
        }

        // TILE not needed to update here
    }

    void TableElement::FinalizeLayout()
    {
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
        for (const auto& child : children)
        {
            child->FinalizeLayout();
        }
    }

    Window* TableElement::GetWindow()
    {

        auto current = this;
        while (current->parent != nullptr)
        {
            current = current->parent;
        }

        return reinterpret_cast<Window*>(current);
    }

    void TableElement::DrawDebug2D()
    {
        static const std::vector colors = {
            RED, BLUE, YELLOW, WHITE, PINK, BLACK, ORANGE, PURPLE, BROWN, DARKGREEN};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& child = children[i];
            Color col = colors[i % children.size()];
            col.a = 150;
            DrawRectangle(child->rec.x, child->rec.y, child->rec.width, child->rec.height, col);
            child->DrawDebug2D();
        }
    }

    void TableElement::Draw2D()
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

                if (textureStretchMode == TextureStretchMode::STRETCH ||
                    textureStretchMode == TextureStretchMode::NONE)
                {
                    DrawTexture(tex.value(), rec.x, rec.y, WHITE);
                }
                else if (
                    textureStretchMode == TextureStretchMode::FILL ||
                    textureStretchMode == TextureStretchMode::TILE)
                {
                    DrawTextureRec(tex.value(), {0, 0, rec.width, rec.height}, {rec.x, rec.y}, WHITE);
                }
            }
        }
        if (element.has_value())
        {
            element.value()->Draw2D();
        }
        else
        {
            for (const auto& child : children)
            {
                child->Draw2D();
            }
        }
    }

    TableElement::TableElement(
        TableElement* _parent, float x, float y, float width, float height, Padding _padding)
        : padding(_padding), parent(_parent)
    {
        rec = {x, y, width, height};
        unscaledDimensions = {rec, padding};
    }

    TableElement::TableElement(TableElement* _parent, Padding _padding) : padding(_padding), parent(_parent)
    {
        unscaledDimensions = {rec, padding};
    }

    TableGrid::TableGrid(Window* _parent, Padding _padding) : Table(_parent, _padding)
    {
    }

    TableGrid::TableGrid(TableCell* _parent, Padding _padding) : Table(_parent, _padding)
    {
    }

    Table::Table(Window* _parent, Padding _padding) : TableElement(_parent, _padding)
    {
    }

    Table::Table(TableCell* _parent, Padding _padding) : TableElement(_parent, _padding)
    {
    }

    TableRowGrid* Table::CreateTableRowGrid(const int cols, const float cellSpacing, Padding _padding)
    {
        children.push_back(std::make_unique<TableRowGrid>(this, _padding));
        const auto& rowGrid = dynamic_cast<TableRowGrid*>(children.back().get());
        rowGrid->cellSpacing = cellSpacing;
        // Create rows and cells with initial autoSize = true
        for (int j = 0; j < cols; ++j)
        {
            rowGrid->CreateTableCell();
        }
        InitLayout();
        return rowGrid;
    }

    TableRow* Table::CreateTableRow(Padding _padding)
    {
        children.push_back(std::make_unique<TableRow>(this, _padding));
        const auto& row = dynamic_cast<TableRow*>(children.back().get());
        InitLayout();
        return row;
    }

    /**
     *
     * @param _requestedHeight The desired height of the cell as a percent (0-100)
     * @param _padding
     * @return
     */
    TableRow* Table::CreateTableRow(const float _requestedHeight, Padding _padding)
    {
        assert(_requestedHeight <= 100 && _requestedHeight >= 0);
        children.push_back(std::make_unique<TableRow>(this, _padding));
        const auto& row = dynamic_cast<TableRow*>(children.back().get());
        row->autoSize = false;
        row->requestedHeight = _requestedHeight;
        InitLayout();
        return row;
    }

    TableCell* TableRow::CreateTableCell(Padding _padding)
    {
        children.push_back(std::make_unique<TableCell>(this, _padding));
        const auto& cell = dynamic_cast<TableCell*>(children.back().get());
        InitLayout();
        return cell;
    }

    /**
     *
     * @param _requestedWidth The desired width of the cell as a percent (0-100)
     * @param _padding
     * @return
     */
    TableCell* TableRow::CreateTableCell(float _requestedWidth, Padding _padding)
    {
        assert(_requestedWidth <= 100 && _requestedWidth >= 0);
        children.push_back(std::make_unique<TableCell>(this, _padding));
        const auto& cell = dynamic_cast<TableCell*>(children.back().get());
        cell->autoSize = false;
        cell->requestedWidth = _requestedWidth;
        InitLayout();
        return cell;
    }

    TableRow::TableRow(Table* _parent, const Padding _padding) : TableElement(_parent, _padding)
    {
    }

    TableRowGrid::TableRowGrid(Table* _parent, const Padding _padding) : TableRow(_parent, _padding)
    {
    }

    TitleBar* TableCell::CreateTitleBar(std::unique_ptr<TitleBar> _titleBar, const std::string& _title)
    {
        element = std::move(_titleBar);
        auto* titleBar = dynamic_cast<TitleBar*>(element.value().get());
        titleBar->SetContent(_title);
        InitLayout();
        return titleBar;
    }

    CloseButton* TableCell::CreateCloseButton(std::unique_ptr<CloseButton> _closeButton)
    {
        element = std::move(_closeButton);
        auto* closeButton = dynamic_cast<CloseButton*>(element.value().get());
        InitLayout();
        return closeButton;
    }

    TextBox* TableCell::CreateTextbox(std::unique_ptr<TextBox> _textBox, const std::string& _content)
    {
        element = std::move(_textBox);
        auto* textbox = dynamic_cast<TextBox*>(element.value().get());
        textbox->SetContent(_content);
        InitLayout();
        return textbox;
    }

    Checkbox* TableCell::CreateCheckbox(std::unique_ptr<Checkbox> _checkbox)
    {
        element = std::move(_checkbox);
        auto* checkbox = dynamic_cast<Checkbox*>(element.value().get());
        InitLayout();
        return checkbox;
    }

    DropdownList* TableCell::CreateDropdownList(std::unique_ptr<DropdownList> _dropdown)
    {
        element = std::move(_dropdown);
        auto* dropdown = dynamic_cast<DropdownList*>(element.value().get());
        InitLayout();
        return dropdown;
    }

    ImageBox* TableCell::CreateImagebox(std::unique_ptr<ImageBox> _imageBox)
    {
        element = std::move(_imageBox);
        auto* image = dynamic_cast<ImageBox*>(element.value().get());
        InitLayout();
        return image;
    }

    GameWindowButton* TableCell::CreateGameWindowButton(std::unique_ptr<GameWindowButton> _button)
    {
        element = std::move(_button);
        auto* button = dynamic_cast<GameWindowButton*>(element.value().get());
        InitLayout();
        return button;
    }

    TableGrid* TableCell::CreateTableGrid(
        const int rows, const int cols, const float cellSpacing, Padding _padding)
    {
        children.push_back(std::make_unique<TableGrid>(this, _padding));
        const auto& table = dynamic_cast<TableGrid*>(children.back().get());
        table->cellSpacing = cellSpacing;
        // Create rows and cells with initial autoSize = true
        for (int i = 0; i < rows; ++i)
        {
            TableRow* row = table->CreateTableRow();
            for (int j = 0; j < cols; ++j)
            {
                row->CreateTableCell();
            }
        }
        InitLayout();
        return table;
    }

    Table* TableCell::CreateTable(Padding _padding)
    {
        children.push_back(std::make_unique<Table>(this, _padding));
        const auto& table = dynamic_cast<Table*>(children.back().get());
        InitLayout();
        return table;
    }

    Table* TableCell::CreateTable(const float _requestedHeight, const Padding _padding)
    {
        const auto table = CreateTable(_padding);
        table->autoSize = false;
        table->requestedHeight = _requestedHeight;
        InitLayout();
        return table;
    }

    TableCell::TableCell(TableRow* _parent, const Padding _padding) : TableElement(_parent, _padding)
    {
    }
} // namespace sage
