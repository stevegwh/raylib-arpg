#pragma once

#include "raylib.h"

#include "entt/entt.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace sage
{
    class GameUIEngine;
    class TextBox;
    class Window;
    struct Settings;

    namespace editor
    {
        class EditorGui
        {
          public:
            struct AssetEntry
            {
                std::string displayName;
                std::string modelKey;
            };

            struct SceneObjectEntry
            {
                entt::entity entity = entt::null;
                std::string displayName;
                int depth = 0;
            };

            enum class TransformField
            {
                PositionX,
                PositionY,
                PositionZ,
                RotationX,
                RotationY,
                RotationZ,
                ScaleX,
                ScaleY,
                ScaleZ
            };

            struct ModelDefaultCallbacks
            {
                std::function<void()> heightDown;
                std::function<void()> heightUp;
                std::function<void()> rotationDown;
                std::function<void()> rotationUp;
                std::function<void()> scaleDown;
                std::function<void()> scaleUp;
                std::function<void()> apply;
                std::function<void()> reset;
            };

            struct InspectorCallbacks
            {
                std::function<void(TransformField, float)> adjustTransform;
                std::function<void(TransformField, float)> setTransform;
                std::function<void()> toggleEditTransform;
                std::function<void()> toggleEditPivot;
            };

            struct DeleteConfirmationCallbacks
            {
                std::function<void()> confirm;
                std::function<void()> cancel;
            };

          private:
            Window* overlayWindow{};
            Window* hierarchyWindow{};
            Window* assetWindow{};
            Window* assetDefaultsWindow{};
            Window* inspectorWindow{};
            Window* deleteConfirmationWindow{};
            Texture2D editorWindowBackgroundTexture{};
            TextBox* hierarchyScrollbarUpText{};
            TextBox* hierarchyScrollbarTrackText{};
            TextBox* hierarchyScrollbarDownText{};
            std::vector<RenderTexture2D> assetThumbnails;
            std::vector<TextBox*> hierarchyRows;
            std::vector<SceneObjectEntry> hierarchyEntries;
            std::size_t hierarchyScrollOffset = 0;
            ModelDefaultCallbacks modelDefaultCallbacks;
            InspectorCallbacks inspectorCallbacks;
            DeleteConfirmationCallbacks deleteConfirmationCallbacks;
            std::optional<std::size_t> selectedAssetIndex;
            std::optional<entt::entity> selectedSceneEntity;
            TextBox* selectedAssetText{};
            TextBox* modeText{};
            TextBox* gridText{};
            TextBox* placementHeightText{};
            TextBox* placementRotationText{};
            TextBox* placementScaleText{};
            TextBox* lastPlacedText{};
            TextBox* defaultsAssetText{};
            TextBox* defaultsPositionText{};
            TextBox* defaultsRotationText{};
            TextBox* defaultsScaleText{};
            TextBox* inspectorSelectionText{};
            TextBox* inspectorPositionXText{};
            TextBox* inspectorPositionYText{};
            TextBox* inspectorPositionZText{};
            TextBox* inspectorRotationXText{};
            TextBox* inspectorRotationYText{};
            TextBox* inspectorRotationZText{};
            TextBox* inspectorScaleXText{};
            TextBox* inspectorScaleYText{};
            TextBox* inspectorScaleZText{};
            TextBox* inspectorEditButtonText{};
            TextBox* inspectorPivotButtonText{};
            bool inspectorPivotButtonVisible = false;
            TextBox* deleteConfirmationText{};

            RenderTexture2D createAssetThumbnail(const AssetEntry& asset) const;
            void createOverlayWindow(GameUIEngine* ui, Settings* settings);
            void createHierarchyWindow(
                GameUIEngine* ui,
                Settings* settings,
                const std::function<void(entt::entity)>& onSceneObjectSelected);
            void createAssetWindow(
                GameUIEngine* ui,
                Settings* settings,
                const std::vector<AssetEntry>& assets,
                const std::function<void(std::size_t)>& onAssetSelected);
            void createAssetDefaultsWindow(GameUIEngine* ui, Settings* settings);
            void createInspectorWindow(GameUIEngine* ui, Settings* settings);
            void createDeleteConfirmationWindow(GameUIEngine* ui, Settings* settings);
            void scrollHierarchy(int amount);

          public:
            void SetPlacementStatus(
                const std::string& mode,
                const std::string& selectedAsset,
                const std::string& hoveredGrid,
                const std::string& placementHeight,
                const std::string& placementRotation,
                const std::string& placementScale,
                const std::string& modelDefaultHeight,
                const std::string& modelDefaultRotation,
                const std::string& modelDefaultScale,
                const std::string& lastPlaced) const;
            void SetSelectedAsset(std::optional<std::size_t> index);
            void SetHierarchy(const std::vector<SceneObjectEntry>& entries, std::optional<entt::entity> selectedEntity);
            void SetInspector(
                const std::string& selectedEntity,
                const std::string& positionX,
                const std::string& positionY,
                const std::string& positionZ,
                const std::string& rotationX,
                const std::string& rotationY,
                const std::string& rotationZ,
                const std::string& scaleX,
                const std::string& scaleY,
                const std::string& scaleZ,
                const std::string& editButtonText,
                const std::string& pivotButtonText);
            void ShowDeleteConfirmation(const std::string& selectedEntity) const;
            void HideDeleteConfirmation() const;
            [[nodiscard]] bool IsDeleteConfirmationVisible() const;
            EditorGui(
                GameUIEngine* ui,
                Settings* settings,
                const std::vector<AssetEntry>& assets,
                std::function<void(std::size_t)> onAssetSelected,
                std::function<void(entt::entity)> onSceneObjectSelected,
                ModelDefaultCallbacks callbacks,
                InspectorCallbacks inspectorCallbacks,
                DeleteConfirmationCallbacks deleteConfirmationCallbacks);
            ~EditorGui();
        };
    } // namespace editor
} // namespace sage
