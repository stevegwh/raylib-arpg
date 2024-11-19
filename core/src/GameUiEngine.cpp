//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

#include "Camera.hpp"
#include "components/CombatableActor.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "GameUiFactory.hpp"
#include "ResourceManager.hpp"
#include "slib.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "UserInput.hpp"

#include <cassert>
#include <ranges>
#include <sstream>

namespace sage
{

#pragma region init

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

            cell->ogDimensions.rec = rec;
            cell->ogDimensions.padding = padding;

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
        onMouseClicked.publish();
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
        float scaleFactor = engine->gameData->settings->GetCurrentScaleFactor();
        fontInfo.fontSize = fontInfo.baseFontSize * scaleFactor;
        fontInfo.fontSize = std::clamp(fontInfo.fontSize, FontInfo::minFontSize, FontInfo::maxFontSize);
    }

    void TextBox::UpdateDimensions()
    {
        UpdateFontScaling();
        float availableWidth = parent->GetRec().width - (parent->padding.left + parent->padding.right);

        if (fontInfo.overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {
            Vector2 textSize =
                MeasureTextEx(fontInfo.font, content.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);
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
        Vector2 textSize = MeasureTextEx(fontInfo.font, content.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);

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
        auto* conversation = option.parent->parent;
        if (option.nextIndex.has_value())
        {
            conversation->SelectOption(option.nextIndex.value());
        }
        else
        {
            conversation->EndConversation();
        }
    }

    DialogOption::DialogOption(
        GameUIEngine* _engine,
        TableCell* _parent,
        const dialog::Option& _option,
        const FontInfo& _fontInfo,
        VertAlignment _vertAlignment,
        HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment), option(_option)
    {
        content = option.description;
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
        auto mousePos = GetMousePosition();
        auto& window = draggedWindow.value();
        auto newPos = Vector2Subtract(mousePos, dragOffset);

        // TODO: Currently does not work.
        window->SetPos(newPos.x, newPos.y);
        window->ClampToScreen();
        // window->ResetAll();
        window->InitLayout();
        // window->ScaleContents();
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
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/outline.fs");
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

    Dimensions ImageBox::handleOverflow(const Dimensions& dimensions, const Dimensions& space) const
    {
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
            engine->registry
                ->get<EquipmentComponent>(engine->gameData->controllableActorSystem->GetSelectedActor())
                .renderTexture;
        renderTexture.texture.width = parent->GetRec().width;
        renderTexture.texture.height = parent->GetRec().height;
    }

    void EquipmentCharacterPreview::RetrieveInfo()
    {
        engine->gameData->equipmentSystem->GenerateRenderTexture(
            engine->gameData->controllableActorSystem->GetSelectedActor(),
            parent->GetRec().width * 4,
            parent->GetRec().height * 4);
        UpdateDimensions();
    }

    void EquipmentCharacterPreview::Draw2D()
    {
        auto renderTexture =
            engine->registry
                ->get<EquipmentComponent>(engine->gameData->controllableActorSystem->GetSelectedActor())
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
        entt::sink sink{_engine->gameData->controllableActorSystem->onSelectedActorChange};
        sink.connect<&EquipmentCharacterPreview::RetrieveInfo>(this);

        entt::sink sink2{engine->gameData->equipmentSystem->onEquipmentUpdated};
        sink2.connect<&EquipmentCharacterPreview::RetrieveInfo>(this);
    }

    void PartyMemberPortrait::RetrieveInfo()
    {
        auto info = engine->gameData->partySystem->GetMember(memberNumber);
        tex = ResourceManager::GetInstance().TextureLoad(info.portraitImage);
        UpdateDimensions();
    }

    void PartyMemberPortrait::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            auto receiver = engine->gameData->partySystem->GetMember(memberNumber).entity;
            auto sender = engine->gameData->controllableActorSystem->GetSelectedActor();
            if (receiver == sender) return;
            auto& receiverInv = engine->registry->get<InventoryComponent>(receiver);
            auto& senderInv = engine->registry->get<InventoryComponent>(sender);
            if (const auto& itemId = senderInv.GetItem(dropped->row, dropped->col); receiverInv.AddItem(itemId))
            {
                senderInv.RemoveItem(dropped->row, dropped->col);
            }
            else
            {
                // inventory full
            }
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            auto receiver = engine->gameData->partySystem->GetMember(memberNumber).entity;
            auto sender = engine->gameData->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(receiver);
            auto droppedItemId = engine->gameData->equipmentSystem->GetItem(sender, droppedE->itemType);

            if (inventory.AddItem(droppedItemId))
            {
                engine->gameData->equipmentSystem->DestroyItem(sender, droppedE->itemType);
                droppedE->RetrieveInfo();
                RetrieveInfo();
            }
            else
            {
                // inventory full
            }
        }
    }

    void PartyMemberPortrait::OnClick()
    {
        engine->gameData->controllableActorSystem->SetSelectedActor(
            engine->gameData->partySystem->GetMember(memberNumber).entity);
    }

    void PartyMemberPortrait::Draw2D()
    {
        if (engine->gameData->controllableActorSystem->GetSelectedActor() ==
            engine->gameData->partySystem->GetMember(memberNumber).entity)
        {
            SetGrayscale();
            // Change portrait bg to selected one (static?)
        }
        portraitBgTex.width = tex.width;
        portraitBgTex.height = tex.height;

        ImageBox::Draw2D();
        DrawTexture(portraitBgTex, rec.x, rec.y, WHITE);
    }

    PartyMemberPortrait::PartyMemberPortrait(GameUIEngine* _engine, TableCell* _parent, unsigned int _memberNumber)
        : ImageBox(
              _engine,
              _parent,
              OverflowBehaviour::SHRINK_ROW_TO_FIT,
              VertAlignment::MIDDLE,
              HoriAlignment::CENTER),
          memberNumber(_memberNumber)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/avatar_border_set.png");
        portraitBgTex = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/avatar_border_set.png");
        entt::sink sink{engine->gameData->controllableActorSystem->onSelectedActorChange};
        sink.connect<&PartyMemberPortrait::RetrieveInfo>(this);
    }

    void AbilitySlot::RetrieveInfo()
    {
        if (const Ability* ability = engine->gameData->playerAbilitySystem->GetAbility(slotNumber))
        {
            tex = ResourceManager::GetInstance().TextureLoad(ability->icon);
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = ResourceManager::GetInstance().TextureLoad("resources/icons/ui/empty.png");
        }
        UpdateDimensions();
    }

    void AbilitySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<AbilitySlot*>(droppedElement))
        {
            engine->gameData->playerAbilitySystem->SwapAbility(slotNumber, dropped->slotNumber);
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
        if (auto* ability = engine->gameData->playerAbilitySystem->GetAbility(slotNumber))
        {
            tooltipWindow = GameUiFactory::CreateAbilityToolTip(engine, *ability, {rec.x, rec.y});
            const auto _rec = tooltipWindow.value()->GetRec();
            tooltipWindow.value()->SetPos(_rec.x, _rec.y - _rec.height);
            tooltipWindow.value()->InitLayout();
        }
    }

    void AbilitySlot::Draw2D()
    {
        auto ability = engine->gameData->playerAbilitySystem->GetAbility(slotNumber);
        if (!ability) return;
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
        engine->gameData->playerAbilitySystem->PressAbility(slotNumber);
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
        entt::sink sink{engine->gameData->controllableActorSystem->onSelectedActorChange};
        sink.connect<&AbilitySlot::RetrieveInfo>(this);
    }

    Texture ItemSlot::getEmptyTex()
    {
        return backgroundTex; // TODO: Replace with a better solution (drawing two background textures with this
                              // solution).
    }

    void ItemSlot::dropItemInWorld()
    {
        const auto itemId = getItemId();
        const auto cursorPos = engine->gameData->cursor->getFirstNaviCollision();
        const auto playerPos =
            engine->registry->get<sgTransform>(engine->gameData->controllableActorSystem->GetSelectedActor())
                .GetWorldPos();
        const auto dist = Vector3Distance(playerPos, cursorPos.point);

        if (const bool outOfRange = dist > ItemComponent::MAX_ITEM_DROP_RANGE; cursorPos.hit && !outOfRange)
        {
            if (GameObjectFactory::spawnInventoryItem(engine->registry, engine->gameData, itemId, cursorPos.point))
            {
                onItemDroppedToWorld();
                RetrieveInfo();
            }
        }
        else
        {
            // TODO: Report to the user that you can't drop it here.
            std::cout << "Out of range \n";
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
        engine->gameData->equipmentSystem->DestroyItem(
            engine->gameData->controllableActorSystem->GetSelectedActor(), itemType);
    }

    bool EquipmentSlot::validateDrop(const ItemComponent& item) const
    {
        if (item.HasFlag(ItemFlags::WEAPON))
        {
            if (itemType == EquipmentSlotName::LEFTHAND)
            {
                return true;
            }
            if (itemType == EquipmentSlotName::RIGHTHAND &&
                !item.HasAnyFlag(ItemFlags::RIGHT_HAND_RESTRICTED_FLAGS))
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
        return engine->gameData->equipmentSystem->GetItem(
            engine->gameData->controllableActorSystem->GetSelectedActor(), itemType);
    }

    void EquipmentSlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            const auto actor = engine->gameData->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(actor);
            const auto itemId = inventory.GetItem(dropped->row, dropped->col);
            auto& item = engine->registry->get<ItemComponent>(itemId);
            if (!validateDrop(item)) return;
            inventory.RemoveItem(dropped->row, dropped->col);
            engine->gameData->equipmentSystem->MoveItemToInventory(actor, itemType);
            engine->gameData->equipmentSystem->EquipItem(actor, itemId, itemType);
            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            const auto actor = engine->gameData->controllableActorSystem->GetSelectedActor();
            if (!engine->gameData->equipmentSystem->SwapItems(actor, itemType, droppedE->itemType))
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

        entt::sink sink{engine->gameData->controllableActorSystem->onSelectedActorChange};
        sink.connect<&EquipmentSlot::RetrieveInfo>(this);
    }

    void InventorySlot::onItemDroppedToWorld()
    {
        auto& inventory = engine->registry->get<InventoryComponent>(
            engine->gameData->controllableActorSystem->GetSelectedActor());
        inventory.RemoveItem(row, col);
    }

    entt::entity InventorySlot::getItemId()
    {
        auto& inventory = engine->registry->get<InventoryComponent>(
            engine->gameData->controllableActorSystem->GetSelectedActor());
        return inventory.GetItem(row, col);
    }

    void InventorySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            auto& inventory = engine->registry->get<InventoryComponent>(
                engine->gameData->controllableActorSystem->GetSelectedActor());
            inventory.SwapItems(row, col, dropped->row, dropped->col);
            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            const auto actor = engine->gameData->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(actor);
            const auto droppedItemId = engine->gameData->equipmentSystem->GetItem(actor, droppedE->itemType);
            engine->gameData->equipmentSystem->DestroyItem(actor, droppedE->itemType);

            if (const auto inventoryItemId = inventory.GetItem(row, col); inventoryItemId != entt::null)
            {
                engine->gameData->equipmentSystem->EquipItem(actor, inventoryItemId, droppedE->itemType);
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
        entt::sink sink{engine->gameData->controllableActorSystem->onSelectedActorChange};
        sink.connect<&InventorySlot::RetrieveInfo>(this);
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
        ResetAll();

        rec = {
            settings->ScaleValue(rec.x),
            settings->ScaleValue(rec.y),
            settings->ScaleValue(rec.width),
            settings->ScaleValue(rec.height)};

        padding = {
            settings->ScaleValue(padding.up),
            settings->ScaleValue(padding.down),
            settings->ScaleValue(padding.left),
            settings->ScaleValue(padding.right)};

        setAlignment();
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
                        cell->children->UpdateDimensions();
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
            xOffset = (settings->screenWidth - rec.width) / 2;
            break;

        case HoriAlignment::RIGHT:
            xOffset = settings->screenWidth - rec.width;
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
            yOffset = (settings->screenHeight - rec.height) / 2;
            break;

        case VertAlignment::BOTTOM:
            yOffset = settings->screenHeight - rec.height;
            break;
        }

        rec.x = xOffset + settings->ScaleValue(baseXOffset);
        rec.y = yOffset + settings->ScaleValue(baseYOffset);
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
        ogDimensions.rec = rec;
        ogDimensions.padding = padding;
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
        ogDimensions.rec = rec;
        ogDimensions.padding = padding;
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
            onHide.publish();
        }
    }

    void Window::Show()
    {
        hidden = false;
    }

    void Window::Hide()
    {
        hidden = true;
        onHide.publish();
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
        onHide.publish();
    }

    void Window::FinalizeLayout()
    {
        ogDimensions.rec = rec;
        ogDimensions.padding = padding;
        for (auto& panel : children)
        {
            panel->ogDimensions.rec = panel->rec;
            panel->ogDimensions.padding = panel->padding;

            for (auto& table : panel->children)
            {
                table->ogDimensions.rec = table->rec;
                table->ogDimensions.padding = table->padding;
                for (auto& row : table->children)
                {
                    row->ogDimensions.rec = row->rec;
                    row->ogDimensions.padding = row->padding;
                    for (auto& cell : row->children)
                    {
                        cell->ogDimensions.rec = cell->rec;
                        cell->ogDimensions.padding = cell->padding;
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

    void Window::ResetAll()
    {
        rec = ogDimensions.rec;
        padding = ogDimensions.padding;

        for (auto& panel : children)
        {
            panel->SetPos(panel->ogDimensions.rec.x, panel->ogDimensions.rec.y);
            panel->SetDimensions(panel->ogDimensions.rec.width, panel->ogDimensions.rec.height);
            panel->padding = panel->ogDimensions.padding;

            for (auto& table : panel->children)
            {
                table->SetPos(table->ogDimensions.rec.x, table->ogDimensions.rec.y);
                table->SetDimensions(table->ogDimensions.rec.width, table->ogDimensions.rec.height);
                table->padding = table->ogDimensions.padding;
                for (auto& row : table->children)
                {
                    row->SetPos(panel->ogDimensions.rec.x, row->ogDimensions.rec.y);
                    row->SetDimensions(row->ogDimensions.rec.width, row->ogDimensions.rec.height);
                    row->padding = row->ogDimensions.padding;
                    for (auto& cell : row->children)
                    {
                        cell->SetPos(cell->ogDimensions.rec.x, cell->ogDimensions.rec.y);
                        cell->SetDimensions(cell->ogDimensions.rec.width, cell->ogDimensions.rec.height);
                        cell->padding = cell->ogDimensions.padding;
                        cell->children->UpdateDimensions();
                    }
                }
            }
        }
    }

    void Window::ScaleContents()
    {
        // assert(finalized);
        if (markForRemoval) return;

        ResetAll();

        rec = {
            settings->ScaleValue(rec.x),
            settings->ScaleValue(rec.y),
            settings->ScaleValue(rec.width),
            settings->ScaleValue(rec.height)};

        padding = {
            settings->ScaleValue(padding.up),
            settings->ScaleValue(padding.down),
            settings->ScaleValue(padding.left),
            settings->ScaleValue(padding.right)};

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
                        cell->children->UpdateDimensions();
                    }
                }
            }
        }

        // InitLayout();
    }

    void Window::ClampToScreen()
    {
        if (rec.x + rec.width > settings->screenWidth)
        {
            rec.x = settings->screenWidth - rec.width;
        }
        if (rec.y + rec.height > settings->screenHeight)
        {
            rec.y = settings->screenHeight - rec.height;
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
        windowUpdateCnx.release();
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
        : TableElement(nullptr, x, y, width, height, _padding), settings(_settings)
    {
        tex = _tex;
        textureStretchMode = _stretchMode;
    }

    Window::Window(Settings* _settings, float x, float y, float width, float height, Padding _padding)
        : TableElement(nullptr, x, y, width, height, _padding), settings(_settings)
    {
    }

    void TooltipWindow::ScaleContents()
    {
        // Tooltips original position is scaled to the screen already
        if (markForRemoval) return;

        rec = {rec.x, rec.y, settings->ScaleValue(rec.width), settings->ScaleValue(rec.height)};

        padding = {
            settings->ScaleValue(padding.up),
            settings->ScaleValue(padding.down),
            settings->ScaleValue(padding.left),
            settings->ScaleValue(padding.right)};

        UpdateTextureDimensions();
    }

    TooltipWindow::~TooltipWindow()
    {
        parentWindowHideCnx.release();
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
            entt::sink sink{parentWindow->onHide};
            parentWindowHideCnx = sink.connect<&TooltipWindow::Remove>(this);
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

    TableGrid* Panel::CreateTableGrid(int rows, int cols, float cellSpacing)
    {
        children.push_back(std::make_unique<TableGrid>(this));
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
        // TODO: Calculate the _requestedHeight etc as a pixel value and save as ogDimensions. Do not use
        // requestedHeight etc after initialisation, use the original pixel values.
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
        // TODO: Calculate the requestedWidth etc as a pixel value and save as ogDimensions. Do not use
        // requestedWidth etc after initialisation, use the original pixel values.
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
        engine->gameData->cursor->DisableContextSwitching();
        engine->gameData->cursor->Disable();

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
        {
            std::vector<unsigned int> toRemove;
            for (unsigned int i = 0; i < windows.size(); ++i)
            {
                const auto& window = windows[i];
                if (window->IsMarkedForRemoval())
                {
                    toRemove.push_back(i);
                }
            }

            for (auto& i : toRemove)
            {
                windows.erase(windows.begin() + i);
            }
        }

        if (tooltipWindow && tooltipWindow->IsMarkedForRemoval())
        {
            tooltipWindow.reset();
        }
    }

    TooltipWindow* GameUIEngine::CreateTooltipWindow(std::unique_ptr<TooltipWindow> _tooltipWindow)
    {
        tooltipWindow = std::move(_tooltipWindow);
        entt::sink sink{gameData->userInput->onWindowUpdate};
        tooltipWindow->windowUpdateCnx = sink.connect<&Window::OnWindowUpdate>(tooltipWindow.get());
        tooltipWindow->InitLayout();
        // tooltipWindow->ScaleContents(); // TODO: Maybe not needed
        return tooltipWindow.get();
    }

    Window* GameUIEngine::CreateWindow(std::unique_ptr<Window> _window)
    {
        windows.push_back(std::move(_window));
        auto* window = windows.back().get();
        entt::sink sink{gameData->userInput->onWindowUpdate};
        window->windowUpdateCnx = sink.connect<&Window::OnWindowUpdate>(window);
        window->InitLayout();
        return window;
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(std::unique_ptr<WindowDocked> _windowDocked)
    {
        windows.push_back(std::move(_windowDocked));
        auto* window = dynamic_cast<WindowDocked*>(windows.back().get());
        entt::sink sink{gameData->userInput->onWindowUpdate};
        window->windowUpdateCnx = sink.connect<&WindowDocked::OnWindowUpdate>(window);
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

        if (tooltipWindow)
        {
            tooltipWindow->Draw2D();
        }

        if (draggedObject.has_value())
        {
            draggedObject.value()->state->Draw();
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

        for (auto& window : std::ranges::reverse_view(windows))
        {
            if (!window || window->IsHidden()) continue;

            if (!PointInsideRect(window->rec, mousePos) || !mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                window->OnHoverStop();
                continue;
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                BringClickedWindowToFront(window.get());
            }

            gameData->cursor->Disable();
            gameData->cursor->DisableContextSwitching();

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
                            element->state->Update();
                        }
                    }
                }
            }
        }
    }

    void GameUIEngine::onWorldItemHover(entt::entity entity) const
    {
        if (!gameData->inventorySystem->CheckWorldItemRange() || tooltipWindow) return;
        auto& item = registry->get<ItemComponent>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        // pos.x += 20; // TODO: magic number
        GameUiFactory::CreateWorldTooltip(gameData->uiEngine.get(), item.name, pos);
    }

    void GameUIEngine::onWorldCombatableHover(entt::entity entity) const
    {
        if (tooltipWindow) return;
        auto& renderable = registry->get<Renderable>(entity);
        auto& combatable = registry->get<CombatableActor>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        pos.x += 20; // TODO: magic number
        GameUiFactory::CreateCombatableTooltip(gameData->uiEngine.get(), renderable.name, combatable, pos);
    }

    void GameUIEngine::onNPCHover(entt::entity entity) const
    {
        if (tooltipWindow) return;
        auto& renderable = registry->get<Renderable>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        pos.x += 20; // TODO: magic number
        GameUiFactory::CreateWorldTooltip(gameData->uiEngine.get(), renderable.name, pos);
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
            gameData->cursor->Enable();
            gameData->cursor->EnableContextSwitching();
            pruneWindows();
            processWindows();
        }
    }

    GameUIEngine::GameUIEngine(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        entt::sink sink{_gameData->cursor->onCombatableHover};
        sink.connect<&GameUIEngine::onWorldCombatableHover>(this);
        entt::sink sink2{_gameData->cursor->onItemHover};
        sink2.connect<&GameUIEngine::onWorldItemHover>(this);
        entt::sink sink3{_gameData->cursor->onStopHover};
        sink3.connect<&GameUIEngine::onStopWorldHover>(this);
        entt::sink sink4{_gameData->cursor->onNPCHover};
        sink4.connect<&GameUIEngine::onNPCHover>(this);
    }
#pragma endregion
} // namespace sage