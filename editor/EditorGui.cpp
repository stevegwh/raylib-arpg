#include "EditorGui.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Settings.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"
#include "engine/ui/UIWindow.hpp"

#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr int kThumbnailSize = 128;
        constexpr int kHierarchyMaxRows = 24;
        constexpr float kInspectorPositionStep = 0.25f;
        constexpr float kInspectorRotationStep = 15.0f;
        constexpr float kInspectorScaleStep = 0.1f;

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

          public:
            void OnClick() override
            {
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

            TextButton(GameUIEngine* ui, TableCell* parent, std::function<void()> callback)
                : TextBox(ui, parent, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::CENTER),
                  onPressed(std::move(callback))
            {
            }
        };

        class HierarchyRowButton final : public TextBox
        {
            std::size_t rowIndex = 0;
            const std::vector<EditorGui::SceneObjectEntry>* entries{};
            const std::optional<entt::entity>* selectedEntity{};
            std::function<void(entt::entity)> onSceneObjectSelected;

          public:
            void OnClick() override
            {
                if (!entries || rowIndex >= entries->size() || !onSceneObjectSelected) return;
                onSceneObjectSelected(entries->at(rowIndex).entity);
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
                const bool hasEntry = entries && rowIndex < entries->size();
                const bool selected = hasEntry && selectedEntity && selectedEntity->has_value() &&
                                      selectedEntity->value() == entries->at(rowIndex).entity;

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

                BeginShaderMode(sdfShader);
                DrawTextEx(
                    fontInfo.font,
                    GetContent().c_str(),
                    Vector2{rec.x + 6.0f, rec.y + (rec.height - fontInfo.fontSize) * 0.5f},
                    fontInfo.fontSize,
                    fontInfo.fontSpacing,
                    BLACK);
                EndShaderMode();
            }

            HierarchyRowButton(
                GameUIEngine* ui,
                TableCell* parent,
                std::size_t index,
                const std::vector<EditorGui::SceneObjectEntry>* sceneEntries,
                const std::optional<entt::entity>* activeEntity,
                std::function<void(entt::entity)> callback)
                : TextBox(ui, parent, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::LEFT),
                  rowIndex(index),
                  entries(sceneEntries),
                  selectedEntity(activeEntity),
                  onSceneObjectSelected(std::move(callback))
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

    void EditorGui::SetHierarchy(
        const std::vector<SceneObjectEntry>& entries,
        const std::optional<entt::entity> selectedEntity)
    {
        hierarchyEntries = entries;
        selectedSceneEntity = selectedEntity;

        for (std::size_t i = 0; i < hierarchyRows.size(); ++i)
        {
            if (i >= hierarchyEntries.size())
            {
                hierarchyRows[i]->SetContent("");
                continue;
            }

            const auto& entry = hierarchyEntries[i];
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
        const auto frame = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto window = std::make_unique<WindowDocked>(
            settings,
            frame,
            TextureStretchMode::STRETCH,
            24.0f,
            -24.0f,
            360.0f,
            236.0f,
            VertAlignment::BOTTOM,
            HoriAlignment::LEFT,
            Padding{20, 16, 14, 14});

        overlayWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = overlayWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(18);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, TextBox::FontInfo{});
            titleCell->CreateTitleBar(std::move(title), "Editor");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* table = contentCell->CreateTable();

            auto addLine = [ui, table](const char* text) {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell();
                auto label = std::make_unique<TextBox>(ui, cell, TextBox::FontInfo{});
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
        const auto frame = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto window = std::make_unique<WindowDocked>(
            settings,
            frame,
            TextureStretchMode::STRETCH,
            24.0f,
            24.0f,
            360.0f,
            560.0f,
            VertAlignment::TOP,
            HoriAlignment::LEFT,
            Padding{20, 16, 14, 14});

        hierarchyWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = hierarchyWindow->CreateTable({0, 0, 4, 0});

        {
            auto* titleRow = mainTable->CreateTableRow(18);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, TextBox::FontInfo{});
            titleCell->CreateTitleBar(std::move(title), "Hierarchy");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* table = contentCell->CreateTable();

            hierarchyRows.reserve(kHierarchyMaxRows);
            for (int i = 0; i < kHierarchyMaxRows; ++i)
            {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell(Padding{1, 1, 1, 1});
                auto button = std::make_unique<HierarchyRowButton>(
                    ui,
                    cell,
                    static_cast<std::size_t>(i),
                    &hierarchyEntries,
                    &selectedSceneEntity,
                    onSceneObjectSelected);
                hierarchyRows.push_back(cell->CreateTextbox(std::move(button), ""));
            }
        }

        hierarchyWindow->FinalizeLayout();
    }

    void EditorGui::createAssetWindow(
        GameUIEngine* ui,
        Settings* settings,
        const std::vector<AssetEntry>& assets,
        const std::function<void(std::size_t)>& onAssetSelected)
    {
        const auto frame = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto window = std::make_unique<WindowDocked>(
            settings,
            frame,
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
            auto title = std::make_unique<TitleBar>(ui, titleCell, TextBox::FontInfo{});
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
        const auto frame = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto window = std::make_unique<WindowDocked>(
            settings,
            frame,
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
            auto title = std::make_unique<TitleBar>(ui, titleCell, TextBox::FontInfo{});
            titleCell->CreateTitleBar(std::move(title), "Asset Defaults");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* table = contentCell->CreateTable();

            auto addLine = [ui, table](const char* text) {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell();
                auto label = std::make_unique<TextBox>(ui, cell, TextBox::FontInfo{});
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
                auto label = std::make_unique<TextBox>(ui, labelCell, TextBox::FontInfo{});
                labelCell->CreateTextbox(std::move(label), labelText);

                auto* downCell = row->CreateTableCell(15.0f, Padding{1, 1, 2, 2});
                auto downButton = std::make_unique<TextButton>(ui, downCell, onDown);
                downCell->CreateTextbox(std::move(downButton), "-");

                auto* valueCell = row->CreateTableCell(34.0f);
                auto value = std::make_unique<TextBox>(
                    ui, valueCell, TextBox::FontInfo{}, VertAlignment::TOP, HoriAlignment::CENTER);
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
        const auto frame = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto window = std::make_unique<WindowDocked>(
            settings,
            frame,
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
            auto title = std::make_unique<TitleBar>(ui, titleCell, TextBox::FontInfo{});
            titleCell->CreateTitleBar(std::move(title), "Inspector");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* table = contentCell->CreateTable();

            auto addLine = [ui, table](const char* text) {
                auto* row = table->CreateTableRow();
                auto* cell = row->CreateTableCell();
                auto label = std::make_unique<TextBox>(ui, cell, TextBox::FontInfo{});
                return cell->CreateTextbox(std::move(label), text);
            };

            inspectorSelectionText = addLine("Selected: None");

            auto addControlRow = [this, ui, table](
                                     const char* labelText,
                                     const char* initialValue,
                                     const TransformField field,
                                     const float step) {
                auto* row = table->CreateTableRow();

                auto* labelCell = row->CreateTableCell(32.0f);
                auto label = std::make_unique<TextBox>(ui, labelCell, TextBox::FontInfo{});
                labelCell->CreateTextbox(std::move(label), labelText);

                auto* downCell = row->CreateTableCell(14.0f, Padding{1, 1, 2, 2});
                auto downButton = std::make_unique<TextButton>(
                    ui,
                    downCell,
                    [this, field, step]() {
                        if (inspectorCallbacks.adjustTransform)
                            inspectorCallbacks.adjustTransform(field, -step);
                    });
                downCell->CreateTextbox(std::move(downButton), "-");

                auto* valueCell = row->CreateTableCell(40.0f);
                auto value = std::make_unique<TextBox>(
                    ui, valueCell, TextBox::FontInfo{}, VertAlignment::TOP, HoriAlignment::CENTER);
                auto* valueText = valueCell->CreateTextbox(std::move(value), initialValue);

                auto* upCell = row->CreateTableCell(14.0f, Padding{1, 1, 2, 2});
                auto upButton = std::make_unique<TextButton>(
                    ui,
                    upCell,
                    [this, field, step]() {
                        if (inspectorCallbacks.adjustTransform)
                            inspectorCallbacks.adjustTransform(field, step);
                    });
                upCell->CreateTextbox(std::move(upButton), "+");

                return valueText;
            };

            inspectorPositionXText = addControlRow("Pos X", "0.00", TransformField::PositionX, kInspectorPositionStep);
            inspectorPositionYText = addControlRow("Pos Y", "0.00", TransformField::PositionY, kInspectorPositionStep);
            inspectorPositionZText = addControlRow("Pos Z", "0.00", TransformField::PositionZ, kInspectorPositionStep);
            inspectorRotationXText = addControlRow("Rot X", "0", TransformField::RotationX, kInspectorRotationStep);
            inspectorRotationYText = addControlRow("Rot Y", "0", TransformField::RotationY, kInspectorRotationStep);
            inspectorRotationZText = addControlRow("Rot Z", "0", TransformField::RotationZ, kInspectorRotationStep);
            inspectorScaleXText = addControlRow("Scale X", "1.00", TransformField::ScaleX, kInspectorScaleStep);
            inspectorScaleYText = addControlRow("Scale Y", "1.00", TransformField::ScaleY, kInspectorScaleStep);
            inspectorScaleZText = addControlRow("Scale Z", "1.00", TransformField::ScaleZ, kInspectorScaleStep);
        }

        inspectorWindow->FinalizeLayout();
    }

    void EditorGui::createDeleteConfirmationWindow(GameUIEngine* ui, Settings* settings)
    {
        const auto frame = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto window = std::make_unique<WindowDocked>(
            settings,
            frame,
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
            auto title = std::make_unique<TitleBar>(ui, titleCell, TextBox::FontInfo{});
            titleCell->CreateTitleBar(std::move(title), "Confirm Delete");
        }

        {
            auto* textRow = mainTable->CreateTableRow(44.0f);
            auto* textCell = textRow->CreateTableCell({8, 8, 8, 8});
            auto text = std::make_unique<TextBox>(
                ui, textCell, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::CENTER);
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
        for (auto& thumbnail : assetThumbnails)
        {
            if (thumbnail.id != 0)
            {
                UnloadRenderTexture(thumbnail);
            }
        }
    }
} // namespace sage::editor
