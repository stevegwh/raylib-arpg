#pragma once

#include "EditorGui.hpp"
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

        struct EditorState
        {
            using Variant = std::variant<EditorSelectState, EditorPickState>;

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

        EngineSystems* sys{};
        std::unique_ptr<editor::EditorGui> gui;
        std::vector<PlaceableMesh> placeables;
        std::size_t selectedPlaceableIndex = 0;
        unsigned int placedMeshCount = 0;
        float placementHeightOffset = 0.0f;
        float placementRotationY = 0.0f;
        float placementScale = 1.0f;
        std::optional<GridSquare> hoveredGridSquare;
        std::optional<Vector3> snappedPlacementPosition;
        std::optional<entt::entity> selectedSceneEntity;
        std::string lastPlacedLabel = "None";
        entt::entity editorStateEntity = entt::null;

        void createGridPickSurface() const;
        void refreshPlacementTarget();
        void refreshOverlay() const;
        void refreshSceneWindows() const;
        void resetPlacementTransform();
        void selectPlaceable(std::size_t index);
        void selectSceneEntity(entt::entity entity);
        bool selectSceneEntityUnderCursor();
        void clearSceneEntitySelection();
        void cyclePlaceable();
        void requestDeleteSelectedEntity();
        void cancelDeleteSelectedEntity();
        void confirmDeleteSelectedEntity();
        void deleteEntityAndChildren(entt::entity entity);
        void releaseNavigationOccupation(entt::entity entity) const;
        void adjustPlacementHeight(float amount);
        void adjustPlacementRotation(float amount);
        void adjustPlacementScale(float amount);
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
        void changeState(EditorSelectState newState);
        void changeState(EditorPickState newState);

        [[nodiscard]] const PlaceableMesh& selectedPlaceable() const;
        [[nodiscard]] bool isPickState() const;
        [[nodiscard]] std::string describeMode() const;
        [[nodiscard]] std::string describeSelectedAsset() const;
        [[nodiscard]] std::string describeHoveredGrid() const;
        [[nodiscard]] std::string describePlacementHeight() const;
        [[nodiscard]] std::string describePlacementRotation() const;
        [[nodiscard]] std::string describePlacementScale() const;
        [[nodiscard]] std::string describeSelectedModelDefaultHeight() const;
        [[nodiscard]] std::string describeSelectedModelDefaultRotation() const;
        [[nodiscard]] std::string describeSelectedModelDefaultScale() const;
        [[nodiscard]] std::string describeEntity(entt::entity entity) const;
        [[nodiscard]] std::string describeSelectedSceneEntity() const;
        [[nodiscard]] std::string describeSelectedPositionX() const;
        [[nodiscard]] std::string describeSelectedPositionY() const;
        [[nodiscard]] std::string describeSelectedPositionZ() const;
        [[nodiscard]] std::string describeSelectedRotationX() const;
        [[nodiscard]] std::string describeSelectedRotationY() const;
        [[nodiscard]] std::string describeSelectedRotationZ() const;
        [[nodiscard]] std::string describeSelectedScaleX() const;
        [[nodiscard]] std::string describeSelectedScaleY() const;
        [[nodiscard]] std::string describeSelectedScaleZ() const;
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

        explicit EditorScene(EngineSystems* _sys);
        ~EditorScene();
    };
} // namespace sage
