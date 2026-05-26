//
// Edit-mode behaviour: gizmo input, pivot toggling, snapshot/restore,
// pivot-aware matrix application, inspector-input commit path. Owned by
// EditorScene; driven by EditorModeStateMachine during EditorEditState.
//

#pragma once

#include "EditGizmo.hpp"
#include "EditorModeStateMachine.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <functional>
#include <string>

namespace sage
{
    class EngineSystems;
}

namespace sage::editor
{
    class EditorTransformEditor
    {
      public:
        enum class PivotMode
        {
            World,
            LocalCenter
        };

        // Invoked after every mutation that touches the selected entity. The
        // scene uses it to keep the placement overlay (snap markers,
        // placement rotation/scale cache) and GUI windows in sync with the
        // entity's new transform.
        using OnApplied = std::function<void(entt::entity)>;

        EditorTransformEditor(EngineSystems* sys, OnApplied onApplied);

        // Lifecycle. EnterEditMode populates the snapshot fields used by RestoreSnapshot.
        void EnterEditMode(entt::entity entity, EditorEditState& outSnapshot);
        void ExitEditMode();
        void RestoreSnapshot(const EditorEditState& snapshot);

        // Per-frame gizmo update / draw. Update is a no-op unless a drag is
        // currently active; the state machine calls TryStartDrag separately on
        // mouse-down to begin one.
        void Update(entt::entity entity);
        bool TryStartDrag(entt::entity entity, Vector2 mousePosition);
        void Draw3D(entt::entity entity) const;

        // Mode + pivot.
        void SetMode(EditGizmo::Mode);
        void TogglePivotMode();
        [[nodiscard]] EditGizmo::Mode Mode() const
        {
            return mode;
        }
        [[nodiscard]] PivotMode Pivot() const
        {
            return pivotMode;
        }
        [[nodiscard]] std::string DescribeMode() const;
        [[nodiscard]] Vector3 PivotWorldPosition(entt::entity entity) const;
        [[nodiscard]] bool IsGizmoDragging() const
        {
            return gizmo.IsDragging();
        }

        // Keyboard / arrow-key paths driven by the state machine.
        void AdjustPosition(entt::entity entity, Vector3 worldDelta);
        void AdjustRotationAxis(entt::entity entity, EditGizmo::Axis axis, float degrees);
        void AdjustScale(entt::entity entity, float delta);

      private:
        EngineSystems* sys;
        OnApplied onApplied;
        EditGizmo gizmo;
        EditGizmo::Mode mode = EditGizmo::Mode::Translate;
        PivotMode pivotMode = PivotMode::LocalCenter;

        void updateEntityCollisionBounds(entt::entity entity) const;
        void notify(entt::entity entity) const;
    };
} // namespace sage::editor
