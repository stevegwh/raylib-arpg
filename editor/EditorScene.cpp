#include "EditorScene.hpp"

#include "EditorComponents.hpp"
#include "EditorTransformMath.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/CollisionLayers.hpp"
#include "engine/Cursor.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/Light.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Settings.hpp"
#include "engine/UserInput.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/CollisionSystem.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "engine/systems/RenderSystem.hpp"
#include "engine/systems/TransformSystem.hpp"

#include "raylib.h"

#include <algorithm>
#include <format>
#include <limits>
#include <vector>

namespace sage
{
    namespace
    {
        constexpr float GRID_SURFACE_Y_STEP = 1.0f;
        constexpr float PLACEMENT_HEIGHT_STEP = 0.25f;
        constexpr float PLACEMENT_ROTATION_STEP = 15.0f;
        constexpr float PLACEMENT_SCALE_STEP = 0.1f;
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
        if (selectedSceneEntity.has_value()) return describeSelectedSceneEntity();
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

    std::string EditorScene::describeSelectedModelDefaultHeight() const
    {
        return isPlaceState() ? std::format("{:.2f}", selectedPlaceable().modelDefaultHeightOffset) : "0.00";
    }

    std::string EditorScene::describeSelectedModelDefaultRotation() const
    {
        return isPlaceState() ? std::format("{:.0f}", selectedPlaceable().modelDefaultRotationY) : "0";
    }

    std::string EditorScene::describeSelectedModelDefaultScale() const
    {
        return isPlaceState() ? std::format("{:.2f}", selectedPlaceable().modelDefaultScale) : "1.00";
    }

    std::string EditorScene::describeEntity(const entt::entity entity) const
    {
        return hierarchyModel->DescribeEntity(entity);
    }

    std::string EditorScene::describeSelectedSceneEntity() const
    {
        if (!selectedSceneEntity.has_value()) return "None";
        if (!sys->registry->valid(*selectedSceneEntity) ||
            !sys->registry->any_of<sgTransform>(*selectedSceneEntity))
        {
            return "None";
        }
        return describeEntity(*selectedSceneEntity);
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
            sys->transformSystem->SetPosition(entity, position);
        }
    }

    void EditorScene::refreshPlacementTarget()
    {
        placementController->RefreshTarget();
    }

    void EditorScene::refreshOverlay() const
    {
        gui->SetOverlayStatus(describeMode(), describeCursorPosition());
        gui->SetAssetDefaultsStatus(
            describeSelectedAsset(),
            describeSelectedModelDefaultHeight(),
            describeSelectedModelDefaultRotation(),
            describeSelectedModelDefaultScale());
        gui->SetSelectedAsset(isPlaceState() ? std::optional<std::size_t>{assetCatalog->SelectedIndex()} : std::nullopt);
    }

    void EditorScene::refreshSceneWindows() const
    {
        const auto activeEntity = selectedSceneEntity.has_value() && sys->registry->valid(*selectedSceneEntity) &&
                                          sys->registry->any_of<sgTransform>(*selectedSceneEntity)
                                      ? selectedSceneEntity
                                      : std::nullopt;
        const auto inspectedComponents =
            activeEntity.has_value() ? inspectorRegistry.InspectEntity(*sys->registry, *activeEntity)
                                     : std::vector<editor::InspectedComponent>{};

        gui->SetHierarchy(hierarchyModel->CollectSceneObjectEntries(), activeEntity);
        gui->SetInspector(describeSelectedSceneEntity(), inspectedComponents);
    }

    void EditorScene::resetPlacementTransform()
    {
        placementController->ResetTransform();
    }

    void EditorScene::selectPlaceable(const std::size_t index)
    {
        if (index >= assetCatalog->Size()) return;
        editorModes->ChangeState(editor::EditorPlaceState{.placeableIndex = index});
    }

    void EditorScene::selectSceneEntity(const entt::entity entity)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;
        selectedSceneEntity = entity;
        editorModes->ChangeState(editor::EditorSelectState{});
        gui->HideDeleteConfirmation();
        refreshSceneWindows();
    }

    std::optional<entt::entity> EditorScene::findSceneEntityUnderCursor() const
    {
        const auto viewport = sys->settings->GetViewPort();
        const auto ray = GetScreenToWorldRayEx(GetMousePosition(), *sys->camera->getRaylibCam(), viewport.x, viewport.y);
        auto collisions = sys->collisionSystem->GetCollisionsWithRay(ray, CollisionMask{~0ull});

        std::vector<CollisionInfo> objectHits;
        std::vector<CollisionInfo> fallbackHits;

        for (auto collision : collisions)
        {
            const auto entity = collision.collidedEntityId;
            if (entity == entt::null || entity == placementController->GridSurfaceEntity() ||
                !sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity))
            {
                continue;
            }

            if (sys->registry->any_of<Renderable>(entity))
            {
                const auto& renderable = sys->registry->get<Renderable>(entity);
                const auto* model = renderable.GetModel();
                if (model == nullptr) continue;

                const auto& transform = sys->registry->get<sgTransform>(entity);
                const Matrix entityMatrix = editor::BuildRenderableEntityMatrix(
                    transform.GetWorldPos(),
                    transform.GetWorldRot(),
                    transform.GetScale());
                bool meshHit = false;
                RayCollision closestMeshHit{};
                closestMeshHit.distance = std::numeric_limits<float>::max();

                for (int meshIndex = 0; meshIndex < model->GetMeshCount(); ++meshIndex)
                {
                    const auto meshCollision = model->GetRayMeshCollision(ray, meshIndex, entityMatrix);
                    if (meshCollision.hit && meshCollision.distance < closestMeshHit.distance)
                    {
                        closestMeshHit = meshCollision;
                        meshHit = true;
                    }
                }

                if (!meshHit) continue;
                collision.rlCollision = closestMeshHit;
            }

            if (IsNavigationLayer(collision.collisionLayer))
            {
                fallbackHits.push_back(collision);
            }
            else
            {
                objectHits.push_back(collision);
            }
        }

        auto selectClosest = [](std::vector<CollisionInfo>& hits) -> std::optional<entt::entity> {
            if (hits.empty()) return std::nullopt;
            CollisionSystem::SortCollisionsByDistance(hits);
            return hits.front().collidedEntityId;
        };

        if (auto object = selectClosest(objectHits); object.has_value()) return object;
        return selectClosest(fallbackHits);
    }

    bool EditorScene::selectSceneEntityUnderCursor()
    {
        if (isEditState()) return false;

        const auto entity = findSceneEntityUnderCursor();
        if (!entity.has_value()) return false;

        selectSceneEntity(*entity);
        return true;
    }

    void EditorScene::clearSceneEntitySelection()
    {
        selectedSceneEntity.reset();
        if (isEditState())
        {
            editorModes->ChangeState(editor::EditorSelectState{});
        }
        gui->HideDeleteConfirmation();
        refreshSceneWindows();
    }

    void EditorScene::toggleEditSelectedTransform()
    {
        if (!selectedSceneEntity.has_value()) return;
        const auto entity = *selectedSceneEntity;
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity))
        {
            clearSceneEntitySelection();
            return;
        }

        if (isEditState())
        {
            finishEditSelectedTransform();
        }
        else
        {
            gui->HideDeleteConfirmation();
            editorModes->ChangeState(editor::EditorEditState{.entity = entity});
        }

        refreshOverlay();
        refreshSceneWindows();
    }

    void EditorScene::finishEditSelectedTransform()
    {
        if (!isEditState()) return;

        editorModes->ChangeState(editor::EditorSelectState{});
        refreshOverlay();
        refreshSceneWindows();
    }

    void EditorScene::cancelEditSelectedTransform()
    {
        if (!isEditState()) return;

        const auto* editState = editorModes->CurrentEditState();
        if (editState == nullptr) return;
        transformEditor->RestoreSnapshot(*editState);

        editorModes->ChangeState(editor::EditorSelectState{});
        refreshOverlay();
        refreshSceneWindows();
    }

    void EditorScene::toggleEditPivotMode()
    {
        if (!isEditState()) return;

        const auto* editState = editorModes->CurrentEditState();
        if (editState == nullptr) return;
        const auto entity = editState->entity;
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity))
        {
            clearSceneEntitySelection();
            return;
        }

        transformEditor->TogglePivotMode();
        syncPlacementFromEntity(entity);
        refreshOverlay();
        refreshSceneWindows();
    }

    void EditorScene::requestDeleteSelectedEntity()
    {
        if (!selectedSceneEntity.has_value()) return;
        if (!sys->registry->valid(*selectedSceneEntity) ||
            !sys->registry->any_of<sgTransform>(*selectedSceneEntity))
        {
            clearSceneEntitySelection();
            return;
        }

        gui->ShowDeleteConfirmation(describeSelectedSceneEntity());
    }

    void EditorScene::cancelDeleteSelectedEntity()
    {
        gui->HideDeleteConfirmation();
    }

    void EditorScene::releaseNavigationOccupation(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<Collideable>(entity)) return;

        const auto& collideable = sys->registry->get<Collideable>(entity);
        if (collideable.blocksNavigation)
        {
            sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false, entity);
        }
    }

    void EditorScene::deleteEntityAndChildren(const entt::entity entity)
    {
        if (!sys->registry->valid(entity)) return;

        std::vector<entt::entity> children;
        if (sys->registry->any_of<sgTransform>(entity))
        {
            children = sys->registry->get<sgTransform>(entity).GetChildren();
        }

        for (const auto child : children)
        {
            deleteEntityAndChildren(child);
        }

        if (sys->registry->valid(entity) && sys->registry->any_of<sgTransform>(entity))
        {
            sys->transformSystem->SetParent(entity, entt::null);
        }

        releaseNavigationOccupation(entity);

        if (sys->registry->valid(entity))
        {
            sys->registry->destroy(entity);
        }
    }

    void EditorScene::confirmDeleteSelectedEntity()
    {
        if (!selectedSceneEntity.has_value())
        {
            gui->HideDeleteConfirmation();
            return;
        }

        const auto entity = *selectedSceneEntity;
        selectedSceneEntity.reset();
        gui->HideDeleteConfirmation();
        deleteEntityAndChildren(entity);
        refreshSceneWindows();
        refreshOverlay();
    }

    void EditorScene::adjustGridSurfaceY(const float amount)
    {
        placementController->AdjustGridSurfaceY(amount);
        refreshOverlay();
    }

    void EditorScene::adjustPlacementRotation(const float amount)
    {
        placementController->AdjustRotation(amount);
        refreshOverlay();
    }

    void EditorScene::adjustPlacementScale(const float amount)
    {
        placementController->AdjustScale(amount);
        refreshOverlay();
    }

    void EditorScene::syncPlacementFromEntity(const entt::entity entity)
    {
        placementController->SyncFromEntity(entity);
    }

    void EditorScene::adjustSelectedModelDefaultHeight(const float amount)
    {
        if (!isPlaceState()) return;
        assetCatalog->AdjustSelectedDefaultHeight(amount);
        refreshOverlay();
    }

    void EditorScene::adjustSelectedModelDefaultRotation(const float amount)
    {
        if (!isPlaceState()) return;
        assetCatalog->AdjustSelectedDefaultRotation(amount);
        refreshOverlay();
    }

    void EditorScene::adjustSelectedModelDefaultScale(const float amount)
    {
        if (!isPlaceState()) return;
        assetCatalog->AdjustSelectedDefaultScale(amount);
        refreshOverlay();
    }

    void EditorScene::applySelectedModelDefaults()
    {
        if (!isPlaceState()) return;
        assetCatalog->ApplySelectedDefaults();
        refreshOverlay();
    }

    void EditorScene::resetSelectedModelDefaults()
    {
        if (!isPlaceState()) return;
        assetCatalog->ResetSelectedDefaults();
        refreshOverlay();
    }

    void EditorScene::placeSelectedMesh()
    {
        if (!isPlaceState()) return;
        const auto entity = placementController->PlaceSelectedMesh();
        if (!entity.has_value()) return;

        selectedSceneEntity = *entity;
        refreshSceneWindows();
        refreshOverlay();
    }

    void EditorScene::drawPlacementPreview() const
    {
        placementController->DrawPreview();
    }

    void EditorScene::Update()
    {
        sys->audioManager->Update();
        sys->userInput->ListenForInput();
        if (sys->UI().IsMouseOverWindow())
        {
            sys->camera->ScrollDisable();
        }
        else
        {
            sys->camera->ScrollEnable();
        }
        sys->camera->Update();
        sys->cursor->Update();
        refreshPlacementTarget();

        if (!TextInput::AnyEditing())
        {
            if (IsKeyPressed(KEY_DELETE) && !gui->IsDeleteConfirmationVisible())
            {
                requestDeleteSelectedEntity();
            }
            if (IsKeyPressed(KEY_EQUAL))
            {
                adjustGridSurfaceY(GRID_SURFACE_Y_STEP);
            }
            if (IsKeyPressed(KEY_MINUS))
            {
                adjustGridSurfaceY(-GRID_SURFACE_Y_STEP);
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

        if (selectedSceneEntity.has_value() && sys->registry->valid(*selectedSceneEntity) &&
            sys->registry->any_of<Collideable>(*selectedSceneEntity))
        {
            DrawBoundingBox(sys->registry->get<Collideable>(*selectedSceneEntity).worldBoundingBox, ORANGE);
        }
    }

    void EditorScene::Draw2D() const
    {
        sys->UI().Draw2D();
    }

    bool EditorScene::HandleEscapePressed()
    {
        if (!isEditState()) return false;

        cancelEditSelectedTransform();
        return true;
    }

    void EditorScene::SetSceneName(const std::string& sceneName) const
    {
        gui->SetSceneName(sceneName);
    }

    EditorScene::EditorScene(EngineSystems* _sys) : sys(_sys)
    {
        editor::RegisterDefaultInspectorComponents(inspectorRegistry);
        assetCatalog = std::make_unique<editor::EditorAssetCatalog>(
            std::vector<editor::PlaceableAsset>{
                editor::PlaceableAsset{"Sphere", "vfx_sphere", "SPHERE"},
                editor::PlaceableAsset{"Flat Torus", "vfx_flattorus", "FLAT_TORUS"},
                editor::PlaceableAsset{"Sword", "mdl_sword", "SWORD"},
            });
        assetCatalog->LoadDefaults();
        hierarchyModel = std::make_unique<editor::EditorHierarchyModel>(sys);
        placementController = std::make_unique<editor::EditorPlacementController>(sys, *assetCatalog);

        applyLitShaderToLoadedRenderables();
        giveTransformsToLights();
        placementController->Initialize();

        transformEditor = std::make_unique<editor::EditorTransformEditor>(
            sys,
            [this](const entt::entity entity) {
                if (isEditState())
                {
                    syncPlacementFromEntity(entity);
                }
                refreshOverlay();
                refreshSceneWindows();
            });
        editorModes = std::make_unique<editor::EditorModeStateMachine>(sys->registry, *this, *transformEditor);

        gui = std::make_unique<editor::EditorGui>(
            &sys->UI(),
            sys->settings,
            assetCatalog->AssetEntries(),
            [this](const std::size_t index) { selectPlaceable(index); },
            [this](const entt::entity entity) { selectSceneEntity(entity); },
            editor::EditorGui::ModelDefaultCallbacks{
                .heightDown = [this]() { adjustSelectedModelDefaultHeight(-PLACEMENT_HEIGHT_STEP); },
                .heightUp = [this]() { adjustSelectedModelDefaultHeight(PLACEMENT_HEIGHT_STEP); },
                .rotationDown = [this]() { adjustSelectedModelDefaultRotation(-PLACEMENT_ROTATION_STEP); },
                .rotationUp = [this]() { adjustSelectedModelDefaultRotation(PLACEMENT_ROTATION_STEP); },
                .scaleDown = [this]() { adjustSelectedModelDefaultScale(-PLACEMENT_SCALE_STEP); },
                .scaleUp = [this]() { adjustSelectedModelDefaultScale(PLACEMENT_SCALE_STEP); },
                .apply = [this]() { applySelectedModelDefaults(); },
                .reset = [this]() { resetSelectedModelDefaults(); }},
            editor::EditorGui::InspectorCallbacks{
                .adjustTransform =
                    [this](const editor::EditorGui::TransformField field, const float amount) {
                        if (selectedSceneEntity.has_value())
                        {
                            transformEditor->ApplyFromInspector(*selectedSceneEntity, field, amount, false);
                        }
                    },
                .setTransform =
                    [this](const editor::EditorGui::TransformField field, const float value) {
                        if (selectedSceneEntity.has_value())
                        {
                            transformEditor->ApplyFromInspector(*selectedSceneEntity, field, value, true);
                        }
                    }},
            editor::EditorGui::DeleteConfirmationCallbacks{
                .confirm = [this]() { confirmDeleteSelectedEntity(); },
                .cancel = [this]() { cancelDeleteSelectedEntity(); }});
        refreshOverlay();
        refreshSceneWindows();
    }

    EditorScene::~EditorScene() = default;
} // namespace sage
