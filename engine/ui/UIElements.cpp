//
// Concrete CellElement subclasses implementation. See UIElements.hpp.
//

#include "UIElements.hpp"

#include "../GameUiEngine.hpp" // for Window/TableCell/TooltipWindow full defs
#include "../Settings.hpp"
#include "../slib.hpp"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

namespace sage
{
    TextInput* TextInput::activeInput = nullptr;
    DropdownList* DropdownList::activeDropdown = nullptr;

    namespace
    {
        constexpr Color CONTROL_BG = {246, 248, 251, 255};
        constexpr Color CONTROL_BG_ACTIVE = {255, 255, 255, 255};
        constexpr Color CONTROL_BORDER = {151, 164, 184, 255};
        constexpr Color CONTROL_BORDER_ACTIVE = {37, 99, 235, 255};
        constexpr Color CONTROL_HOVER_BG = {236, 242, 252, 255};
        constexpr Color CONTROL_SELECTED_BG = {219, 234, 254, 255};
        constexpr Color CONTROL_TEXT = {17, 24, 39, 255};

        const std::string& emptyDropdownText()
        {
            static const std::string empty;
            return empty;
        }
    } // namespace

    const std::string& TextBox::GetContent() const
    {
        return content;
    }

    Font TextBox::DefaultFont()
    {
        return ResourceManager::GetInstance().FontLoad("resources/fonts/FiraCode/FiraCode-Regular.ttf");
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
        fontInfo.fontSize = std::clamp(fontInfo.fontSize, fontInfo.minFontSize, fontInfo.maxFontSize);
    }

    void TextBox::UpdateDimensions()
    {
        UpdateFontScaling();
        float availableWidth = parent->GetRec().width - (parent->padding.left + parent->padding.right);
        Vector2 textSize = MeasureTextEx(fontInfo.font, content.c_str(), fontInfo.fontSize, fontInfo.fontSpacing);

        if (fontInfo.overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {

            while (textSize.x > availableWidth && fontInfo.fontSize > fontInfo.minFontSize)
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
        const Rectangle clip = {
            parent->GetRec().x + parent->padding.left,
            parent->GetRec().y + parent->padding.up,
            std::max(0.0f, parent->GetRec().width - parent->padding.left - parent->padding.right),
            std::max(0.0f, parent->GetRec().height - parent->padding.up - parent->padding.down)};
        const ScissorScope scissor{clip};
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

        // Shrink the rendered font size to keep content inside the input box. The
        // input rect itself is sized by UpdateDimensions to fill the cell, so
        // measuring against rec.width is the right "available space" for text.
        const float reservedSidePadding = 12.0f; // matches the 6.0f left/right gutter used below
        const float availableTextWidth = std::max(0.0f, rec.width - reservedSidePadding);
        float renderFontSize = fontInfo.fontSize;
        if (fontInfo.overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {
            Vector2 textSize =
                MeasureTextEx(fontInfo.font, content.c_str(), renderFontSize, fontInfo.fontSpacing);
            while (textSize.x > availableTextWidth && renderFontSize > fontInfo.minFontSize)
            {
                renderFontSize -= 1.0f;
                textSize = MeasureTextEx(fontInfo.font, content.c_str(), renderFontSize, fontInfo.fontSpacing);
            }
        }

        const ScissorScope scissor{rec};

        DrawTextEx(
            fontInfo.font,
            content.c_str(),
            Vector2{rec.x + 6.0f, rec.y + (rec.height - renderFontSize) * 0.5f},
            renderFontSize,
            fontInfo.fontSpacing,
            fontInfo.color);

        if (editing && static_cast<int>(GetTime() * 2.0) % 2 == 0)
        {
            const std::string caretPrefix = content.substr(0, caretIndex);
            const Vector2 textSize =
                MeasureTextEx(fontInfo.font, caretPrefix.c_str(), renderFontSize, fontInfo.fontSpacing);
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

    void Checkbox::SetChecked(const bool _checked)
    {
        checked = _checked;
    }

    bool Checkbox::IsChecked() const
    {
        return checked;
    }

    void Checkbox::OnClick()
    {
        CellElement::OnClick();
        checked = !checked;
        onValueChanged.Publish(checked);
    }

    void Checkbox::UpdateDimensions()
    {
        const float availableWidth = parent->GetRec().width - parent->padding.left - parent->padding.right;
        const float availableHeight = parent->GetRec().height - parent->padding.up - parent->padding.down;
        const float preferredSize = 18.0f * engine->settings->GetCurrentScaleFactor();
        const float size = std::max(0.0f, std::min({availableWidth, availableHeight, preferredSize}));

        float horiOffset = 0.0f;
        float vertOffset = 0.0f;
        if (horiAlignment == HoriAlignment::CENTER || horiAlignment == HoriAlignment::WINDOW_CENTER)
        {
            horiOffset = (availableWidth - size) * 0.5f;
        }
        else if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = availableWidth - size;
        }

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (availableHeight - size) * 0.5f;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = availableHeight - size;
        }

        rec = {
            parent->GetRec().x + parent->padding.left + horiOffset,
            parent->GetRec().y + parent->padding.up + vertOffset,
            size,
            size};
    }

    void Checkbox::Draw2D()
    {
        const bool focused =
            std::holds_alternative<HoverState>(state) || std::holds_alternative<DragDelayState>(state);
        DrawRectangleRec(rec, checked ? CONTROL_BORDER_ACTIVE : CONTROL_BG);
        DrawRectangleLinesEx(
            rec, focused || checked ? 2.0f : 1.0f, focused ? CONTROL_BORDER_ACTIVE : CONTROL_BORDER);

        if (!checked) return;

        const float lineWidth = std::max(2.0f, rec.width * 0.12f);
        const Vector2 start{rec.x + rec.width * 0.24f, rec.y + rec.height * 0.52f};
        const Vector2 middle{rec.x + rec.width * 0.43f, rec.y + rec.height * 0.70f};
        const Vector2 end{rec.x + rec.width * 0.78f, rec.y + rec.height * 0.30f};
        DrawLineEx(start, middle, lineWidth, WHITE);
        DrawLineEx(middle, end, lineWidth, WHITE);
    }

    Checkbox::Checkbox(
        GameUIEngine* _engine,
        TableCell* _parent,
        const bool _checked,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment)
        : CellElement(_engine, _parent, _vertAlignment, _horiAlignment), checked(_checked)
    {
    }

    void DropdownList::updateFontScaling()
    {
        const float scaleFactor = engine->settings->GetCurrentScaleFactor();
        fontInfo.fontSize = fontInfo.baseFontSize * scaleFactor;
        fontInfo.fontSize = std::clamp(fontInfo.fontSize, fontInfo.minFontSize, fontInfo.maxFontSize);
    }

    float DropdownList::optionHeight() const
    {
        return std::max(rec.height, fontInfo.fontSize + 10.0f);
    }

    std::size_t DropdownList::visibleOptionCount() const
    {
        return std::min(maxVisibleOptions, options.size());
    }

    Rectangle DropdownList::expandedListRec() const
    {
        const float listHeight = optionHeight() * static_cast<float>(visibleOptionCount());
        const float y = dropDirection == DropDirection::DOWN ? rec.y + rec.height : rec.y - listHeight;
        return {rec.x, y, rec.width, listHeight};
    }

    std::optional<std::size_t> DropdownList::optionIndexAt(const Vector2 point) const
    {
        const Rectangle listRec = expandedListRec();
        if (!PointInsideRect(listRec, point) || options.empty()) return std::nullopt;

        const auto rowIndex =
            static_cast<std::size_t>(std::floor((point.y - listRec.y) / std::max(1.0f, optionHeight())));
        const std::size_t optionIndex = firstVisibleOption + rowIndex;
        if (optionIndex >= options.size()) return std::nullopt;
        return optionIndex;
    }

    void DropdownList::clampScrollOffset()
    {
        const std::size_t visibleCount = visibleOptionCount();
        const std::size_t maxOffset = options.size() > visibleCount ? options.size() - visibleCount : 0;
        firstVisibleOption = std::min(firstVisibleOption, maxOffset);
    }

    void DropdownList::scrollOptions(const int signedDelta)
    {
        const std::size_t visibleCount = visibleOptionCount();
        const std::size_t maxOffset = options.size() > visibleCount ? options.size() - visibleCount : 0;

        if (signedDelta < 0)
        {
            const auto positiveDelta = static_cast<std::size_t>(-signedDelta);
            firstVisibleOption = positiveDelta > firstVisibleOption ? 0 : firstVisibleOption - positiveDelta;
        }
        else if (signedDelta > 0)
        {
            firstVisibleOption = std::min(maxOffset, firstVisibleOption + static_cast<std::size_t>(signedDelta));
        }
        clampScrollOffset();
    }

    void DropdownList::close()
    {
        expanded = false;
        if (activeDropdown == this)
        {
            activeDropdown = nullptr;
        }
    }

    void DropdownList::handleExpandedInput()
    {
        if (!expanded) return;
        if (activeDropdown && activeDropdown != this)
        {
            close();
            return;
        }

        const Vector2 mousePos = GetMousePosition();
        const Rectangle listRec = expandedListRec();
        const bool insideList = PointInsideRect(listRec, mousePos);
        const bool insideControl = PointInsideRect(rec, mousePos);

        if (insideList)
        {
            const float wheel = GetMouseWheelMove();
            if (wheel > 0.0f)
            {
                scrollOptions(-static_cast<int>(std::ceil(wheel)));
            }
            else if (wheel < 0.0f)
            {
                scrollOptions(static_cast<int>(std::ceil(-wheel)));
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (const auto clickedIndex = optionIndexAt(mousePos))
                {
                    SetSelectedIndex(*clickedIndex, true);
                }
                close();
            }
            return;
        }

        if (!insideControl && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            close();
        }
    }

    void DropdownList::drawCollapsed() const
    {
        const bool focused =
            expanded || std::holds_alternative<HoverState>(state) || std::holds_alternative<DragDelayState>(state);
        DrawRectangleRec(rec, expanded ? CONTROL_BG_ACTIVE : CONTROL_BG);
        DrawRectangleLinesEx(rec, focused ? 2.0f : 1.0f, focused ? CONTROL_BORDER_ACTIVE : CONTROL_BORDER);

        const float arrowWidth = std::min(20.0f, rec.width * 0.22f);
        const Rectangle textClip = {
            rec.x + 6.0f, rec.y, std::max(0.0f, rec.width - arrowWidth - 10.0f), rec.height};

        {
            const ScissorScope scissor{textClip};
            DrawTextEx(
                fontInfo.font,
                GetSelectedText().c_str(),
                Vector2{textClip.x, rec.y + (rec.height - fontInfo.fontSize) * 0.5f},
                fontInfo.fontSize,
                fontInfo.fontSpacing,
                fontInfo.color);
        }

        const float scale = engine->settings->GetCurrentScaleFactor();
        const float arrowSize = std::min(7.0f * scale, rec.height * 0.25f);
        const float centerX = rec.x + rec.width - arrowWidth * 0.5f;
        const float centerY = rec.y + rec.height * 0.5f;
        if (expanded && dropDirection == DropDirection::UP)
        {
            DrawTriangle(
                Vector2{centerX, centerY - arrowSize * 0.5f},
                Vector2{centerX - arrowSize, centerY + arrowSize * 0.5f},
                Vector2{centerX + arrowSize, centerY + arrowSize * 0.5f},
                CONTROL_TEXT);
        }
        else
        {
            DrawTriangle(
                Vector2{centerX - arrowSize, centerY - arrowSize * 0.5f},
                Vector2{centerX + arrowSize, centerY - arrowSize * 0.5f},
                Vector2{centerX, centerY + arrowSize * 0.5f},
                CONTROL_TEXT);
        }
    }

    void DropdownList::drawExpandedList() const
    {
        if (!expanded || options.empty()) return;

        const Rectangle listRec = expandedListRec();
        DrawRectangleRec(listRec, CONTROL_BG_ACTIVE);
        DrawRectangleLinesEx(listRec, 1.0f, CONTROL_BORDER_ACTIVE);

        const ScissorScope scissor{listRec};
        const float rowHeight = optionHeight();
        const Vector2 mousePos = GetMousePosition();
        const auto highlightedIndex = optionIndexAt(mousePos);
        const std::size_t visibleCount = visibleOptionCount();

        for (std::size_t row = 0; row < visibleCount; ++row)
        {
            const std::size_t optionIndex = firstVisibleOption + row;
            if (optionIndex >= options.size()) break;

            const Rectangle optionRec = {
                listRec.x, listRec.y + static_cast<float>(row) * rowHeight, listRec.width, rowHeight};
            const bool selected = selectedIndex && *selectedIndex == optionIndex;
            const bool highlighted = highlightedIndex && *highlightedIndex == optionIndex;

            DrawRectangleRec(
                optionRec,
                selected      ? CONTROL_SELECTED_BG
                : highlighted ? CONTROL_HOVER_BG
                              : CONTROL_BG_ACTIVE);
            if (row > 0)
            {
                DrawLineEx(
                    Vector2{optionRec.x, optionRec.y},
                    Vector2{optionRec.x + optionRec.width, optionRec.y},
                    1.0f,
                    Color{226, 232, 240, 255});
            }

            const Rectangle textClip = {
                optionRec.x + 6.0f, optionRec.y, std::max(0.0f, optionRec.width - 12.0f), optionRec.height};
            const ScissorScope optionScissor{textClip};
            DrawTextEx(
                fontInfo.font,
                options[optionIndex].c_str(),
                Vector2{textClip.x, optionRec.y + (optionRec.height - fontInfo.fontSize) * 0.5f},
                fontInfo.fontSize,
                fontInfo.fontSpacing,
                fontInfo.color);
        }
    }

    DropdownList::~DropdownList()
    {
        if (activeDropdown == this)
        {
            activeDropdown = nullptr;
        }
    }

    void DropdownList::SetOptions(std::vector<std::string> _options)
    {
        options = std::move(_options);
        if (options.empty())
        {
            selectedIndex.reset();
            firstVisibleOption = 0;
            close();
            return;
        }

        if (!selectedIndex || *selectedIndex >= options.size())
        {
            selectedIndex = 0;
        }
        clampScrollOffset();
    }

    void DropdownList::SetSelectedIndex(const std::size_t index, const bool publish)
    {
        if (index >= options.size()) return;

        const bool changed = !selectedIndex || *selectedIndex != index;
        selectedIndex = index;
        const std::size_t visibleCount = visibleOptionCount();
        if (visibleCount > 0)
        {
            if (index < firstVisibleOption)
            {
                firstVisibleOption = index;
            }
            else if (index >= firstVisibleOption + visibleCount)
            {
                firstVisibleOption = index - visibleCount + 1;
            }
        }
        clampScrollOffset();

        if (publish && changed)
        {
            onSelectionChanged.Publish(index, options[index]);
        }
    }

    void DropdownList::SetMaxVisibleOptions(const std::size_t count)
    {
        maxVisibleOptions = std::max<std::size_t>(1, count);
        clampScrollOffset();
    }

    void DropdownList::SetDropDirection(const DropDirection direction)
    {
        dropDirection = direction;
    }

    const std::vector<std::string>& DropdownList::GetOptions() const
    {
        return options;
    }

    std::optional<std::size_t> DropdownList::GetSelectedIndex() const
    {
        return selectedIndex;
    }

    const std::string& DropdownList::GetSelectedText() const
    {
        if (!selectedIndex || *selectedIndex >= options.size()) return emptyDropdownText();
        return options[*selectedIndex];
    }

    bool DropdownList::IsExpanded() const
    {
        return expanded;
    }

    bool DropdownList::ActiveDropdownCapturesCursor(const Vector2 point)
    {
        return activeDropdown && activeDropdown->CapturesCursor(point);
    }

    bool DropdownList::CapturesCursor(const Vector2 point) const
    {
        return expanded && (PointInsideRect(rec, point) || PointInsideRect(expandedListRec(), point));
    }

    void DropdownList::OnClick()
    {
        CellElement::OnClick();
        if (options.empty()) return;

        if (activeDropdown && activeDropdown != this)
        {
            activeDropdown->close();
        }

        expanded = !expanded;
        activeDropdown = expanded ? this : nullptr;
        if (!expanded) return;

        if (selectedIndex)
        {
            SetSelectedIndex(*selectedIndex);
        }
        clampScrollOffset();
    }

    void DropdownList::UpdateDimensions()
    {
        updateFontScaling();
        rec = {
            parent->GetRec().x + parent->padding.left,
            parent->GetRec().y + parent->padding.up,
            parent->GetRec().width - parent->padding.left - parent->padding.right,
            parent->GetRec().height - parent->padding.up - parent->padding.down};
    }

    void DropdownList::Draw2D()
    {
        handleExpandedInput();
        drawCollapsed();
        if (expanded)
        {
            engine->QueueOverlayDraw([this]() { drawExpandedList(); });
        }
    }

    DropdownList::DropdownList(
        GameUIEngine* _engine,
        TableCell* _parent,
        std::vector<std::string> _options,
        const std::size_t _selectedIndex,
        const TextBox::FontInfo& _fontInfo,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment)
        : CellElement(_engine, _parent, _vertAlignment, _horiAlignment),
          options(std::move(_options)),
          fontInfo(_fontInfo)
    {
        fontInfo.color = fontInfo.color.a == 0 ? CONTROL_TEXT : fontInfo.color;
        if (!options.empty())
        {
            selectedIndex = std::min(_selectedIndex, options.size() - 1);
        }
        updateFontScaling();
        SetTextureFilter(fontInfo.font.texture, TEXTURE_FILTER_BILINEAR);
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
