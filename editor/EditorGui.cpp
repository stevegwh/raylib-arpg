#include "EditorGui.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Settings.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"
#include "engine/ui/UIWindow.hpp"

#include "raymath.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <memory>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr int kThumbnailSize = 128;
        constexpr int kHierarchyMaxRows = 18;
        constexpr Color kEditorWindowBackground = {35, 38, 43, 245};
        constexpr Color kEditorText = {230, 234, 240, 255};

        TextBox::FontInfo EditorTextFontInfo()
        {
            auto info = TextBox::FontInfo{};
            info.color = kEditorText;
            return info;
        }

        class AssetThumbnailButton final : public ImageBox
        {
            std::size_t assetIndex = 0;
            std::string label;
            RenderTexture2D* thumbnail{};
            std::optional<std::size_t>* selectedAssetIndex{};
            std::function<void(std::size_t)> onAssetSelected;

          public:
            void OnClick() override
            {
                if (onAssetSelected) onAssetSelected(assetIndex);
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
                const bool selected =
                    selectedAssetIndex && selectedAssetIndex->has_value() && **selectedAssetIndex == assetIndex;
                const Color background = selected ? Color{221, 235, 255, 255} : Color{245, 247, 250, 255};
                const Color border = selected ? Color{37, 99, 235, 255} : Color{171, 181, 196, 255};

                DrawRectangleRec(rec, background);
                DrawRectangleLinesEx(rec, selected ? 3.0f : 1.0f, border);

                const float labelHeight = 22.0f;
                const float imageSize = std::max(0.0f, std::min(rec.width, rec.height - labelHeight - 6.0f));
                const Rectangle imageDest = {
                    rec.x + (rec.width - imageSize) * 0.5f,
                    rec.y + 6.0f,
                    imageSize,
                    imageSize};

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

                constexpr int fontSize = 14;
                const int textWidth = MeasureText(label.c_str(), fontSize);
                DrawText(
                    label.c_str(),
                    static_cast<int>(rec.x + (rec.width - static_cast<float>(textWidth)) * 0.5f),
                    static_cast<int>(rec.y + rec.height - labelHeight),
                    fontSize,
                    BLACK);
            }

            AssetThumbnailButton(
                GameUIEngine* ui,
                TableCell* parent,
                std::size_t index,
                std::string displayName,
                RenderTexture2D* assetThumbnail,
                std::optional<std::size_t>* selectedIndex,
                std::function<void(std::size_t)> callback)
                : ImageBox(
                      ui,
                      parent,
                      ResourceManager::GetInstance().TextureLoad("resources/transpixel.png"),
                      ImageBox::OverflowBehaviour::ALLOW_OVERFLOW),
                  assetIndex(index),
                  label(std::move(displayName)),
                  thumbnail(assetThumbnail),
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
                const int textWidth = MeasureText(GetContent().c_str(), fontSize);
                DrawText(
                    GetContent().c_str(),
                    static_cast<int>(rec.x + (rec.width - static_cast<float>(textWidth)) * 0.5f),
                    static_cast<int>(rec.y + (rec.height - static_cast<float>(fontSize)) * 0.5f),
                    fontSize,
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
            const std::size_t* scrollOffset{};
            const std::optional<entt::entity>* selectedEntity{};
            std::function<void(entt::entity)> onSceneObjectSelected;

          public:
            void OnClick() override
            {
                if (!entries || !scrollOffset || !onSceneObjectSelected) return;
                const std::size_t entryIndex = *scrollOffset + rowIndex;
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
                const std::size_t entryIndex = scrollOffset ? *scrollOffset + rowIndex : rowIndex;
                const bool hasEntry = entries && entryIndex < entries->size();
                const bool selected = hasEntry && selectedEntity && selectedEntity->has_value() &&
                                      selectedEntity->value() == entries->at(entryIndex).entity;

                if (hasEntry)
                {
                    DrawRectangleRec(
                        rec,
                        selected ? Color{221, 235, 255, 255} : Color{246, 248, 251, 255});
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
                const std::size_t* firstVisibleEntry,
                const std::optional<entt::entity>* activeEntity,
                std::function<void(entt::entity)> callback)
                : TextBox(ui, parent, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::LEFT),
                  rowIndex(index),
                  entries(sceneEntries),
                  scrollOffset(firstVisibleEntry),
                  selectedEntity(activeEntity),
                  onSceneObjectSelected(std::move(callback))
            {
            }
        };

        class ScrollbarTrack final : public TextBox
        {
            const std::vector<EditorGui::SceneObjectEntry>* entries{};
            const std::size_t* scrollOffset{};
            const std::vector<TextBox*>* visibleRows{};

          public:
            [[nodiscard]] bool ShouldShow() const
            {
                return entries && visibleRows && entries->size() > visibleRows->size();
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
                if (!ShouldShow()) return;
                DrawRectangleRec(rec, Color{24, 26, 30, 255});
                DrawRectangleLinesEx(rec, 1.0f, Color{69, 74, 84, 255});

                if (!entries || !scrollOffset || !visibleRows || entries->empty() || visibleRows->empty()) return;
                const float visibleCount = static_cast<float>(visibleRows->size());
                const float totalCount = static_cast<float>(entries->size());
                const float thumbHeight = totalCount <= visibleCount
                                              ? rec.height
                                              : std::max(18.0f, rec.height * (visibleCount / totalCount));
                const std::size_t maxOffset =
                    entries->size() > visibleRows->size() ? entries->size() - visibleRows->size() : 0;
                const float scrollRatio =
                    maxOffset == 0 ? 0.0f : static_cast<float>(*scrollOffset) / static_cast<float>(maxOffset);
                const float thumbY = rec.y + (rec.height - thumbHeight) * scrollRatio;
                DrawRectangleRec({rec.x + 2.0f, thumbY + 2.0f, rec.width - 4.0f, thumbHeight - 4.0f}, Color{139, 148, 164, 255});
            }

            ScrollbarTrack(
                GameUIEngine* ui,
                TableCell* parent,
                const std::vector<EditorGui::SceneObjectEntry>* sceneEntries,
                const std::size_t* firstVisibleEntry,
                const std::vector<TextBox*>* rowElements)
                : TextBox(ui, parent, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::CENTER),
                  entries(sceneEntries),
                  scrollOffset(firstVisibleEntry),
                  visibleRows(rowElements)
            {
            }
        };
    } // namespace

    void EditorGui::SetPlacementStatus(
        const std::string& mode,
        const std::string& selectedAsset,
        const std::string& hoveredGrid,
        const std::string& placementHeight,
        const std::string& placementRotation,
        const std::string& placementScale,
        const std::string& modelDefaultHeight,
        const std::string& modelDefaultRotation,
        const std::string& modelDefaultScale,
        const std::string& lastPlaced) const
    {
        if (modeText) modeText->SetContent("Mode: " + mode);
        if (selectedAssetText) selectedAssetText->SetContent("Asset: " + selectedAsset);
        if (gridText) gridText->SetContent("Grid: " + hoveredGrid);
        if (placementHeightText) placementHeightText->SetContent("Z: " + placementHeight);
        if (placementRotationText) placementRotationText->SetContent("Rot: " + placementRotation);
        if (placementScaleText) placementScaleText->SetContent("Scale: " + placementScale);
        if (lastPlacedText) lastPlacedText->SetContent("Last: " + lastPlaced);
        if (defaultsAssetText) defaultsAssetText->SetContent("Asset: " + selectedAsset);
        if (defaultsPositionText) defaultsPositionText->SetContent(modelDefaultHeight);
        if (defaultsRotationText) defaultsRotationText->SetContent(modelDefaultRotation);
        if (defaultsScaleText) defaultsScaleText->SetContent(modelDefaultScale);
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

    void EditorGui::scrollHierarchy(const int amount)
    {
        const std::size_t visibleRows = hierarchyRows.size();
        const std::size_t maxOffset = hierarchyEntries.size() > visibleRows ? hierarchyEntries.size() - visibleRows : 0;

        if (amount < 0)
        {
            const auto positiveAmount = static_cast<std::size_t>(-amount);
            hierarchyScrollOffset =
                positiveAmount > hierarchyScrollOffset ? 0 : hierarchyScrollOffset - positiveAmount;
        }
        else if (amount > 0)
        {
            hierarchyScrollOffset = std::min(maxOffset, hierarchyScrollOffset + static_cast<std::size_t>(amount));
        }
    }

    void EditorGui::SetHierarchy(
        const std::vector<SceneObjectEntry>& entries,
        const std::optional<entt::entity> selectedEntity)
    {
        hierarchyEntries = entries;
        selectedSceneEntity = selectedEntity;

        const std::size_t visibleRows = hierarchyRows.size();
        const std::size_t maxOffset = hierarchyEntries.size() > visibleRows ? hierarchyEntries.size() - visibleRows : 0;
        hierarchyScrollOffset = std::min(hierarchyScrollOffset, maxOffset);
        const bool showScrollbar = maxOffset > 0;
        if (hierarchyScrollbarUpText)
            hierarchyScrollbarUpText->SetContent(showScrollbar ? "^" : "");
        if (hierarchyScrollbarTrackText)
            hierarchyScrollbarTrackText->SetContent(showScrollbar ? " " : "");
        if (hierarchyScrollbarDownText)
            hierarchyScrollbarDownText->SetContent(showScrollbar ? "v" : "");

        if (hierarchyWindow && !hierarchyWindow->IsHidden() && maxOffset > 0)
        {
            const auto mousePosition = GetMousePosition();
            if (CheckCollisionPointRec(mousePosition, hierarchyWindow->GetRec()))
            {
                const float wheelMove = GetMouseWheelMove();
                if (wheelMove > 0.0f)
                {
                    scrollHierarchy(-static_cast<int>(std::ceil(wheelMove)));
                }
                else if (wheelMove < 0.0f)
                {
                    scrollHierarchy(static_cast<int>(std::ceil(-wheelMove)));
                }
            }
        }

        for (std::size_t i = 0; i < hierarchyRows.size(); ++i)
        {
            const std::size_t entryIndex = hierarchyScrollOffset + i;
            if (entryIndex >= hierarchyEntries.size())
            {
                hierarchyRows[i]->SetContent("");
                continue;
            }

            const auto& entry = hierarchyEntries[entryIndex];
            hierarchyRows[i]->SetContent(std::string(static_cast<std::size_t>(entry.depth * 2), ' ') + entry.displayName);
        }
    }

    void EditorGui::SetInspector(
        const std::string& selectedEntity,
        const std::string& positionX,
        const std::string& positionY,
        const std::string& positionZ,
        const std::string& rotationX,
        const std::string& rotationY,
        const std::string& rotationZ,
        const std::string& scaleX,
        const std::string& scaleY,
        const std::string& scaleZ) const
    {
        if (inspectorSelectionText) inspectorSelectionText->SetContent("Selected: " + selectedEntity);
        if (inspectorPositionXText) inspectorPositionXText->SetContent(positionX);
        if (inspectorPositionYText) inspectorPositionYText->SetContent(positionY);
        if (inspectorPositionZText) inspectorPositionZText->SetContent(positionZ);
        if (inspectorRotationXText) inspectorRotationXText->SetContent(rotationX);
        if (inspectorRotationYText) inspectorRotationYText->SetContent(rotationY);
        if (inspectorRotationZText) inspectorRotationZText->SetContent(rotationZ);
        if (inspectorScaleXText) inspectorScaleXText->SetContent(scaleX);
        if (inspectorScaleYText) inspectorScaleYText->SetContent(scaleY);
        if (inspectorScaleZText) inspectorScaleZText->SetContent(scaleZ);
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

    RenderTexture2D EditorGui::createAssetThumbnail(const AssetEntry& asset) const
    {
        auto thumbnail = LoadRenderTexture(kThumbnailSize, kThumbnailSize);
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

    void EditorGui::createOverlayWindow(GameUIEngine* ui, Settings* settings)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            24.0f,
            -24.0f,
            360.0f,
            320.0f,
            VertAlignment::BOTTOM,
            HoriAlignment::LEFT,
            Padding{20, 16, 14, 14});

        overlayWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = overlayWindow->CreateTable({0, 0, 1, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(8);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Editor");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({1, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({4, 4, 8, 8});
            auto* table = contentCell->CreateTable();

            auto addLine = [ui, table](const char* text) {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell(Padding{2, 2, 2, 2});
                auto label = std::make_unique<TextBox>(
                    ui, cell, EditorTextFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                return cell->CreateTextbox(std::move(label), text);
            };

            addLine("Scene: Grid workspace");
            modeText = addLine("Mode: Select");
            selectedAssetText = addLine("Asset: None");
            gridText = addLine("Grid: None");
            placementHeightText = addLine("Z: 0.00");
            placementRotationText = addLine("Rot: 0");
            placementScaleText = addLine("Scale: 1.00");
            lastPlacedText = addLine("Last: None");
        }

        overlayWindow->FinalizeLayout();
    }

    void EditorGui::createHierarchyWindow(
        GameUIEngine* ui,
        Settings* settings,
        const std::function<void(entt::entity)>& onSceneObjectSelected)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            24.0f,
            24.0f,
            360.0f,
            560.0f,
            VertAlignment::TOP,
            HoriAlignment::LEFT,
            Padding{20, 16, 14, 14});

        hierarchyWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = hierarchyWindow->CreateTable({0, 0, 1, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(8);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Hierarchy");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({1, 0, 0, 0});
            auto hierarchyHasOverflow = [this]() { return hierarchyEntries.size() > hierarchyRows.size(); };

            auto* listCell = contentRow->CreateTableCell(94.0f, Padding{2, 8, 8, 1});
            auto* scrollbarCell = contentRow->CreateTableCell(6.0f, Padding{2, 8, 0, 2});
            auto* table = listCell->CreateTable();

            hierarchyRows.reserve(kHierarchyMaxRows);
            for (int i = 0; i < kHierarchyMaxRows; ++i)
            {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell(Padding{3, 3, 6, 6});
                auto button = std::make_unique<HierarchyRowButton>(
                    ui,
                    cell,
                    static_cast<std::size_t>(i),
                    &hierarchyEntries,
                    &hierarchyScrollOffset,
                    &selectedSceneEntity,
                    onSceneObjectSelected);
                hierarchyRows.push_back(cell->CreateTextbox(std::move(button), ""));
            }

            auto* scrollbarTable = scrollbarCell->CreateTable();

            auto* upRow = scrollbarTable->CreateTableRow(12.0f);
            auto* upCell = upRow->CreateTableCell(Padding{1, 1, 0, 0});
            auto upButton =
                std::make_unique<TextButton>(ui, upCell, [this]() { scrollHierarchy(-1); }, hierarchyHasOverflow);
            hierarchyScrollbarUpText = upCell->CreateTextbox(std::move(upButton), "^");

            auto* trackRow = scrollbarTable->CreateTableRow({0, 0, 0, 0});
            auto* trackCell = trackRow->CreateTableCell(Padding{2, 2, 0, 0});
            auto track = std::make_unique<ScrollbarTrack>(
                ui,
                trackCell,
                &hierarchyEntries,
                &hierarchyScrollOffset,
                &hierarchyRows);
            hierarchyScrollbarTrackText = trackCell->CreateTextbox(std::move(track), "");

            auto* downRow = scrollbarTable->CreateTableRow(12.0f);
            auto* downCell = downRow->CreateTableCell(Padding{1, 1, 0, 0});
            auto downButton =
                std::make_unique<TextButton>(ui, downCell, [this]() { scrollHierarchy(1); }, hierarchyHasOverflow);
            hierarchyScrollbarDownText = downCell->CreateTextbox(std::move(downButton), "v");
        }

        hierarchyWindow->FinalizeLayout();
    }

    void EditorGui::createAssetWindow(
        GameUIEngine* ui,
        Settings* settings,
        const std::vector<AssetEntry>& assets,
        const std::function<void(std::size_t)>& onAssetSelected)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            -24.0f,
            24.0f,
            392.0f,
            212.0f,
            VertAlignment::TOP,
            HoriAlignment::RIGHT,
            Padding{20, 16, 14, 14});

        assetWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = assetWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(18);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Assets");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* grid = contentCell->CreateTableGrid(1, static_cast<int>(assets.size()), 8.0f);

            if (!grid->children.empty())
            {
                auto* row = dynamic_cast<TableRow*>(grid->children.front().get());
                for (std::size_t i = 0; i < assets.size(); ++i)
                {
                    auto* cell = dynamic_cast<TableCell*>(row->children.at(i).get());
                    auto thumbnail = std::make_unique<AssetThumbnailButton>(
                        ui,
                        cell,
                        i,
                        assets[i].displayName,
                        &assetThumbnails.at(i),
                        &selectedAssetIndex,
                        onAssetSelected);
                    cell->CreateImagebox(std::move(thumbnail));
                }
            }
        }

        assetWindow->FinalizeLayout();
    }

    void EditorGui::createAssetDefaultsWindow(GameUIEngine* ui, Settings* settings)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            -24.0f,
            248.0f,
            392.0f,
            248.0f,
            VertAlignment::TOP,
            HoriAlignment::RIGHT,
            Padding{20, 16, 14, 14});

        assetDefaultsWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = assetDefaultsWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(18);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Asset Defaults");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* table = contentCell->CreateTable();

            auto addLine = [ui, table](const char* text) {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell();
                auto label = std::make_unique<TextBox>(ui, cell, EditorTextFontInfo());
                return cell->CreateTextbox(std::move(label), text);
            };

            defaultsAssetText = addLine("Asset: None");

            auto addControlRow = [ui, table](
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

            defaultsPositionText = addControlRow(
                "Z",
                "0.00",
                modelDefaultCallbacks.heightDown,
                modelDefaultCallbacks.heightUp);
            defaultsRotationText = addControlRow(
                "Rot Y",
                "0",
                modelDefaultCallbacks.rotationDown,
                modelDefaultCallbacks.rotationUp);
            defaultsScaleText = addControlRow(
                "Scale",
                "1.00",
                modelDefaultCallbacks.scaleDown,
                modelDefaultCallbacks.scaleUp);

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

    void EditorGui::createInspectorWindow(GameUIEngine* ui, Settings* settings)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            -24.0f,
            508.0f,
            392.0f,
            388.0f,
            VertAlignment::TOP,
            HoriAlignment::RIGHT,
            Padding{20, 16, 14, 14});

        inspectorWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = inspectorWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(18);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Inspector");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* table = contentCell->CreateTable();

            auto addLine = [ui, table](const char* text) {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell();
                auto label = std::make_unique<TextBox>(ui, cell, EditorTextFontInfo());
                return cell->CreateTextbox(std::move(label), text);
            };

            inspectorSelectionText = addLine("Selected: None");

            auto createValueInput = [this, ui](
                                        TableCell* valueCell,
                                        const char* initialValue,
                                        const TransformField field) {
                auto value = std::make_unique<TextInput>(
                    ui,
                    valueCell,
                    [this, field](const std::string& submittedValue) {
                        if (!inspectorCallbacks.setTransform) return;

                        try
                        {
                            inspectorCallbacks.setTransform(field, std::stof(submittedValue));
                        }
                        catch (const std::exception&)
                        {
                        }
                    },
                    TextBox::FontInfo{},
                    VertAlignment::MIDDLE,
                    HoriAlignment::LEFT);
                return valueCell->CreateTextbox(std::move(value), initialValue);
            };

            auto addVectorRow = [createValueInput, ui, table](
                                    const char* labelText,
                                    const char* initialX,
                                    const char* initialY,
                                    const char* initialZ,
                                    const TransformField fieldX,
                                    const TransformField fieldY,
                                    const TransformField fieldZ) {
                auto* row = table->CreateTableRow();

                auto* labelCell = row->CreateTableCell(16.0f);
                auto label = std::make_unique<TextBox>(
                    ui, labelCell, EditorTextFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                labelCell->CreateTextbox(std::move(label), labelText);

                auto* xLabelCell = row->CreateTableCell(6.0f);
                auto xLabel = std::make_unique<TextBox>(
                    ui, xLabelCell, EditorTextFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                xLabelCell->CreateTextbox(std::move(xLabel), "X");

                auto* xValueCell = row->CreateTableCell(21.0f, Padding{1, 1, 2, 2});
                auto* xValueText = createValueInput(xValueCell, initialX, fieldX);

                auto* yLabelCell = row->CreateTableCell(6.0f);
                auto yLabel = std::make_unique<TextBox>(
                    ui, yLabelCell, EditorTextFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                yLabelCell->CreateTextbox(std::move(yLabel), "Y");

                auto* yValueCell = row->CreateTableCell(21.0f, Padding{1, 1, 2, 2});
                auto* yValueText = createValueInput(yValueCell, initialY, fieldY);

                auto* zLabelCell = row->CreateTableCell(6.0f);
                auto zLabel = std::make_unique<TextBox>(
                    ui, zLabelCell, EditorTextFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                zLabelCell->CreateTextbox(std::move(zLabel), "Z");

                auto* zValueCell = row->CreateTableCell(24.0f, Padding{1, 1, 2, 2});
                auto* zValueText = createValueInput(zValueCell, initialZ, fieldZ);

                return std::array<TextBox*, 3>{xValueText, yValueText, zValueText};
            };

            const auto positionTexts = addVectorRow(
                "Pos:",
                "0.00",
                "0.00",
                "0.00",
                TransformField::PositionX,
                TransformField::PositionY,
                TransformField::PositionZ);
            inspectorPositionXText = positionTexts[0];
            inspectorPositionYText = positionTexts[1];
            inspectorPositionZText = positionTexts[2];

            const auto rotationTexts = addVectorRow(
                "Rot:",
                "0",
                "0",
                "0",
                TransformField::RotationX,
                TransformField::RotationY,
                TransformField::RotationZ);
            inspectorRotationXText = rotationTexts[0];
            inspectorRotationYText = rotationTexts[1];
            inspectorRotationZText = rotationTexts[2];

            const auto scaleTexts = addVectorRow(
                "Scale:",
                "1.00",
                "1.00",
                "1.00",
                TransformField::ScaleX,
                TransformField::ScaleY,
                TransformField::ScaleZ);
            inspectorScaleXText = scaleTexts[0];
            inspectorScaleYText = scaleTexts[1];
            inspectorScaleZText = scaleTexts[2];
        }

        inspectorWindow->FinalizeLayout();
    }

    void EditorGui::createDeleteConfirmationWindow(GameUIEngine* ui, Settings* settings)
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
            auto* titleRow = mainTable->CreateTableRow(18);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
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
            auto confirmButton =
                std::make_unique<TextButton>(ui, confirmCell, deleteConfirmationCallbacks.confirm);
            confirmCell->CreateTextbox(std::move(confirmButton), "Delete");

            auto* cancelCell = buttonRow->CreateTableCell(50.0f, Padding{3, 3, 4, 4});
            auto cancelButton =
                std::make_unique<TextButton>(ui, cancelCell, deleteConfirmationCallbacks.cancel);
            cancelCell->CreateTextbox(std::move(cancelButton), "Cancel");
        }

        deleteConfirmationWindow->FinalizeLayout();
        deleteConfirmationWindow->Hide();
    }

    EditorGui::EditorGui(
        GameUIEngine* ui,
        Settings* settings,
        const std::vector<AssetEntry>& assets,
        std::function<void(std::size_t)> onAssetSelected,
        std::function<void(entt::entity)> onSceneObjectSelected,
        ModelDefaultCallbacks callbacks,
        InspectorCallbacks inspectorCallbacks,
        DeleteConfirmationCallbacks deleteConfirmationCallbacks)
        : modelDefaultCallbacks(std::move(callbacks)),
          inspectorCallbacks(std::move(inspectorCallbacks)),
          deleteConfirmationCallbacks(std::move(deleteConfirmationCallbacks))
    {
        Image panelImage = GenImageColor(1, 1, kEditorWindowBackground);
        editorWindowBackgroundTexture = LoadTextureFromImage(panelImage);
        UnloadImage(panelImage);

        assetThumbnails.reserve(assets.size());
        for (const auto& asset : assets)
        {
            assetThumbnails.push_back(createAssetThumbnail(asset));
        }

        createOverlayWindow(ui, settings);
        createHierarchyWindow(ui, settings, onSceneObjectSelected);
        createAssetWindow(ui, settings, assets, onAssetSelected);
        createAssetDefaultsWindow(ui, settings);
        createInspectorWindow(ui, settings);
        createDeleteConfirmationWindow(ui, settings);
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
