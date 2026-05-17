//
// Concrete CellElement subclasses implementation. See UIElements.hpp.
//

#include "UIElements.hpp"

#include "../GameUiEngine.hpp" // for Window/TableCell/TooltipWindow full defs
#include "../Settings.hpp"

#include "raylib.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace sage
{
    TextInput* TextInput::activeInput = nullptr;

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

    float TextBox::WrappedHeight(const FontInfo& info, const std::string& text, float availableWidth)
    {
        // Mirrors the WORD_WRAP branch of TextBox::UpdateDimensions so callers
        // measure the same number of lines that UpdateDimensions will produce.
        std::string wrappedText;
        std::string currentLine;
        std::istringstream words(text);
        std::string word;
        while (words >> word)
        {
            std::string testLine = currentLine;
            if (!testLine.empty()) testLine += " ";
            testLine += word;

            const Vector2 lineSize =
                MeasureTextEx(info.font, testLine.c_str(), info.fontSize, info.fontSpacing);

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
        return MeasureTextEx(info.font, wrappedText.c_str(), info.fontSize, info.fontSpacing).y;
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
        DrawTextEx(
            fontInfo.font,
            content.c_str(),
            Vector2{rec.x, rec.y},
            fontInfo.fontSize,
            fontInfo.fontSpacing,
            fontInfo.color);
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

    void TextInput::processKeyboardInput()
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key >= 32 && key <= 126)
            {
                content.insert(content.begin() + static_cast<std::ptrdiff_t>(caretIndex), static_cast<char>(key));
                ++caretIndex;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !content.empty())
        {
            if (caretIndex > 0)
            {
                content.erase(content.begin() + static_cast<std::ptrdiff_t>(caretIndex - 1));
                --caretIndex;
            }
        }

        if (IsKeyPressed(KEY_DELETE) && caretIndex < content.size())
        {
            content.erase(content.begin() + static_cast<std::ptrdiff_t>(caretIndex));
        }
        if (IsKeyPressed(KEY_LEFT) && caretIndex > 0)
        {
            --caretIndex;
        }
        if (IsKeyPressed(KEY_RIGHT) && caretIndex < content.size())
        {
            ++caretIndex;
        }
        if (IsKeyPressed(KEY_HOME))
        {
            caretIndex = 0;
        }
        if (IsKeyPressed(KEY_END))
        {
            caretIndex = content.size();
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        {
            commitEdit();
        }
        else if (IsKeyPressed(KEY_ESCAPE))
        {
            cancelEdit();
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !PointInsideRect(rec, GetMousePosition()))
        {
            commitEdit();
        }
    }

    void TextInput::commitEdit()
    {
        if (!editing) return;
        editing = false;
        if (activeInput == this)
        {
            activeInput = nullptr;
        }
        if (onSubmit)
        {
            onSubmit(content);
        }
    }

    void TextInput::cancelEdit()
    {
        if (!editing) return;
        content = editStartContent;
        editing = false;
        caretIndex = content.size();
        if (activeInput == this)
        {
            activeInput = nullptr;
        }
        UpdateDimensions();
    }

    void TextInput::SetContent(const std::string& _content)
    {
        if (editing) return;
        TextBox::SetContent(_content);
    }

    void TextInput::OnClick()
    {
        if (activeInput && activeInput != this)
        {
            activeInput->commitEdit();
        }
        activeInput = this;
        if (!editing)
        {
            editStartContent = content;
        }
        editing = true;
        setCaretFromMousePosition(GetMousePosition());
    }

    void TextInput::setCaretFromMousePosition(const Vector2 mousePosition)
    {
        const float relativeX = std::max(0.0f, mousePosition.x - rec.x - 6.0f);
        caretIndex = content.size();

        for (std::size_t i = 0; i <= content.size(); ++i)
        {
            const std::string prefix = content.substr(0, i);
            const Vector2 prefixSize = MeasureTextEx(fontInfo.font, prefix.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);
            if (relativeX <= prefixSize.x)
            {
                caretIndex = i;
                return;
            }
        }
    }

    void TextInput::UpdateDimensions()
    {
        UpdateFontScaling();
        rec = {
            parent->GetRec().x + parent->padding.left,
            parent->GetRec().y + parent->padding.up,
            parent->GetRec().width - parent->padding.left - parent->padding.right,
            parent->GetRec().height - parent->padding.up - parent->padding.down};
    }

    void TextInput::Draw2D()
    {
        if (editing)
        {
            processKeyboardInput();
        }

        DrawRectangleRec(rec, editing ? Color{255, 255, 255, 255} : Color{246, 248, 251, 255});
        DrawRectangleLinesEx(rec, editing ? 2.0f : 1.0f, editing ? Color{37, 99, 235, 255} : Color{151, 164, 184, 255});

        DrawTextEx(
            fontInfo.font,
            content.c_str(),
            Vector2{rec.x + 6.0f, rec.y + (rec.height - fontInfo.fontSize) * 0.5f},
            fontInfo.fontSize,
            fontInfo.fontSpacing,
            fontInfo.color);

        if (editing && static_cast<int>(GetTime() * 2.0) % 2 == 0)
        {
            const std::string caretPrefix = content.substr(0, caretIndex);
            const Vector2 textSize =
                MeasureTextEx(fontInfo.font, caretPrefix.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);
            const float caretX = std::min(rec.x + rec.width - 6.0f, rec.x + 7.0f + textSize.x);
            DrawLineEx(
                Vector2{caretX, rec.y + 5.0f},
                Vector2{caretX, rec.y + rec.height - 5.0f},
                1.5f,
                Color{37, 99, 235, 255});
        }
    }

    bool TextInput::IsEditing() const
    {
        return editing;
    }

    bool TextInput::AnyEditing()
    {
        return activeInput && activeInput->editing;
    }

    void TextInput::SetOnSubmit(std::function<void(const std::string&)> callback)
    {
        onSubmit = std::move(callback);
    }

    TextInput::TextInput(
        GameUIEngine* _engine,
        TableCell* _parent,
        std::function<void(const std::string&)> callback,
        const FontInfo& _fontInfo,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment),
          onSubmit(std::move(callback))
    {
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
} // namespace sage
