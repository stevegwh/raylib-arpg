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

#include <memory>
#include <string>

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

        void applyLitShaderToLoadedRenderables() const;
        void giveTransformsToLights() const;
        void refreshOverlay() const;
        void refreshSceneWindows() const;

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
        [[nodiscard]] bool HandleEscapePressed() const;

        void SetSceneName(const std::string& sceneName) const;

        explicit EditorScene(EngineSystems* _sys);
        ~EditorScene();
    };
} // namespace sage
