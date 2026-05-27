#include "EditorScene.hpp"

#include "EditorComponents.hpp"
#include "EditorFlatpack.hpp"
#include "EditorMapLoader.hpp"
#include "EditorTransformMath.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/CollisionLayers.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/Cursor.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/Light.hpp"
#include "engine/LightManager.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/systems/RenderSystem.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "engine/UserInput.hpp"

#include "imgui.h"
#include "imfilebrowser.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <vector>

namespace sage
{
    namespace
    {
        constexpr float GRID_SURFACE_Y_STEP = 1.0f;
        constexpr float EDITOR_FOCUS_CAMERA_DISTANCE = 38.0f;
        constexpr float EDITOR_FOCUS_RADIUS_PADDING = 2.4f;
        constexpr const char* UNTITLED_SCENE_NAME = "Untitled";
        constexpr const char* DEFAULT_SAVE_FILENAME = "untitled.map";
        constexpr const char* DEFAULT_MAP_BASE_NAME = "_MAPBASE_EDITOR_BASE";
        constexpr const char* DEFAULT_MAP_BASE_MODEL_KEY = "primitive_plane";
        constexpr const char* DEFAULT_MAP_BASE_CATEGORY = "Map";
        constexpr float DEFAULT_MAP_BASE_SIZE = 1000.0f;
        constexpr float DEFAULT_MAP_BASE_HALF_HEIGHT = 0.02f;
        constexpr float DEFAULT_LIGHT_HEIGHT_OFFSET = 6.0f;
        constexpr float DEFAULT_LIGHT_BRIGHTNESS = 3.0f;
        constexpr Color DEFAULT_LIGHT_COLOR = {255, 244, 214, 255};
        constexpr ImGuiFileBrowserFlags LOAD_BROWSER_FLAGS =
            ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_SkipItemsCausingError;
        constexpr ImGuiFileBrowserFlags SAVE_BROWSER_FLAGS =
            LOAD_BROWSER_FLAGS | ImGuiFileBrowserFlags_EnterNewFilename;

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

        std::filesystem::path ensureMapExtension(std::filesystem::path path)
        {
            if (path.extension() != ".map")
            {
                path.replace_extension(".map");
            }
            return path;
        }

        std::filesystem::path defaultBrowserDirectory(const std::filesystem::path& currentMapPath)
        {
            if (!currentMapPath.empty() && !currentMapPath.parent_path().empty())
            {
                return currentMapPath.parent_path();
            }

            const std::filesystem::path resources{"resources"};
            if (std::filesystem::is_directory(resources))
            {
                return resources;
            }

            return std::filesystem::current_path();
        }

        std::string sceneNameFromPath(const std::filesystem::path& path)
        {
            const auto stem = path.stem().string();
            return stem.empty() ? UNTITLED_SCENE_NAME : stem;
        }

        bool isMapBaseRenderable(const Renderable& renderable)
        {
            return renderable.GetName().find("_MAPBASE_") != std::string::npos;
        }

        bool modelKeyAvailable(const std::string& key)
        {
            const auto keys = ResourceManager::GetInstance().GetModelKeys(true);
            return std::ranges::find(keys, key) != keys.end();
        }

        std::string lightLabel(const entt::entity entity)
        {
            return std::format("light_{}", entt::to_integral(entity));
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

    std::string EditorScene::describeSelectedSceneEntity() const
    {
        const auto entity = selection->ActiveTransformEntity();
        return entity.has_value() ? hierarchyTree->DescribeEntity(*entity) : "None";
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
            if (!sys->registry->any_of<sgTransform>(entity))
            {
                const auto position = sys->registry->get<Light>(entity).position;
                sys->registry->emplace<sgTransform>(entity).position.world = position;
            }
            if (!sys->registry->any_of<editor::EditorObjectDescriptor>(entity))
            {
                sys->registry->emplace<editor::EditorObjectDescriptor>(
                    entity,
                    editor::EditorObjectDescriptor{
                        .name = lightLabel(entity),
                        .category = "Light",
                        .selectable = true,
                        .visibleInHierarchy = true,
                        .locked = false});
            }
        }
    }

    void EditorScene::refreshOverlay() const
    {
        const auto defaultsStatus = modelDefaults->Status(describeSelectedAsset());
        gui->SetOverlayStatus(editorModes->GetStateName(), describeCursorPosition());
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

    void EditorScene::Update() const
    {
        // TODO: Fullscreen game viewport (switch state)
        sys->collisionSystem->Update();
        sys->audioManager->Update();
        sys->userInput->ListenForInput();
        const bool uiBlocksScroll = !viewportFullscreen && sys->UI().IsMouseOverWindow();
        if (uiBlocksScroll || !sys->settings->IsPointInRenderViewport(GetMousePosition()))
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
        syncLightTransforms();
        sys->lightSubSystem->Update();
        sys->lightSubSystem->RefreshLights();
        refreshOverlay();
        refreshSceneWindows();
        if (!viewportFullscreen) sys->UI().Update();
    }

    void EditorScene::Draw3D() const
    {
        sys->renderSystem->Draw();
        sys->lightSubSystem->DrawDebugLights();
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
        if (viewportFullscreen) return;
        sys->UI().Draw2D();
    }

    void EditorScene::DrawOverlay2D() const
    {
        if (viewportFullscreen) return;
        gui->DrawSceneViewInfo();
    }

    void EditorScene::DrawImGui() const
    {
        if (viewportFullscreen) return;
        gui->StartImGui();
        drawMainMenuBar();
        drawFileBrowsers();
        drawHierarchyContextMenu();
        gui->EndImGui();
    }

    void EditorScene::drawHierarchyContextMenu() const
    {
        constexpr const char* kPopupId = "hierarchy_context_menu";

        // Right-click outside any ImGui window: capture the cursor over the
        // hierarchy and arm the popup.
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !ImGui::GetIO().WantCaptureMouse)
        {
            const auto viewportMouse = sys->settings->ScreenToViewportPosition(GetMousePosition());
            if (const auto entity = gui->HierarchyEntityAtViewportPos(viewportMouse); entity.has_value())
            {
                hierarchyContextEntity = *entity;
                ImGui::OpenPopup(kPopupId);
            }
        }

        if (ImGui::BeginPopup(kPopupId))
        {
            if (!sys->registry->valid(hierarchyContextEntity))
            {
                ImGui::CloseCurrentPopup();
            }
            else
            {
                if (ImGui::MenuItem("Create Flatpack"))
                {
                    createFlatpackFromEntity(hierarchyContextEntity);
                }
            }
            ImGui::EndPopup();
        }
    }

    void EditorScene::createFlatpackFromEntity(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        const auto safeName = hierarchyTree ? hierarchyTree->DescribeEntity(entity)
                                            : std::format("entity_{}", entt::to_integral(entity));
        const std::filesystem::path flatpacksDir{"resources/flatpacks"};
        const auto outputPath = flatpacksDir / (safeName + ".flatpack");
        if (editor::SaveFlatpack(*sys->registry, entity, outputPath.string().c_str()))
        {
            std::cout << "Flatpack saved: " << outputPath << std::endl;
            refreshFlatpackCatalog();
        }
        else
        {
            std::cerr << "ERROR: Failed to save flatpack: " << outputPath << std::endl;
        }
    }

    void EditorScene::refreshFlatpackCatalog() const
    {
        auto catalog = editor::ListFlatpacks(std::filesystem::path{"resources/flatpacks"});
        std::vector<editor::EditorGui::FlatpackEntry> entries;
        entries.reserve(catalog.size());
        for (auto& item : catalog)
        {
            entries.push_back({.displayName = std::move(item.displayName), .path = std::move(item.path)});
        }
        if (gui) gui->SetFlatpacks(std::move(entries));
    }

    std::optional<entt::entity> EditorScene::PlaceFlatpackAt(
        const std::filesystem::path& path, const Vector3 anchor) const
    {
        const auto root = editor::LoadFlatpack(*sys->registry, path.string().c_str(), anchor);
        if (root == entt::null) return std::nullopt;

        // The loaded subtree has Renderables without an UberShaderComponent;
        // applyLitShaderToLoadedRenderables attaches one with Lit set, matching
        // the shader hookup the rest of the editor relies on. Bounds need to be
        // re-derived from the new world transforms (the saved boxes are stale).
        applyLitShaderToLoadedRenderables();
        if (transformEditor) transformEditor->RefreshCollisionBoundsRecursive(root);
        return root;
    }

    void EditorScene::drawMainMenuBar() const
    {
        if (!ImGui::BeginMainMenuBar()) return;
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load Map"))
            {
                openLoadMapBrowser();
            }
            if (ImGui::MenuItem("Save Map"))
            {
                saveMap();
            }
            if (ImGui::MenuItem("Save Map As..."))
            {
                openSaveMapBrowser();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("Light"))
            {
                addLight();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    void EditorScene::drawFileBrowsers() const
    {
        if (loadMapBrowser)
        {
            loadMapBrowser->Display();
            if (loadMapBrowser->HasSelected())
            {
                loadMap(loadMapBrowser->GetSelected());
                loadMapBrowser->ClearSelected();
            }
        }

        if (saveMapBrowser)
        {
            saveMapBrowser->Display();
            if (saveMapBrowser->HasSelected())
            {
                saveMapAs(saveMapBrowser->GetSelected());
                saveMapBrowser->ClearSelected();
            }
        }
    }

    void EditorScene::openLoadMapBrowser() const
    {
        if (!loadMapBrowser) return;
        loadMapBrowser->SetDirectory(defaultBrowserDirectory(currentMapPath));
        loadMapBrowser->Open();
    }

    void EditorScene::openSaveMapBrowser() const
    {
        if (!saveMapBrowser) return;
        saveMapBrowser->SetDirectory(defaultBrowserDirectory(currentMapPath));
        saveMapBrowser->SetInputName(
            currentMapPath.empty() ? DEFAULT_SAVE_FILENAME : currentMapPath.filename().string());
        saveMapBrowser->Open();
    }

    void EditorScene::addLight() const
    {
        Vector3 position = Vector3Add(sys->camera->getRaylibCam()->target, {0.0f, DEFAULT_LIGHT_HEIGHT_OFFSET, 0.0f});
        if (const auto snappedPosition = placementController->SnappedPlacementPosition(); snappedPosition.has_value())
        {
            position = Vector3Add(*snappedPosition, {0.0f, DEFAULT_LIGHT_HEIGHT_OFFSET, 0.0f});
        }

        const auto entity = sys->registry->create();
        sys->registry->emplace<editor::EditorMapEntity>(entity);
        sys->registry->emplace<editor::EditorObjectDescriptor>(
            entity,
            editor::EditorObjectDescriptor{
                .name = lightLabel(entity),
                .category = "Light",
                .selectable = true,
                .visibleInHierarchy = true,
                .locked = false});

        auto& transform = sys->registry->emplace<sgTransform>(entity);
        transform.position.world = position;

        sys->registry->emplace<Light>(
            entity,
            Light{
                .type = LIGHT_POINT,
                .enabled = true,
                .position = position,
                .target = Vector3Zero(),
                .color = DEFAULT_LIGHT_COLOR,
                .brightness = DEFAULT_LIGHT_BRIGHTNESS});

        sys->lightSubSystem->RefreshLights();
        editorModes->SelectSceneEntity(entity);
    }

    void EditorScene::loadMap(const std::filesystem::path& path) const
    {
        const auto selectedPath = ensureMapExtension(path);
        const auto pathString = selectedPath.string();
        if (!editor::IsEditorLayoutMap(pathString.c_str()))
        {
            std::cerr << "ERROR: Not an editor layout map: " << pathString << std::endl;
            return;
        }

        clearCurrentMap();
        if (!editor::LoadMap(sys->registry, pathString.c_str())) return;
        currentMapPath = selectedPath;
        SetSceneName(sceneNameFromPath(currentMapPath));
        refreshAfterMapLoad();
    }

    void EditorScene::saveMap() const
    {
        if (currentMapPath.empty())
        {
            openSaveMapBrowser();
            return;
        }
        saveMapAs(currentMapPath);
    }

    void EditorScene::saveMapAs(const std::filesystem::path& path) const
    {
        ensureDefaultMapBase();
        currentMapPath = ensureMapExtension(path);
        const auto pathString = currentMapPath.string();
        editor::SaveMap(*sys->registry, pathString.c_str());
        SetSceneName(sceneNameFromPath(currentMapPath));
    }

    void EditorScene::clearCurrentMap() const
    {
        if (editorModes)
        {
            editorModes->ChangeState<editor::EditorSelectState>();
        }
        if (selection)
        {
            selection->Clear();
        }
        if (gui)
        {
            gui->HideDeleteConfirmation();
        }

        std::vector<entt::entity> mapEntities;
        for (const auto entity : sys->registry->view<editor::EditorMapEntity>())
        {
            mapEntities.push_back(entity);
        }

        for (const auto entity : mapEntities)
        {
            if (sys->registry->valid(entity))
            {
                sys->registry->destroy(entity);
            }
        }
    }

    void EditorScene::ensureDefaultMapBase() const
    {
        bool hasMapBase = false;
        const auto existingBaseView = sys->registry->view<editor::EditorMapEntity, Renderable, Collideable>();
        for (const auto entity : existingBaseView)
        {
            auto& renderable = existingBaseView.get<Renderable>(entity);
            if (!isMapBaseRenderable(renderable)) continue;

            hasMapBase = true;
            if (!sys->registry->any_of<editor::EditorMapBase>(entity))
            {
                sys->registry->emplace<editor::EditorMapBase>(entity);
            }
            if (!sys->registry->any_of<editor::EditorObjectDescriptor>(entity))
            {
                sys->registry->emplace<editor::EditorObjectDescriptor>(
                    entity,
                    editor::EditorObjectDescriptor{
                        .name = renderable.GetName(),
                        .category = DEFAULT_MAP_BASE_CATEGORY,
                        .selectable = false,
                        .visibleInHierarchy = true,
                        .locked = true});
            }
            if (!sys->registry->any_of<editor::AssetReference>(entity))
            {
                if (const auto* model = renderable.GetModel(); model != nullptr)
                {
                    sys->registry->emplace<editor::AssetReference>(
                        entity, editor::AssetReference{.assetKey = model->GetKey()});
                }
            }

            auto& collideable = existingBaseView.get<Collideable>(entity);
            collideable.SetCollisionLayer(collision_layers::Background);
            collideable.isStatic = true;
            collideable.blocksNavigation = false;
            collideable.active = true;
            renderable.active = true;
        }

        if (hasMapBase) return;

        if (!modelKeyAvailable(DEFAULT_MAP_BASE_MODEL_KEY))
        {
            std::cerr << "ERROR: Cannot create default map base. Missing model key: "
                      << DEFAULT_MAP_BASE_MODEL_KEY << std::endl;
            return;
        }

        const auto entity = sys->registry->create();
        sys->registry->emplace<editor::EditorMapEntity>(entity);
        sys->registry->emplace<editor::EditorMapBase>(entity);
        sys->registry->emplace<editor::EditorObjectDescriptor>(
            entity,
            editor::EditorObjectDescriptor{
                .name = DEFAULT_MAP_BASE_NAME,
                .category = DEFAULT_MAP_BASE_CATEGORY,
                .selectable = false,
                .visibleInHierarchy = true,
                .locked = true});
        sys->registry->emplace<editor::AssetReference>(
            entity, editor::AssetReference{.assetKey = DEFAULT_MAP_BASE_MODEL_KEY});

        auto& transform = sys->registry->emplace<sgTransform>(entity);
        transform.scale.world = {DEFAULT_MAP_BASE_SIZE, 1.0f, DEFAULT_MAP_BASE_SIZE};

        auto model = ResourceManager::GetInstance().GetModelView(DEFAULT_MAP_BASE_MODEL_KEY);
        auto& renderable = sys->registry->emplace<Renderable>(entity, std::move(model), MatrixIdentity());
        renderable.SetName(DEFAULT_MAP_BASE_NAME);
        renderable.active = true;

        const BoundingBox localBounds = {
            {-0.5f, -DEFAULT_MAP_BASE_HALF_HEIGHT, -0.5f},
            {0.5f, DEFAULT_MAP_BASE_HALF_HEIGHT, 0.5f}};
        auto& collideable = sys->registry->emplace<Collideable>(entity, localBounds, transform.GetMatrixNoRot());
        collideable.SetCollisionLayer(collision_layers::Background);
        collideable.isStatic = true;
        collideable.blocksNavigation = false;
        collideable.active = true;
    }

    void EditorScene::syncLightTransforms() const
    {
        const auto view = sys->registry->view<Light, sgTransform>();
        for (const auto entity : view)
        {
            auto& light = view.get<Light>(entity);
            light.position = view.get<sgTransform>(entity).GetWorldPos();
        }
    }

    void EditorScene::refreshAfterMapLoad() const
    {
        ensureDefaultMapBase();
        applyLitShaderToLoadedRenderables();
        giveTransformsToLights();
        placementController->Initialize();
        selection->Clear();
        editorModes->ChangeState<editor::EditorSelectState>();
        refreshOverlay();
        refreshSceneWindows();
    }

    void EditorScene::reparentEntity(const entt::entity dragged, const entt::entity newParent) const
    {
        if (!sys->registry->valid(dragged) || !sys->registry->any_of<sgTransform>(dragged)) return;
        if (!sys->registry->valid(newParent) || !sys->registry->any_of<sgTransform>(newParent)) return;
        if (dragged == newParent) return;

        // Refuse cycles: walk newParent's ancestor chain. If dragged appears, the
        // requested parenting would put dragged below itself.
        for (auto cur = newParent; cur != entt::null;)
        {
            if (!sys->registry->any_of<sgTransform>(cur)) break;
            if (cur == dragged) return;
            cur = sys->registry->get<sgTransform>(cur).GetParent();
        }

        sys->transformSystem->SetParent(dragged, newParent);
        refreshSceneWindows();
    }

    bool EditorScene::HandleEscapePressed() const
    {
        return editorModes->HandleEscapePressed();
    }

    void EditorScene::SetViewportFullscreen(const bool fullscreen)
    {
        viewportFullscreen = fullscreen;
    }

    void EditorScene::SetSceneName(const std::string& sceneName) const
    {
        gui->SetSceneName(sceneName);
    }

    EditorScene::EditorScene(EngineSystems* _sys) : sys(_sys)
    {
        editor::RegisterDefaultInspectorComponents(inspectorRegistry);
        assetCatalog =
            std::make_unique<editor::EditorAssetCatalog>(editor::EditorAssetCatalog::FromLoadedModels());
        assetCatalog->LoadDefaults();
        modelDefaults = std::make_unique<editor::EditorModelDefaultsController>(
            *assetCatalog, [this]() { return isPlaceState(); }, [this]() { refreshOverlay(); });
        selection = std::make_unique<editor::EditorSelection>(sys);
        pickingService = std::make_unique<editor::EditorPickingService>(sys);
        entityOperations = std::make_unique<editor::EditorEntityOperations>(sys);
        hierarchyTree = std::make_unique<editor::EditorHierarchyTree>(sys);
        placementController = std::make_unique<editor::EditorPlacementController>(sys, *assetCatalog);

        ensureDefaultMapBase();
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
            [this](std::filesystem::path path) { editorModes->SelectFlatpack(std::move(path)); },
            [this](const entt::entity entity) { editorModes->SelectSceneEntity(entity); },
            [this](const entt::entity dragged, const entt::entity newParent) {
                reparentEntity(dragged, newParent);
            },
            modelDefaults->Callbacks());
        SetSceneName(UNTITLED_SCENE_NAME);
        refreshOverlay();
        refreshSceneWindows();
        refreshFlatpackCatalog();
        loadMapBrowser = std::make_unique<ImGui::FileBrowser>(LOAD_BROWSER_FLAGS);
        loadMapBrowser->SetTitle("Load map");
        loadMapBrowser->SetTypeFilters({".map"});
        loadMapBrowser->SetDirectory(defaultBrowserDirectory(currentMapPath));

        saveMapBrowser = std::make_unique<ImGui::FileBrowser>(SAVE_BROWSER_FLAGS);
        saveMapBrowser->SetTitle("Save map as");
        saveMapBrowser->SetTypeFilters({".map"});
        saveMapBrowser->SetDirectory(defaultBrowserDirectory(currentMapPath));
        saveMapBrowser->SetInputName(DEFAULT_SAVE_FILENAME);
    }

    EditorScene::~EditorScene() = default;
} // namespace sage
