#pragma once

#include "InspectorFieldBuilder.hpp"

#include "engine/Event.hpp"
#include "raylib.h"

#include "entt/entt.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace sage
{
    class GameUIEngine;
    class ImageBox;
    class Table;
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

            enum class DeleteConfirmationAction
            {
                None,
                Confirm,
                Cancel
            };

          private:
            Window* hierarchyWindow{};
            Window* assetWindow{};
            Window* assetDefaultsWindow{};
            Window* inspectorWindow{};
            Window* deleteConfirmationWindow{};
            GameUIEngine* ui{};
            Texture2D editorWindowBackgroundTexture{};
            std::vector<AssetEntry> assetEntries;
            std::vector<RenderTexture2D> assetThumbnails;
            std::vector<ImageBox*> assetButtons;
            std::vector<TextBox*> hierarchyRows;
            std::vector<SceneObjectEntry> hierarchyEntries;
            Subscription assetScrollSub{};
            Subscription hierarchyScrollSub{};
            InspectorFieldBuilder inspectorFieldBlueprints;
            ModelDefaultCallbacks modelDefaultCallbacks;
            DeleteConfirmationAction pendingDeleteConfirmationAction = DeleteConfirmationAction::None;
            std::optional<std::size_t> selectedAssetIndex;
            std::optional<entt::entity> selectedSceneEntity;
            bool assetDrawerOpen = false;
            mutable std::string sceneNameStatus = "Scene";
            mutable std::string modeStatus = "Select";
            mutable std::string cursorStatus = "-";
            TextBox* defaultsAssetText{};
            TextBox* defaultsPositionText{};
            TextBox* defaultsRotationText{};
            TextBox* defaultsScaleText{};
            TextBox* inspectorSelectionText{};
            TextBox* deleteConfirmationText{};

            RenderTexture2D createAssetThumbnail(const AssetEntry& asset) const;
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
            void refreshAssetButtonContent();
            void refreshHierarchyRowContent();
            void setAssetDrawerOpen(bool open);

          public:
            void SetOverlayStatus(const std::string& mode, const std::string& cursor) const;
            void SetAssetDefaultsStatus(
                const std::string& assetName,
                const std::string& modelDefaultHeight,
                const std::string& modelDefaultRotation,
                const std::string& modelDefaultScale) const;
            void SetSceneName(const std::string& sceneName) const;
            void SetSelectedAsset(std::optional<std::size_t> index);
            void SetHierarchy(
                const std::vector<SceneObjectEntry>& entries, std::optional<entt::entity> selectedEntity);
            void FocusHierarchyOnEntity(entt::entity entity);
            void SetInspector(
                const std::string& selectedEntity, const std::vector<InspectedComponent>& inspectedComponents);
            void DrawSceneViewInfo() const;
            void ShowDeleteConfirmation(const std::string& selectedEntity) const;
            void HideDeleteConfirmation() const;
            [[nodiscard]] bool IsDeleteConfirmationVisible() const;
            [[nodiscard]] DeleteConfirmationAction ConsumeDeleteConfirmationAction();
            EditorGui(
                GameUIEngine* ui,
                Settings* settings,
                const std::vector<AssetEntry>& assets,
                std::function<void(std::size_t)> onAssetSelected,
                std::function<void(entt::entity)> onSceneObjectSelected,
                ModelDefaultCallbacks callbacks);
            ~EditorGui();
        };
    } // namespace editor
} // namespace sage
