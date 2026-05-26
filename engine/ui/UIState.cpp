//
// UI state machine implementation. See UIState.hpp.
//

#include "UIState.hpp"

#include "../Cursor.hpp"
#include "../GameUiEngine.hpp"
#include "../Settings.hpp"
#include "../slib.hpp"

#include "raylib.h"

#include <optional>
#include <utility>

namespace sage
{
    namespace
    {
        template <class... Ts>
        struct overloaded : Ts...
        {
            using Ts::operator()...;
        };
        template <class... Ts>
        overloaded(Ts...) -> overloaded<Ts...>;

        // Side effects when entering a state.
        void runEnter(UIState& s, CellElement& el, GameUIEngine& engine)
        {
            std::visit(
                overloaded{
                    [&](IdleState&) { el.OnIdleStart(); },
                    [&](HoverState&) { el.OnHoverStart(); },
                    [&](DragDelayState& d) {
                        el.OnHoverStart();
                        d.dragTimer.SetMaxTime(el.dragDelayTime);
                        d.dragTimer.SetAutoFinish(false);
                    },
                    [&](DragState&) {
                        engine.SetDraggedObject(&el);
                        el.OnDragStart();
                    }},
                s);
        }

        // Side effects when leaving a state.
        void runExit(UIState& s, CellElement& el, GameUIEngine& engine)
        {
            std::visit(
                overloaded{
                    [&](IdleState&) { el.OnIdleStop(); },
                    [&](HoverState&) { el.OnHoverStop(); },
                    [&](DragDelayState&) {
                        engine.ReleaseHoveredDraggableLock();
                        el.OnHoverStop();
                    },
                    [&](DragState&) {
                        auto* cell = engine.GetCellUnderCursor();
                        el.OnDrop(cell);
                        engine.ClearDraggedObject();
                    }},
                s);
        }
    } // namespace

    InputSnapshot InputSnapshot::Capture(const Settings& settings)
    {
        return InputSnapshot{
            settings.ScreenToViewportPosition(GetMousePosition()),
            IsMouseButtonDown(MOUSE_BUTTON_LEFT),
            IsMouseButtonReleased(MOUSE_BUTTON_LEFT),
            GetFrameTime()};
    }

    void transitionTo(CellElement& el, GameUIEngine& engine, UIState newState)
    {
        if (el.stateLocked) return;
        runExit(el.state, el, engine);
        el.state = std::move(newState);
        runEnter(el.state, el, engine);
    }

    void updateUIState(CellElement& el, GameUIEngine& engine, const InputSnapshot& in)
    {
        // Compute the next state during visit; apply *after* visit so we don't
        // mutate the variant we're holding a reference into.
        std::optional<UIState> nextState;

        std::visit(
            overloaded{
                [&](IdleState&) {
                    if (PointInsideRect(el.parent->GetRec(), in.mousePos))
                    {
                        nextState = HoverState{};
                    }
                },
                [&](HoverState&) {
                    engine.cursor->DisableContextSwitching();
                    engine.cursor->Disable();

                    if (!PointInsideRect(el.parent->GetRec(), in.mousePos))
                    {
                        nextState = IdleState{};
                        return;
                    }

                    if (in.leftReleased)
                    {
                        el.OnClick();
                        nextState = IdleState{};
                        return;
                    }

                    if (in.leftDown && el.draggable)
                    {
                        nextState = DragDelayState{};
                        return;
                    }

                    el.HoverUpdate();
                },
                [&](DragDelayState& s) {
                    s.dragTimer.Update(in.frameTime);

                    if (!engine.ObjectBeingDragged() && in.leftDown)
                    {
                        if (s.dragTimer.HasExceededMaxTime())
                        {
                            nextState = DragState{};
                            return;
                        }
                        if (!s.dragTimer.IsRunning())
                        {
                            engine.ClaimHoveredDraggableLock(&el);
                            s.dragTimer.Start();
                        }
                        const auto held = engine.GetHoveredDraggableLock();
                        if (held && *held != &el)
                        {
                            nextState = IdleState{};
                        }
                    }
                    else
                    {
                        el.OnClick();
                        nextState = IdleState{};
                    }
                },
                [&](DragState&) {
                    if (!in.leftDown)
                    {
                        nextState = IdleState{};
                        return;
                    }
                    el.DragUpdate();
                }},
            el.state);

        if (nextState)
        {
            transitionTo(el, engine, std::move(*nextState));
        }
    }

    void drawUIState(CellElement& el)
    {
        std::visit(
            overloaded{
                [](IdleState&) {},
                [](HoverState&) {},
                [](DragDelayState&) {},
                [&](DragState&) { el.DragDraw(); }},
            el.state);
    }
} // namespace sage
