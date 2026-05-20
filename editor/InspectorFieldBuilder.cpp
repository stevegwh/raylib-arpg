#include "InspectorFieldBuilder.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"

#include <array>
#include <cmath>
#include <exception>
#include <format>
#include <memory>
#include <sstream>
#include <string_view>
#include <typeindex>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr std::size_t INSPECTOR_VISIBLE_ROWS = 13;
        constexpr Color EDITOR_TEXT = {230, 234, 240, 255};

        TextBox::FontInfo EditorInspectorFontInfo()
        {
            auto info = TextBox::FontInfo{};
            info.color = EDITOR_TEXT;
            info.baseFontSize = 14;
            info.minFontSize = 11;
            return info;
        }

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
            for (const auto& inspectedComponent : components)
            {
                if (inspectedComponent.displayName != componentName) continue;
                for (const auto& candidateField : inspectedComponent.fields)
                {
                    if (candidateField.label == fieldName) return &candidateField;
                }
            }
            return nullptr;
        }

    } // namespace

    struct InspectorFieldBuilder::FieldRow
    {
        enum class Type
        {
            ComponentHeader,
            Field
        };

        Type type = Type::Field;
        std::string componentName;
        std::string fieldName;
        InspectorFieldKind fieldKind = InspectorFieldKind::Unsupported;
        std::type_index valueType = typeid(void);
        bool isMutable = false;
    };

    struct InspectorFieldBuilder::FieldBinding
    {
        FieldRow inspectorRow;
        std::vector<TextBox*> valueTexts;
        Vector3* mutableVector = nullptr;
    };

    InspectorFieldBuilder::InspectorFieldBuilder() = default;

    InspectorFieldBuilder::~InspectorFieldBuilder() = default;

    void InspectorFieldBuilder::Attach(GameUIEngine* ui, Table* fieldTable)
    {
        this->ui = ui;
        this->fieldTable = fieldTable;
    }

    void InspectorFieldBuilder::SetScrollbarControls(TextBox* upText, TextBox* trackText, TextBox* downText)
    {
        scrollbarUpText = upText;
        scrollbarTrackText = trackText;
        scrollbarDownText = downText;
        updateScrollbarText();
    }

    void InspectorFieldBuilder::Rebuild(
        const std::vector<InspectedComponent>& inspectedComponents, const Rectangle* mouseWheelBounds)
    {
        if (!fieldTable || !ui) return;

        const auto signature = buildBlueprintSignature(inspectedComponents);
        if (signature != blueprintSignature)
        {
            blueprintSignature = signature;
            rebuildRows(inspectedComponents);
        }

        updateRowMetrics();
        if (mouseWheelBounds && HasOverflow()) scrollFromMouseWheel(*mouseWheelBounds);
        updateScrollbarText();

        const auto newRowSignature = buildRowSignature();
        if (newRowSignature == rowSignature) return;

        rowSignature = newRowSignature;
        bindings.clear();
        fieldTable->children.clear();

        const std::size_t lastVisibleRow = std::min(rows.size(), scrollOffset + visibleRows);
        for (std::size_t i = scrollOffset; i < lastVisibleRow; ++i)
        {
            const auto& inspectorRow = rows[i];
            switch (inspectorRow.type)
            {
            case FieldRow::Type::ComponentHeader:
                createHeaderRow(inspectorRow.componentName);
                break;
            case FieldRow::Type::Field:
                executeFieldRowBuilder(inspectorRow);
                break;
            }
        }

        fieldTable->InitLayout();
    }

    void InspectorFieldBuilder::Draw(const std::vector<InspectedComponent>& inspectedComponents)
    {
        for (auto& binding : bindings)
        {
            const auto* inspectedField =
                FindField(inspectedComponents, binding.inspectorRow.componentName, binding.inspectorRow.fieldName);
            if (!inspectedField || inspectedField->valueType != binding.inspectorRow.valueType ||
                !inspectedField->value)
            {
                continue;
            }

            switch (binding.inspectorRow.fieldKind)
            {
            case InspectorFieldKind::Vector3:
                if (inspectedField->mutableValue)
                {
                    auto& vectorValue = *static_cast<Vector3*>(inspectedField->mutableValue);
                    binding.mutableVector = &vectorValue;
                    RenderUI(vectorValue, binding);
                }
                else
                {
                    binding.mutableVector = nullptr;
                    RenderUI(*static_cast<const Vector3*>(inspectedField->value), binding);
                }
                break;
            default:
                break;
            }
        }
    }

    void InspectorFieldBuilder::Scroll(const int amount)
    {
        const std::size_t maxOffset = totalRows > visibleRows ? totalRows - visibleRows : 0;

        if (amount < 0)
        {
            const auto positiveAmount = static_cast<std::size_t>(-amount);
            scrollOffset = positiveAmount > scrollOffset ? 0 : scrollOffset - positiveAmount;
        }
        else if (amount > 0)
        {
            scrollOffset = std::min(maxOffset, scrollOffset + static_cast<std::size_t>(amount));
        }
    }

    bool InspectorFieldBuilder::HasOverflow() const
    {
        return totalRows > visibleRows;
    }

    std::size_t InspectorFieldBuilder::TotalRows() const
    {
        return totalRows;
    }

    std::size_t InspectorFieldBuilder::VisibleRows() const
    {
        return visibleRows;
    }

    std::size_t InspectorFieldBuilder::ScrollOffset() const
    {
        return scrollOffset;
    }

    void InspectorFieldBuilder::rebuildRows(const std::vector<InspectedComponent>& inspectedComponents)
    {
        rows.clear();
        rowSignature.clear();

        for (const auto& inspectedComponent : inspectedComponents)
        {
            const auto headerIndex = rows.size();
            rows.push_back(
                FieldRow{
                    .type = FieldRow::Type::ComponentHeader, .componentName = inspectedComponent.displayName});

            std::size_t supportedFields = 0;
            for (const auto& inspectedField : inspectedComponent.fields)
            {
                switch (inspectedField.kind)
                {
                case InspectorFieldKind::Vector3: {
                    auto inspectorRow = FieldRow{
                        .type = FieldRow::Type::Field,
                        .componentName = inspectedComponent.displayName,
                        .fieldName = inspectedField.label,
                        .fieldKind = inspectedField.kind,
                        .valueType = inspectedField.valueType,
                        .isMutable = inspectedField.IsMutable()};

                    rows.push_back(std::move(inspectorRow));
                    ++supportedFields;
                    break;
                }
                default:
                    break;
                }
            }

            if (supportedFields == 0)
            {
                rows.resize(headerIndex);
            }
        }
    }

    void InspectorFieldBuilder::createHeaderRow(const std::string& label) const
    {
        auto* headerRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
        auto* headerCell = headerRow->CreateTableCell(Padding{2, 2, 2, 2});
        auto header = std::make_unique<TextBox>(
            ui, headerCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        headerCell->CreateTextbox(std::move(header), label);
    }

    void InspectorFieldBuilder::executeFieldRowBuilder(const FieldRow& inspectorRow)
    {
        switch (inspectorRow.fieldKind)
        {
        case InspectorFieldKind::Vector3:
            createVector3Row(inspectorRow);
            break;
        default:
            break;
        }
    }

    void InspectorFieldBuilder::createVector3Row(const FieldRow& vectorRow)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
        const std::size_t bindingIndex = bindings.size();

        auto* labelCell = uiRow->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), vectorRow.fieldName + ":");

        const auto addAxis =
            [this, uiRow, bindingIndex, &vectorRow](const char* axis, const std::size_t axisIndex) {
                auto* axisCell = uiRow->CreateTableCell(4.0f, Padding{1, 1, 1, 1});
                auto axisLabel = std::make_unique<TextBox>(
                    ui, axisCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::CENTER);
                axisCell->CreateTextbox(std::move(axisLabel), axis);

                auto* valueCell = uiRow->CreateTableCell(18.0f, Padding{1, 1, 2, 2});
                if (!vectorRow.isMutable)
                {
                    auto valueText = std::make_unique<TextBox>(
                        ui, valueCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::RIGHT);
                    return valueCell->CreateTextbox(std::move(valueText), "0.00");
                }

                auto valueText = std::make_unique<TextInput>(
                    ui,
                    valueCell,
                    [this, bindingIndex, axisIndex](const std::string& submittedValue) {
                        setVector3Axis(bindingIndex, axisIndex, submittedValue);
                    },
                    EditorInspectorInputFontInfo(),
                    VertAlignment::MIDDLE,
                    HoriAlignment::RIGHT);
                return valueCell->CreateTextbox(std::move(valueText), "0.00");
            };

        auto* xText = addAxis("X", 0);
        auto* yText = addAxis("Y", 1);
        auto* zText = addAxis("Z", 2);

        bindings.push_back(FieldBinding{.inspectorRow = vectorRow, .valueTexts = {xText, yText, zText}});
    }

    void InspectorFieldBuilder::updateRowMetrics()
    {
        visibleRows = INSPECTOR_VISIBLE_ROWS;
        totalRows = rows.size();

        const std::size_t maxOffset = totalRows > visibleRows ? totalRows - visibleRows : 0;
        scrollOffset = std::min(scrollOffset, maxOffset);
    }

    void InspectorFieldBuilder::updateScrollbarText() const
    {
        const bool showScrollbar = HasOverflow();
        if (scrollbarUpText) scrollbarUpText->SetContent(showScrollbar ? "^" : "");
        if (scrollbarTrackText) scrollbarTrackText->SetContent(showScrollbar ? " " : "");
        if (scrollbarDownText) scrollbarDownText->SetContent(showScrollbar ? "v" : "");
    }

    void InspectorFieldBuilder::scrollFromMouseWheel(const Rectangle& bounds)
    {
        const auto mousePosition = GetMousePosition();
        if (!CheckCollisionPointRec(mousePosition, bounds)) return;

        const float wheelMove = GetMouseWheelMove();
        if (wheelMove > 0.0f)
        {
            Scroll(-static_cast<int>(std::ceil(wheelMove)));
        }
        else if (wheelMove < 0.0f)
        {
            Scroll(static_cast<int>(std::ceil(-wheelMove)));
        }
    }

    void InspectorFieldBuilder::setVector3Axis(
        const std::size_t bindingIndex, const std::size_t axisIndex, const std::string& submittedValue)
    {
        if (bindingIndex >= bindings.size() || axisIndex >= 3) return;

        auto* value = bindings[bindingIndex].mutableVector;
        if (!value) return;

        try
        {
            const float submittedFloat = std::stof(submittedValue);
            const auto axes = std::array<float*, 3>{&value->x, &value->y, &value->z};
            *axes[axisIndex] = submittedFloat;
        }
        catch (const std::exception&)
        {
        }
    }

    void InspectorFieldBuilder::RenderUI(Vector3& value, const FieldBinding& binding)
    {
        RenderUI(static_cast<const Vector3&>(value), binding);
    }

    void InspectorFieldBuilder::RenderUI(const Vector3& value, const FieldBinding& binding)
    {
        if (binding.valueTexts.size() < 3) return;

        if (binding.valueTexts[0]) binding.valueTexts[0]->SetContent(FormatFloat(value.x));
        if (binding.valueTexts[1]) binding.valueTexts[1]->SetContent(FormatFloat(value.y));
        if (binding.valueTexts[2]) binding.valueTexts[2]->SetContent(FormatFloat(value.z));
    }

    std::string InspectorFieldBuilder::buildBlueprintSignature(
        const std::vector<InspectedComponent>& inspectedComponents)
    {
        std::ostringstream signature;
        for (const auto& inspectedComponent : inspectedComponents)
        {
            signature << inspectedComponent.displayName << '{';
            for (const auto& inspectedField : inspectedComponent.fields)
            {
                switch (inspectedField.kind)
                {
                case InspectorFieldKind::Vector3:
                    signature << inspectedField.label << ':' << inspectedField.valueType.name() << ':'
                              << (inspectedField.IsMutable() ? "rw" : "ro") << ';';
                    break;
                default:
                    break;
                }
            }
            signature << '}';
        }
        return signature.str();
    }

    std::string InspectorFieldBuilder::buildRowSignature() const
    {
        std::ostringstream signature;
        signature << blueprintSignature << "scroll:" << scrollOffset << "rows:" << totalRows;
        return signature.str();
    }
} // namespace sage::editor
