#pragma once

#include "EditGizmo.hpp"
#include "EditorComponents.hpp"
#include "EditorGui.hpp"
#include "EditorInspector.hpp"
#include "engine/components/NavigationGridSquare.hpp"
#include "cereal/cereal.hpp"
#include "raylib.h"
#include "raymath.h"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace sage
{
    class EngineSystems;

    class EditorScene
    {
        struct EditorSelectState
        {
        };

        struct EditorPickState
        {
            std::size_t placeableIndex = 0;
        };

        struct EditorEditState
        {
            entt::entity entity = entt::null;
            Vector3 originalPosition{};
            Vector3 originalRotation{};
            Vector3 originalScale{1.0f, 1.0f, 1.0f};
            Matrix originalRenderableTransform = MatrixIdentity();
            Matrix originalRenderableInitialTransform = MatrixIdentity();
            BoundingBox originalLocalBoundingBox{};
            BoundingBox originalWorldBoundingBox{};
            bool hadRenderable = false;
            bool hadCollideable = false;
        };

        struct EditorState
        {
            using Variant = std::variant<EditorSelectState, EditorPickState, EditorEditState>;

            Variant current = EditorSelectState{};

            void RemoveAllSubscriptions()
            {
            }
        };

        struct PlaceableMesh
        {
            std::string displayName;
            std::string modelKey;
            std::string labelStem;
            float modelDefaultHeightOffset = 0.0f;
            float modelDefaultRotationY = 0.0f;
            float modelDefaultScale = 1.0f;
            Matrix appliedModelDefaultTransform = MatrixIdentity();
        };

        struct SerializedAssetDefaults
        {
            std::string displayName;
            std::string modelKey;
            float modelDefaultHeightOffset = 0.0f;
            float modelDefaultRotationY = 0.0f;
            float modelDefaultScale = 1.0f;
            Matrix appliedModelDefaultTransform = MatrixIdentity();

            template <class Archive>
            void serialize(Archive& archive)
            {
                archive(
                    CEREAL_NVP(displayName),
                    CEREAL_NVP(modelKey),
                    CEREAL_NVP(modelDefaultHeightOffset),
                    CEREAL_NVP(modelDefaultRotationY),
                    CEREAL_NVP(modelDefaultScale),
                    CEREAL_NVP(appliedModelDefaultTransform));
            }
        };

        enum class EditPivotMode
        {
            World,
            LocalCenter
        };

        using EditTransformMode = editor::EditGizmo::Mode;
        using EditGizmoAxis = editor::EditGizmo::Axis;

        EngineSystems* sys{};
        std::unique_ptr<editor::EditorGui> gui;
        editor::ComponentInspectorRegistry inspectorRegistry;
        std::vector<PlaceableMesh> placeables;
        std::size_t selectedPlaceableIndex = 0;
        unsigned int placedMeshCount = 0;
        float gridSurfaceY = 0.0f;
        float gridHalfExtent = 50.0f;
        entt::entity gridPickSurfaceEntity = entt::null;
        float placementRotationY = 0.0f;
        float placementScale = 1.0f;
        std::optional<GridSquare> hoveredGridSquare;
        std::optional<Vector3> snappedPlacementPosition;
        std::optional<entt::entity> selectedSceneEntity;
        entt::entity editorStateEntity = entt::null;
        EditPivotMode editPivotMode = EditPivotMode::LocalCenter;
        EditTransformMode editTransformMode = EditTransformMode::Translate;
        editor::EditGizmo editGizmo;

        void createGridPickSurface();
        void sizeGridToLoadedScene();
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
        void restoreEditSnapshot(const EditorEditState& editState);
        void requestDeleteSelectedEntity();
        void cancelDeleteSelectedEntity();
        void confirmDeleteSelectedEntity();
        void deleteEntityAndChildren(entt::entity entity);
        void releaseNavigationOccupation(entt::entity entity) const;
        void adjustGridSurfaceY(float amount);
        void adjustPlacementRotation(float amount);
        void adjustPlacementScale(float amount);
        void applyEditWorldMatrix(
            entt::entity entity,
            Matrix desiredWorldMatrix,
            Vector3 position,
            Vector3 rotation,
            Vector3 scale);
        void adjustEditPosition(Vector3 amount);
        void adjustEditRotation(float amount);
        void adjustEditRotationAxis(EditGizmoAxis axis, float amount);
        void adjustEditScale(float amount);
        void setEditTargetFromEntity(entt::entity entity);
        void syncEditControlsFromEntity(entt::entity entity);
        void updateEditGizmoDrag(entt::entity entity);
        void endEditGizmoDrag();
        void adjustSelectedTransform(editor::EditorGui::TransformField field, float amount);
        void setSelectedTransform(editor::EditorGui::TransformField field, float value);
        void adjustSelectedModelDefaultHeight(float amount);
        void adjustSelectedModelDefaultRotation(float amount);
        void adjustSelectedModelDefaultScale(float amount);
        void applySelectedModelDefaults();
        void resetSelectedModelDefaults();
        void loadAssetDefaults();
        void loadAssetDefaults(PlaceableMesh& placeable) const;
        void saveAssetDefaults(const PlaceableMesh& placeable) const;
        void placeSelectedMesh();
        void updateEntityCollisionBounds(entt::entity entity) const;
        void drawPlacementPreview() const;
        void enterState(EditorSelectState& state);
        void exitState(EditorSelectState& state);
        void updateState(EditorSelectState& state);
        void drawState3D(const EditorSelectState& state) const;
        void enterState(EditorPickState& state);
        void exitState(EditorPickState& state);
        void updateState(EditorPickState& state);
        void drawState3D(const EditorPickState& state) const;
        void enterState(EditorEditState& state);
        void exitState(EditorEditState& state);
        void updateState(EditorEditState& state);
        void drawState3D(const EditorEditState& state) const;
        void changeState(EditorSelectState newState);
        void changeState(EditorPickState newState);
        void changeState(EditorEditState newState);

        [[nodiscard]] const PlaceableMesh& selectedPlaceable() const;
        [[nodiscard]] bool isPickState() const;
        [[nodiscard]] bool isEditState() const;
        [[nodiscard]] std::string describeMode() const;
        [[nodiscard]] std::string describeEditTransformMode() const;
        [[nodiscard]] std::string describeSelectedAsset() const;
        [[nodiscard]] std::string describeCursorPosition() const;
        [[nodiscard]] std::string describeSelectedModelDefaultHeight() const;
        [[nodiscard]] std::string describeSelectedModelDefaultRotation() const;
        [[nodiscard]] std::string describeSelectedModelDefaultScale() const;
        [[nodiscard]] std::string describeEntity(entt::entity entity) const;
        [[nodiscard]] std::string describeSelectedSceneEntity() const;
        [[nodiscard]] Vector3 editPivotWorldPosition(entt::entity entity) const;
        [[nodiscard]] Matrix selectedModelDefaultTransform() const;
        [[nodiscard]] Matrix modelDefaultTransform(const PlaceableMesh& placeable) const;
        [[nodiscard]] SerializedAssetDefaults serializeAssetDefaults(const PlaceableMesh& placeable) const;
        [[nodiscard]] std::string assetDefaultsPath(const PlaceableMesh& placeable) const;
        [[nodiscard]] std::string makePlacedLabel(entt::entity entity) const;
        [[nodiscard]] std::vector<editor::EditorGui::SceneObjectEntry> collectSceneObjectEntries() const;
        void appendSceneObjectEntry(
            std::vector<editor::EditorGui::SceneObjectEntry>& entries,
            entt::entity entity,
            int depth) const;
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
