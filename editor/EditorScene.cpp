#include "EditorScene.hpp"

#include "EditorComponents.hpp"
#include "EditorTransformMath.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/Cursor.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/Light.hpp"
#include "engine/systems/RenderSystem.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "engine/UserInput.hpp"

#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <format>
#include <vector>

namespace sage
{
    namespace
    {
        constexpr float GRID_SURFACE_Y_STEP = 1.0f;
        constexpr float EDITOR_FOCUS_CAMERA_DISTANCE = 38.0f;
        constexpr float EDITOR_FOCUS_RADIUS_PADDING = 2.4f;

        struct FocusTarget
        {
            Vector3 position{};
            float radius = 1.0f;
        };

        FocusTarget focusTargetFromBounds(const BoundingBox& bounds)
        {
            const Vector3 center = editor::BoundingBoxCenter(bounds);
            const Vector3 halfSize = Vector3Scale(Vector3Subtract(bounds.max, bounds.min), 0.5f);
            return {.position = center, .radius = std::max(1.0f, Vector3Length(halfSize))};
        }

        void applyInspectorEditMode(
            std::vector<editor::InspectedComponent>& inspectedComponents, const bool editMode)
        {
            for (auto& component : inspectedComponents)
            {
                for (auto& field : component.fields)
                {
                    field.editable = field.editable && editMode;
                }
            }
        }
    } // namespace

    const editor::PlaceableAsset& EditorScene::selectedPlaceable() const
    {
        return assetCatalog->Selected();
    }

    bool EditorScene::isPlaceState() const
    {
        return editorModes->IsPlaceMode();
    }

    bool EditorScene::isEditState() const
    {
        return editorModes->IsEditMode();
    }

    std::string EditorScene::describeMode() const
    {
        if (isPlaceState()) return "Place";
        if (isEditState()) return "Edit: " + transformEditor->DescribeMode();
        return "Select";
    }

    std::string EditorScene::describeSelectedAsset() const
    {
        if (isPlaceState()) return selectedPlaceable().displayName;
        if (selection->HasSelection()) return describeSelectedSceneEntity();
        return "None";
    }

    std::string EditorScene::describeCursorPosition() const
    {
        if (const auto& position = placementController->SnappedPlacementPosition(); position.has_value())
        {
            return std::format("{:.2f}, {:.2f}, {:.2f}", position->x, position->y, position->z);
        }
        if (const auto& square = placementController->HoveredGridSquare(); square.has_value())
        {
            return std::format("row {}, col {}", square->row, square->col);
        }
        return "-";
    }

    std::string EditorScene::describeEntity(const entt::entity entity) const
    {
        return hierarchyTree->DescribeEntity(entity);
    }

    std::string EditorScene::describeSelectedSceneEntity() const
    {
        const auto entity = selection->ActiveTransformEntity();
        return entity.has_value() ? describeEntity(*entity) : "None";
    }

    void EditorScene::applyLitShaderToLoadedRenderables() const
    {
        for (const auto entity : sys->registry->view<Renderable>())
        {
            if (sys->registry->any_of<UberShaderComponent>(entity)) continue;
            auto& renderable = sys->registry->get<Renderable>(entity);
            if (renderable.GetModel() == nullptr) continue;
            auto& uber =
                sys->registry->emplace<UberShaderComponent>(entity, renderable.GetModel()->GetMaterialCount());
            uber.SetFlagAll(UberShaderComponent::Flags::Lit);
        }
    }

    void EditorScene::giveTransformsToLights() const
    {
        for (const auto entity : sys->registry->view<Light>())
        {
            if (sys->registry->any_of<sgTransform>(entity)) continue;
            const auto position = sys->registry->get<Light>(entity).position;
            sys->registry->emplace<sgTransform>(entity);
            sys->registry->get<sage::sgTransform>(entity).SetWorldPos(position);
        }
    }

    void EditorScene::refreshOverlay() const
    {
        const auto defaultsStatus = modelDefaults->Status(describeSelectedAsset());
        gui->SetOverlayStatus(describeMode(), describeCursorPosition());
        gui->SetAssetDefaultsStatus(
            defaultsStatus.assetName, defaultsStatus.height, defaultsStatus.rotation, defaultsStatus.scale);
        gui->SetSelectedAsset(
            isPlaceState() ? std::optional<std::size_t>{assetCatalog->SelectedIndex()} : std::nullopt);
    }

    void EditorScene::refreshSceneWindows() const
    {
        const auto activeEntity = selection->ActiveTransformEntity();
        auto inspectedComponents = activeEntity.has_value()
                                       ? inspectorRegistry.Inspect(*sys->registry, *activeEntity)
                                       : std::vector<editor::InspectedComponent>{};
        applyInspectorEditMode(inspectedComponents, isEditState());

        gui->SetHierarchy(hierarchyTree->CollectSceneObjectEntries(), activeEntity);
        gui->SetInspector(describeSelectedSceneEntity(), inspectedComponents);
    }

    void EditorScene::focusSelectedObject() const
    {
        const auto selectedEntity = selection->ActiveTransformEntity();
        if (!selectedEntity.has_value()) return;

        const auto entity = *selectedEntity;
        FocusTarget target{.position = sys->registry->get<sgTransform>(entity).GetWorldPos()};

        if (sys->registry->any_of<Collideable>(entity))
        {
            target = focusTargetFromBounds(sys->registry->get<Collideable>(entity).worldBoundingBox);
        }
        else if (sys->registry->any_of<Renderable>(entity))
        {
            const auto& transform = sys->registry->get<sgTransform>(entity);
            const auto& renderable = sys->registry->get<Renderable>(entity);
            if (const auto* model = renderable.GetModel(); model != nullptr)
            {
                const Matrix entityMatrix = editor::BuildRenderableEntityMatrix(
                    transform.GetWorldPos(), transform.GetWorldRot(), transform.GetScale());
                const Matrix worldMatrix = MatrixMultiply(model->GetTransform(), entityMatrix);
                target = focusTargetFromBounds(
                    editor::TransformBoundingBoxByCorners(model->CalcLocalBoundingBox(), worldMatrix));
            }
        }

        const float focusDistance =
            std::max(EDITOR_FOCUS_CAMERA_DISTANCE, target.radius * EDITOR_FOCUS_RADIUS_PADDING);
        sys->camera->FocusPoint(target.position, focusDistance);
    }

    void EditorScene::focusSelectedObjectInHierarchy() const
    {
        const auto selectedEntity = selection->ActiveTransformEntity();
        if (!selectedEntity.has_value()) return;
        gui->FocusHierarchyOnEntity(*selectedEntity);
    }

    void EditorScene::Update()
    {
        sys->collisionSystem->Update();
        sys->audioManager->Update();
        sys->userInput->ListenForInput();
        if (sys->UI().IsMouseOverWindow() || !sys->settings->IsPointInRenderViewport(GetMousePosition()))
        {
            sys->camera->ScrollDisable();
        }
        else
        {
            sys->camera->ScrollEnable();
        }
        sys->camera->Update();
        sys->cursor->Update();
        editorModes->RefreshPlacementTarget();

        // TODO: Should be part of some mode
        if (!TextInput::AnyEditing())
        {
            if (IsKeyPressed(KEY_EQUAL))
            {
                editorModes->AdjustGridSurfaceY(GRID_SURFACE_Y_STEP);
            }
            if (IsKeyPressed(KEY_MINUS))
            {
                editorModes->AdjustGridSurfaceY(-GRID_SURFACE_Y_STEP);
            }
        }

        editorModes->Update();

        refreshOverlay();
        refreshSceneWindows();
        sys->UI().Update();
    }

    void EditorScene::Draw3D() const
    {
        sys->renderSystem->Draw();
        placementController->DrawGridAndAxes();
        editorModes->Draw3D();

        const auto selectedEntity = selection->ActiveTransformEntity();
        if (selectedEntity.has_value() && sys->registry->any_of<Collideable>(*selectedEntity))
        {
            DrawBoundingBox(sys->registry->get<Collideable>(*selectedEntity).worldBoundingBox, ORANGE);
        }
    }

    void EditorScene::Draw2D() const
    {
        sys->UI().Draw2D();
    }

    void EditorScene::DrawOverlay2D() const
    {
        gui->DrawSceneViewInfo();
    }

    bool EditorScene::HandleEscapePressed()
    {
        return editorModes->HandleEscapePressed();
    }

    void EditorScene::SetSceneName(const std::string& sceneName) const
    {
        gui->SetSceneName(sceneName);
    }

    EditorScene::EditorScene(EngineSystems* _sys) : sys(_sys)
    {
        editor::RegisterDefaultInspectorComponents(inspectorRegistry);
        assetCatalog = std::make_unique<editor::EditorAssetCatalog>(
            editor::EditorAssetCatalog::FromLoadedModels());
        assetCatalog->LoadDefaults();
        modelDefaults = std::make_unique<editor::EditorModelDefaultsController>(
            *assetCatalog, [this]() { return isPlaceState(); }, [this]() { refreshOverlay(); });
        selection = std::make_unique<editor::EditorSelection>(sys);
        pickingService = std::make_unique<editor::EditorPickingService>(sys);
        entityOperations = std::make_unique<editor::EditorEntityOperations>(sys);
        hierarchyTree = std::make_unique<editor::EditorHierarchyTree>(sys);
        placementController = std::make_unique<editor::EditorPlacementController>(sys, *assetCatalog);

        applyLitShaderToLoadedRenderables();
        giveTransformsToLights();
        placementController->Initialize();

        transformEditor = std::make_unique<editor::EditorTransformEditor>(sys, [this](const entt::entity entity) {
            if (editorModes)
            {
                editorModes->OnTransformApplied(entity);
            }
        });
        editorModes = std::make_unique<editor::EditorModeStateMachine>(*this, *transformEditor);

        gui = std::make_unique<editor::EditorGui>(
            &sys->UI(),
            sys->settings,
            assetCatalog->AssetEntries(),
            [this](const std::size_t index) { editorModes->SelectPlaceable(index); },
            [this](const entt::entity entity) { editorModes->SelectSceneEntity(entity); },
            modelDefaults->Callbacks());
        refreshOverlay();
        refreshSceneWindows();
    }

    EditorScene::~EditorScene() = default;
} // namespace sage
