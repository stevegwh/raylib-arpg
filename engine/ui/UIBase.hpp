//
// UI base types: dimensions, padding, alignment enums, the UIElement / CellElement
// abstract bases. CellElement holds a unique_ptr<UIState>, so UIState.hpp must be
// complete here.
//

#pragma once

#include "../Event.hpp"
#include "UIState.hpp"

#include "raylib.h"

#include <memory>
#include <vector>

namespace sage
{
    class GameUIEngine;
    class TableCell;

    enum class TextureStretchMode
    {
        NONE,
        STRETCH, // maximises or minimises image to fit, ignores aspect ratio, whole image is kept
        FILL,    // maximises or minimises image to fit, maintains aspect ratio (parts that fall outside of aspect
                 // ratio are clipped)
        TILE     // Repeats image if not big enough to fill rec
    };

    struct Dimensions
    {
        float width = 0;
        float height = 0;
    };

    enum class HoriAlignment
    {
        LEFT,
        RIGHT,
        CENTER,
        WINDOW_CENTER
    };

    enum class VertAlignment
    {
        TOP,
        MIDDLE,
        BOTTOM
    };

    struct Padding
    {
        float up = 0;
        float down = 0;
        float left = 0;
        float right = 0;
    };

    class UIElement
    {
      protected:
      public:
        Rectangle rec{};
        [[nodiscard]] const Rectangle& GetRec() const
        {
            return rec;
        }
        virtual void OnIdleStart() {};
        virtual void OnIdleStop() {};
        virtual void OnHoverStart();
        virtual void OnHoverStop();
        virtual ~UIElement() = default;
        UIElement() = default;
    };

    // RAII helper around raylib's BeginScissorMode / EndScissorMode. raylib's
    // EndScissorMode disables scissoring entirely instead of restoring the
    // previous rect, so naive nested clips would tear the parent's clip the
    // moment an inner element finished drawing. ScissorScope keeps a per-frame
    // stack: each push intersects with the current top and re-issues the
    // scissor; each pop restores the previous rect (or disables scissoring if
    // we're back at the bottom of the stack).
    class ScissorScope
    {
        bool empty = false;

      public:
        explicit ScissorScope(Rectangle clip);
        ~ScissorScope();
        ScissorScope(const ScissorScope&) = delete;
        ScissorScope& operator=(const ScissorScope&) = delete;
        ScissorScope(ScissorScope&&) = delete;
        ScissorScope& operator=(ScissorScope&&) = delete;
    };

    class CellElement : public UIElement
    {
      protected:
        Event<> onMouseClicked;
        Vector2 dragOffset{};
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;

      public:
        TableCell* parent{};
        GameUIEngine* engine{};

        UIState state{IdleState{}};
        Texture tex{};
        bool canReceiveDragDrops = false;
        bool draggable = false;
        bool beingDragged = false;
        bool stateLocked = false;
        float dragDelayTime = 0.1;

        virtual void RetrieveInfo() {};
        virtual void OnClick();
        virtual void HoverUpdate();
        virtual void OnDragStart();
        virtual void DragUpdate() {};
        virtual void DragDraw() {};
        virtual void OnDrop(CellElement* receiver);
        virtual void ReceiveDrop(CellElement* droppedElement);
        virtual void UpdateDimensions();
        virtual void Draw2D() = 0;
        explicit CellElement(
            GameUIEngine* _engine,
            TableCell* _parent,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
        ~CellElement() override = default;
        friend class TableCell;
    };
} // namespace sage
