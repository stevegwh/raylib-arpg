//
// Concrete CellElement subclasses: TextBox, TitleBar, ImageBox, GameWindowButton,
// CloseButton.
//

#pragma once

#include "../ResourceManager.hpp"
#include "UIBase.hpp"

#include <functional>
#include <optional>
#include <string>

namespace sage
{
    class Window;
    class TooltipWindow;

    class TextBox : public CellElement
    {
      public:
        enum class OverflowBehaviour
        {
            SHRINK_TO_FIT, // Text will be shrunk down to a minimum size (then left to overflow)
            WORD_WRAP // Words will be broken onto a new line if they do not fit. Will be shrunk if the new line
                      // also does not fit. Does not truncate words.
            // SHRINK_THEN_EXTEND ?
        };

        struct FontInfo
        {
            float baseFontSize;
            float fontSize;
            float fontSpacing;
            Font font;
            Color color;
            static constexpr float minFontSize = 16.0f;
            static constexpr float maxFontSize = 72.0f;
            OverflowBehaviour overflowBehaviour;
            FontInfo()
                : baseFontSize(16),
                  fontSize(16),
                  fontSpacing(1.0f),
                  font(GetFontDefault()),
                  color(BLACK),
                  overflowBehaviour(OverflowBehaviour::SHRINK_TO_FIT)
            {
            }
        };
        [[nodiscard]] const std::string& GetContent() const;
        virtual void SetContent(const std::string& _content);
        [[nodiscard]] Font GetFont() const;
        void UpdateFontScaling();
        void UpdateDimensions() override;
        void Draw2D() override;
        ~TextBox() override = default;

        // Returns the rendered height (in viewport pixels) of `text` wrapped at
        // `availableWidth`, using the same algorithm as UpdateDimensions's
        // WORD_WRAP path. Used by tooltip factories to size containers to fit
        // their text.
        [[nodiscard]] static float WrappedHeight(
            const FontInfo& info, const std::string& text, float availableWidth);

        TextBox(
            GameUIEngine* _engine,
            TableCell* _parent,
            const FontInfo& _fontInfo = FontInfo(),
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);

      protected:
        Shader sdfShader;
        FontInfo fontInfo;
        std::string content;
    };

    class TextInput : public TextBox
    {
        static TextInput* activeInput;
        std::function<void(const std::string&)> onSubmit;
        std::string editStartContent;
        std::size_t caretIndex = 0;
        bool editing = false;

        void processKeyboardInput();
        void commitEdit();
        void cancelEdit();
        void setCaretFromMousePosition(Vector2 mousePosition);

      public:
        void SetContent(const std::string& _content) override;
        void OnClick() override;
        void UpdateDimensions() override;
        void Draw2D() override;
        [[nodiscard]] bool IsEditing() const;
        [[nodiscard]] static bool AnyEditing();
        void SetOnSubmit(std::function<void(const std::string&)> callback);

        TextInput(
            GameUIEngine* _engine,
            TableCell* _parent,
            std::function<void(const std::string&)> callback,
            const FontInfo& _fontInfo = FontInfo(),
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
    };

    class TitleBar final : public TextBox
    {

      public:
        std::optional<Window*> draggedWindow;
        ~TitleBar() override = default;
        void OnDragStart() override;
        void DragUpdate() override;
        void OnDrop(CellElement* droppedElement) override;
        TitleBar(GameUIEngine* _engine, TableCell* _parent, const FontInfo& _fontInfo);
    };

    class ImageBox : public CellElement
    {
      public:
        enum class OverflowBehaviour
        {
            ALLOW_OVERFLOW,
            SHRINK_TO_FIT,
            SHRINK_ROW_TO_FIT,
            SHRINK_COL_TO_FIT
        };
        void OnIdleStart() override;
        void OnHoverStart() override;
        void OnHoverStop() override;
        void OnDragStart() override;
        void DragDraw() override;
        void OnDrop(CellElement* droppedElement) override;
        // void SetOverflowBehaviour(OverflowBehaviour _behaviour);
        void SetHoverShader();
        void SetGrayscale();
        void RemoveShader();
        void UpdateDimensions() override;
        void Draw2D() override;
        ImageBox(
            GameUIEngine* _engine,
            TableCell* _parent,
            const Texture& _tex,
            OverflowBehaviour _behaviour = OverflowBehaviour::SHRINK_TO_FIT,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
        ImageBox(
            GameUIEngine* _engine,
            TableCell* _parent,
            OverflowBehaviour _behaviour = OverflowBehaviour::SHRINK_TO_FIT,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
        ~ImageBox() override = default;

      protected:
        double hoverTimer = 0;
        float hoverTimerThreshold = 0.8;
        std::optional<TooltipWindow*> tooltipWindow;
        OverflowBehaviour overflowBehaviour;
        std::optional<Shader> shader;
        virtual void updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space);

      private:
        [[nodiscard]] Dimensions calculateAvailableSpace() const;
        [[nodiscard]] float calculateAspectRatio() const;
        [[nodiscard]] Dimensions calculateInitialDimensions(const Dimensions& space) const;
        [[nodiscard]] Vector2 calculateAlignmentOffset(
            const Dimensions& dimensions, const Dimensions& space) const;
        void shrinkRowToFit() const;
        void shrinkColToFit() const;
        [[nodiscard]] size_t findMyColumnIndex() const;
        [[nodiscard]] Dimensions handleOverflow(const Dimensions& dimensions, const Dimensions& space) const;
    };

    class GameWindowButton : public ImageBox
    {
        Window* toOpen{};

      public:
        void OnClick() override;
        GameWindowButton(GameUIEngine* _engine, TableCell* _parent, const Texture& _tex, Window* _toOpen);
    };

    class CloseButton final : public ImageBox
    {
        bool closeDeletesWindow = false;

      public:
        ~CloseButton() override = default;
        void OnClick() override;
        CloseButton(
            GameUIEngine* _engine, TableCell* _parent, const Texture& _tex, bool _closeDeletesWindow = false);
    };
} // namespace sage
