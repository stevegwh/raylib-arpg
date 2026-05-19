#pragma once

#include "EditorAssetCatalog.hpp"
#include "EditorGui.hpp"
#include "EditorHierarchyModel.hpp"
#include "EditorInspector.hpp"
#include "EditorModeStateMachine.hpp"
#include "EditorPlacementController.hpp"
#include "EditorTransformEditor.hpp"

#include "entt/entt.hpp"

#include <memory>
#include <optional>
#include <string>

namespace sage
{
    class EngineSystems;

    class EditorScene
    {
        friend class editor::EditorModeStateMachine;

        EngineSystems* sys{};
        std::unique_ptr<editor::EditorGui> gui;
        editor::ComponentInspectorRegistry inspectorRegistry;
        std::unique_ptr<editor::EditorAssetCatalog> assetCatalog;
        std::unique_ptr<editor::EditorHierarchyModel> hierarchyModel;
        std::unique_ptr<editor::EditorPlacementController> placementController;
        std::optional<entt::entity> selectedSceneEntity;
        std::unique_ptr<editor::EditorTransformEditor> transformEditor;
        std::unique_ptr<editor::EditorModeStateMachine> editorModes;

        void applyLitShaderToLoadedRenderables() const;
        void giveTransformsToLights() const;
        void refreshPlacementTarget();
        void refreshOverlay() const;
        void refreshSceneWindows() const;
        void resetPlacementTransform();
        void selectPlaceable(std::size_t index);
        void selectSceneEntity(entt::entity entity);
        std::optional<entt::entity> findSceneEntityUnderCursor() const;
        bool selectSceneEntityUnderCursor();
        void clearSceneEntitySelection();
        void toggleEditSelectedTransform();
        void finishEditSelectedTransform();
        void cancelEditSelectedTransform();
        void toggleEditPivotMode();
        void syncPlacementFromEntity(entt::entity entity);
        void requestDeleteSelectedEntity();
        void cancelDeleteSelectedEntity();
        void confirmDeleteSelectedEntity();
        void deleteEntityAndChildren(entt::entity entity);
        void releaseNavigationOccupation(entt::entity entity) const;
        void adjustGridSurfaceY(float amount);
        void adjustPlacementRotation(float amount);
        void adjustPlacementScale(float amount);
        void adjustSelectedModelDefaultHeight(float amount);
        void adjustSelectedModelDefaultRotation(float amount);
        void adjustSelectedModelDefaultScale(float amount);
        void applySelectedModelDefaults();
        void resetSelectedModelDefaults();
        void placeSelectedMesh();
        void drawPlacementPreview() const;

        [[nodiscard]] const editor::PlaceableAsset& selectedPlaceable() const;
        [[nodiscard]] bool isPlaceState() const;
        [[nodiscard]] bool isEditState() const;
        [[nodiscard]] std::string describeMode() const;
        [[nodiscard]] std::string describeSelectedAsset() const;
        [[nodiscard]] std::string describeCursorPosition() const;
        [[nodiscard]] std::string describeSelectedModelDefaultHeight() const;
        [[nodiscard]] std::string describeSelectedModelDefaultRotation() const;
        [[nodiscard]] std::string describeSelectedModelDefaultScale() const;
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
