//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "EngineSystems.hpp"
#include "ResourceManager.hpp"
#include "slib.hpp"
#include "UserInput.hpp"

#include <cassert>
#include <format>
#include <queue>
#include <ranges>
#include <sstream>

namespace sage
{

#pragma region init

    void Window::InitLayout()
    {
        // assert(!finalized);
        if (children.empty()) return;
        const float availableWidth = rec.width - (padding.left + padding.right);
        const float availableHeight = rec.height - (padding.up + padding.down);
        const float startX = rec.x + padding.left;
        const float startY = rec.y + padding.up;

        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based heights
        for (const auto& p : children)
        {
            if (const auto table = reinterpret_cast<Table*>(p.get()); table->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += table->requestedHeight;
            }
        }

        totalRequestedPercent = std::min(totalRequestedPercent, 100.0f);
        const float remainingPercent = 100.0f - totalRequestedPercent;
        const float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each table
        float currentY = startY;
        for (const auto& p : children)
        {
            const auto table = reinterpret_cast<Table*>(p.get());
            table->parent = this;
            table->rec = rec;

            float panelHeight;
            if (table->autoSize)
            {
                panelHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
            }
            else
            {
                panelHeight = std::ceil(availableHeight * (table->requestedHeight / 100.0f));
            }

            table->rec.height = panelHeight;
            table->rec.y = currentY;
            table->rec.width = availableWidth;
            table->rec.x = startX;

            UpdateTextureDimensions();

            if (!table->children.empty()) table->InitLayout();

            currentY += panelHeight;
        }
    }

    void Table::InitLayout()
    {
        // assert(!GetWindow()->finalized);
        // Account for table padding
        const float availableHeight = rec.height - (padding.up + padding.down);
        const float startY = rec.y + padding.up;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based heights
        for (const auto& r : children)
        {
            auto row = reinterpret_cast<TableRow*>(r.get());
            if (row->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += row->requestedHeight;
            }
        }

        if (totalRequestedPercent > 100.0f)
        {
            totalRequestedPercent = 100.0f;
        }

        const float remainingPercent = 100.0f - totalRequestedPercent;
        const float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each row
        float currentY = startY;
        for (const auto& r : children)
        {
            const auto row = reinterpret_cast<TableRow*>(r.get());
            row->parent = this;
            row->rec = rec;

            float rowHeight;
            if (row->autoSize)
            {
                rowHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
            }
            else
            {
                rowHeight = std::ceil(availableHeight * (row->requestedHeight / 100.0f));
            }

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
        // assert(!GetWindow()->finalized);
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
        // assert(!GetWindow()->finalized);
        const float availableWidth = rec.width - (padding.left + padding.right);
        const float startX = rec.x + padding.left;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based widths
        for (const auto& c : children)
        {
            if (const auto cell = reinterpret_cast<TableCell*>(c.get()); cell->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += cell->requestedWidth;
            }
        }

        if (totalRequestedPercent > 100.0f)
        {
            totalRequestedPercent = 100.0f;
        }

        const float remainingPercent = 100.0f - totalRequestedPercent;
        const float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each cell
        float currentX = startX;
        for (const auto& c : children)
        {
            const auto cell = reinterpret_cast<TableCell*>(c.get());
            cell->parent = this;
            cell->rec = rec;

            float cellWidth;
            if (cell->autoSize)
            {
                cellWidth = std::ceil(availableWidth * (autoSizePercent / 100.0f));
            }
            else
            {
                cellWidth = std::ceil(availableWidth * (cell->requestedWidth / 100.0f));
            }

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
        // assert(!GetWindow()->finalized);
        if (element.has_value())
        {
            if (element.value())
            {
                element.value()->parent = this;
                element.value()->rec = rec;
                element.value()->UpdateDimensions();
            }
        }
        else
        {
            // Same as Window::InitLayout
            const float availableWidth = rec.width - (padding.left + padding.right);
            const float availableHeight = rec.height - (padding.up + padding.down);
            const float startX = rec.x + padding.left;
            const float startY = rec.y + padding.up;

            float totalRequestedPercent = 0.0f;
            int autoSizeCount = 0;

            // First pass: Calculate total of percentage-based heights
            for (const auto& p : children)
            {
                auto table = reinterpret_cast<Table*>(p.get());
                if (table->autoSize)
                {
                    autoSizeCount++;
                }
                else
                {
                    totalRequestedPercent += table->requestedHeight;
                }
            }

            totalRequestedPercent = std::min(totalRequestedPercent, 100.0f);
            const float remainingPercent = 100.0f - totalRequestedPercent;
            const float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

            // Second pass: Update each table
            float currentY = startY;
            for (const auto& p : children)
            {
                const auto table = reinterpret_cast<Table*>(p.get());
                table->parent = this;
                table->rec = rec;

                float panelHeight;
                if (table->autoSize)
                {
                    panelHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
                }
                else
                {
                    panelHeight = std::ceil(availableHeight * (table->requestedHeight / 100.0f));
                }

                table->rec.height = panelHeight;
                table->rec.y = currentY;
                table->rec.width = availableWidth;
                table->rec.x = startX;

                UpdateTextureDimensions();

                if (!table->children.empty()) table->InitLayout();

                currentY += panelHeight;
            }
        }
    }

#pragma endregion

#pragma region UIElements
    void UIElement::OnHoverStart()
    {
    }

    void UIElement::OnHoverStop()
    {
    }

    void CellElement::OnClick()
    {
        onMouseClicked.Publish();
    }

    void CellElement::HoverUpdate()
    {
    }

    void CellElement::OnDragStart()
    {
        beingDragged = true;
    }

    void CellElement::OnDrop(CellElement* receiver)
    {
        beingDragged = false;
        if (receiver && receiver->canReceiveDragDrops)
        {
            receiver->ReceiveDrop(this);
        }
    }

    void CellElement::ReceiveDrop(CellElement* droppedElement)
    {
        if (!canReceiveDragDrops) return;
    }

    void CellElement::ChangeState(std::unique_ptr<UIState> newState)
    {
        if (newState == state || stateLocked) return;

        state->Exit();
        state = std::move(newState);
        state->Enter();
    }

    void CellElement::UpdateDimensions()
    {
        tex.width = parent->GetRec().width;
        tex.height = parent->GetRec().height;
    }

    CellElement::CellElement(
        GameUIEngine* _engine,
        TableCell* _parent,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment)
        : vertAlignment(_vertAlignment),
          horiAlignment(_horiAlignment),
          parent(_parent),
          engine(_engine),
          state(std::make_unique<IdleState>(this, engine))
    {
    }

    const std::string& TextBox::GetContent() const
    {
        return content;
    }

    void TextBox::SetContent(const std::string& _content)
    {
        content = _content;
        UpdateDimensions();
    }

    Font TextBox::GetFont() const
    {
        return fontInfo.font;
    }

    void TextBox::UpdateFontScaling()
    {
        const float scaleFactor = engine->settings->GetCurrentScaleFactor();
        fontInfo.fontSize = fontInfo.baseFontSize * scaleFactor;
        fontInfo.fontSize = std::clamp(fontInfo.fontSize, FontInfo::minFontSize, FontInfo::maxFontSize);
    }

    void TextBox::UpdateDimensions()
    {
        UpdateFontScaling();
        float availableWidth = parent->GetRec().width - (parent->padding.left + parent->padding.right);
        Vector2 textSize = MeasureTextEx(fontInfo.font, content.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);

        if (fontInfo.overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {

            while (textSize.x > availableWidth && fontInfo.fontSize > FontInfo::minFontSize)
            {
                fontInfo.fontSize -= 1;
                textSize = MeasureTextEx(fontInfo.font, content.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);
            }
        }
        else if (fontInfo.overflowBehaviour == OverflowBehaviour::WORD_WRAP)
        {
            std::string wrappedText;
            std::string currentLine;
            std::istringstream words(content);
            std::string word;

            while (words >> word)
            {
                std::string testLine = currentLine;
                if (!testLine.empty()) testLine += " ";
                testLine += word;

                Vector2 lineSize =
                    MeasureTextEx(fontInfo.font, testLine.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);

                if (lineSize.x <= availableWidth)
                {
                    currentLine = testLine;
                }
                else
                {
                    if (!wrappedText.empty()) wrappedText += "\n";
                    wrappedText += currentLine;
                    currentLine = word;
                }
            }

            if (!currentLine.empty())
            {
                if (!wrappedText.empty()) wrappedText += "\n";
                wrappedText += currentLine;
            }

            content = wrappedText;
        }

        textSize = MeasureTextEx(fontInfo.font, content.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);

        float horiOffset = 0;
        float vertOffset = 0;
        float availableHeight = parent->GetRec().height - (parent->padding.up + parent->padding.down);

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (availableHeight - textSize.y) / 2;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = availableHeight - textSize.y;
        }

        if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = availableWidth - textSize.x;
        }
        else if (horiAlignment == HoriAlignment::CENTER)
        {
            horiOffset = (availableWidth - textSize.x) / 2;
        }
        else if (horiAlignment == HoriAlignment::WINDOW_CENTER)
        {
            auto window = parent->GetWindow();
            float windowAvailableWidth = window->GetRec().width - (window->padding.left + window->padding.right);
            horiOffset = (windowAvailableWidth - textSize.x) / 2;
        }

        rec = {
            parent->GetRec().x + parent->padding.left + horiOffset,
            parent->GetRec().y + parent->padding.up + vertOffset,
            textSize.x,
            textSize.y};
    }

    void TextBox::Draw2D()
    {
        BeginShaderMode(sdfShader);
        DrawTextEx(
            fontInfo.font, content.c_str(), Vector2{rec.x, rec.y}, fontInfo.fontSize, fontInfo.fontSpacing, BLACK);
        EndShaderMode();
    }

    TextBox::TextBox(
        GameUIEngine* _engine,
        TableCell* _parent,
        const FontInfo& _fontInfo,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment)
        : CellElement(_engine, _parent, _vertAlignment, _horiAlignment),
          sdfShader(ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/sdf.fs")),
          fontInfo(_fontInfo)
    {
        UpdateFontScaling();
        SetTextureFilter(GetFont().texture, TEXTURE_FILTER_BILINEAR);
    }

    void TitleBar::OnDragStart()
    {
        draggedWindow = parent->GetWindow();
        const auto [x, y] = GetMousePosition();
        dragOffset = {x - draggedWindow.value()->GetRec().x, y - draggedWindow.value()->GetRec().y};
    }

    void TitleBar::DragUpdate()
    {
        const auto mousePos = GetMousePosition();
        const auto& window = draggedWindow.value();
        const auto newPos = Vector2Subtract(mousePos, dragOffset);

        window->SetPos(newPos.x, newPos.y);
        // window->ClampToScreen();
    }

    void TitleBar::OnDrop(CellElement* droppedElement)
    {
        draggedWindow.reset();
        dragOffset = {0, 0};
    }

    TitleBar::TitleBar(GameUIEngine* _engine, TableCell* _parent, const FontInfo& _fontInfo)
        : TextBox(_engine, _parent, _fontInfo, VertAlignment::TOP, HoriAlignment::WINDOW_CENTER)
    {
        draggable = true;
        dragDelayTime = 0.0f;
        dragOffset = {0, 0};
    }

    void ImageBox::SetHoverShader()
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/ui_hover.fs");
    }

    void ImageBox::SetGrayscale()
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/grayscale.fs");
    }

    void ImageBox::RemoveShader()
    {
        shader.reset();
    }

    void ImageBox::OnIdleStart()
    {
        RemoveShader();
    }

    void ImageBox::OnHoverStart()
    {
        hoverTimer = GetTime();
        SetHoverShader();
        CellElement::OnHoverStart();
    }

    void ImageBox::OnHoverStop()
    {
        hoverTimer = 0;
        if (tooltipWindow.has_value())
        {
            tooltipWindow.value()->Remove();
            tooltipWindow.reset();
        }
        // SetGrayscale();
        RemoveShader();
        CellElement::OnHoverStop();
    }

    void ImageBox::OnDragStart()
    {
        CellElement::OnDragStart();
    }

    void ImageBox::DragDraw()
    {
        auto mousePos = GetMousePosition();
        DrawTexture(tex, mousePos.x - rec.width / 2, mousePos.y - rec.height / 2, WHITE);
    }

    void ImageBox::OnDrop(CellElement* droppedElement)
    {
        CellElement::OnDrop(droppedElement);
    }

    Dimensions ImageBox::calculateAvailableSpace() const
    {
        return {
            parent->GetRec().width - (parent->padding.left + parent->padding.right),
            parent->GetRec().height - (parent->padding.up + parent->padding.down)};
    }

    float ImageBox::calculateAspectRatio() const
    {
        return static_cast<float>(tex.width) / tex.height;
    }

    Dimensions ImageBox::calculateInitialDimensions(const Dimensions& space) const
    {
        const float originalRatio = calculateAspectRatio();

        if (originalRatio > 1.0f)
        { // Wider than tall
            return {space.width, space.width / originalRatio};
        }
        else
        { // Taller than wide
            return {space.height * originalRatio, space.height};
        }
    }

    Vector2 ImageBox::calculateAlignmentOffset(const Dimensions& dimensions, const Dimensions& space) const
    {
        float vertOffset = 0;
        float horiOffset = 0;

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (space.height - dimensions.height) / 2;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = space.height - dimensions.height;
        }

        if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = space.width - dimensions.width;
        }
        else if (horiAlignment == HoriAlignment::CENTER)
        {
            horiOffset = (space.width - dimensions.width) / 2;
        }

        return {horiOffset, vertOffset};
    }

    void ImageBox::updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space)
    {
        rec = Rectangle{
            parent->GetRec().x + parent->padding.left + offset.x,
            parent->GetRec().y + parent->padding.up + offset.y,
            dimensions.width,
            dimensions.height};

        tex.width = dimensions.width;
        tex.height = dimensions.height;
    }

    void ImageBox::shrinkRowToFit() const
    {
        const auto& row = parent->parent->children;

        // First pass: calculate the required scale factor for each cell
        float minScaleFactor = 1.0f;

        for (const auto& cell : row)
        {
            if (!cell->element.has_value()) continue;
            const auto* imageBox = dynamic_cast<ImageBox*>(cell->element.value().get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            const auto dimensions = imageBox->calculateInitialDimensions(space);

            // Calculate scale factor needed for this image
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            // Keep track of the smallest scale factor needed
            minScaleFactor = std::min(minScaleFactor, scaleFactor);
        }

        // Second pass: apply the minimum scale factor to all cells
        for (const auto& cell : row)
        {
            if (!cell->element.has_value()) continue;
            auto* imageBox = dynamic_cast<ImageBox*>(cell->element.value().get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            // Apply uniform scaling
            dimensions.width *= minScaleFactor;
            dimensions.height *= minScaleFactor;

            auto offset = imageBox->calculateAlignmentOffset(dimensions, space);
            imageBox->updateRectangle(dimensions, offset, space);
        }
    }

    void ImageBox::shrinkColToFit() const
    {
        const auto* tableCell = parent;
        const auto* tableRow = tableCell->parent;
        const auto* table = tableRow->parent;
        const auto& allRows = table->children;

        const size_t myColIndex = findMyColumnIndex();

        // First pass: calculate the required scale factor for each cell in this column
        float minScaleFactor = 1.0f;

        for (const auto& row : allRows)
        {
            // Skip if the row doesn't have enough cells
            if (row->children.size() <= myColIndex) continue;

            const auto* cellInColumn = row->children[myColIndex].get();
            if (!cellInColumn->element.has_value()) continue;
            const auto* imageBox = dynamic_cast<ImageBox*>(cellInColumn->element.value().get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            const auto dimensions = imageBox->calculateInitialDimensions(space);

            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            minScaleFactor = std::min(minScaleFactor, scaleFactor);
        }

        // Second pass: apply the minimum scale factor to all cells in this column
        for (const auto& row : allRows)
        {
            if (row->children.size() <= myColIndex) continue;

            const auto* cellInColumn = row->children[myColIndex].get();
            if (!cellInColumn->element.has_value()) continue;
            auto* imageBox = dynamic_cast<ImageBox*>(cellInColumn->element.value().get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            dimensions.width *= minScaleFactor;
            dimensions.height *= minScaleFactor;

            auto offset = imageBox->calculateAlignmentOffset(dimensions, space);
            imageBox->updateRectangle(dimensions, offset, space);
        }
    }

    // Helper function to find the column index of the current ImageBox
    size_t ImageBox::findMyColumnIndex() const
    {
        const auto* myCell = parent;
        const auto* myRow = myCell->parent;

        // Find this cell's index in its row
        size_t colIndex = 0;
        for (size_t i = 0; i < myRow->children.size(); ++i)
        {
            if (myRow->children[i].get() == myCell)
            {
                colIndex = i;
                break;
            }
        }
        return colIndex;
    }

    Dimensions ImageBox::handleOverflow(const Dimensions& dimensions, const Dimensions& space) const
    {
        if (overflowBehaviour == OverflowBehaviour::ALLOW_OVERFLOW) return dimensions;
        if (dimensions.width <= space.width && dimensions.height <= space.height)
        {
            return dimensions;
        }

        if (overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {
            const float widthRatio = space.width / dimensions.width;
            const float heightRatio = space.height / dimensions.height;
            const float scaleFactor = std::min(widthRatio, heightRatio);

            return {dimensions.width * scaleFactor, dimensions.height * scaleFactor};
        }

        return dimensions; // Return original dimensions if no scaling needed
    }

    void ImageBox::UpdateDimensions()
    {
        if (overflowBehaviour == OverflowBehaviour::SHRINK_ROW_TO_FIT)
        {
            shrinkRowToFit();
            return;
        }
        if (overflowBehaviour == OverflowBehaviour::SHRINK_COL_TO_FIT)
        {
            shrinkColToFit();
            return;
        }
        auto space = calculateAvailableSpace();

        auto dimensions = calculateInitialDimensions(space);
        dimensions = handleOverflow(dimensions, space);
        auto offset = calculateAlignmentOffset(dimensions, space);

        updateRectangle(dimensions, offset, space);
    }

    void ImageBox::Draw2D()
    {
        if (beingDragged) return;
        if (shader.has_value())
        {
            BeginShaderMode(shader.value());
        }
        DrawTexture(tex, rec.x, rec.y, WHITE);
        if (shader.has_value())
        {
            EndShaderMode();
        }
    }

    ImageBox::ImageBox(
        GameUIEngine* _engine,
        TableCell* _parent,
        const Texture& _tex,
        OverflowBehaviour _behaviour,
        VertAlignment _vertAlignment,
        HoriAlignment _horiAlignment)
        : CellElement(_engine, _parent, _vertAlignment, _horiAlignment), overflowBehaviour(_behaviour)
    {
        tex = _tex;
    }

    ImageBox::ImageBox(
        GameUIEngine* _engine,
        TableCell* _parent,
        OverflowBehaviour _behaviour,
        VertAlignment _vertAlignment,
        HoriAlignment _horiAlignment)
        : CellElement(_engine, _parent, _vertAlignment, _horiAlignment), overflowBehaviour(_behaviour)
    {
    }

    void GameWindowButton::OnClick()
    {
        toOpen->ToggleHide();
    }

    GameWindowButton::GameWindowButton(
        GameUIEngine* _engine, TableCell* _parent, const Texture& _tex, Window* _toOpen)
        : ImageBox(_engine, _parent, _tex), toOpen(_toOpen)
    {
    }

    void CloseButton::OnClick()
    {
        if (closeDeletesWindow)
        {
            parent->GetWindow()->Remove();
        }
        else
        {
            parent->GetWindow()->Hide();
        }
    }

    CloseButton::CloseButton(
        GameUIEngine* _engine, TableCell* _parent, const Texture& _tex, const bool _closeDeletesWindow)
        : ImageBox(
              _engine, _parent, _tex, OverflowBehaviour::SHRINK_TO_FIT, VertAlignment::TOP, HoriAlignment::RIGHT),
          closeDeletesWindow(_closeDeletesWindow)
    {
    }

    void WindowDocked::ScaleContents(Settings* _settings)
    {
        Reset();
        setAlignment();

        rec = {
            settings->ScaleValueWidth(rec.x),
            settings->ScaleValueHeight(rec.y),
            settings->ScaleValueWidth(rec.width),
            settings->ScaleValueHeight(rec.height)};

        padding = {
            settings->ScaleValueHeight(padding.up),
            settings->ScaleValueHeight(padding.down),
            settings->ScaleValueWidth(padding.left),
            settings->ScaleValueWidth(padding.right)};

        UpdateTextureDimensions();

        for (auto& child : children)
        {
            child->ScaleContents(settings);
        }
    }

    void WindowDocked::setAlignment()
    {

        float xOffset = 0;
        float yOffset = 0;

        // Calculate horizontal position
        switch (horiAlignment)
        {
        case HoriAlignment::LEFT:
            xOffset = 0;
            break;

        case HoriAlignment::CENTER:
            xOffset = (Settings::TARGET_SCREEN_WIDTH - rec.width) / 2;
            break;

        case HoriAlignment::RIGHT:
            xOffset = Settings::TARGET_SCREEN_WIDTH - rec.width;
            break;
        default:;
        }

        // Calculate vertical position
        switch (vertAlignment)
        {
        case VertAlignment::TOP:
            yOffset = 0;
            break;

        case VertAlignment::MIDDLE:
            yOffset = (Settings::TARGET_SCREEN_HEIGHT - rec.height) / 2;
            break;

        case VertAlignment::BOTTOM:
            yOffset = Settings::TARGET_SCREEN_HEIGHT - rec.height;
            break;
        }

        rec.x = xOffset + baseXOffset;
        rec.y = yOffset + baseYOffset;
    }

    WindowDocked::WindowDocked(
        Settings* _settings,
        const float _xOffset,
        const float _yOffset,
        const float _width,
        const float _height,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment,
        const Padding _padding)
        : Window(_settings, _padding),
          baseXOffset(_xOffset),
          baseYOffset(_yOffset),
          vertAlignment(_vertAlignment),
          horiAlignment(_horiAlignment)
    {
        rec.width = _width;
        rec.height = _height;
        setAlignment();
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
    }

    WindowDocked::WindowDocked(
        Settings* _settings,
        Texture _tex,
        const TextureStretchMode _textureStretchMode,
        const float _xOffset,
        const float _yOffset,
        const float _width,
        const float _height,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment,
        const Padding _padding)
        : Window(_settings, _padding),
          baseXOffset(_xOffset),
          baseYOffset(_yOffset),
          vertAlignment(_vertAlignment),
          horiAlignment(_horiAlignment)
    {
        tex = _tex;
        textureStretchMode = _textureStretchMode;
        rec.width = _width;
        rec.height = _height;
        setAlignment();
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
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
            if (PointInsideRect(rec, mousePos))
            {
                return element.value().get();
            }
            return nullptr;
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

    void TableElement::Update()
    {
        assert(!(element.has_value() && !children.empty()));
        if (element.has_value())
        {
            element.value()->state->Update();
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
            element.value()->ChangeState(
                std::make_unique<IdleState>(element.value().get(), element.value()->engine));
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

    void Window::SetPos(const float x, const float y)
    {
        const auto old = Vector2{rec.x, rec.y};
        rec = {x, y, rec.width, rec.height};
        ClampToScreen();
        const auto diff = Vector2Subtract(Vector2{rec.x, rec.y}, Vector2{old.x, old.y});

        std::queue<TableElement*> elementsToProcess;

        for (auto& panel : children)
        {
            elementsToProcess.push(panel.get());
        }

        while (!elementsToProcess.empty())
        {
            const auto current = elementsToProcess.front();
            elementsToProcess.pop();

            current->rec.x += diff.x;
            current->rec.y += diff.y;

            if (current->element.has_value())
            {
                current->element.value()->UpdateDimensions();
                continue; // Skip adding children since this is an end node
            }

            for (auto& child : current->children)
            {
                elementsToProcess.push(child.get());
            }
        }
    }

    void Window::ToggleHide()
    {
        hidden = !hidden;
        if (hidden)
        {
            onHide.Publish();
        }
        else
        {
            onShow.Publish();
        }
    }

    void Window::Show()
    {
        hidden = false;
        onShow.Publish();
    }

    void Window::Hide()
    {
        hidden = true;
        onHide.Publish();
    }

    bool Window::IsHidden() const
    {
        return hidden;
    }

    bool Window::IsMarkedForRemoval() const
    {
        return markForRemoval;
    }

    void Window::Remove()
    {
        hidden = true;
        markForRemoval = true;
        onHide.Publish();
    }

    void Window::FinalizeLayout()
    {
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
        for (const auto& child : children)
        {
            child->FinalizeLayout();
        }
        ScaleContents(settings);
    }

    void Window::OnWindowUpdate(Vector2 prev, Vector2 current)
    {
        ScaleContents(settings);
    }

    void Window::ScaleContents(Settings* _settings)
    {
        // assert(finalized);
        if (markForRemoval) return;

        Reset();

        rec = {
            settings->ScaleValueWidth(rec.x),
            settings->ScaleValueHeight(rec.y),
            settings->ScaleValueWidth(rec.width),
            settings->ScaleValueHeight(rec.height)};

        padding = {
            settings->ScaleValueHeight(padding.up),
            settings->ScaleValueHeight(padding.down),
            settings->ScaleValueWidth(padding.left),
            settings->ScaleValueWidth(padding.right)};

        UpdateTextureDimensions();

        for (const auto& child : children)
        {
            child->ScaleContents(settings);
        }
    }

    void Window::ClampToScreen()
    {
        const auto viewport = settings->GetViewPort();
        if (rec.x + rec.width > viewport.x)
        {
            rec.x = viewport.x - rec.width;
        }
        if (rec.y + rec.height > viewport.y + rec.height / 2)
        {
            rec.y = viewport.y - rec.height / 2;
        }
        if (rec.x < 0)
        {
            rec.x = 0;
        }
        if (rec.y < 0)
        {
            rec.y = 0;
        }
    }

    void Window::OnHoverStart()
    {
        UIElement::OnHoverStart();
    }

    Window::~Window()
    {
        windowUpdateSub.UnSubscribe();
    }

    Window::Window(Settings* _settings, const Padding _padding)
        : TableElement(nullptr, _padding), settings(_settings)
    {
    }

    Window::Window(
        Settings* _settings,
        const Texture& _tex,
        const TextureStretchMode _stretchMode,
        const float x,
        const float y,
        const float width,
        const float height,
        const Padding _padding)
        : TableElement(nullptr, x, y, width, height, _padding),

          settings(_settings)
    {
        tex = _tex;
        textureStretchMode = _stretchMode;
    }

    Window::Window(
        Settings* _settings,
        const float x,
        const float y,
        const float width,
        const float height,
        const Padding _padding)
        : TableElement(nullptr, x, y, width, height, _padding), settings(_settings)
    {
    }

    void TooltipWindow::Remove()
    {
        hidden = true;
        markForRemoval = true;
    }

    void TooltipWindow::ScaleContents(Settings* _settings)
    {
        // Tooltip's original position is scaled to the screen already
        if (markForRemoval) return;

        rec = {
            rec.x,
            rec.y,
            settings->ScaleValueMaintainRatio(rec.width),
            settings->ScaleValueMaintainRatio(rec.height)};

        padding = {
            settings->ScaleValueMaintainRatio(padding.up),
            settings->ScaleValueMaintainRatio(padding.down),
            settings->ScaleValueMaintainRatio(padding.left),
            settings->ScaleValueMaintainRatio(padding.right)};

        UpdateTextureDimensions();

        // Update children?
    }

    TooltipWindow::~TooltipWindow()
    {
        if (parentWindowHideSub.IsActive())
        {
            parentWindowHideSub.UnSubscribe();
        }
    }

    TooltipWindow::TooltipWindow(
        Settings* _settings,
        Window* parentWindow,
        const Texture& _tex,
        TextureStretchMode _stretchMode,
        const float x,
        const float y,
        const float width,
        const float height,
        const Padding _padding)
        : Window(_settings, x, y, width, height, _padding)
    {
        if (parentWindow)
        {
            parentWindowHideSub = parentWindow->onHide.Subscribe([this]() { Remove(); });
        }
        tex = _tex;
        textureStretchMode = _stretchMode;
    }

    TableGrid* Window::CreateTableGrid(const int rows, const int cols, const float cellSpacing, Padding _padding)
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

    Table* Window::CreateTable(Padding _padding)
    {
        children.push_back(std::make_unique<Table>(this, _padding));
        const auto& table = dynamic_cast<Table*>(children.back().get());
        InitLayout();
        return table;
    }

    Table* Window::CreateTable(const float _requestedHeight, const Padding _padding)
    {
        const auto table = CreateTable(_padding);
        table->autoSize = false;
        table->requestedHeight = _requestedHeight;
        InitLayout();
        return table;
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

    bool ErrorMessage::Finished() const
    {
        return GetTime() > initialTime + totalDisplayTime;
    }

    void ErrorMessage::Draw2D() const
    {
        const auto currentTime = GetTime();

        if (currentTime > initialTime + totalDisplayTime)
        {
            return;
        }

        const auto elapsedTime = currentTime - initialTime;
        unsigned char a = 255;

        if (elapsedTime > (totalDisplayTime - fadeOut))
        {
            const float fadeProgress = (elapsedTime - (totalDisplayTime - fadeOut)) / fadeOut;
            a = static_cast<unsigned char>((1.0f - fadeProgress) * 255);
        }

        const auto [width, height] = settings->GetViewPort();
        auto col = RAYWHITE;
        col.a = a;

        const auto scaledFontSize = settings->ScaleValueMaintainRatio(24);
        const auto textSize = MeasureText(msg.c_str(), scaledFontSize);

        DrawTextEx(
            font, msg.c_str(), Vector2{(width - textSize) / 2, height / 4}, scaledFontSize, fontSpacing, col);
    }

    ErrorMessage::ErrorMessage(Settings* _settings, std::string _msg)
        : settings(_settings),
          font(
              ResourceManager::GetInstance().FontLoad(
                  "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf")),
          fontSpacing(1.5),
          msg(std::move(_msg)),
          initialTime(GetTime())
    {
    }

#pragma endregion

#pragma region UIStates

    void IdleState::Enter()
    {
        element->OnIdleStart();
    }

    void IdleState::Update()
    {
        auto mousePos = GetMousePosition();
        if (PointInsideRect(element->parent->GetRec(), mousePos))
        {
            element->ChangeState(std::make_unique<HoverState>(element, engine));
        }
    }

    void IdleState::Exit()
    {
        element->OnIdleStop();
    }

    IdleState::IdleState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void HoverState::Enter()
    {
        element->OnHoverStart();
    }

    void HoverState::Exit()
    {
        // if we swap to predrag then this will be called...
        element->OnHoverStop();
    }

    void HoverState::Update()
    {
        engine->cursor->DisableContextSwitching();
        engine->cursor->Disable();

        auto mousePos = GetMousePosition();
        if (!PointInsideRect(element->parent->GetRec(), mousePos))
        {
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            element->OnClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable)
        {
            element->ChangeState(std::make_unique<DragDelayState>(element, engine));
            return;
        }

        element->HoverUpdate();
    }

    HoverState::HoverState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DragDelayState::Enter()
    {
        element->OnHoverStart();
        dragTimer.SetMaxTime(element->dragDelayTime);
        dragTimer.SetAutoFinish(false);
    }

    void DragDelayState::Exit()
    {
        engine->hoveredDraggableCellElement.reset();
        element->OnHoverStop();
    }

    void DragDelayState::Update()
    {
        dragTimer.Update(GetFrameTime());
        if (!engine->ObjectBeingDragged() && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (dragTimer.HasExceededMaxTime())
            {
                element->ChangeState(std::make_unique<DragState>(element, engine));
                return;
            }
            if (!dragTimer.IsRunning())
            {
                engine->hoveredDraggableCellElement = element;
                dragTimer.Start();
            }
            if (engine->hoveredDraggableCellElement.has_value() &&
                engine->hoveredDraggableCellElement.value() != element)
            {
                element->ChangeState(std::make_unique<IdleState>(element, engine));
            }
        }
        else
        {
            element->OnClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
        }
    }

    DragDelayState::DragDelayState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DragState::Enter()
    {
        engine->draggedObject = element;
        element->OnDragStart();
    }

    void DragState::Exit()
    {
        auto cell = engine->GetCellUnderCursor();
        element->OnDrop(cell);
        engine->draggedObject.reset();
    }

    void DragState::Update()
    {
        // Determine if object is still being dragged
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            // Drop item
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }
        element->DragUpdate(); // Update drag
    }

    void DragState::Draw()
    {
        element->DragDraw();
    }

    DragState::DragState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    UIState::UIState(CellElement* _element, GameUIEngine* _engine) : element(_element), engine(_engine)
    {
    }
#pragma endregion

#pragma region GameUIEngine
    void GameUIEngine::pruneWindows()
    {
        if (tooltipWindow && tooltipWindow->IsMarkedForRemoval())
        {
            tooltipWindow.reset();
        }

        windows.erase(
            std::ranges::remove_if(windows, [](const auto& window) { return window->IsMarkedForRemoval(); })
                .begin(),
            windows.end());
    }

    void GameUIEngine::CreateErrorMessage(const std::string& msg)
    {
        errorMessage.emplace(settings, msg);
    }

    TooltipWindow* GameUIEngine::CreateTooltipWindow(std::unique_ptr<TooltipWindow> _tooltipWindow)
    {
        tooltipWindow = std::move(_tooltipWindow);
        tooltipWindow->windowUpdateSub = userInput->onWindowUpdate.Subscribe(
            [this](Vector2 prev, Vector2 current) { tooltipWindow->OnWindowUpdate(prev, current); });

        tooltipWindow->InitLayout();
        // FinalizeLayout called externally
        return tooltipWindow.get();
    }

    Window* GameUIEngine::CreateWindow(std::unique_ptr<Window> _window)
    {
        windows.push_back(std::move(_window));
        auto* window = windows.back().get();
        window->windowUpdateSub = userInput->onWindowUpdate.Subscribe(
            [window](Vector2 prev, Vector2 current) { window->OnWindowUpdate(prev, current); });
        window->InitLayout();
        return window;
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(std::unique_ptr<WindowDocked> _windowDocked)
    {
        windows.push_back(std::move(_windowDocked));
        auto* window = dynamic_cast<WindowDocked*>(windows.back().get());
        window->windowUpdateSub = userInput->onWindowUpdate.Subscribe(
            [window](Vector2 prev, Vector2 current) { window->OnWindowUpdate(prev, current); });
        window->InitLayout();
        return window;
    }

    bool GameUIEngine::ObjectBeingDragged() const
    {
        return draggedObject.has_value();
    }

    Window* GameUIEngine::GetWindowCollision(const Window* toCheck) const
    {
        for (auto& window : windows)
        {
            if (window.get() == toCheck || window->IsHidden()) continue;
            if (CheckCollisionRecs(window->rec, toCheck->rec))
            {
                return window.get();
            }
        }
        return nullptr;
    }

    CellElement* GameUIEngine::GetCellUnderCursor() const
    {
        Window* windowUnderCursor = nullptr;
        const auto mousePos = GetMousePosition();
        for (auto& window : windows)
        {
            if (window->IsHidden()) continue;
            if (PointInsideRect(window->rec, mousePos) && mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                windowUnderCursor = window.get();
            }
        }
        if (windowUnderCursor == nullptr) return nullptr;
        for (const auto& child : windowUnderCursor->children)
        {

            if (auto childElement = child->GetCellUnderCursor())
            {
                return childElement;
            }
        }
        return nullptr;
    }

    Rectangle GameUIEngine::GetOverlap(Rectangle rec1, Rectangle rec2)
    {
        float x1 = std::max(rec1.x, rec2.x);
        float y1 = std::max(rec1.y, rec2.y);
        float x2 = std::min(rec1.x + rec1.width, rec2.x + rec2.width);
        float y2 = std::min(rec1.y + rec1.height, rec2.y + rec2.height);

        if (x1 < x2 && y1 < y2)
        {
            Rectangle overlap;
            overlap.x = x1;
            overlap.y = y1;
            overlap.width = x2 - x1;
            overlap.height = y2 - y1;
            return overlap;
        }
        return Rectangle{0, 0, 0, 0};
    }

    void GameUIEngine::BringClickedWindowToFront(Window* clicked)
    {
        const auto it = std::ranges::find_if(
            windows, [clicked](const std::unique_ptr<Window>& ptr) { return ptr.get() == clicked; });
        std::rotate(it, it + 1, windows.end());
    }

    void GameUIEngine::DrawDebug2D() const
    {
        for (const auto& window : windows)
        {
            if (window->IsHidden()) continue;
            window->DrawDebug2D();
        }
    }

    void GameUIEngine::Draw2D() const
    {
        for (const auto& window : windows)
        {
            if (window->IsHidden()) continue;
            window->Draw2D();
        }

        if (tooltipWindow && !tooltipWindow->hidden)
        {
            tooltipWindow->Draw2D();
        }

        if (draggedObject.has_value())
        {
            draggedObject.value()->state->Draw();
        }

        if (errorMessage.has_value())
        {
            errorMessage->Draw2D();
        }
    }

    /**
     *
     * @return Whether the window region is not obscured by another window
     */
    bool GameUIEngine::mouseInNonObscuredWindowRegion(Window* window, Vector2 mousePos) const
    {
        if (auto collision = GetWindowCollision(window))
        {
            // check if window is lower
            auto windowIt = std::ranges::find_if(
                windows, [window](const std::unique_ptr<Window>& ptr) { return ptr.get() == window; });

            const auto colIt = std::ranges::find_if(
                windows, [collision](const std::unique_ptr<Window>& ptr) { return ptr.get() == collision; });

            const auto windowDist = std::distance(windows.begin(), windowIt);

            if (auto colDist = std::distance(windows.begin(), colIt); windowDist < colDist)
            {
                auto rec = GetOverlap(window->rec, collision->rec);
                if (PointInsideRect(rec, mousePos))
                {
                    // this part of the window is being obscured by another
                    return false;
                }
            }
        }
        return true;
    }

    void GameUIEngine::processWindows()
    {
        const auto mousePos = GetMousePosition();

        for (auto& window : windows)
        {
            if (!window || window->IsMarkedForRemoval() || window->IsHidden()) continue;

            if (!PointInsideRect(window->rec, mousePos) || !mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                window->OnHoverStop();
                continue;
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                BringClickedWindowToFront(window.get());
            }

            cursor->Disable();
            cursor->DisableContextSwitching();

            window->OnHoverStart(); // TODO: Need to check if it was already being hovered?
            window->Update();
        }
    }

    void GameUIEngine::Update()
    {
        if (draggedObject.has_value())
        {
            draggedObject.value()->state->Update();
        }
        else
        {
            cursor->Enable();
            cursor->EnableContextSwitching();
            processWindows();
            pruneWindows();
        }

        if (errorMessage.has_value() && errorMessage->Finished())
        {
            errorMessage.reset();
        }
    }

    GameUIEngine::GameUIEngine(entt::registry* _registry, const EngineSystems* _sys)
        : registry(_registry),
          userInput(_sys->userInput.get()),
          cursor(_sys->cursor.get()),
          settings(_sys->settings)
    {
    }
#pragma endregion
} // namespace sage