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
            sys->transformSystem->SetPosition(entity, position);
        }
    }

    void EditorScene::refreshPlacementTarget()
    {
        placementController->RefreshTarget();
    }

    void EditorScene::refreshOverlay() const
    {
        const auto defaultsStatus = modelDefaults->Status(describeSelectedAsset());
        gui->SetOverlayStatus(describeMode(), describeCursorPosition());
        gui->SetAssetDefaultsStatus(
            defaultsStatus.assetName, defaultsStatus.height, defaultsStatus.rotation, defaultsStatus.scale);
        gui->SetSelectedAsset(isPlaceState() ? std::optional<std::size_t>{assetCatalog->SelectedIndex()} : std::nullopt);
    }

    void EditorScene::refreshSceneWindows() const
    {
        const auto activeEntity = selection->ActiveTransformEntity();
        const auto inspectedComponents =
            activeEntity.has_value() ? inspectorRegistry.InspectEntity(*sys->registry, *activeEntity)
                                     : std::vector<editor::InspectedComponent>{};

        gui->SetHierarchy(hierarchyTree->CollectSceneObjectEntries(), activeEntity);
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
            modelDefaults->Callbacks(),
            editor::EditorGui::DeleteConfirmationCallbacks{
                .confirm = [this]() { editor::EditorSelectState{}.ConfirmDeleteSelectedEntity(*editorModes); },
                .cancel = [this]() { editor::EditorSelectState{}.CancelDeleteSelectedEntity(*editorModes); }});
        refreshOverlay();
        refreshSceneWindows();
    }

    EditorScene::~EditorScene() = default;
} // namespace sage
