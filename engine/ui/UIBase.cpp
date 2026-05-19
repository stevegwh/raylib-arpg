//
// UI base types implementation. See UIBase.hpp.
//

#include "UIBase.hpp"

#include "../GameUiEngine.hpp" // for full TableCell (parent->GetRec)
#include "UIState.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace sage
{
    namespace
    {
        // Process-wide scissor stack. Each ScissorScope pushes an intersected
        // clip rect and re-issues BeginScissorMode; on destruction it pops and
        // restores the previous rect (or ends scissoring if the stack is now
        // empty). This works around raylib's non-stacking scissor API.
        std::vector<Rectangle>& ScissorStack()
        {
            static std::vector<Rectangle> stack;
            return stack;
        }

        Rectangle IntersectRects(const Rectangle& a, const Rectangle& b)
        {
            const float x1 = std::max(a.x, b.x);
            const float y1 = std::max(a.y, b.y);
            const float x2 = std::min(a.x + a.width, b.x + b.width);
            const float y2 = std::min(a.y + a.height, b.y + b.height);
            if (x2 <= x1 || y2 <= y1) return {x1, y1, 0.0f, 0.0f};
            return {x1, y1, x2 - x1, y2 - y1};
        }
    } // namespace

    ScissorScope::ScissorScope(Rectangle clip)
    {
        auto& stack = ScissorStack();
        Rectangle effective = clip;
        if (!stack.empty())
        {
            effective = IntersectRects(stack.back(), clip);
        }
        empty = effective.width <= 0.0f || effective.height <= 0.0f;
        stack.push_back(effective);
        BeginScissorMode(
            static_cast<int>(effective.x),
            static_cast<int>(effective.y),
            static_cast<int>(std::max(0.0f, effective.width)),
            static_cast<int>(std::max(0.0f, effective.height)));
    }

    ScissorScope::~ScissorScope()
    {
        auto& stack = ScissorStack();
        if (!stack.empty()) stack.pop_back();
        if (stack.empty())
        {
            EndScissorMode();
        }
        else
        {
            const auto& prev = stack.back();
            BeginScissorMode(
                static_cast<int>(prev.x),
                static_cast<int>(prev.y),
                static_cast<int>(std::max(0.0f, prev.width)),
                static_cast<int>(std::max(0.0f, prev.height)));
        }
    }

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
          engine(_engine)
    {
        // state is default-initialised to IdleState (the variant's first alternative).
    }
} // namespace sage
