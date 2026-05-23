#include "EditorGui.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Settings.hpp"
#include "engine/ui/Scrollbar.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"
#include "engine/ui/UIWindow.hpp"

#include "raymath.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <memory>
#include <optional>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr int THUMBNAIL_SIZE = 128;
        constexpr int ASSET_GRID_COLUMNS = 5;
        constexpr int ASSET_VISIBLE_ROWS = 2;
        constexpr int HIERARCHY_MAX_ROWS = 32;
        constexpr float LEFT_DOCK_WIDTH = 340.0f;
        constexpr float RIGHT_DOCK_WIDTH = 440.0f;
        constexpr float DOCK_HEIGHT = 1080.0f;
        constexpr float ASSET_BROWSER_MARGIN = 18.0f;
        constexpr float ASSET_BROWSER_HEIGHT = 344.0f;
        constexpr float ASSET_BROWSER_WIDTH =
            1920.0f - LEFT_DOCK_WIDTH - RIGHT_DOCK_WIDTH - ASSET_BROWSER_MARGIN * 2.0f;
        constexpr Color EDITOR_WINDOW_BACKGROUND = {35, 38, 43, 245};
        constexpr Color EDITOR_TEXT = {230, 234, 240, 255};
        constexpr float SIDE_DOCK_TITLE_ROW_HEIGHT = 4.0f;
        constexpr float BOTTOM_DOCK_TITLE_ROW_HEIGHT = 10.0f;
        constexpr float FLOATING_PANEL_TITLE_ROW_HEIGHT = 15.0f;
        constexpr Padding CONTENT_ROW_PADDING = {2, 0, 0, 0};
        constexpr Padding CONTENT_CELL_PADDING = {2, 6, 8, 8};

        TextBox::FontInfo EditorTextFontInfo()
        {
            auto info = TextBox::FontInfo{};
            info.color = EDITOR_TEXT;
            return info;
        }

        TextBox::FontInfo EditorTitleFontInfo()
        {
            auto info = EditorTextFontInfo();
            info.font = ResourceManager::GetInstance().FontLoad("resources/fonts/NotoSans/NotoSans-ExtraBold.ttf");
            info.baseFontSize = 22;
            info.minFontSize = 18;
            return info;
        }

        TextBox::FontInfo EditorInspectorFontInfo()
        {
            auto info = EditorTextFontInfo();
            info.font = ResourceManager::GetInstance().FontLoad("resources/fonts/NotoSans/NotoSans-SemiBold.ttf");
            info.baseFontSize = 18;
            info.minFontSize = 15;
            return info;
        }

        // TextInput boxes paint a light background, so the inspector's normal
        // light text colour would be unreadable inside them.
        TextBox::FontInfo EditorInspectorInputFontInfo()
        {
            auto info = EditorInspectorFontInfo();
            info.color = Color{17, 24, 39, 255};
            info.font = ResourceManager::GetInstance().FontLoad("resources/fonts/FiraCode/FiraCode-SemiBold.ttf");
            return info;
        }

        class AssetThumbnailButton final : public ImageBox
        {
            std::optional<std::size_t> assetIndex;
            std::string label;
            RenderTexture2D* thumbnail{};
            std::optional<std::size_t>* selectedAssetIndex{};
            std::function<void(std::size_t)> onAssetSelected;

          public:
            void OnClick() override
            {
                if (assetIndex.has_value() && onAssetSelected) onAssetSelected(*assetIndex);
            }

            void SetAsset(
                const std::optional<std::size_t> index,
                std::string displayName = "",
                RenderTexture2D* assetThumbnail = nullptr)
            {
                assetIndex = index;
                label = std::move(displayName);
                thumbnail = assetThumbnail;
            }

            void UpdateDimensions() override
            {
                rec = {
                    parent->GetRec().x + parent->padding.left,
                    parent->GetRec().y + parent->padding.up,
                    parent->GetRec().width - parent->padding.left - parent->padding.right,
                    parent->GetRec().height - parent->padding.up - parent->padding.down};
            }

            void Draw2D() override
            {
                if (!assetIndex.has_value())
                {
                    DrawRectangleRec(rec, Color{39, 42, 47, 180});
                    DrawRectangleLinesEx(rec, 1.0f, Color{75, 82, 94, 180});
                    return;
                }

                const bool selected =
                    selectedAssetIndex && selectedAssetIndex->has_value() && **selectedAssetIndex == *assetIndex;
                const Color background = selected ? Color{221, 235, 255, 255} : Color{245, 247, 250, 255};
                const Color border = selected ? Color{37, 99, 235, 255} : Color{171, 181, 196, 255};

                DrawRectangleRec(rec, background);
                DrawRectangleLinesEx(rec, selected ? 3.0f : 1.0f, border);

                const float labelHeight = 28.0f;
                const float imageSize = std::max(0.0f, std::min(rec.width, rec.height - labelHeight - 6.0f));
                const Rectangle imageDest = {
                    rec.x + (rec.width - imageSize) * 0.5f, rec.y + 6.0f, imageSize, imageSize};

                if (thumbnail && thumbnail->texture.id != 0)
                {
                    DrawTexturePro(
                        thumbnail->texture,
                        {0.0f,
                         0.0f,
                         static_cast<float>(thumbnail->texture.width),
                         -static_cast<float>(thumbnail->texture.height)},
                        imageDest,
                        Vector2Zero(),
                        0.0f,
                        WHITE);
                }

                int fontSize = 16;
                const Font font =
                    ResourceManager::GetInstance().FontLoad("resources/fonts/FiraCode/FiraCode-SemiBold.ttf");
                const int maxTextWidth = static_cast<int>(std::max(0.0f, rec.width - 12.0f));
                while (fontSize > 12 && MeasureTextEx(font, label.c_str(), fontSize, 1.0f).x > maxTextWidth)
                {
                    --fontSize;
                }
                const Vector2 textSize = MeasureTextEx(font, label.c_str(), fontSize, 1.0f);
                DrawTextEx(
                    font,
                    label.c_str(),
                    {rec.x + (rec.width - textSize.x) * 0.5f,
                     rec.y + rec.height - labelHeight + (labelHeight - textSize.y) * 0.5f},
                    fontSize,
                    1.0f,
                    BLACK);
            }

            AssetThumbnailButton(
                GameUIEngine* ui,
                TableCell* parent,
                std::optional<std::size_t>* selectedIndex,
                std::function<void(std::size_t)> callback)
                : ImageBox(
                      ui,
                      parent,
                      ResourceManager::GetInstance().TextureLoad("resources/transpixel.png"),
                      ImageBox::OverflowBehaviour::ALLOW_OVERFLOW),
                  selectedAssetIndex(selectedIndex),
                  onAssetSelected(std::move(callback))
            {
            }
        };

        class TextButton final : public TextBox
        {
            std::function<void()> onPressed;
            std::function<bool()> isVisible;

          public:
            void OnClick() override
            {
                if (isVisible && !isVisible()) return;
                if (onPressed) onPressed();
            }

            void UpdateDimensions() override
            {
                rec = {
                    parent->GetRec().x + parent->padding.left,
                    parent->GetRec().y + parent->padding.up,
                    parent->GetRec().width - parent->padding.left - parent->padding.right,
                    parent->GetRec().height - parent->padding.up - parent->padding.down};
            }

            void Draw2D() override
            {
                if (isVisible && !isVisible()) return;
                DrawRectangleRec(rec, Color{233, 238, 246, 255});
                DrawRectangleLinesEx(rec, 1.0f, Color{151, 164, 184, 255});
                const int fontSize = static_cast<int>(fontInfo.fontSize);
                const Vector2 textSize =
                    MeasureTextEx(fontInfo.font, GetContent().c_str(), fontSize, fontInfo.fontSpacing);
                DrawTextEx(
                    fontInfo.font,
                    GetContent().c_str(),
                    {rec.x + (rec.width - textSize.x) * 0.5f, rec.y + (rec.height - textSize.y) * 0.5f},
                    fontSize,
                    fontInfo.fontSpacing,
                    BLACK);
            }

            TextButton(
                GameUIEngine* ui,
                TableCell* parent,
                std::function<void()> callback,
                std::function<bool()> visiblePredicate = {})
                : TextBox(ui, parent, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::CENTER),
                  onPressed(std::move(callback)),
                  isVisible(std::move(visiblePredicate))
            {
            }
        };

        class HierarchyRowButton final : public TextBox
        {
            std::size_t rowIndex = 0;
            const std::vector<EditorGui::SceneObjectEntry>* entries{};
            std::function<std::size_t()> scrollOffset;
            const std::optional<entt::entity>* selectedEntity{};
            std::function<void(entt::entity)> onSceneObjectSelected;

          public:
            void OnClick() override
            {
                if (!entries || !scrollOffset || !onSceneObjectSelected) return;
                const std::size_t entryIndex = scrollOffset() + rowIndex;
                if (entryIndex >= entries->size()) return;
                onSceneObjectSelected(entries->at(entryIndex).entity);
            }

            void UpdateDimensions() override
            {
                UpdateFontScaling();
                rec = {
                    parent->GetRec().x + parent->padding.left,
                    parent->GetRec().y + parent->padding.up,
                    parent->GetRec().width - parent->padding.left - parent->padding.right,
                    parent->GetRec().height - parent->padding.up - parent->padding.down};
            }

            void Draw2D() override
            {
                const std::size_t entryIndex = scrollOffset ? scrollOffset() + rowIndex : rowIndex;
                const bool hasEntry = entries && entryIndex < entries->size();
                const bool selected = hasEntry && selectedEntity && selectedEntity->has_value() &&
                                      selectedEntity->value() == entries->at(entryIndex).entity;

                if (hasEntry)
                {
                    DrawRectangleRec(rec, selected ? Color{221, 235, 255, 255} : Color{246, 248, 251, 255});
                    DrawRectangleLinesEx(
                        rec,
                        selected ? 2.0f : 1.0f,
                        selected ? Color{37, 99, 235, 255} : Color{208, 216, 228, 255});
                }

                if (GetContent().empty()) return;

                DrawTextEx(
                    fontInfo.font,
                    GetContent().c_str(),
                    Vector2{rec.x + 14.0f, rec.y + (rec.height - fontInfo.fontSize) * 0.5f},
                    fontInfo.fontSize,
                    fontInfo.fontSpacing,
                    fontInfo.color);
            }

            HierarchyRowButton(
                GameUIEngine* ui,
                TableCell* parent,
                std::size_t index,
                const std::vector<EditorGui::SceneObjectEntry>* sceneEntries,
                std::function<std::size_t()> firstVisibleEntry,
                const std::optional<entt::entity>* activeEntity,
                std::function<void(entt::entity)> callback)
                : TextBox(ui, parent, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::LEFT),
                  rowIndex(index),
                  entries(sceneEntries),
                  scrollOffset(std::move(firstVisibleEntry)),
                  selectedEntity(activeEntity),
                  onSceneObjectSelected(std::move(callback))
            {
            }
        };

        void DrawTextFit(
            const Font font,
            const std::string& text,
            const Vector2 position,
            const float maxWidth,
            int fontSize,
            const Color color)
        {
            while (fontSize > 12 && MeasureTextEx(font, text.c_str(), fontSize, 1.0f).x > maxWidth)
            {
                --fontSize;
            }

            DrawTextEx(font, text.c_str(), {position.x + 1.0f, position.y + 1.0f}, fontSize, 1.0f, BLACK);
            DrawTextEx(font, text.c_str(), position, fontSize, 1.0f, color);
        }

    } // namespace

    void EditorGui::SetOverlayStatus(const std::string& mode, const std::string& cursor) const
    {
        modeStatus = mode;
        cursorStatus = cursor;
    }

    void EditorGui::SetAssetDefaultsStatus(
        const std::string& assetName,
        const std::string& modelDefaultHeight,
        const std::string& modelDefaultRotation,
        const std::string& modelDefaultScale) const
    {
        if (defaultsAssetText) defaultsAssetText->SetContent("Asset: " + assetName);
        if (defaultsPositionText) defaultsPositionText->SetContent(modelDefaultHeight);
        if (defaultsRotationText) defaultsRotationText->SetContent(modelDefaultRotation);
        if (defaultsScaleText) defaultsScaleText->SetContent(modelDefaultScale);
    }

    void EditorGui::SetSceneName(const std::string& sceneName) const
    {
        sceneNameStatus = sceneName;
    }

    void EditorGui::SetSelectedAsset(const std::optional<std::size_t> index)
    {
        selectedAssetIndex = index;
        if (!assetDefaultsWindow) return;
        if (selectedAssetIndex.has_value())
        {
            if (assetDefaultsWindow->IsHidden()) assetDefaultsWindow->Show();
        }
        else
        {
            if (!assetDefaultsWindow->IsHidden()) assetDefaultsWindow->Hide();
        }
    }

    void EditorGui::refreshHierarchyRowContent()
    {
        const std::size_t scrollOffset = hierarchyWindow && hierarchyWindow->GetScrollbar()
                                             ? hierarchyWindow->GetScrollbar()->ScrollOffset()
                                             : 0;
        for (std::size_t i = 0; i < hierarchyRows.size(); ++i)
        {
            const std::size_t entryIndex = scrollOffset + i;
            if (entryIndex >= hierarchyEntries.size())
            {
                hierarchyRows[i]->SetContent("");
                continue;
            }

            const auto& entry = hierarchyEntries[entryIndex];
            hierarchyRows[i]->SetContent(
                std::string(static_cast<std::size_t>(entry.depth * 2), ' ') + entry.displayName);
        }
    }

    void EditorGui::SetHierarchy(
        const std::vector<SceneObjectEntry>& entries, const std::optional<entt::entity> selectedEntity)
    {
        hierarchyEntries = entries;
        selectedSceneEntity = selectedEntity;
        if (auto* sb = hierarchyWindow ? hierarchyWindow->GetScrollbar() : nullptr) sb->ClampOffset();
        refreshHierarchyRowContent();
    }

    void EditorGui::FocusHierarchyOnEntity(const entt::entity entity)
    {
        auto* sb = hierarchyWindow ? hierarchyWindow->GetScrollbar() : nullptr;
        if (sb == nullptr) return;

        const auto it = std::ranges::find_if(
            hierarchyEntries, [entity](const SceneObjectEntry& entry) { return entry.entity == entity; });
        if (it == hierarchyEntries.end()) return;

        const auto entryIndex = static_cast<std::size_t>(std::distance(hierarchyEntries.begin(), it));
        const std::size_t visibleRows = sb->VisibleRows();
        const std::size_t centeredOffset = entryIndex > visibleRows / 2 ? entryIndex - visibleRows / 2 : 0;
        sb->SetScrollOffset(centeredOffset);
        refreshHierarchyRowContent();
    }

    void EditorGui::SetInspector(
        const std::string& selectedEntity, const std::vector<InspectedComponent>& inspectedComponents)
    {
        if (inspectorSelectionText) inspectorSelectionText->SetContent("Selected: " + selectedEntity);
        inspectorFieldBlueprints.Rebuild(inspectedComponents);
        inspectorFieldBlueprints.Draw();
    }

    void EditorGui::DrawSceneViewInfo() const
    {
        if (!ui || !ui->settings) return;

        const auto renderViewport = ui->settings->GetRenderViewportScreenRect();
        const float x = renderViewport.x + ui->settings->ScaleValueWidth(16.0f);
        const float y = renderViewport.y + ui->settings->ScaleValueHeight(14.0f);
        const float maxWidth = std::max(1.0f, renderViewport.width - ui->settings->ScaleValueWidth(32.0f));
        const int titleSize = std::max(22, static_cast<int>(ui->settings->ScaleValueMaintainRatio(22.0f)));
        const int metaSize = std::max(16, static_cast<int>(ui->settings->ScaleValueMaintainRatio(16.0f)));
        const Font titleFont =
            ResourceManager::GetInstance().FontLoad("resources/fonts/FiraCode/FiraCode-Bold.ttf");
        const Font metaFont =
            ResourceManager::GetInstance().FontLoad("resources/fonts/FiraCode/FiraCode-SemiBold.ttf");

        DrawTextFit(titleFont, sceneNameStatus, {x, y}, maxWidth, titleSize, EDITOR_TEXT);
        DrawTextFit(
            metaFont,
            "Mode: " + modeStatus + "  |  Cursor: " + cursorStatus,
            {x, y + ui->settings->ScaleValueHeight(28.0f)},
            maxWidth,
            metaSize,
            Color{202, 211, 224, 255});
    }

    void EditorGui::ShowDeleteConfirmation(const std::string& selectedEntity) const
    {
        if (deleteConfirmationText)
        {
            deleteConfirmationText->SetContent("Delete " + selectedEntity + "?");
        }
        if (deleteConfirmationWindow && deleteConfirmationWindow->IsHidden())
        {
            deleteConfirmationWindow->Show();
        }
    }

    void EditorGui::HideDeleteConfirmation() const
    {
        if (deleteConfirmationWindow && !deleteConfirmationWindow->IsHidden())
        {
            deleteConfirmationWindow->Hide();
        }
    }

    bool EditorGui::IsDeleteConfirmationVisible() const
    {
        return deleteConfirmationWindow && !deleteConfirmationWindow->IsHidden();
    }

    EditorGui::DeleteConfirmationAction EditorGui::ConsumeDeleteConfirmationAction()
    {
        const auto action = pendingDeleteConfirmationAction;
        pendingDeleteConfirmationAction = DeleteConfirmationAction::None;
        return action;
    }

    void EditorGui::refreshAssetButtonContent()
    {
        const std::size_t scrollRow =
            assetWindow && assetWindow->GetScrollbar() ? assetWindow->GetScrollbar()->ScrollOffset() : 0;
        const std::size_t firstAssetIndex = scrollRow * ASSET_GRID_COLUMNS;

        for (std::size_t slot = 0; slot < assetButtons.size(); ++slot)
        {
            auto* button = dynamic_cast<AssetThumbnailButton*>(assetButtons[slot]);
            if (!button) continue;

            const std::size_t assetIndex = firstAssetIndex + slot;
            if (assetIndex >= assetEntries.size())
            {
                button->SetAsset(std::nullopt);
                continue;
            }

            button->SetAsset(assetIndex, assetEntries[assetIndex].displayName, &assetThumbnails.at(assetIndex));
        }
    }

    RenderTexture2D EditorGui::createAssetThumbnail(const AssetEntry& asset) const
    {
        auto thumbnail = LoadRenderTexture(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
        auto model = ResourceManager::GetInstance().GetModelView(asset.modelKey);
        const auto bounds = model.CalcLocalBoundingBox();
        const Vector3 size = Vector3Subtract(bounds.max, bounds.min);
        const Vector3 center = Vector3Scale(Vector3Add(bounds.min, bounds.max), 0.5f);
        const float radius = std::max({std::fabs(size.x), std::fabs(size.y), std::fabs(size.z), 1.0f});

        Camera3D camera{};
        camera.position = Vector3Add(center, {radius * 1.35f, radius * 0.85f, radius * 1.65f});
        camera.target = center;
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 32.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        BeginTextureMode(thumbnail);
        ClearBackground(Color{244, 247, 251, 255});
        BeginMode3D(camera);
        model.Draw(Vector3Zero(), {0.0f, 1.0f, 0.0f}, 0.0f, Vector3One(), WHITE);
        EndMode3D();
        EndTextureMode();

        return thumbnail;
    }

    void EditorGui::createHierarchyWindow(const std::function<void(entt::entity)>& onSceneObjectSelected)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            0.0f,
            0.0f,
            LEFT_DOCK_WIDTH,
            DOCK_HEIGHT,
            VertAlignment::TOP,
            HoriAlignment::LEFT,
            Padding{18, 16, 14, 14});

        hierarchyWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = hierarchyWindow->CreateTable({0, 0, 1, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(SIDE_DOCK_TITLE_ROW_HEIGHT);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTitleFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Hierarchy");
        }

        hierarchyWindow->SetOverflowContingency(OverflowContingency::SCROLLBAR);

        {
            auto* contentRow = mainTable->CreateTableRow(CONTENT_ROW_PADDING);
            auto* listCell = contentRow->CreateTableCell(Padding{2, 8, 8, 2});
            auto* table = listCell->CreateTable();

            hierarchyRows.reserve(HIERARCHY_MAX_ROWS);
            for (int i = 0; i < HIERARCHY_MAX_ROWS; ++i)
            {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell(Padding{3, 3, 6, 6});
                auto button = std::make_unique<HierarchyRowButton>(
                    ui,
                    cell,
                    static_cast<std::size_t>(i),
                    &hierarchyEntries,
                    [this]() {
                        auto* sb = hierarchyWindow ? hierarchyWindow->GetScrollbar() : nullptr;
                        return sb ? sb->ScrollOffset() : std::size_t{0};
                    },
                    &selectedSceneEntity,
                    onSceneObjectSelected);
                hierarchyRows.push_back(cell->CreateTextbox(std::move(button), ""));
            }
        }

        if (auto* sb = hierarchyWindow->GetScrollbar())
        {
            sb->SetProviders(
                [this]() { return hierarchyEntries.size(); }, [this]() { return hierarchyRows.size(); });
            hierarchyScrollSub = sb->onScrollChanged.Subscribe([this]() { refreshHierarchyRowContent(); });
        }

        hierarchyWindow->FinalizeLayout();
    }

    void EditorGui::createAssetWindow(
        const std::vector<AssetEntry>& assets, const std::function<void(std::size_t)>& onAssetSelected)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            LEFT_DOCK_WIDTH + ASSET_BROWSER_MARGIN,
            -ASSET_BROWSER_MARGIN,
            ASSET_BROWSER_WIDTH,
            ASSET_BROWSER_HEIGHT,
            VertAlignment::BOTTOM,
            HoriAlignment::LEFT,
            Padding{20, 16, 14, 14});

        assetWindow = ui->CreateWindowDocked(std::move(window));
        assetWindow->SetOverflowContingency(OverflowContingency::SCROLLBAR);
        auto* mainTable = assetWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(BOTTOM_DOCK_TITLE_ROW_HEIGHT);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTitleFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Assets");
        }

        {
            auto* contentRow = mainTable->CreateTableRow(CONTENT_ROW_PADDING);
            auto* contentCell = contentRow->CreateTableCell(CONTENT_CELL_PADDING);
            auto* table = contentCell->CreateTable();

            assetButtons.clear();
            assetButtons.reserve(ASSET_VISIBLE_ROWS * ASSET_GRID_COLUMNS);
            for (int rowIndex = 0; rowIndex < ASSET_VISIBLE_ROWS; ++rowIndex)
            {
                auto* row = table->CreateTableRow(Padding{0, 0, 0, 0});
                for (int colIndex = 0; colIndex < ASSET_GRID_COLUMNS; ++colIndex)
                {
                    auto* cell = row->CreateTableCell(Padding{5, 5, 5, 5});
                    auto thumbnail =
                        std::make_unique<AssetThumbnailButton>(ui, cell, &selectedAssetIndex, onAssetSelected);
                    assetButtons.push_back(cell->CreateImagebox(std::move(thumbnail)));
                }
            }
        }

        if (auto* sb = assetWindow->GetScrollbar())
        {
            sb->SetProviders(
                [this]() { return (assetEntries.size() + ASSET_GRID_COLUMNS - 1) / ASSET_GRID_COLUMNS; },
                []() { return static_cast<std::size_t>(ASSET_VISIBLE_ROWS); });
            assetScrollSub = sb->onScrollChanged.Subscribe([this]() { refreshAssetButtonContent(); });
        }

        assetWindow->FinalizeLayout();
        refreshAssetButtonContent();
    }

    void EditorGui::createAssetDefaultsWindow()
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            LEFT_DOCK_WIDTH + ASSET_BROWSER_MARGIN,
            -(ASSET_BROWSER_MARGIN + ASSET_BROWSER_HEIGHT + 12.0f),
            392.0f,
            232.0f,
            VertAlignment::BOTTOM,
            HoriAlignment::LEFT,
            Padding{20, 16, 14, 14});

        assetDefaultsWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = assetDefaultsWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(FLOATING_PANEL_TITLE_ROW_HEIGHT);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTitleFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Asset Defaults");
        }

        {
            auto* contentRow = mainTable->CreateTableRow(CONTENT_ROW_PADDING);
            auto* contentCell = contentRow->CreateTableCell(CONTENT_CELL_PADDING);
            auto* table = contentCell->CreateTable();

            auto addLine = [this, table](const char* text) {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell();
                auto label = std::make_unique<TextBox>(ui, cell, EditorTextFontInfo());
                return cell->CreateTextbox(std::move(label), text);
            };

            defaultsAssetText = addLine("Asset: None");

            auto addControlRow = [this, table](
                                     const char* labelText,
                                     const char* initialValue,
                                     const std::function<void()>& onDown,
                                     const std::function<void()>& onUp) {
                auto* row = table->CreateTableRow();

                auto* labelCell = row->CreateTableCell(36.0f);
                auto label = std::make_unique<TextBox>(ui, labelCell, EditorTextFontInfo());
                labelCell->CreateTextbox(std::move(label), labelText);

                auto* downCell = row->CreateTableCell(15.0f, Padding{1, 1, 2, 2});
                auto downButton = std::make_unique<TextButton>(ui, downCell, onDown);
                downCell->CreateTextbox(std::move(downButton), "-");

                auto* valueCell = row->CreateTableCell(34.0f);
                auto value = std::make_unique<TextBox>(
                    ui, valueCell, EditorTextFontInfo(), VertAlignment::TOP, HoriAlignment::CENTER);
                auto* valueText = valueCell->CreateTextbox(std::move(value), initialValue);

                auto* upCell = row->CreateTableCell(15.0f, Padding{1, 1, 2, 2});
                auto upButton = std::make_unique<TextButton>(ui, upCell, onUp);
                upCell->CreateTextbox(std::move(upButton), "+");

                return valueText;
            };

            defaultsPositionText =
                addControlRow("Z", "0.00", modelDefaultCallbacks.heightDown, modelDefaultCallbacks.heightUp);
            defaultsRotationText =
                addControlRow("Rot Y", "0", modelDefaultCallbacks.rotationDown, modelDefaultCallbacks.rotationUp);
            defaultsScaleText =
                addControlRow("Scale", "1.00", modelDefaultCallbacks.scaleDown, modelDefaultCallbacks.scaleUp);

            auto* buttonRow = table->CreateTableRow();

            auto* applyCell = buttonRow->CreateTableCell(50.0f, Padding{3, 3, 4, 4});
            auto applyButton = std::make_unique<TextButton>(ui, applyCell, modelDefaultCallbacks.apply);
            applyCell->CreateTextbox(std::move(applyButton), "Apply");

            auto* resetCell = buttonRow->CreateTableCell(50.0f, Padding{3, 3, 4, 4});
            auto resetButton = std::make_unique<TextButton>(ui, resetCell, modelDefaultCallbacks.reset);
            resetCell->CreateTextbox(std::move(resetButton), "Reset");
        }

        assetDefaultsWindow->FinalizeLayout();
        assetDefaultsWindow->Hide();
    }

    void EditorGui::createInspectorWindow()
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            0.0f,
            0.0f,
            RIGHT_DOCK_WIDTH,
            DOCK_HEIGHT,
            VertAlignment::TOP,
            HoriAlignment::RIGHT,
            Padding{20, 16, 14, 14});

        inspectorWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = inspectorWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(SIDE_DOCK_TITLE_ROW_HEIGHT);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTitleFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Inspector");
        }

        inspectorWindow->SetOverflowContingency(OverflowContingency::SCROLLBAR);

        {
            auto* contentRow = mainTable->CreateTableRow(CONTENT_ROW_PADDING);
            auto* contentCell = contentRow->CreateTableCell(CONTENT_CELL_PADDING);
            auto* table = contentCell->CreateTable();

            auto addLine = [this, table](const char* text) {
                auto* row = table->CreateTableRow(6.0f);
                auto* cell = row->CreateTableCell(Padding{2, 2, 2, 2});
                auto label = std::make_unique<TextBox>(
                    ui, cell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                return cell->CreateTextbox(std::move(label), text);
            };

            inspectorSelectionText = addLine("Selected: None");

            auto* fieldsRow = table->CreateTableRow(Padding{2, 0, 0, 0});
            auto* fieldsCell = fieldsRow->CreateTableCell(Padding{2, 2, 2, 2});
            auto* inspectorFieldsTable = fieldsCell->CreateTable();
            inspectorFieldBlueprints.Attach(ui, inspectorFieldsTable);
        }

        if (auto* sb = inspectorWindow->GetScrollbar())
        {
            sb->SetProviders(
                [this]() { return inspectorFieldBlueprints.TotalRows(); },
                [this]() { return inspectorFieldBlueprints.VisibleRows(); });
            inspectorFieldBlueprints.AttachScrollbar(sb);
        }

        inspectorWindow->FinalizeLayout();
    }

    void EditorGui::createDeleteConfirmationWindow()
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            0.0f,
            0.0f,
            420.0f,
            164.0f,
            VertAlignment::MIDDLE,
            HoriAlignment::CENTER,
            Padding{20, 16, 14, 14});

        deleteConfirmationWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = deleteConfirmationWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(FLOATING_PANEL_TITLE_ROW_HEIGHT);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTitleFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Confirm Delete");
        }

        {
            auto* textRow = mainTable->CreateTableRow(44.0f);
            auto* textCell = textRow->CreateTableCell({8, 8, 8, 8});
            auto text = std::make_unique<TextBox>(
                ui, textCell, EditorTextFontInfo(), VertAlignment::MIDDLE, HoriAlignment::CENTER);
            deleteConfirmationText = textCell->CreateTextbox(std::move(text), "Delete selected entity?");
        }

        {
            auto* buttonRow = mainTable->CreateTableRow(34.0f);

            auto* confirmCell = buttonRow->CreateTableCell(50.0f, Padding{3, 3, 4, 4});
            auto confirmButton = std::make_unique<TextButton>(ui, confirmCell, [this]() {
                pendingDeleteConfirmationAction = DeleteConfirmationAction::Confirm;
            });
            confirmCell->CreateTextbox(std::move(confirmButton), "Delete");

            auto* cancelCell = buttonRow->CreateTableCell(50.0f, Padding{3, 3, 4, 4});
            auto cancelButton = std::make_unique<TextButton>(
                ui, cancelCell, [this]() { pendingDeleteConfirmationAction = DeleteConfirmationAction::Cancel; });
            cancelCell->CreateTextbox(std::move(cancelButton), "Cancel");
        }

        deleteConfirmationWindow->FinalizeLayout();
        deleteConfirmationWindow->Hide();
    }

    EditorGui::EditorGui(
        GameUIEngine* _ui,
        Settings* _settings,
        const std::vector<AssetEntry>& assets,
        const std::function<void(std::size_t)>& onAssetSelected,
        const std::function<void(entt::entity)>& onSceneObjectSelected,
        ModelDefaultCallbacks callbacks)
        : ui(_ui), settings(_settings), modelDefaultCallbacks(std::move(callbacks))
    {
        Image panelImage = GenImageColor(1, 1, EDITOR_WINDOW_BACKGROUND);
        editorWindowBackgroundTexture = LoadTextureFromImage(panelImage);
        UnloadImage(panelImage);

        assetEntries = assets;
        assetThumbnails.reserve(assetEntries.size());
        for (const auto& asset : assetEntries)
        {
            assetThumbnails.push_back(createAssetThumbnail(asset));
        }

        createHierarchyWindow(onSceneObjectSelected);
        createAssetWindow(assets, onAssetSelected);
        createAssetDefaultsWindow();
        createInspectorWindow();
        createDeleteConfirmationWindow();
    }

    EditorGui::~EditorGui()
    {
        if (editorWindowBackgroundTexture.id != 0)
        {
            UnloadTexture(editorWindowBackgroundTexture);
        }

        for (auto& thumbnail : assetThumbnails)
        {
            if (thumbnail.id != 0)
            {
                UnloadRenderTexture(thumbnail);
            }
        }
    }
} // namespace sage::editor
