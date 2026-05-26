//
// UI input state machine. Each CellElement owns a `UIState` (variant of POD
// structs). Behaviour lives in free functions (updateUIState / drawUIState /
// transitionTo) so the states themselves are plain data and tests don't need
// to subclass anything.
//
// Inputs to the state machine come via InputSnapshot, populated once per frame
// from raylib globals at the top of GameUIEngine::Update. State code never
// touches raylib's input globals directly.
//

#pragma once

#include "../Timer.hpp"

#include "raylib.h"

#include <variant>

namespace sage
{
    class CellElement;
    class GameUIEngine;
    struct Settings;

    struct IdleState
    {
    };

    struct HoverState
    {
        Timer dragTimer;
    };

    struct DragDelayState
    {
        Timer dragTimer;
    };

    struct DragState
    {
    };

    using UIState = std::variant<IdleState, HoverState, DragDelayState, DragState>;

    // Per-frame mouse / time snapshot. Captured once at the top of the UI update
    // pass; passed (via GameUIEngine::Input()) to state functions so they don't
    // reach into raylib globals.
    struct InputSnapshot
    {
        Vector2 mousePos{};
        bool leftDown = false;
        bool leftReleased = false;
        float frameTime = 0.0f;

        // Captures the current frame from raylib's globals and normalizes mouse
        // coordinates into the app viewport used by UI layout/drawing.
        static InputSnapshot Capture(const Settings& settings);
    };

    // Drives the state machine for one CellElement. Handles transitions and
    // per-frame side effects. Skips work if `el.stateLocked` is true.
    void updateUIState(CellElement& el, GameUIEngine& engine, const InputSnapshot& input);

    // Calls state-specific draw (currently only Drag draws an overlay).
    void drawUIState(CellElement& el);

    // Runs Exit on the current state, replaces it, runs Enter on the new state.
    // Honours `el.stateLocked` (no-op while locked).
    void transitionTo(CellElement& el, GameUIEngine& engine, UIState newState);
} // namespace sage
