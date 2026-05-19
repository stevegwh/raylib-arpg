#pragma once

#include "EditorInspector.hpp"

#include "raylib.h"

#include "entt/entt.hpp"

#include <functional>
#include <optional>
#include <string>
#include <typeindex>
#include <type_traits>
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
        struct InspectorFieldBinding
        {
            std::string componentName;
            std::string fieldName;
            std::type_index valueType = typeid(void);
            std::vector<TextBox*> valueTexts;
        };

        template <class T>
        struct EditorFieldDrawer;

        template <>
        struct EditorFieldDrawer<Vector3>
        {
            static void Draw(const InspectorField& field, const InspectorFieldBinding& binding);
        };

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
            TextBox* inspectorScrollbarUpText{};
            TextBox* inspectorScrollbarTrackText{};
            TextBox* inspectorScrollbarDownText{};
            std::vector<RenderTexture2D> assetThumbnails;
            std::vector<TextBox*> hierarchyRows;
            std::vector<SceneObjectEntry> hierarchyEntries;
            std::size_t hierarchyScrollOffset = 0;
            std::size_t inspectorFieldScrollOffset = 0;
            std::size_t inspectorFieldVisibleRows = 0;
            std::size_t inspectorFieldTotalRows = 0;
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
            Table* inspectorFieldsTable{};
            TextBox* deleteConfirmationText{};

            using InspectorFieldDrawer = std::function<void(const InspectorField&, const InspectorFieldBinding&)>;

            std::vector<InspectorFieldBinding> inspectorFieldBindings;
            std::vector<std::pair<std::type_index, InspectorFieldDrawer>> inspectorFieldDrawers;
            std::string inspectorFieldsSignature;

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
            void registerInspectorFieldDrawers();
            void rebuildInspectorFieldRows(const std::vector<InspectedComponent>& inspectedComponents);
            void drawInspectedFields(const std::vector<InspectedComponent>& inspectedComponents) const;
            [[nodiscard]] const InspectorFieldDrawer* findInspectorFieldDrawer(std::type_index valueType) const;
            [[nodiscard]] std::string buildInspectorFieldsSignature(
                const std::vector<InspectedComponent>& inspectedComponents) const;

            template <class T>
            void registerInspectorFieldDrawer()
            {
                using Value = std::remove_cvref_t<T>;
                inspectorFieldDrawers.push_back(
                    {typeid(Value),
                     [](const InspectorField& field, const InspectorFieldBinding& binding) {
                         EditorFieldDrawer<Value>::Draw(field, binding);
                     }});
            }

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
                const std::vector<InspectedComponent>& inspectedComponents);
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
