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
        friend struct editor::EditorSelectState;
        friend struct editor::EditorPlaceState;
        friend struct editor::EditorEditState;

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
        // TODO: This should be in place mode
        void refreshPlacementTarget();
        void refreshOverlay() const;
        void refreshSceneWindows() const;
        // TODO: This should be in place mode
        void resetPlacementTransform();
        // TODO: This should be in asset browser?
        void selectPlaceable(std::size_t index);
        void syncPlacementFromEntity(entt::entity entity);
        void adjustGridSurfaceY(float amount);
        // TODO: This should be in place mode
        void adjustPlacementRotation(float amount);
        // TODO: This should be in place mode
        void adjustPlacementScale(float amount);
        // TODO: This should be in place mode
        [[nodiscard]] bool placeSelectedMesh();
        // TODO: This should be in place mode
        void drawPlacementPreview() const;

        void focusSelectedObject() const;
        void focusSelectedObjectInHierarchy() const;

        [[nodiscard]] const editor::PlaceableAsset& selectedPlaceable() const;
        [[nodiscard]] bool isPlaceState() const;
        [[nodiscard]] bool isEditState() const;
        [[nodiscard]] std::string describeMode() const;
        [[nodiscard]] std::string describeSelectedAsset() const;
        [[nodiscard]] std::string describeCursorPosition() const;
        [[nodiscard]] std::string describeEntity(entt::entity entity) const;
        [[nodiscard]] std::string describeSelectedSceneEntity() const;

      public:
        void Update();
        void Draw3D() const;
        void Draw2D() const;
        [[nodiscard]] bool HandleEscapePressed();

        void SetSceneName(const std::string& sceneName) const;

        explicit EditorScene(EngineSystems* _sys);
        ~EditorScene();
    };
} // namespace sage
