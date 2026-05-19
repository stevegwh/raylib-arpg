#pragma once

#include "EditorAssetCatalog.hpp"
#include "engine/components/NavigationGridSquare.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <optional>
#include <string>

namespace sage
{
    class EngineSystems;
}

namespace sage::editor
{
    class EditorPlacementController
    {
        EngineSystems* sys;
        EditorAssetCatalog& assets;
        float gridSurfaceY = 0.0f;
        float gridHalfExtent = 50.0f;
        entt::entity gridPlacementSurfaceEntity = entt::null;
        float placementRotationY = 0.0f;
        float placementScale = 1.0f;
        std::optional<GridSquare> hoveredGridSquare;
        std::optional<Vector3> snappedPlacementPosition;

        void createGridPlacementSurface();
        void sizeGridToLoadedScene();
        [[nodiscard]] std::string makePlacedLabel(entt::entity entity) const;

      public:
        EditorPlacementController(EngineSystems* sys, EditorAssetCatalog& assets);

        void Initialize();
        void RefreshTarget();
        void ResetTransform();
        void AdjustGridSurfaceY(float amount);
        void AdjustRotation(float amount);
        void AdjustScale(float amount);
        void SyncFromEntity(entt::entity entity);
        std::optional<entt::entity> PlaceSelectedMesh();
        void DrawPreview() const;
        void DrawGridAndAxes() const;

        [[nodiscard]] entt::entity GridSurfaceEntity() const;
        [[nodiscard]] const std::optional<GridSquare>& HoveredGridSquare() const;
        [[nodiscard]] const std::optional<Vector3>& SnappedPlacementPosition() const;
        [[nodiscard]] float RotationY() const;
        [[nodiscard]] float Scale() const;
    };
} // namespace sage::editor
