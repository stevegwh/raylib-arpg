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
#include <format>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr int THUMBNAIL_SIZE = 128;
        constexpr int HIERARCHY_MAX_ROWS = 18;
        constexpr Color EDITOR_WINDOW_BACKGROUND = {35, 38, 43, 245};
        constexpr Color EDITOR_TEXT = {230, 234, 240, 255};

        TextBox::FontInfo EditorTextFontInfo()
        {
            auto info = TextBox::FontInfo{};
            info.color = EDITOR_TEXT;
            return info;
        }

        // Inspector packs many labels/inputs into a narrow column. Allow the
        // shrink-to-fit logic to go below the global default of 16 so long
        // labels like "Local Bounds Min" actually fit at the typical scale.
        TextBox::FontInfo EditorInspectorFontInfo()
        {
            auto info = EditorTextFontInfo();
            info.baseFontSize = 14;
            info.minFontSize = 11;
            return info;
        }

        // TextInput boxes paint a light background, so the inspector's normal
        // light text colour would be unreadable inside them.
        TextBox::FontInfo EditorInspectorInputFontInfo()
        {
            auto info = EditorInspectorFontInfo();
            info.color = Color{17, 24, 39, 255};
            return info;
        }

        std::string FormatFloat(const float value)
        {
            return std::format("{:.2f}", value);
        }

        const InspectorField* FindField(
            const std::vector<InspectedComponent>& components,
            const std::string_view componentName,
            const std::string_view fieldName)
        {
            for (const auto& component : components)
            {
                if (component.displayName != componentName) continue;
                for (const auto& field : component.fields)
                {
                    if (field.label == fieldName) return &field;
                }
            }
            return nullptr;
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
            std::function<std::size_t()> totalRows;
            std::function<std::size_t()> visibleRows;
            const std::size_t* scrollOffset{};

          public:
            [[nodiscard]] bool ShouldShow() const
            {
                return totalRows && visibleRows && totalRows() > visibleRows();
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

                if (!totalRows || !scrollOffset || !visibleRows || totalRows() == 0 || visibleRows() == 0) return;
                const float visibleCount = static_cast<float>(visibleRows());
                const float totalCount = static_cast<float>(totalRows());
                const float thumbHeight = totalCount <= visibleCount
                                              ? rec.height
                                              : std::max(18.0f, rec.height * (visibleCount / totalCount));
                const std::size_t maxOffset = totalRows() > visibleRows() ? totalRows() - visibleRows() : 0;
                const float scrollRatio =
                    maxOffset == 0 ? 0.0f : static_cast<float>(*scrollOffset) / static_cast<float>(maxOffset);
                const float thumbY = rec.y + (rec.height - thumbHeight) * scrollRatio;
                DrawRectangleRec(
                    {rec.x + 2.0f, thumbY + 2.0f, rec.width - 4.0f, thumbHeight - 4.0f},
                    Color{139, 148, 164, 255});
            }

            ScrollbarTrack(
                GameUIEngine* ui,
                TableCell* parent,
                std::function<std::size_t()> totalRowProvider,
                const std::size_t* firstVisibleEntry,
                std::function<std::size_t()> visibleRowProvider)
                : TextBox(ui, parent, TextBox::FontInfo{}, VertAlignment::MIDDLE, HoriAlignment::CENTER),
                  totalRows(std::move(totalRowProvider)),
                  scrollOffset(firstVisibleEntry),
                  visibleRows(std::move(visibleRowProvider))
            {
            }
        };
    } // namespace

    void EditorGui::SetOverlayStatus(const std::string& mode, const std::string& cursor) const
    {
        if (modeText) modeText->SetContent("Mode: " + mode);
        if (cursorText) cursorText->SetContent("Cursor: " + cursor);
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
        if (overlayTitleText) overlayTitleText->SetContent("SAGE - " + sceneName);
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
        const std::size_t maxOffset =
            hierarchyEntries.size() > visibleRows ? hierarchyEntries.size() - visibleRows : 0;

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

    void EditorGui::scrollInspectorFields(const int amount)
    {
        const std::size_t maxOffset = inspectorFieldTotalRows > inspectorFieldVisibleRows
                                          ? inspectorFieldTotalRows - inspectorFieldVisibleRows
                                          : 0;

        if (amount < 0)
        {
            const auto positiveAmount = static_cast<std::size_t>(-amount);
            inspectorFieldScrollOffset =
                positiveAmount > inspectorFieldScrollOffset ? 0 : inspectorFieldScrollOffset - positiveAmount;
        }
        else if (amount > 0)
        {
            inspectorFieldScrollOffset =
                std::min(maxOffset, inspectorFieldScrollOffset + static_cast<std::size_t>(amount));
        }
    }

    void EditorGui::SetHierarchy(
        const std::vector<SceneObjectEntry>& entries, const std::optional<entt::entity> selectedEntity)
    {
        hierarchyEntries = entries;
        selectedSceneEntity = selectedEntity;

        const std::size_t visibleRows = hierarchyRows.size();
        const std::size_t maxOffset =
            hierarchyEntries.size() > visibleRows ? hierarchyEntries.size() - visibleRows : 0;
        hierarchyScrollOffset = std::min(hierarchyScrollOffset, maxOffset);
        const bool showScrollbar = maxOffset > 0;
        if (hierarchyScrollbarUpText) hierarchyScrollbarUpText->SetContent(showScrollbar ? "^" : "");
        if (hierarchyScrollbarTrackText) hierarchyScrollbarTrackText->SetContent(showScrollbar ? " " : "");
        if (hierarchyScrollbarDownText) hierarchyScrollbarDownText->SetContent(showScrollbar ? "v" : "");

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
            hierarchyRows[i]->SetContent(
                std::string(static_cast<std::size_t>(entry.depth * 2), ' ') + entry.displayName);
        }
    }

    void EditorGui::SetInspector(
        const std::string& selectedEntity, const std::vector<InspectedComponent>& inspectedComponents)
    {
        if (inspectorSelectionText) inspectorSelectionText->SetContent("Selected: " + selectedEntity);
        rebuildInspectorFieldRows(inspectedComponents);
        drawInspectedFields(inspectedComponents);
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

    void EditorFieldDrawer<Vector3>::Draw(const InspectorField& field, const InspectorFieldBinding& binding)
    {
        if (!field.value || binding.valueTexts.size() < 3) return;

        const auto& value = *static_cast<const Vector3*>(field.value);
        if (binding.valueTexts[0]) binding.valueTexts[0]->SetContent(FormatFloat(value.x));
        if (binding.valueTexts[1]) binding.valueTexts[1]->SetContent(FormatFloat(value.y));
        if (binding.valueTexts[2]) binding.valueTexts[2]->SetContent(FormatFloat(value.z));
    }

    const EditorGui::InspectorFieldDrawer* EditorGui::findInspectorFieldDrawer(
        const std::type_index valueType) const
    {
        const auto it = std::ranges::find_if(
            inspectorFieldDrawers, [valueType](const auto& entry) { return entry.first == valueType; });
        return it == inspectorFieldDrawers.end() ? nullptr : &it->second;
    }

    std::string EditorGui::buildInspectorFieldsSignature(
        const std::vector<InspectedComponent>& inspectedComponents) const
    {
        std::ostringstream signature;
        std::size_t drawableRows = 0;
        for (const auto& component : inspectedComponents)
        {
            signature << component.displayName << '{';
            bool componentHeaderCounted = false;
            for (const auto& field : component.fields)
            {
                if (!findInspectorFieldDrawer(field.valueType)) continue;
                if (!componentHeaderCounted)
                {
                    ++drawableRows;
                    componentHeaderCounted = true;
                }
                ++drawableRows;
                signature << field.label << ':' << field.valueType.name() << ';';
            }
            signature << '}';
        }
        signature << "scroll:" << inspectorFieldScrollOffset << "rows:" << drawableRows;
        return signature.str();
    }

    void EditorGui::rebuildInspectorFieldRows(const std::vector<InspectedComponent>& inspectedComponents)
    {
        if (!inspectorFieldsTable || !ui) return;

        constexpr std::size_t VISIBLE_ROWS = 13;
        inspectorFieldVisibleRows = VISIBLE_ROWS;

        std::size_t totalRows = 0;
        for (const auto& component : inspectedComponents)
        {
            bool componentHeaderCounted = false;
            for (const auto& field : component.fields)
            {
                if (!findInspectorFieldDrawer(field.valueType)) continue;
                if (!componentHeaderCounted)
                {
                    ++totalRows;
                    componentHeaderCounted = true;
                }
                ++totalRows;
            }
        }
        inspectorFieldTotalRows = totalRows;
        const std::size_t maxOffset = inspectorFieldTotalRows > inspectorFieldVisibleRows
                                          ? inspectorFieldTotalRows - inspectorFieldVisibleRows
                                          : 0;
        inspectorFieldScrollOffset = std::min(inspectorFieldScrollOffset, maxOffset);

        const bool showScrollbar = inspectorFieldTotalRows > inspectorFieldVisibleRows;
        if (inspectorScrollbarUpText) inspectorScrollbarUpText->SetContent(showScrollbar ? "^" : "");
        if (inspectorScrollbarTrackText) inspectorScrollbarTrackText->SetContent(showScrollbar ? " " : "");
        if (inspectorScrollbarDownText) inspectorScrollbarDownText->SetContent(showScrollbar ? "v" : "");

        if (inspectorWindow && !inspectorWindow->IsHidden() && showScrollbar)
        {
            const auto mousePosition = GetMousePosition();
            if (CheckCollisionPointRec(mousePosition, inspectorWindow->GetRec()))
            {
                const float wheelMove = GetMouseWheelMove();
                if (wheelMove > 0.0f)
                {
                    scrollInspectorFields(-static_cast<int>(std::ceil(wheelMove)));
                }
                else if (wheelMove < 0.0f)
                {
                    scrollInspectorFields(static_cast<int>(std::ceil(-wheelMove)));
                }
            }
        }

        const auto signature = buildInspectorFieldsSignature(inspectedComponents);
        if (signature == inspectorFieldsSignature) return;

        inspectorFieldsSignature = signature;
        inspectorFieldBindings.clear();
        inspectorFieldsTable->children.clear();

        std::size_t rowCursor = 0;
        std::size_t visibleCreated = 0;
        const auto shouldShowRow = [&]() {
            return rowCursor >= inspectorFieldScrollOffset && visibleCreated < inspectorFieldVisibleRows;
        };
        const auto advanceRow = [&]() {
            ++rowCursor;
            if (rowCursor > inspectorFieldScrollOffset && visibleCreated < inspectorFieldVisibleRows)
            {
                ++visibleCreated;
            }
        };

        for (const auto& component : inspectedComponents)
        {
            // TODO: This is awful.
            bool componentHeaderCreated = false;
            for (const auto& field : component.fields)
            {
                if (!findInspectorFieldDrawer(field.valueType)) continue;

                if (!componentHeaderCreated)
                {
                    if (shouldShowRow())
                    {
                        // autoSize so the visible slots split the field area
                        // evenly. The previous explicit percent (22) was being
                        // interpreted as "22% of inspectorFieldsTable height"
                        // — which, combined with 13 row slots, caused rows to
                        // overlap each other and the buttons below.
                        auto* headerRow = inspectorFieldsTable->CreateTableRow(Padding{2, 2, 2, 2});
                        auto* headerCell = headerRow->CreateTableCell(Padding{2, 2, 2, 2});
                        auto header = std::make_unique<TextBox>(
                            ui, headerCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                        headerCell->CreateTextbox(std::move(header), component.displayName);
                    }
                    advanceRow();
                    componentHeaderCreated = true;
                }

                if (field.valueType == typeid(Vector3))
                {
                    if (!shouldShowRow())
                    {
                        advanceRow();
                        continue;
                    }

                    auto* row = inspectorFieldsTable->CreateTableRow(Padding{2, 2, 2, 2});

                    auto* labelCell = row->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
                    auto label = std::make_unique<TextBox>(
                        ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                    labelCell->CreateTextbox(std::move(label), field.label + ":");

                    auto transformSetter =
                        [this, &component, &field](const char axis) -> std::function<void(float)> {
                        if (component.displayName != "Transform") return {};

                        auto makeSetter = [this](const TransformField transformField) {
                            return [this, transformField](const float value) {
                                if (inspectorCallbacks.setTransform)
                                    inspectorCallbacks.setTransform(transformField, value);
                            };
                        };

                        if (field.label == "Position")
                        {
                            if (axis == 'X') return makeSetter(TransformField::PositionX);
                            if (axis == 'Y') return makeSetter(TransformField::PositionY);
                            if (axis == 'Z') return makeSetter(TransformField::PositionZ);
                        }
                        if (field.label == "Rotation")
                        {
                            if (axis == 'X') return makeSetter(TransformField::RotationX);
                            if (axis == 'Y') return makeSetter(TransformField::RotationY);
                            if (axis == 'Z') return makeSetter(TransformField::RotationZ);
                        }
                        if (field.label == "Scale")
                        {
                            if (axis == 'X') return makeSetter(TransformField::ScaleX);
                            if (axis == 'Y') return makeSetter(TransformField::ScaleY);
                            if (axis == 'Z') return makeSetter(TransformField::ScaleZ);
                        }
                        return {};
                    };

                    auto addAxis = [this, row](const char* axis, std::function<void(float)> setter) {
                        auto* axisCell = row->CreateTableCell(4.0f, Padding{1, 1, 1, 1});
                        auto axisLabel = std::make_unique<TextBox>(
                            ui, axisCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::CENTER);
                        axisCell->CreateTextbox(std::move(axisLabel), axis);

                        auto* valueCell = row->CreateTableCell(18.0f, Padding{1, 1, 2, 2});
                        auto valueText = std::make_unique<TextInput>(
                            ui,
                            valueCell,
                            [setter = std::move(setter)](const std::string& submittedValue) {
                                if (!setter) return;
                                try
                                {
                                    setter(std::stof(submittedValue));
                                }
                                catch (const std::exception&)
                                {
                                }
                            },
                            EditorInspectorInputFontInfo(),
                            VertAlignment::MIDDLE,
                            HoriAlignment::RIGHT);
                        return valueCell->CreateTextbox(std::move(valueText), "0.00");
                    };

                    auto* xText = addAxis("X", transformSetter('X'));
                    auto* yText = addAxis("Y", transformSetter('Y'));
                    auto* zText = addAxis("Z", transformSetter('Z'));

                    inspectorFieldBindings.push_back(
                        InspectorFieldBinding{
                            .componentName = component.displayName,
                            .fieldName = field.label,
                            .valueType = field.valueType,
                            .valueTexts = {xText, yText, zText}});
                    advanceRow();
                }
            }
        }

        inspectorFieldsTable->InitLayout();
    }

    void EditorGui::drawInspectedFields(const std::vector<InspectedComponent>& inspectedComponents) const
    {
        for (const auto& binding : inspectorFieldBindings)
        {
            const auto* field = FindField(inspectedComponents, binding.componentName, binding.fieldName);
            if (!field || field->valueType != binding.valueType) continue;

            const auto* drawer = findInspectorFieldDrawer(field->valueType);
            if (!drawer) continue;
            (*drawer)(*field, binding);
        }
    }

    void EditorGui::registerInspectorFieldDrawers()
    {
        registerInspectorFieldDrawer<Vector3>();
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

    void EditorGui::createOverlayWindow(GameUIEngine* ui, Settings* settings)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            24.0f,
            -24.0f,
            360.0f,
            120.0f,
            VertAlignment::BOTTOM,
            HoriAlignment::LEFT,
            Padding{20, 16, 14, 14});

        overlayWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = overlayWindow->CreateTable({0, 0, 1, 0});

        {
            // The title doubles as the scene-name display, so capture the
            // pointer for later SetSceneName updates.
            auto* titleRow = mainTable->CreateTableRow(30);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
            overlayTitleText = titleCell->CreateTitleBar(std::move(title), "");
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

            modeText = addLine("Mode: Select");
            cursorText = addLine("Cursor: -");
        }

        overlayWindow->FinalizeLayout();
    }

    void EditorGui::createHierarchyWindow(
        GameUIEngine* ui, Settings* settings, const std::function<void(entt::entity)>& onSceneObjectSelected)
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
                [this]() { return hierarchyEntries.size(); },
                &hierarchyScrollOffset,
                [this]() { return hierarchyRows.size(); });
            hierarchyScrollbarTrackText = trackCell->CreateTextbox(std::move(track), "");

            auto* downRow = scrollbarTable->CreateTableRow(12.0f);
            auto* downCell = downRow->CreateTableCell(Padding{1, 1, 0, 0});
            auto downButton =
                std::make_unique<TextButton>(ui, downCell, [this]() { scrollHierarchy(1); }, hierarchyHasOverflow);
            hierarchyScrollbarDownText = downCell->CreateTextbox(std::move(downButton), "v");
        }

        hierarchyWindow->SetOverflowContingency(OverflowContingency::SCROLLBAR);
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

    void EditorGui::createInspectorWindow(GameUIEngine* ui, Settings* settings)
    {
        auto window = std::make_unique<WindowDocked>(
            settings,
            editorWindowBackgroundTexture,
            TextureStretchMode::STRETCH,
            -24.0f,
            508.0f,
            460.0f,
            520.0f,
            VertAlignment::TOP,
            HoriAlignment::RIGHT,
            Padding{20, 16, 14, 14});

        inspectorWindow = ui->CreateWindowDocked(std::move(window));
        auto* mainTable = inspectorWindow->CreateTable({0, 0, 4, 0});

        {
            // 6% gives the title bar a compact strip (~29px in the current
            // 520px window) — the previous 18 was being read as percent, so
            // the title cell was ~87px tall and dropped ~70px of dead space
            // below the WINDOW_CENTER / TOP-aligned text.
            auto* titleRow = mainTable->CreateTableRow(6);
            auto* titleCell = titleRow->CreateTableCell();
            auto title = std::make_unique<TitleBar>(ui, titleCell, EditorTextFontInfo());
            titleCell->CreateTitleBar(std::move(title), "Inspector");
        }

        {
            auto* contentRow = mainTable->CreateTableRow({10, 0, 0, 0});
            auto* contentCell = contentRow->CreateTableCell({8, 8, 8, 8});
            auto* table = contentCell->CreateTable();

            auto addLine = [ui, table](const char* text) {
                // 6% of the inspector content table leaves a single-line strip
                // for the selection label. MIDDLE alignment keeps the text
                // visually adjacent to the field rows below it (TOP would
                // leave dead space underneath the text).
                auto* row = table->CreateTableRow(6.0f);
                auto* cell = row->CreateTableCell(Padding{2, 2, 2, 2});
                auto label = std::make_unique<TextBox>(
                    ui, cell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                return cell->CreateTextbox(std::move(label), text);
            };

            inspectorSelectionText = addLine("Selected: None");

            auto* fieldsRow = table->CreateTableRow(Padding{2, 0, 0, 0});
            auto inspectorHasOverflow = [this]() { return inspectorFieldTotalRows > inspectorFieldVisibleRows; };

            auto* fieldsCell = fieldsRow->CreateTableCell(94.0f, Padding{2, 2, 2, 2});
            auto* scrollbarCell = fieldsRow->CreateTableCell(6.0f, Padding{2, 2, 0, 2});
            inspectorFieldsTable = fieldsCell->CreateTable();

            auto* scrollbarTable = scrollbarCell->CreateTable();

            auto* upRow = scrollbarTable->CreateTableRow(12.0f);
            auto* upCell = upRow->CreateTableCell(Padding{1, 1, 0, 0});
            auto upButton = std::make_unique<TextButton>(
                ui, upCell, [this]() { scrollInspectorFields(-1); }, inspectorHasOverflow);
            inspectorScrollbarUpText = upCell->CreateTextbox(std::move(upButton), "^");

            auto* trackRow = scrollbarTable->CreateTableRow({0, 0, 0, 0});
            auto* trackCell = trackRow->CreateTableCell(Padding{2, 2, 0, 0});
            auto track = std::make_unique<ScrollbarTrack>(
                ui,
                trackCell,
                [this]() { return inspectorFieldTotalRows; },
                &inspectorFieldScrollOffset,
                [this]() { return inspectorFieldVisibleRows; });
            inspectorScrollbarTrackText = trackCell->CreateTextbox(std::move(track), "");

            auto* downRow = scrollbarTable->CreateTableRow(12.0f);
            auto* downCell = downRow->CreateTableCell(Padding{1, 1, 0, 0});
            auto downButton = std::make_unique<TextButton>(
                ui, downCell, [this]() { scrollInspectorFields(1); }, inspectorHasOverflow);
            inspectorScrollbarDownText = downCell->CreateTextbox(std::move(downButton), "v");
        }

        inspectorWindow->SetOverflowContingency(OverflowContingency::SCROLLBAR);
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
            auto cancelButton = std::make_unique<TextButton>(ui, cancelCell, deleteConfirmationCallbacks.cancel);
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
          deleteConfirmationCallbacks(std::move(deleteConfirmationCallbacks)),
          ui(ui)
    {
        Image panelImage = GenImageColor(1, 1, EDITOR_WINDOW_BACKGROUND);
        editorWindowBackgroundTexture = LoadTextureFromImage(panelImage);
        UnloadImage(panelImage);

        registerInspectorFieldDrawers();

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
