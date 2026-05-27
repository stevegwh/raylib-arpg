#include "EditorPlacementController.hpp"

#include "EditorComponents.hpp"
#include "EditorTransformMath.hpp"
#include "engine/Camera.hpp"
#include "engine/CollisionLayers.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/Cursor.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "engine/systems/TransformSystem.hpp"

#include "raymath.h"
#include "rlgl.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr float GRID_DEFAULT_HALF_EXTENT = 50.0f;
        constexpr float GRID_PLACEMENT_SURFACE_HALF_HEIGHT = 0.02f;
        constexpr float PLACEMENT_MIN_SCALE = 0.1f;
        constexpr Color PLACEMENT_PREVIEW_TINT = {255, 255, 255, 150};
        constexpr Color PLACEMENT_PREVIEW_BOUNDS_COLOR = {37, 99, 235, 210};

        Matrix BuildPlacementMatrix(const Vector3 position, const float rotationY, const float scale)
        {
            return MatrixMultiply(
                MatrixMultiply(MatrixScale(scale, scale, scale), MatrixRotateY(rotationY * DEG2RAD)),
                MatrixTranslate(position.x, position.y, position.z));
        }
    } // namespace

    EditorPlacementController::EditorPlacementController(EngineSystems* _sys, EditorAssetCatalog& _assets)
        : sys(_sys), assets(_assets)
    {
    }

    void EditorPlacementController::Initialize()
    {
        if (sys->registry->valid(gridPlacementSurfaceEntity))
        {
            sys->registry->destroy(gridPlacementSurfaceEntity);
            gridPlacementSurfaceEntity = entt::null;
        }
        sizeGridToLoadedScene();
        createGridPlacementSurface();
    }

    void EditorPlacementController::RefreshTarget()
    {
        hoveredGridSquare.reset();
        snappedPlacementPosition.reset();

        const auto& collision = sys->cursor->getFirstNaviCollision();
        if (!collision.hit) return;

        GridSquare square{};
        if (!sys->navigationGridSystem->WorldToGridSpace(collision.point, square)) return;

        const auto* gridSquare = sys->navigationGridSystem->GetGridSquare(square.row, square.col);
        if (!gridSquare) return;

        hoveredGridSquare = square;
        snappedPlacementPosition = {gridSquare->worldPosCentre.x, collision.point.y, gridSquare->worldPosCentre.z};
    }

    void EditorPlacementController::ResetTransform()
    {
        placementRotationY = 0.0f;
        placementScale = 1.0f;
        RefreshTarget();
    }

    void EditorPlacementController::AdjustGridSurfaceY(const float amount)
    {
        gridSurfaceY += amount;
        if (sys->registry->valid(gridPlacementSurfaceEntity) &&
            sys->registry->any_of<Collideable>(gridPlacementSurfaceEntity))
        {
            auto& collideable = sys->registry->get<Collideable>(gridPlacementSurfaceEntity);
            collideable.worldBoundingBox.min.y = gridSurfaceY - GRID_PLACEMENT_SURFACE_HALF_HEIGHT;
            collideable.worldBoundingBox.max.y = gridSurfaceY + GRID_PLACEMENT_SURFACE_HALF_HEIGHT;
        }
        sys->camera->SetFloorYOffset(gridSurfaceY);
        RefreshTarget();
    }

    void EditorPlacementController::AdjustRotation(const float amount)
    {
        placementRotationY += amount;
        if (placementRotationY >= 360.0f) placementRotationY -= 360.0f;
        if (placementRotationY < 0.0f) placementRotationY += 360.0f;
    }

    void EditorPlacementController::AdjustScale(const float amount)
    {
        placementScale = std::max(PLACEMENT_MIN_SCALE, placementScale + amount);
    }

    void EditorPlacementController::SyncFromEntity(const entt::entity entity)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        const auto& transform = sys->registry->get<sgTransform>(entity);
        const auto position = transform.GetWorldPos();
        placementRotationY = transform.GetWorldRot().y;
        placementScale = std::max(
            PLACEMENT_MIN_SCALE,
            (transform.GetScale().x + transform.GetScale().y + transform.GetScale().z) / 3.0f);

        GridSquare square{};
        if (sys->navigationGridSystem->WorldToGridSpace(position, square))
        {
            hoveredGridSquare = square;
            const auto* gridSquare = sys->navigationGridSystem->GetGridSquare(square.row, square.col);
            snappedPlacementPosition =
                gridSquare ? Vector3{gridSquare->worldPosCentre.x, position.y, gridSquare->worldPosCentre.z}
                           : position;
        }
        else
        {
            hoveredGridSquare.reset();
            snappedPlacementPosition = position;
        }
    }

    std::optional<entt::entity> EditorPlacementController::PlaceSelectedMesh()
    {
        if (!snappedPlacementPosition.has_value()) return std::nullopt;

        const auto& placeable = assets.Selected();
        const auto entity = sys->registry->create();
        sys->registry->emplace<EditorMapEntity>(entity);
        sys->registry->emplace<EditorObjectDescriptor>(
            entity,
            EditorObjectDescriptor{
                .name = makePlacedLabel(entity),
                .category = placeable.labelStem,
                .selectable = true,
                .visibleInHierarchy = true,
                .locked = false});
        sys->registry->emplace<AssetReference>(entity, AssetReference{.assetKey = placeable.modelKey});
        auto& transform = sys->registry->emplace<sgTransform>(entity);
        transform.position.world = *snappedPlacementPosition;
        transform.scale.world = {placementScale, placementScale, placementScale};
        transform.rotation.world = {0.0f, placementRotationY, 0.0f};

        auto model = ResourceManager::GetInstance().GetModelView(placeable.modelKey);
        auto& renderable =
            sys->registry->emplace<Renderable>(entity, std::move(model), assets.DefaultTransform(placeable));
        renderable.SetName(sys->registry->get<EditorObjectDescriptor>(entity).name);
        auto& uber =
            sys->registry->emplace<UberShaderComponent>(entity, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(UberShaderComponent::Flags::Lit);

        const auto localBounds = renderable.GetModel()->CalcLocalBoundingBox();
        const auto placementMatrix =
            BuildPlacementMatrix(*snappedPlacementPosition, placementRotationY, placementScale);
        auto& collideable = sys->registry->emplace<Collideable>(
            entity, localBounds, sys->registry->get<sgTransform>(entity).GetMatrixNoRot());
        collideable.worldBoundingBox = TransformBoundingBoxByCorners(localBounds, placementMatrix);
        collideable.SetCollisionLayer(collision_layers::Obstacle);
        collideable.blocksNavigation = true;
        collideable.isStatic = true;
        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);

        return entity;
    }

    void EditorPlacementController::DrawPreview() const
    {
        if (!snappedPlacementPosition.has_value()) return;

        const auto& placeable = assets.Selected();
        auto previewModel = ResourceManager::GetInstance().GetModelView(placeable.modelKey);
        previewModel.SetTransform(assets.SelectedDefaultTransform());
        previewModel.Draw(
            *snappedPlacementPosition,
            {0.0f, 1.0f, 0.0f},
            placementRotationY,
            {placementScale, placementScale, placementScale},
            PLACEMENT_PREVIEW_TINT);

        const auto previewBounds = TransformBoundingBoxByCorners(
            previewModel.CalcLocalBoundingBox(),
            BuildPlacementMatrix(*snappedPlacementPosition, placementRotationY, placementScale));
        DrawBoundingBox(previewBounds, PLACEMENT_PREVIEW_BOUNDS_COLOR);
    }

    void EditorPlacementController::DrawGridAndAxes() const
    {
        const int gridSlices = std::max(1, static_cast<int>(std::ceil(gridHalfExtent * 2.0f)));
        rlPushMatrix();
        rlTranslatef(0.0f, gridSurfaceY, 0.0f);
        DrawGrid(gridSlices, 10.0f);
        rlPopMatrix();

        DrawLine3D({0, 0.02f, 0}, {8, 0.02f, 0}, RED);
        DrawLine3D({0, 0.02f, 0}, {0, 8, 0}, GREEN);
        DrawLine3D({0, 0.02f, 0}, {0, 0.02f, 8}, BLUE);
    }

    entt::entity EditorPlacementController::GridSurfaceEntity() const
    {
        return gridPlacementSurfaceEntity;
    }

    const std::optional<GridSquare>& EditorPlacementController::HoveredGridSquare() const
    {
        return hoveredGridSquare;
    }

    const std::optional<Vector3>& EditorPlacementController::SnappedPlacementPosition() const
    {
        return snappedPlacementPosition;
    }

    float EditorPlacementController::RotationY() const
    {
        return placementRotationY;
    }

    float EditorPlacementController::Scale() const
    {
        return placementScale;
    }

    void EditorPlacementController::createGridPlacementSurface()
    {
        gridPlacementSurfaceEntity = sys->registry->create();
        const BoundingBox localBounds = {
            {-gridHalfExtent, -GRID_PLACEMENT_SURFACE_HALF_HEIGHT, -gridHalfExtent},
            {gridHalfExtent, GRID_PLACEMENT_SURFACE_HALF_HEIGHT, gridHalfExtent}};
        auto& collideable =
            sys->registry->emplace<Collideable>(gridPlacementSurfaceEntity, localBounds, MatrixIdentity());
        collideable.worldBoundingBox = {
            {-gridHalfExtent, gridSurfaceY - GRID_PLACEMENT_SURFACE_HALF_HEIGHT, -gridHalfExtent},
            {gridHalfExtent, gridSurfaceY + GRID_PLACEMENT_SURFACE_HALF_HEIGHT, gridHalfExtent}};
        collideable.SetCollisionLayer(collision_layers::GeometrySimple);
        collideable.isStatic = true;
    }

    void EditorPlacementController::sizeGridToLoadedScene()
    {
        bool hasBounds = false;
        BoundingBox sceneBounds{};
        for (const auto entity : sys->registry->view<EditorMapEntity, Collideable>())
        {
            const auto& c = sys->registry->get<Collideable>(entity);
            if (!hasBounds)
            {
                sceneBounds = c.worldBoundingBox;
                hasBounds = true;
                continue;
            }
            sceneBounds.min.x = std::min(sceneBounds.min.x, c.worldBoundingBox.min.x);
            sceneBounds.min.z = std::min(sceneBounds.min.z, c.worldBoundingBox.min.z);
            sceneBounds.max.x = std::max(sceneBounds.max.x, c.worldBoundingBox.max.x);
            sceneBounds.max.z = std::max(sceneBounds.max.z, c.worldBoundingBox.max.z);
        }

        float required = GRID_DEFAULT_HALF_EXTENT;
        if (hasBounds)
        {
            required = std::max(
                {std::abs(sceneBounds.min.x),
                 std::abs(sceneBounds.max.x),
                 std::abs(sceneBounds.min.z),
                 std::abs(sceneBounds.max.z),
                 GRID_DEFAULT_HALF_EXTENT});
        }

        gridHalfExtent = std::ceil(required);
        const int slices = static_cast<int>(std::ceil(gridHalfExtent * 2.0f));
        sys->navigationGridSystem->Init(slices, 1.0f);
    }

    std::string EditorPlacementController::makePlacedLabel(const entt::entity entity) const
    {
        return std::format("entity_{}", entt::to_integral(entity));
    }
} // namespace sage::editor
