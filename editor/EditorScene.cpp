#include "EditorScene.hpp"

#include "EditorComponents.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/Cursor.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/Light.hpp"
#include "engine/UserInput.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/RenderSystem.hpp"
#include "engine/systems/TransformSystem.hpp"

#include "raylib.h"

#include <format>
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
        const auto activeEntity = selection->ActiveTransformEntity();
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

        selection->Select(*entity);
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
                editor::EditorSelectState{}.RequestDeleteSelectedEntity(*editorModes);
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

    bool EditorScene::HandleEscapePressed()
    {
        auto* editState = editorModes->CurrentEditState();
        return editState != nullptr && editState->CancelEditSelectedTransform(*editorModes);
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
        selection = std::make_unique<editor::EditorSelection>(sys);
        pickingService = std::make_unique<editor::EditorPickingService>(sys);
        entityOperations = std::make_unique<editor::EditorEntityOperations>(sys);
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
            [this](const entt::entity entity) {
                editor::EditorSelectState{}.SelectSceneEntity(*editorModes, entity);
            },
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
                        if (const auto selectedEntity = selection->ActiveTransformEntity(); selectedEntity.has_value())
                        {
                            transformEditor->ApplyFromInspector(*selectedEntity, field, amount, false);
                        }
                    },
                .setTransform =
                    [this](const editor::EditorGui::TransformField field, const float value) {
                        if (const auto selectedEntity = selection->ActiveTransformEntity(); selectedEntity.has_value())
                        {
                            transformEditor->ApplyFromInspector(*selectedEntity, field, value, true);
                        }
                    }},
            editor::EditorGui::DeleteConfirmationCallbacks{
                .confirm = [this]() { editor::EditorSelectState{}.ConfirmDeleteSelectedEntity(*editorModes); },
                .cancel = [this]() { editor::EditorSelectState{}.CancelDeleteSelectedEntity(*editorModes); }});
        refreshOverlay();
        refreshSceneWindows();
    }

    EditorScene::~EditorScene() = default;
} // namespace sage
