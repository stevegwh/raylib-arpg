#pragma once

#include "EditorAssetCatalog.hpp"
#include "EditorEntityOperations.hpp"
#include "EditorGui.hpp"
#include "EditorHierarchyTree.hpp"
#include "EditorInspector.hpp"
#include "EditorModelDefaultsController.hpp"
#include "EditorModeStateMachine.hpp"
#include "EditorPickingService.hpp"
#include "EditorPlacementController.hpp"
#include "EditorSelection.hpp"
#include "EditorTransformEditor.hpp"

#include "entt/entt.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace ImGui
{
    class FileBrowser;
}

namespace sage
{
    class EngineSystems;

    class EditorScene
    {
        friend class editor::EditorModeStateMachine;

        EngineSystems* sys{};
        std::unique_ptr<editor::EditorGui> gui;
        editor::InspectorRegistry inspectorRegistry;
        std::unique_ptr<editor::EditorAssetCatalog> assetCatalog;
        std::unique_ptr<editor::EditorModelDefaultsController> modelDefaults;
        std::unique_ptr<editor::EditorSelection> selection;
        std::unique_ptr<editor::EditorPickingService> pickingService;
        std::unique_ptr<editor::EditorEntityOperations> entityOperations;
        std::unique_ptr<editor::EditorHierarchyTree> hierarchyTree;
        std::unique_ptr<editor::EditorPlacementController> placementController;
        std::unique_ptr<editor::EditorTransformEditor> transformEditor;
        std::unique_ptr<editor::EditorModeStateMachine> editorModes;
        std::unique_ptr<ImGui::FileBrowser> loadMapBrowser{};
        std::unique_ptr<ImGui::FileBrowser> saveMapBrowser{};
        mutable std::filesystem::path currentMapPath{};
        bool viewportFullscreen = false;
        void applyLitShaderToLoadedRenderables() const;
        void giveTransformsToLights() const;
        void refreshOverlay() const;
        void refreshSceneWindows() const;
        void drawMainMenuBar() const;
        void drawFileBrowsers() const;
        void openLoadMapBrowser() const;
        void openSaveMapBrowser() const;
        void addLight() const;
        void loadMap(const std::filesystem::path& path) const;
        void saveMap() const;
        void saveMapAs(const std::filesystem::path& path) const;
        void clearCurrentMap() const;
        void ensureDefaultMapBase() const;
        void syncLightTransforms() const;
        void refreshAfterMapLoad() const;
        void reparentEntity(entt::entity dragged, entt::entity newParent) const;

        void focusSelectedObject() const;
        void focusSelectedObjectInHierarchy() const;

        [[nodiscard]] const editor::PlaceableAsset& selectedPlaceable() const;
        [[nodiscard]] bool isPlaceState() const;
        [[nodiscard]] bool isEditState() const;
        [[nodiscard]] std::string describeSelectedAsset() const;
        [[nodiscard]] std::string describeCursorPosition() const;
        [[nodiscard]] std::string describeSelectedSceneEntity() const;

      public:
        void Update() const;
        void Draw3D() const;
        void Draw2D() const;
        void DrawOverlay2D() const;
        void DrawImGui() const;
        [[nodiscard]] bool HandleEscapePressed() const;

        void SetViewportFullscreen(bool fullscreen);
        void SetSceneName(const std::string& sceneName) const;

        explicit EditorScene(EngineSystems* _sys);
        ~EditorScene();
    };
} // namespace sage
