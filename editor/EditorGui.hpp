#pragma once

#include "InspectorFieldBlueprint.hpp"

#include "raylib.h"

#include "entt/entt.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace sage
{
    class GameUIEngine;
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
            GameUIEngine* ui{};
            Texture2D editorWindowBackgroundTexture{};
            TextBox* hierarchyScrollbarUpText{};
            TextBox* hierarchyScrollbarTrackText{};
            TextBox* hierarchyScrollbarDownText{};
            std::vector<RenderTexture2D> assetThumbnails;
            std::vector<TextBox*> hierarchyRows;
            std::vector<SceneObjectEntry> hierarchyEntries;
            std::size_t hierarchyScrollOffset = 0;
            InspectorFieldBlueprint inspectorFieldBlueprints;
            ModelDefaultCallbacks modelDefaultCallbacks;
            DeleteConfirmationCallbacks deleteConfirmationCallbacks;
            std::optional<std::size_t> selectedAssetIndex;
            std::optional<entt::entity> selectedSceneEntity;
            TextBox* overlayTitleText{};
            TextBox* modeText{};
            TextBox* cursorText{};
            TextBox* defaultsAssetText{};
            TextBox* defaultsPositionText{};
            TextBox* defaultsRotationText{};
            TextBox* defaultsScaleText{};
            TextBox* inspectorSelectionText{};
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
            void scrollInspectorFields(int amount);

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
            void SetInspector(
                const std::string& selectedEntity, const std::vector<InspectedComponent>& inspectedComponents);
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
                DeleteConfirmationCallbacks deleteConfirmationCallbacks);
            ~EditorGui();
        };
    } // namespace editor
} // namespace sage
