//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

#include "Camera.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "GameObjectFactory.hpp"
#include "GameUiFactory.hpp"
#include "ResourceManager.hpp"
#include "slib.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "UserInput.hpp"

#include <cassert>
#include <format>
#include <ranges>
#include <sstream>

namespace sage
{

#pragma region init

    void TableRowGrid::InitLayout()
    {
        // If no children (cells), return early
        if (children.empty()) return;

        // 1. Get number of columns (number of cells in this row)
        unsigned int cols = children.size();

        // 2. Calculate available space
        float availableWidth = rec.width - (padding.left + padding.right);
        float availableHeight = rec.height - (padding.up + padding.down);

        // 3. Find the maximum power-of-two cell size that fits
        int maxCellSize =
            1 << static_cast<int>(std::floor(std::log2(std::min(availableWidth / cols, availableHeight))));

        // 4. Calculate actual cell size with spacing
        float cellSize =
            (std::min(availableWidth / cols, availableHeight) - cellSpacing) / maxCellSize * maxCellSize;

        // 5. Calculate total grid width
        float gridWidth = cellSize * cols + cellSpacing * (cols - 1); // Account for spacing between columns

        // 6. Calculate starting position to center the row
        float startX = rec.x + (availableWidth - gridWidth) / 2.0f;
        float startY = rec.y + (availableHeight - cellSize) / 2.0f;

        // 7. Apply dimensions to cells
        float currentX = startX;
        for (const auto& cell : children)
        {
            cell->SetPos(currentX, startY);
            cell->SetDimensions(cellSize, cellSize);
            currentX += cellSize + cellSpacing; // Add spacing after each cell

            // Update dimensions of cell's children if any
            if (cell->children)
            {
                cell->children->UpdateDimensions();
            }
        }
    }

    void TableGrid::InitLayout()
    {
        // assert(!GetWindow()->finalized);
        if (children.empty()) return;
        // 1. Get number of columns
        unsigned int cols = children[0]->children.size();

        // 2. Calculate available space
        float availableWidth = rec.width - (padding.left + padding.right);
        float availableHeight = rec.height - (padding.up + padding.down);

        // 3. Find the maximum power-of-two cell size that fits
        int maxCellSize = 1 << static_cast<int>(std::floor(
                              std::log2(std::min(availableWidth / cols, availableHeight / children.size()))));

        // 4. Calculate actual cell size with spacing
        float cellSize = (std::min(availableWidth / cols, availableHeight / children.size()) - cellSpacing) /
                         maxCellSize * maxCellSize;

        // 5. Calculate total grid dimensions
        float gridWidth = cellSize * cols + cellSpacing * (cols - 1); // Account for spacing between columns
        float gridHeight =
            cellSize * children.size() + cellSpacing * (children.size() - 1); // Account for spacing between rows

        // 6. Calculate starting position to center the grid
        float startX = rec.x + (availableWidth - gridWidth) / 2.0f;
        float startY = rec.y + (availableHeight - gridHeight) / 2.0f;

        // 7. Apply dimensions to rows and cells
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

        for (auto& row : children)
        {
            for (auto& cell : row->children)
            {
                if (cell->children)
                {
                    cell->children->UpdateDimensions();
                }
            }
        }
    }

    void TableRow::InitLayout()
    {
        // assert(!GetWindow()->finalized);
        // Account for row padding
        float availableWidth = rec.width - (padding.left + padding.right);
        float startX = rec.x + padding.left;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based widths
        for (const auto& cell : children)
        {
            if (cell->autoSize)
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

        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each cell
        float currentX = startX;
        for (const auto& cell : children)
        {
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

    // In Window class - always stacks panels vertically
    void Window::InitLayout()
    {
        // assert(!finalized);
        if (children.empty()) return;
        float availableWidth = rec.width - (padding.left + padding.right);
        float availableHeight = rec.height - (padding.up + padding.down);
        float startX = rec.x + padding.left;
        float startY = rec.y + padding.up;

        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based heights
        for (const auto& panel : children)
        {
            if (panel->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += panel->requestedHeight;
            }
        }

        totalRequestedPercent = std::min(totalRequestedPercent, 100.0f);
        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each panel
        float currentY = startY;
        for (const auto& panel : children)
        {
            panel->parent = this;
            panel->rec = rec;

            float panelHeight;
            if (panel->autoSize)
            {
                panelHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
            }
            else
            {
                panelHeight = std::ceil(availableHeight * (panel->requestedHeight / 100.0f));
            }

            panel->rec.height = panelHeight;
            panel->rec.y = currentY;
            panel->rec.width = availableWidth;
            panel->rec.x = startX;

            UpdateTextureDimensions();

            if (!panel->children.empty()) panel->InitLayout();

            currentY += panelHeight;
        }
    }

    void Panel::InitLayout()
    {
        if (children.empty()) return;

        float availableWidth = rec.width - (padding.left + padding.right);
        float availableHeight = rec.height - (padding.up + padding.down);
        float startX = rec.x + padding.left;
        float startY = rec.y + padding.up;

        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based widths
        for (const auto& table : children)
        {
            if (table->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += table->requestedWidth;
            }
        }

        totalRequestedPercent = std::min(totalRequestedPercent, 100.0f);
        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each table
        float currentX = startX;
        for (const auto& table : children)
        {
            table->parent = this;
            table->rec = rec;

            float tableWidth;
            if (table->autoSize)
            {
                tableWidth = std::ceil(availableWidth * (autoSizePercent / 100.0f));
            }
            else
            {
                tableWidth = std::ceil(availableWidth * (table->requestedWidth / 100.0f));
            }

            table->rec.width = tableWidth;
            table->rec.x = currentX;
            table->rec.height = availableHeight;
            table->rec.y = startY;

            UpdateTextureDimensions();

            if (!table->children.empty()) table->InitLayout();

            currentX += tableWidth;
        }
    }

    void Table::InitLayout()
    {
        // assert(!GetWindow()->finalized);
        // Account for table padding
        float availableHeight = rec.height - (padding.up + padding.down);
        float startY = rec.y + padding.up;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based heights
        for (const auto& row : children)
        {
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

        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each row
        float currentY = startY;
        for (const auto& row : children)
        {
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
        GameUIEngine* _engine, TableCell* _parent, VertAlignment _vertAlignment, HoriAlignment _horiAlignment)
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
        float scaleFactor = engine->sys->settings->GetCurrentScaleFactor();
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
        else if (fontInfo.overflowBehaviour == OverflowBehaviour::EXTEND_WINDOW)
        {
            float availableHeight = parent->GetRec().height - (parent->padding.up + parent->padding.down);
            if (textSize.x > availableWidth)
            {
                parent->GetWindow()->SetDimensions(
                    textSize.x + parent->padding.left + parent->padding.right,
                    parent->GetWindow()->GetRec().height);
            }
            if (textSize.y > availableHeight)
            {
                parent->GetWindow()->SetDimensions(
                    parent->GetWindow()->GetRec().width, textSize.y + parent->padding.up + parent->padding.down);
            }
            parent->UpdateTextureDimensions();
            // N.B.: Must call the window's "InitLayout" function for these changes to proliferate. (Calling it
            // here will cause a stack overflow).
        }
        else if (fontInfo.overflowBehaviour == OverflowBehaviour::WORD_WRAP)
        {
            std::string wrappedText;
            std::string currentLine;
            std::istringstream words(content);
            std::string word;

            // Process each word
            while (words >> word)
            {
                // Create temporary line with new word
                std::string testLine = currentLine;
                if (!testLine.empty()) testLine += " "; // Add space between words
                testLine += word;

                // Measure the line with the new word
                Vector2 lineSize =
                    MeasureTextEx(fontInfo.font, testLine.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);

                if (lineSize.x <= availableWidth)
                {
                    // Word fits, add it to current line
                    currentLine = testLine;
                }
                else
                {
                    // Word doesn't fit, start new line
                    if (!wrappedText.empty()) wrappedText += "\n";
                    wrappedText += currentLine;
                    currentLine = word;
                }
            }

            // Add the last line
            if (!currentLine.empty())
            {
                if (!wrappedText.empty()) wrappedText += "\n";
                wrappedText += currentLine;
            }

            // Update the content with wrapped text
            content = wrappedText;
        }

        // Measure final text size
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
        FontInfo _fontInfo,
        VertAlignment _vertAlignment,
        HoriAlignment _horiAlignment)
        : CellElement(_engine, _parent, _vertAlignment, _horiAlignment),
          sdfShader(ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/sdf.fs")),
          fontInfo(_fontInfo)
    {
        UpdateFontScaling();
        SetTextureFilter(GetFont().texture, TEXTURE_FILTER_BILINEAR);
    }

    void DialogOption::OnHoverStart()
    {
        drawHighlight = true;
        TextBox::OnHoverStart();
    }

    void DialogOption::OnHoverStop()
    {
        drawHighlight = false;
        TextBox::OnHoverStop();
    }

    void DialogOption::Draw2D()
    {
        TextBox::Draw2D();
        if (drawHighlight)
        {
            float offset = 10 * parent->GetWindow()->settings->GetCurrentScaleFactor();
            DrawRectangleLines(
                rec.x - offset, rec.y - offset, rec.width + offset * 2, rec.height + offset * 2, BLACK);
        }
    }

    void DialogOption::OnClick()
    {
        auto* conversation = option->parent->parent;
        conversation->SelectOption(option);
    }

    DialogOption::DialogOption(
        GameUIEngine* _engine,
        TableCell* _parent,
        dialog::Option* _option,
        unsigned int _index,
        const FontInfo& _fontInfo,
        VertAlignment _vertAlignment,
        HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment), option(_option), index(_index)
    {
        content = std::format("{}: {}", _index, option->description);
    }

    void TitleBar::OnDragStart()
    {
        draggedWindow = parent->GetWindow();
        auto mousePos = GetMousePosition();
        dragOffset = {
            mousePos.x - draggedWindow.value()->GetRec().x, mousePos.y - draggedWindow.value()->GetRec().y};
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

    void CharacterStatText::RetrieveInfo()
    {
        auto& combatable =
            engine->registry->get<CombatableActor>(engine->sys->controllableActorSystem->GetSelectedActor());
        if (statisticType == StatisticType::NAME)
        {
            auto& renderable =
                engine->registry->get<Renderable>(engine->sys->controllableActorSystem->GetSelectedActor());
            SetContent(std::format("{}", renderable.GetVanityName()));
        }
        else if (statisticType == StatisticType::STRENGTH)
        {
            SetContent(std::format("Strength: {}", combatable.baseStatistics.strength));
        }
        else if (statisticType == StatisticType::AGILITY)
        {
            SetContent(std::format("Agility: {}", combatable.baseStatistics.agility));
        }
        else if (statisticType == StatisticType::INTELLIGENCE)
        {
            SetContent(std::format("Intelligence: {}", combatable.baseStatistics.intelligence));
        }
        else if (statisticType == StatisticType::CONSTITUTION)
        {
            SetContent(std::format("Constitution: {}", combatable.baseStatistics.constitution));
        }
        else if (statisticType == StatisticType::WITS)
        {
            SetContent(std::format("Wits: {}", combatable.baseStatistics.wits));
        }
        else if (statisticType == StatisticType::MEMORY)
        {
            SetContent(std::format("Memory: {}", combatable.baseStatistics.memory));
        }
    }

    CharacterStatText::CharacterStatText(
        GameUIEngine* _engine, TableCell* _parent, const FontInfo& _fontInfo, StatisticType _statisticType)
        : TextBox(_engine, _parent, _fontInfo), statisticType(_statisticType)
    {
        _engine->sys->controllableActorSystem->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });

        _engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe([this](entt::entity) { RetrieveInfo(); });

        if (_statisticType == StatisticType::NAME)
        {
            horiAlignment = HoriAlignment::CENTER;
        }

        RetrieveInfo();
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
        float originalRatio = calculateAspectRatio();

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
        auto& row = parent->parent->children;

        // First pass: calculate the required scale factor for each cell
        float minScaleFactor = 1.0f;

        for (auto& cell : row)
        {
            auto* imageBox = dynamic_cast<ImageBox*>(cell->children.get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            // Calculate scale factor needed for this image
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            // Keep track of the smallest scale factor needed
            minScaleFactor = std::min(minScaleFactor, scaleFactor);
        }

        // Second pass: apply the minimum scale factor to all cells
        for (auto& cell : row)
        {
            auto* imageBox = dynamic_cast<ImageBox*>(cell->children.get());
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
        // Navigate up to get the Table
        auto* tableCell = parent;
        auto* tableRow = tableCell->parent;
        auto* table = tableRow->parent;
        auto& allRows = table->children;

        size_t myColIndex = findMyColumnIndex();

        // First pass: calculate the required scale factor for each cell in this column
        float minScaleFactor = 1.0f;

        for (auto& row : allRows)
        {
            // Skip if the row doesn't have enough cells
            if (row->children.size() <= myColIndex) continue;

            auto* cellInColumn = row->children[myColIndex].get();
            auto* imageBox = dynamic_cast<ImageBox*>(cellInColumn->children.get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            // Calculate scale factor needed for this image
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            // Keep track of the smallest scale factor needed
            minScaleFactor = std::min(minScaleFactor, scaleFactor);
        }

        // Second pass: apply the minimum scale factor to all cells in this column
        for (auto& row : allRows)
        {
            // Skip if the row doesn't have enough cells
            if (row->children.size() <= myColIndex) continue;

            auto* cellInColumn = row->children[myColIndex].get();
            auto* imageBox = dynamic_cast<ImageBox*>(cellInColumn->children.get());
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

    // Helper function to find the column index of the current ImageBox
    size_t ImageBox::findMyColumnIndex() const
    {
        auto* myCell = parent;
        auto* myRow = myCell->parent;

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
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

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

    void EquipmentCharacterPreview::UpdateDimensions()
    {
        ImageBox::UpdateDimensions();
        auto& renderTexture =
            engine->registry->get<EquipmentComponent>(engine->sys->controllableActorSystem->GetSelectedActor())
                .renderTexture;
        renderTexture.texture.width = parent->GetRec().width;
        renderTexture.texture.height = parent->GetRec().height;
    }

    void EquipmentCharacterPreview::RetrieveInfo()
    {
        engine->sys->equipmentSystem->GenerateRenderTexture(
            engine->sys->controllableActorSystem->GetSelectedActor(),
            parent->GetRec().width * 4,
            parent->GetRec().height * 4);
        UpdateDimensions();
    }

    void EquipmentCharacterPreview::Draw2D()
    {
        auto renderTexture =
            engine->registry->get<EquipmentComponent>(engine->sys->controllableActorSystem->GetSelectedActor())
                .renderTexture;
        DrawTextureRec(
            renderTexture.texture,
            {0,
             0,
             static_cast<float>(renderTexture.texture.width),
             static_cast<float>(-renderTexture.texture.height)},
            {rec.x, rec.y},
            WHITE);

        //        DrawTextureEx(renderTexture.texture, {rec.x, rec.y}, 0, 0.75f, WHITE);
    }

    EquipmentCharacterPreview::EquipmentCharacterPreview(
        GameUIEngine* _engine, TableCell* _parent, VertAlignment _vertAlignment, HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_TO_FIT, _vertAlignment, _horiAlignment)
    {
        _engine->sys->controllableActorSystem->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });

        _engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe([this](entt::entity) { RetrieveInfo(); });
    }

    void PartyMemberPortrait::UpdateDimensions()
    {
        ImageBox::UpdateDimensions();
        portraitBgTex.width = tex.width + engine->sys->settings->ScaleValueWidth(10);
        portraitBgTex.height = tex.height + engine->sys->settings->ScaleValueHeight(10);
    }

    void PartyMemberPortrait::RetrieveInfo()
    {
        const auto entity = engine->sys->partySystem->GetMember(memberNumber);
        if (entity != entt::null)
        {
            const auto& info = engine->registry->get<PartyMemberComponent>(entity);

            engine->sys->equipmentSystem->GeneratePortraitRenderTexture(entity, tex.width * 4, tex.height * 4);

            tex.id = info.portraitImg.texture.id;
        }
        UpdateDimensions();
    }

    void PartyMemberPortrait::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            const auto receiver = engine->sys->partySystem->GetMember(memberNumber);
            const auto sender = engine->sys->controllableActorSystem->GetSelectedActor();
            if (receiver == sender) return;
            auto& receiverInv = engine->registry->get<InventoryComponent>(receiver);
            auto& senderInv = engine->registry->get<InventoryComponent>(sender);
            if (const auto& itemId = senderInv.GetItem(dropped->row, dropped->col); receiverInv.AddItem(itemId))
            {
                senderInv.RemoveItem(dropped->row, dropped->col);
            }
            else
            {
                engine->CreateErrorMessage("Inventory full.");
            }
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            const auto receiver = engine->sys->partySystem->GetMember(memberNumber);
            auto sender = engine->sys->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(receiver);
            auto droppedItemId = engine->sys->equipmentSystem->GetItem(sender, droppedE->itemType);

            if (inventory.AddItem(droppedItemId))
            {
                engine->sys->equipmentSystem->DestroyItem(sender, droppedE->itemType);
                droppedE->RetrieveInfo();
                RetrieveInfo();
            }
            else
            {
                engine->CreateErrorMessage("Inventory full.");
            }
        }
    }

    void PartyMemberPortrait::OnClick()
    {
        const auto entity = engine->sys->partySystem->GetMember(memberNumber);
        engine->sys->controllableActorSystem->SetSelectedActor(entity);
    }

    void PartyMemberPortrait::Draw2D()
    {
        const auto entity = engine->sys->partySystem->GetMember(memberNumber);
        if (entity == entt::null) return;
        if (engine->sys->controllableActorSystem->GetSelectedActor() == entity)
        {
            SetHoverShader();
        }

        DrawTextureRec(
            tex, {0, 0, static_cast<float>(tex.width), static_cast<float>(-tex.height)}, {rec.x, rec.y}, WHITE);

        if (shader.has_value())
        {
            BeginShaderMode(shader.value());
        }
        DrawTexture(
            portraitBgTex,
            rec.x - engine->sys->settings->ScaleValueWidth(5),
            rec.y - engine->sys->settings->ScaleValueHeight(5),
            WHITE);
        if (shader.has_value())
        {
            EndShaderMode();
        }
    }

    PartyMemberPortrait::PartyMemberPortrait(
        GameUIEngine* _engine, TableCell* _parent, unsigned int _memberNumber, int _width, int _height)
        : ImageBox(
              _engine, _parent, OverflowBehaviour::ALLOW_OVERFLOW, VertAlignment::MIDDLE, HoriAlignment::CENTER),
          memberNumber(_memberNumber),
          width(_width),
          height(_height)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/avatar_border_set.png");
        portraitBgTex = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/avatar_border_set.png");
        portraitBgTex.width = width;
        portraitBgTex.height = height;
        tex.width = width;
        tex.height = height;
        canReceiveDragDrops = true;
        _engine->sys->controllableActorSystem->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
        _engine->sys->partySystem->onPartyChange.Subscribe([this]() { RetrieveInfo(); });
    }

    void DialogPortrait::Draw2D()
    {
        DrawTextureRec(
            tex, {0, 0, static_cast<float>(tex.width), static_cast<float>(-tex.height)}, {rec.x, rec.y}, WHITE);
    }

    DialogPortrait::DialogPortrait(GameUIEngine* _engine, TableCell* _parent, Texture _tex)
        : ImageBox(
              _engine,
              _parent,
              _tex,
              OverflowBehaviour::SHRINK_TO_FIT,
              VertAlignment::MIDDLE,
              HoriAlignment::CENTER)
    {
    }

    void AbilitySlot::RetrieveInfo()
    {
        if (const Ability* ability = engine->sys->playerAbilitySystem->GetAbility(slotNumber))
        {
            tex = ResourceManager::GetInstance().TextureLoad(ability->icon);
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/empty-inv_slot.png");
        }
        UpdateDimensions();
    }

    void AbilitySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<AbilitySlot*>(droppedElement))
        {
            engine->sys->playerAbilitySystem->SwapAbility(slotNumber, dropped->slotNumber);
            dropped->RetrieveInfo();
            RetrieveInfo();
        }
    }

    void AbilitySlot::HoverUpdate()
    {
        if (!shader.has_value())
        {
            SetHoverShader();
        }
        ImageBox::HoverUpdate();
        if (tooltipWindow.has_value() || GetTime() < hoverTimer + hoverTimerThreshold) return;
        if (auto* ability = engine->sys->playerAbilitySystem->GetAbility(slotNumber))
        {
            tooltipWindow = GameUiFactory::CreateAbilityToolTip(engine, *ability, {rec.x, rec.y});
            const auto _rec = tooltipWindow.value()->GetRec();
            tooltipWindow.value()->SetPos(_rec.x, _rec.y - _rec.height);
            tooltipWindow.value()->InitLayout();
        }
    }

    void AbilitySlot::Draw2D()
    {
        const auto ability = engine->sys->playerAbilitySystem->GetAbility(slotNumber);
        if (!ability)
        {
            ImageBox::Draw2D();
            return;
        }
        if (ability->CooldownReady())
        {
            ImageBox::Draw2D();
        }
        else
        {
            SetGrayscale();
            ImageBox::Draw2D();
            RemoveShader();
        }
    }

    void AbilitySlot::OnClick()
    {
        engine->sys->playerAbilitySystem->PressAbility(slotNumber);
        CellElement::OnClick();
    }

    AbilitySlot::AbilitySlot(GameUIEngine* _engine, TableCell* _parent, unsigned int _slotNumber)
        : ImageBox(
              _engine,
              _parent,
              OverflowBehaviour::SHRINK_ROW_TO_FIT,
              VertAlignment::MIDDLE,
              HoriAlignment::CENTER),
          slotNumber(_slotNumber)
    {
        draggable = true;
        canReceiveDragDrops = true;
        engine->sys->controllableActorSystem->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
    }

    Texture ItemSlot::getEmptyTex()
    {
        return backgroundTex; // TODO: Replace with a better solution (drawing two background textures with this
                              // solution).
    }

    void ItemSlot::dropItemInWorld()
    {
        // TODO: If you're going to have it so that an item can only be picked up if you can pathfind there, you
        // should do the same for this.
        const auto itemId = getItemId();
        const auto cursorPos = engine->sys->cursor->getFirstNaviCollision();
        const auto playerPos =
            engine->registry->get<sgTransform>(engine->sys->controllableActorSystem->GetSelectedActor())
                .GetWorldPos();
        const auto dist = Vector3Distance(playerPos, cursorPos.point);

        if (const bool outOfRange = dist > ItemComponent::MAX_ITEM_DROP_RANGE; cursorPos.hit && !outOfRange)
        {
            if (GameObjectFactory::spawnItemInWorld(engine->registry, engine->sys, itemId, cursorPos.point))
            {
                onItemDroppedToWorld();
                RetrieveInfo();
            }
            else
            {
                engine->CreateErrorMessage("Item cannot be dropped.");
            }
        }
        else
        {
            engine->CreateErrorMessage("Out of range.");
        }
    }

    void ItemSlot::updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space)
    {
        ImageBox::updateRectangle(dimensions, offset, space);
        backgroundTex.width = dimensions.width;
        backgroundTex.height = dimensions.height;
    }

    void ItemSlot::HoverUpdate()
    {
        ImageBox::HoverUpdate();
        if (tooltipWindow.has_value() || GetTime() < hoverTimer + hoverTimerThreshold) return;
        auto itemId = getItemId();
        if (itemId != entt::null)
        {
            auto& item = engine->registry->get<ItemComponent>(itemId);
            tooltipWindow = GameUiFactory::CreateItemTooltip(
                engine, item, parent->GetWindow(), {rec.x + rec.width, rec.y - rec.height});
        }
    }

    void ItemSlot::RetrieveInfo()
    {
        auto itemId = getItemId();
        if (itemId != entt::null)
        {
            if (engine->registry->any_of<Renderable>(itemId))
            {
                auto& renderable = engine->registry->get<Renderable>(itemId);
                std::cout << renderable.GetModel()->GetKey() << std::endl;
            }
            auto& item = engine->registry->get<ItemComponent>(itemId);
            tex = ResourceManager::GetInstance().TextureLoad(item.icon);
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = getEmptyTex();
        }
        UpdateDimensions();
    }

    void ItemSlot::Draw2D()
    {
        DrawTexture(backgroundTex, rec.x, rec.y, WHITE);
        ImageBox::Draw2D();
    }

    void ItemSlot::OnDrop(CellElement* receiver)
    {
        beingDragged = false;

        if (receiver && receiver->canReceiveDragDrops)
        {
            receiver->ReceiveDrop(this);
        }
        else
        {
            if (const auto* characterWindow = parent->GetWindow();
                !PointInsideRect(characterWindow->GetRec(), GetMousePosition()))
            {
                dropItemInWorld();
            }
        }
    }

    ItemSlot::ItemSlot(
        GameUIEngine* _engine, TableCell* _parent, VertAlignment _vertAlignment, HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_ROW_TO_FIT, _vertAlignment, _horiAlignment)
    {
        draggable = true;
        canReceiveDragDrops = true;
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/empty-inv_slot.png");
        backgroundTex = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/empty-inv_slot.png");
    }

    Texture EquipmentSlot::getEmptyTex()
    {
        if (itemType == EquipmentSlotName::HELM)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/helm.png");
        }
        if (itemType == EquipmentSlotName::ARMS)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/arms.png");
        }
        if (itemType == EquipmentSlotName::CHEST)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/chest.png");
        }
        if (itemType == EquipmentSlotName::BELT)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/belt.png");
        }
        if (itemType == EquipmentSlotName::BOOTS)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/boots.png");
        }
        if (itemType == EquipmentSlotName::LEGS)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/legs.png");
        }
        if (itemType == EquipmentSlotName::LEFTHAND)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/mainhand.png");
        }
        if (itemType == EquipmentSlotName::RIGHTHAND)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/offhand.png");
        }
        if (itemType == EquipmentSlotName::RING1)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/ring.png");
        }
        if (itemType == EquipmentSlotName::RING2)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/ring.png");
        }
        if (itemType == EquipmentSlotName::AMULET)
        {
            return ResourceManager::GetInstance().TextureLoad("resources/textures/ui/amulet.png");
        }
        return backgroundTex;
    }

    void EquipmentSlot::onItemDroppedToWorld()
    {
        engine->sys->equipmentSystem->DestroyItem(
            engine->sys->controllableActorSystem->GetSelectedActor(), itemType);
    }

    bool EquipmentSlot::validateDrop(const ItemComponent& item) const
    {
        if (item.HasFlag(ItemFlags::WEAPON))
        {
            if (itemType == EquipmentSlotName::LEFTHAND)
            {
                return true;
            }
            const ItemFlags RIGHT_HAND_RESTRICTED_FLAGS =
                ItemFlags::MAIN_HAND_ONLY | ItemFlags::TWO_HANDED | ItemFlags::BOW | ItemFlags::CROSSBOW;
            if (itemType == EquipmentSlotName::RIGHTHAND && !item.HasAnyFlag(RIGHT_HAND_RESTRICTED_FLAGS))
            {
                return true;
            }
        }
        else if (item.HasFlag(ItemFlags::ARMOR))
        {
            if (itemType == EquipmentSlotName::HELM && item.HasFlag(ItemFlags::HELMET))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::AMULET && item.HasFlag(ItemFlags::AMULET))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::CHEST && item.HasFlag(ItemFlags::CHEST))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::BELT && item.HasFlag(ItemFlags::BELT))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::ARMS && item.HasFlag(ItemFlags::ARMS))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::LEGS && item.HasFlag(ItemFlags::LEGS))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::BOOTS && item.HasFlag(ItemFlags::BOOTS))
            {
                return true;
            }
            if ((itemType == EquipmentSlotName::RING1 || itemType == EquipmentSlotName::RING2) &&
                item.HasFlag(ItemFlags::RING))
            {
                return true;
            }
        }

        return false;
    }

    entt::entity EquipmentSlot::getItemId()
    {
        return engine->sys->equipmentSystem->GetItem(
            engine->sys->controllableActorSystem->GetSelectedActor(), itemType);
    }

    void EquipmentSlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            const auto actor = engine->sys->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(actor);
            const auto itemId = inventory.GetItem(dropped->row, dropped->col);
            auto& item = engine->registry->get<ItemComponent>(itemId);
            if (!validateDrop(item)) return;
            inventory.RemoveItem(dropped->row, dropped->col);
            engine->sys->equipmentSystem->MoveItemToInventory(actor, itemType);
            engine->sys->equipmentSystem->EquipItem(actor, itemId, itemType);
            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            // TODO: BUG: Can swap main hand only to offhand here
            const auto actor = engine->sys->controllableActorSystem->GetSelectedActor();
            if (!engine->sys->equipmentSystem->SwapItems(actor, itemType, droppedE->itemType))
            {
                // handle swap fail?
            }
        }
    }

    EquipmentSlot::EquipmentSlot(GameUIEngine* _engine, TableCell* _parent, EquipmentSlotName _itemType)
        : ItemSlot(_engine, _parent, VertAlignment::MIDDLE, HoriAlignment::CENTER), itemType(_itemType)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/amulet.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/helm.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/arms.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/chest.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/belt.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/boots.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/legs.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/mainhand.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/offhand.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/ring.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/ring.png");

        engine->sys->controllableActorSystem->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
    }

    void InventorySlot::onItemDroppedToWorld()
    {
        auto& inventory =
            engine->registry->get<InventoryComponent>(engine->sys->controllableActorSystem->GetSelectedActor());
        inventory.RemoveItem(row, col);
    }

    entt::entity InventorySlot::getItemId()
    {
        auto& inventory =
            engine->registry->get<InventoryComponent>(engine->sys->controllableActorSystem->GetSelectedActor());
        return inventory.GetItem(row, col);
    }

    void InventorySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            auto& inventory = engine->registry->get<InventoryComponent>(
                engine->sys->controllableActorSystem->GetSelectedActor());
            inventory.SwapItems(row, col, dropped->row, dropped->col);
            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            const auto actor = engine->sys->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(actor);
            const auto droppedItemId = engine->sys->equipmentSystem->GetItem(actor, droppedE->itemType);
            engine->sys->equipmentSystem->DestroyItem(actor, droppedE->itemType);

            if (const auto inventoryItemId = inventory.GetItem(row, col); inventoryItemId != entt::null)
            {
                engine->sys->equipmentSystem->EquipItem(actor, inventoryItemId, droppedE->itemType);
            }

            inventory.AddItem(droppedItemId, row, col);
            droppedE->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
    }

    InventorySlot::InventorySlot(GameUIEngine* _engine, TableCell* _parent, unsigned int _row, unsigned int _col)
        : ItemSlot(_engine, _parent, VertAlignment::MIDDLE, HoriAlignment::CENTER), row(_row), col(_col)
    {
        engine->sys->controllableActorSystem->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
    }

    void CloseButton::OnClick()
    {
        parent->GetWindow()->Hide();
    }

    CloseButton::CloseButton(GameUIEngine* _engine, TableCell* _parent, const Texture& _tex)
        : ImageBox(
              _engine, _parent, _tex, OverflowBehaviour::SHRINK_TO_FIT, VertAlignment::TOP, HoriAlignment::RIGHT)
    {
    }

    void WindowDocked::ScaleContents()
    {
        resetAll();
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

        for (auto& panel : children)
        {
            panel->ScaleContents(settings);
            for (auto& table : panel->children)
            {
                table->ScaleContents(settings);
                for (auto& row : table->children)
                {
                    row->ScaleContents(settings);
                    for (auto& cell : row->children)
                    {
                        cell->ScaleContents(settings);
                        if (cell->children)
                        {
                            cell->children->UpdateDimensions();
                        }
                    }
                }
            }
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
        float _xOffset,
        float _yOffset,
        float _width,
        float _height,
        VertAlignment _vertAlignment,
        HoriAlignment _horiAlignment,
        Padding _padding)
        : Window(_settings, _padding),
          vertAlignment(_vertAlignment),
          horiAlignment(_horiAlignment),
          baseXOffset(_xOffset),
          baseYOffset(_yOffset)
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
        TextureStretchMode _textureStretchMode,
        float _xOffset,
        float _yOffset,
        float _width,
        float _height,
        VertAlignment _vertAlignment,
        HoriAlignment _horiAlignment,
        Padding _padding)
        : Window(_settings, _padding),
          vertAlignment(_vertAlignment),
          horiAlignment(_horiAlignment),
          baseXOffset(_xOffset),
          baseYOffset(_yOffset)
    {
        tex = _tex;
        textureStretchMode = _textureStretchMode;
        rec.width = _width;
        rec.height = _height;
        setAlignment();
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
    }

    void Window::SetPos(float x, float y)
    {
        auto old = Vector2{rec.x, rec.y};
        rec = {x, y, rec.width, rec.height};
        ClampToScreen();
        auto diff = Vector2Subtract(Vector2{rec.x, rec.y}, Vector2{old.x, old.y});
        for (auto& panel : children)
        {
            panel->rec.x += diff.x;
            panel->rec.y += diff.y;
            for (auto& table : panel->children)
            {
                table->rec.x += diff.x;
                table->rec.y += diff.y;
                for (auto& row : table->children)
                {
                    row->rec.x += diff.x;
                    row->rec.y += diff.y;
                    for (auto& cell : row->children)
                    {
                        cell->rec.x += diff.x;
                        cell->rec.y += diff.y;
                        if (cell->children)
                        {
                            cell->children->UpdateDimensions();
                        }
                    }
                }
            }
        }
    }

    Panel* Window::CreatePanel(Padding _padding)
    {
        children.push_back(std::make_unique<Panel>(this, _padding));
        const auto& panel = children.back();
        InitLayout();
        return panel.get();
    }

    Panel* Window::CreatePanel(float _requestedHeight, Padding _padding)
    {
        auto panel = CreatePanel(_padding);
        panel->requestedHeight = _requestedHeight;
        panel->autoSize = false;
        InitLayout();
        return panel;
    }

    void Window::ToggleHide()
    {
        hidden = !hidden;
        if (hidden)
        {
            onHide.Publish();
        }
    }

    void Window::Show()
    {
        hidden = false;
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
        for (auto& panel : children)
        {
            panel->unscaledDimensions.rec = panel->rec;
            panel->unscaledDimensions.padding = panel->padding;

            for (auto& table : panel->children)
            {
                table->unscaledDimensions.rec = table->rec;
                table->unscaledDimensions.padding = table->padding;
                for (auto& row : table->children)
                {
                    row->unscaledDimensions.rec = row->rec;
                    row->unscaledDimensions.padding = row->padding;
                    for (auto& cell : row->children)
                    {
                        cell->unscaledDimensions.rec = cell->rec;
                        cell->unscaledDimensions.padding = cell->padding;
                    }
                }
            }
        }
        ScaleContents();
    }

    void Window::OnWindowUpdate(Vector2 prev, Vector2 current)
    {
        ScaleContents();
    }

    void Window::resetAll()
    {
        rec = unscaledDimensions.rec;
        padding = unscaledDimensions.padding;

        for (auto& panel : children)
        {
            panel->SetPos(panel->unscaledDimensions.rec.x, panel->unscaledDimensions.rec.y);
            panel->SetDimensions(panel->unscaledDimensions.rec.width, panel->unscaledDimensions.rec.height);
            panel->padding = panel->unscaledDimensions.padding;

            for (auto& table : panel->children)
            {
                table->SetPos(table->unscaledDimensions.rec.x, table->unscaledDimensions.rec.y);
                table->SetDimensions(table->unscaledDimensions.rec.width, table->unscaledDimensions.rec.height);
                table->padding = table->unscaledDimensions.padding;
                for (auto& row : table->children)
                {
                    row->SetPos(panel->unscaledDimensions.rec.x, row->unscaledDimensions.rec.y);
                    row->SetDimensions(row->unscaledDimensions.rec.width, row->unscaledDimensions.rec.height);
                    row->padding = row->unscaledDimensions.padding;
                    for (auto& cell : row->children)
                    {
                        cell->SetPos(cell->unscaledDimensions.rec.x, cell->unscaledDimensions.rec.y);
                        cell->SetDimensions(
                            cell->unscaledDimensions.rec.width, cell->unscaledDimensions.rec.height);
                        cell->padding = cell->unscaledDimensions.padding;
                        if (cell->children)
                        {
                            cell->children->UpdateDimensions();
                        }
                    }
                }
            }
        }
    }

    void Window::ScaleContents()
    {
        // assert(finalized);
        if (markForRemoval) return;

        resetAll();

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

        for (auto& panel : children)
        {
            panel->ScaleContents(settings);
            for (auto& table : panel->children)
            {
                table->ScaleContents(settings);
                for (auto& row : table->children)
                {
                    row->ScaleContents(settings);
                    for (auto& cell : row->children)
                    {
                        cell->ScaleContents(settings);
                        if (cell->children)
                        {
                            cell->children->UpdateDimensions();
                        }
                    }
                }
            }
        }

        // InitLayout();
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

    void Window::OnHoverStop()
    {
        UIElement::OnHoverStop();
        for (auto& panel : children)
        {
            for (auto& table : panel->children)
            {
                for (auto& row : table->children)
                {
                    for (auto& cell : row->children)
                    {
                        auto& element = cell->children;
                        if (!element) continue;
                        // Allow elements being dragged to continue being active outside of window's bounds
                        if (element->beingDragged) continue;
                        element->ChangeState(std::make_unique<IdleState>(element.get(), element->engine));
                    }
                }
            }
        }
    }

    void Window::DrawDebug2D()
    {
        std::vector colors = {RED, BLUE, YELLOW, WHITE, PINK};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& panel = children[i];
            Color col = colors[i];
            col.a = 150;
            // DrawRectangle(panel->rec.x, panel->rec.y, panel->rec.width, panel->rec.height, col);
            panel->DrawDebug2D();
        }
    }

    void Window::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& child : children)
        {
            child->Draw2D();
        }
    }

    Window::~Window()
    {
        windowUpdateCnx->UnSubscribe();
    }

    Window::Window(Settings* _settings, Padding _padding) : TableElement(nullptr, _padding), settings(_settings)
    {
    }

    Window::Window(
        Settings* _settings,
        const Texture& _tex,
        TextureStretchMode _stretchMode,
        float x,
        float y,
        float width,
        float height,
        Padding _padding)
        : TableElement(nullptr, x, y, width, height, _padding),

          settings(_settings)
    {
        tex = _tex;
        textureStretchMode = _stretchMode;
    }

    Window::Window(Settings* _settings, float x, float y, float width, float height, Padding _padding)
        : TableElement(nullptr, x, y, width, height, _padding), settings(_settings)
    {
    }

    void TooltipWindow::Remove()
    {
        hidden = true;
        markForRemoval = true;
    }

    void TooltipWindow::ScaleContents()
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
    }

    TooltipWindow::~TooltipWindow()
    {
        if (parent)
        {
            parentWindowHideCnx->UnSubscribe();
        }
    }

    TooltipWindow::TooltipWindow(
        Settings* _settings,
        Window* parentWindow,
        const Texture& _tex,
        TextureStretchMode _stretchMode,
        float x,
        float y,
        float width,
        float height,
        Padding _padding)
        : Window(_settings, x, y, width, height, _padding)
    {
        if (parentWindow)
        {
            parentWindowHideCnx = parentWindow->onHide.Subscribe([this]() { Remove(); });
        }
        tex = _tex;
        textureStretchMode = _stretchMode;
    }

    void Panel::DrawDebug2D()
    {
        std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& table = children[i];
            Color col = colors[i];
            col.a = 150;
            // DrawRectangle(table->rec.x, table->rec.y, table->rec.width, table->rec.height, col);
            table->DrawDebug2D();
        }
    }

    void Panel::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& table : children)
        {
            table->Draw2D();
        }
    }

    TableGrid* Panel::CreateTableGrid(int rows, int cols, float cellSpacing, Padding _padding)
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

    Table* Panel::CreateTable(Padding _padding)
    {
        children.push_back(std::make_unique<Table>(this, _padding));
        const auto& table = children.back();
        InitLayout();
        return table.get();
    }

    Table* Panel::CreateTable(float _requestedWidth, Padding _padding)
    {
        auto table = CreateTable(_padding);
        table->autoSize = false;
        table->requestedWidth = _requestedWidth;
        InitLayout();
        return table;
    }

    Panel::Panel(Window* _parent, Padding _padding) : TableElement(_parent, _padding)
    {
    }

    TableGrid::TableGrid(Panel* _parent, Padding _padding) : Table(_parent, _padding)
    {
    }

    void Table::DrawDebug2D()
    {
        std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& row = children[i];
            Color col = colors[i % colors.size()];
            col.a = 150;
            DrawRectangle(row->rec.x, row->rec.y, row->rec.width, row->rec.height, col);
            row->DrawDebug2D();
        }
    }

    void Table::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& row : children)
        {
            row->Draw2D();
        }
    }

    Table::Table(Panel* _parent, Padding _padding) : TableElement(_parent, _padding)
    {
    }

    TableRowGrid* Table::CreateTableRowGrid(int cols, float cellSpacing, Padding _padding)
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
        const auto& row = children.back();
        InitLayout();
        return row.get();
    }

    /**
     *
     * @param _requestedHeight The desired height of the cell as a percent (0-100)
     * @param _padding
     * @return
     */
    TableRow* Table::CreateTableRow(float _requestedHeight, Padding _padding)
    {
        assert(_requestedHeight <= 100 && _requestedHeight >= 0);
        children.push_back(std::make_unique<TableRow>(this, _padding));
        const auto& row = children.back();
        row->autoSize = false;
        row->requestedHeight = _requestedHeight;
        InitLayout();
        return row.get();
    }

    TableCell* TableRow::CreateTableCell(Padding _padding)
    {
        children.push_back(std::make_unique<TableCell>(this, _padding));
        const auto& cell = children.back();
        InitLayout();
        return cell.get();
    }

    /**
     *
     * @param requestedWidth The desired width of the cell as a percent (0-100)
     * @param _padding
     * @return
     */
    TableCell* TableRow::CreateTableCell(float requestedWidth, Padding _padding)
    {
        assert(requestedWidth <= 100 && requestedWidth >= 0);
        children.push_back(std::make_unique<TableCell>(this, _padding));
        const auto& cell = children.back();
        cell->autoSize = false;
        cell->requestedWidth = requestedWidth;
        InitLayout();
        return cell.get();
    }

    void TableRow::DrawDebug2D()
    {
        std::vector colors = {RED, BLUE, YELLOW, WHITE, PINK};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& cell = children[i];
            Color col = colors[i % colors.size()];
            col.a = 100;
            DrawRectangle(cell->rec.x, cell->rec.y, cell->rec.width, cell->rec.height, col);
            cell->DrawDebug2D();
        }
    }

    void TableRow::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& cell : children)
        {
            cell->Draw2D();
        }
    }

    TableRow::TableRow(Table* _parent, Padding _padding) : TableElement(_parent, _padding)
    {
    }

    TableRowGrid::TableRowGrid(Table* _parent, Padding _padding) : TableRow(_parent, _padding)
    {
    }

    PartyMemberPortrait* TableCell::CreatePartyMemberPortrait(std::unique_ptr<PartyMemberPortrait> _portrait)
    {
        children = std::move(_portrait);
        auto* portrait = dynamic_cast<PartyMemberPortrait*>(children.get());
        portrait->RetrieveInfo();
        InitLayout();
        return portrait;
    }

    GameWindowButton* TableCell::CreateGameWindowButton(std::unique_ptr<GameWindowButton> _button)
    {
        children = std::move(_button);
        auto* button = dynamic_cast<GameWindowButton*>(children.get());
        InitLayout();
        return button;
    }

    AbilitySlot* TableCell::CreateAbilitySlot(std::unique_ptr<AbilitySlot> _slot)
    {
        children = std::move(_slot);
        auto* abilitySlot = dynamic_cast<AbilitySlot*>(children.get());
        abilitySlot->RetrieveInfo();
        InitLayout();
        return abilitySlot;
    }

    EquipmentSlot* TableCell::CreateEquipmentSlot(std::unique_ptr<EquipmentSlot> _slot)
    {
        children = std::move(_slot);
        auto* slot = dynamic_cast<EquipmentSlot*>(children.get());
        slot->RetrieveInfo();
        InitLayout();
        return slot;
    }

    InventorySlot* TableCell::CreateInventorySlot(std::unique_ptr<InventorySlot> _slot)
    {
        children = std::move(_slot);
        auto* slot = dynamic_cast<InventorySlot*>(children.get());
        slot->RetrieveInfo();
        InitLayout();
        return slot;
    }

    TitleBar* TableCell::CreateTitleBar(std::unique_ptr<TitleBar> _titleBar, const std::string& _title)
    {
        children = std::move(_titleBar);
        auto* titleBar = dynamic_cast<TitleBar*>(children.get());
        titleBar->SetContent(_title);
        InitLayout();
        return titleBar;
    }

    CharacterStatText* TableCell::CreateCharacterStatText(std::unique_ptr<CharacterStatText> _statText)
    {
        children = std::move(_statText);
        auto* charStatText = dynamic_cast<CharacterStatText*>(children.get());
        InitLayout();
        return charStatText;
    }

    CloseButton* TableCell::CreateCloseButton(std::unique_ptr<CloseButton> _closeButton)
    {
        children = std::move(_closeButton);
        auto* closeButton = dynamic_cast<CloseButton*>(children.get());
        InitLayout();
        return closeButton;
    }

    TextBox* TableCell::CreateTextbox(std::unique_ptr<TextBox> _textBox, const std::string& _content)
    {
        children = std::move(_textBox);
        auto* textbox = dynamic_cast<TextBox*>(children.get());
        textbox->SetContent(_content);
        InitLayout();
        return textbox;
    }

    DialogOption* TableCell::CreateDialogOption(std::unique_ptr<DialogOption> _dialogOption)
    {
        children = std::move(_dialogOption);
        auto* textbox = dynamic_cast<DialogOption*>(children.get());
        InitLayout();
        return textbox;
    }

    ImageBox* TableCell::CreateImagebox(std::unique_ptr<ImageBox> _imageBox)
    {
        children = std::move(_imageBox);
        auto* image = dynamic_cast<ImageBox*>(children.get());
        InitLayout();
        return image;
    }

    EquipmentCharacterPreview* TableCell::CreateEquipmentCharacterPreview(
        std::unique_ptr<EquipmentCharacterPreview> _preview)
    {
        children = std::move(_preview);
        auto* image = dynamic_cast<EquipmentCharacterPreview*>(children.get());
        image->draggable = false;
        InitLayout();
        image->RetrieveInfo();
        return image;
    }

    void TableCell::InitLayout()
    {
        // assert(!GetWindow()->finalized);
        if (auto& element = children)
        {
            element->parent = this;
            element->rec = rec;
            element->UpdateDimensions();
        }
    }

    void TableCell::Draw2D()
    {
        TableElement::Draw2D();
        if (auto& element = children) // hide if dragged
        {
            element->Draw2D();
        }
    }

    void TableCell::DrawDebug2D()
    {
        auto col = BLACK;
        col.a = 50;
        // DrawRectangle(children->rec.x, children->rec.y, children->rec.width, children->rec.height, col);
    }

    TableCell::TableCell(TableRow* _parent, Padding _padding) : TableElement(_parent, _padding)
    {
    }

    bool ErrorMessage::Finished() const
    {
        return GetTime() > initialTime + totalDisplayTime;
    }

    void ErrorMessage::Draw2D() const
    {
        auto currentTime = GetTime();

        if (currentTime > initialTime + totalDisplayTime)
        {
            return;
        }

        auto elapsedTime = currentTime - initialTime;
        unsigned char a = 255;

        if (elapsedTime > (totalDisplayTime - fadeOut))
        {
            float fadeProgress = (elapsedTime - (totalDisplayTime - fadeOut)) / fadeOut;
            a = static_cast<unsigned char>((1.0f - fadeProgress) * 255);
        }

        const auto [width, height] = settings->GetViewPort();
        auto col = RAYWHITE;
        col.a = a;
        auto textSize = MeasureText(msg.c_str(), 18);

        DrawText(msg.c_str(), (width - textSize) / 2, height / 4, 18, col);
    }

    ErrorMessage::ErrorMessage(Settings* _settings, std::string _msg)
        : settings(_settings), msg(std::move(_msg)), initialTime(GetTime())
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
        engine->sys->cursor->DisableContextSwitching();
        engine->sys->cursor->Disable();

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
            std::remove_if(
                windows.begin(), windows.end(), [](const auto& window) { return window->IsMarkedForRemoval(); }),
            windows.end());
    }

    void GameUIEngine::CreateErrorMessage(const std::string& msg)
    {
        errorMessage.emplace(sys->settings, msg);
    }

    TooltipWindow* GameUIEngine::CreateTooltipWindow(std::unique_ptr<TooltipWindow> _tooltipWindow)
    {
        tooltipWindow = std::move(_tooltipWindow);
        tooltipWindow->windowUpdateCnx = sys->userInput->onWindowUpdate.Subscribe(
            [this](Vector2 prev, Vector2 current) { tooltipWindow->OnWindowUpdate(prev, current); });
        tooltipWindow->InitLayout();
        // tooltipWindow->ScaleContents(); // TODO: Maybe not needed
        return tooltipWindow.get();
    }

    Window* GameUIEngine::CreateWindow(std::unique_ptr<Window> _window)
    {
        windows.push_back(std::move(_window));
        auto* window = windows.back().get();
        window->windowUpdateCnx = sys->userInput->onWindowUpdate.Subscribe(
            [window](Vector2 prev, Vector2 current) { window->OnWindowUpdate(prev, current); });
        window->InitLayout();
        return window;
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(std::unique_ptr<WindowDocked> _windowDocked)
    {
        windows.push_back(std::move(_windowDocked));
        auto* window = dynamic_cast<WindowDocked*>(windows.back().get());
        window->windowUpdateCnx = sys->userInput->onWindowUpdate.Subscribe(
            [window](Vector2 prev, Vector2 current) { window->OnWindowUpdate(prev, current); });
        window->InitLayout();
        return window;
    }

    void GameUIEngine::PlaceWindow(Window* window, Vector2 requestedPos) const
    {
        window->rec.x = requestedPos.x;
        window->rec.y = requestedPos.y;
        window->ClampToScreen();
        window->InitLayout();

        if (auto collision = GetWindowCollision(window))
        {
            window->rec.x = collision->rec.x - window->rec.width;
            window->ClampToScreen();
            collision = GetWindowCollision(window);
            if (collision)
            {
                window->rec.x = collision->rec.x + collision->rec.width;
                window->ClampToScreen();
                collision = GetWindowCollision(window);
                if (collision)
                {
                    assert(0);
                }
            }
        }
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
        for (const auto& panel : windowUnderCursor->children)
        {
            for (const auto& table : panel->children)
            {
                for (const auto& row : table->children)
                {
                    for (const auto& cell : row->children)
                    {
                        const auto element = cell->children.get();
                        if (PointInsideRect(cell->GetRec(), mousePos))
                        {
                            return element;
                        }
                    }
                }
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

            sys->cursor->Disable();
            sys->cursor->DisableContextSwitching();

            window->OnHoverStart(); // TODO: Need to check if it was already being hovered?
            for (const auto& panel : window->children)
            {
                for (const auto& table : panel->children)
                {
                    for (const auto& row : table->children)
                    {
                        for (const auto& cell : row->children)
                        {
                            const auto element = cell->children.get();
                            if (element)
                            {
                                element->state->Update();
                            }
                        }
                    }
                }
            }
        }
    }

    void GameUIEngine::onWorldItemHover(entt::entity entity) const
    {
        if (!sys->inventorySystem->CheckWorldItemRange(true) || tooltipWindow) return;
        auto& item = registry->get<ItemComponent>(entity);
        auto viewport = sys->settings->GetViewPort();
        Vector2 pos = GetWorldToScreenEx(
            sys->cursor->getMouseHitInfo().rlCollision.point,
            *sys->camera->getRaylibCam(),
            viewport.x,
            viewport.y);
        pos.x += sys->settings->ScaleValueWidth(20); // TODO: magic number
        GameUiFactory::CreateWorldTooltip(sys->uiEngine.get(), item.localizedName, pos);
    }

    void GameUIEngine::onNPCHover(entt::entity entity) const
    {
        if (tooltipWindow) return;
        auto& renderable = registry->get<Renderable>(entity);
        auto viewport = sys->settings->GetViewPort();
        Vector2 pos = GetWorldToScreenEx(
            sys->cursor->getMouseHitInfo().rlCollision.point,
            *sys->camera->getRaylibCam(),
            viewport.x,
            viewport.y);
        pos.x += sys->settings->ScaleValueWidth(20); // TODO: magic number
        GameUiFactory::CreateWorldTooltip(sys->uiEngine.get(), renderable.GetVanityName(), pos);
    }

    void GameUIEngine::onStopWorldHover() const
    {
        if (tooltipWindow)
        {
            tooltipWindow->Remove();
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
            sys->cursor->Enable();
            sys->cursor->EnableContextSwitching();
            processWindows();
            pruneWindows();
        }

        if (errorMessage.has_value() && errorMessage->Finished())
        {
            errorMessage.reset();
        }
    }

    GameUIEngine::GameUIEngine(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
        _sys->cursor->onItemHover.Subscribe([this](const entt::entity entity) { onWorldItemHover(entity); });
        _sys->cursor->onStopHover.Subscribe([this]() { onStopWorldHover(); });
        _sys->cursor->onNPCHover.Subscribe([this](const entt::entity entity) { onNPCHover(entity); });
    }
#pragma endregion
} // namespace sage