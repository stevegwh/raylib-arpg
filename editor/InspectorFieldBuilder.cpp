#include "InspectorFieldBuilder.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ui/Scrollbar.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
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

        bool IsSupportedFieldKind(const InspectorFieldKind kind)
        {
            return kind != InspectorFieldKind::Unsupported;
        }

        bool IsScalarFieldKind(const InspectorFieldKind kind)
        {
            return kind == InspectorFieldKind::SignedInteger || kind == InspectorFieldKind::UnsignedInteger ||
                   kind == InspectorFieldKind::FloatingPoint || kind == InspectorFieldKind::String;
        }

        bool IsComponentFieldKind(const InspectorFieldKind kind)
        {
            return kind == InspectorFieldKind::Vector2 || kind == InspectorFieldKind::Vector3 ||
                   kind == InspectorFieldKind::Color;
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
        std::vector<std::string> componentLabels;
        std::vector<std::string> enumOptions;
        bool isMutable = false;
    };

    struct InspectorFieldBuilder::FieldBinding
    {
        FieldRow inspectorRow;
        std::vector<TextBox*> valueTexts;
        Checkbox* checkbox = nullptr;
        DropdownList* dropdown = nullptr;
        std::function<void(bool)> currentBoolSetter;
        std::function<bool(const std::string&)> currentScalarSetter;
        std::function<bool(std::size_t, const std::string&)> currentComponentSetter;
        std::function<void(std::size_t)> currentEnumSetter;
    };

    InspectorFieldBuilder::InspectorFieldBuilder() = default;

    InspectorFieldBuilder::~InspectorFieldBuilder() = default;

    void InspectorFieldBuilder::Attach(GameUIEngine* ui, Table* fieldTable)
    {
        this->ui = ui;
        this->fieldTable = fieldTable;
    }

    void InspectorFieldBuilder::AttachScrollbar(Scrollbar* sb)
    {
        scrollbar = sb;
        if (scrollbar)
        {
            scrollSub = scrollbar->onScrollChanged.Subscribe([this]() { pendingRebuild = true; });
        }
    }

    void InspectorFieldBuilder::Rebuild(const std::vector<InspectedComponent>& inspectedComponents)
    {
        if (!fieldTable || !ui) return;

        const auto signature = buildBlueprintSignature(inspectedComponents);
        if (signature != blueprintSignature)
        {
            blueprintSignature = signature;
            rebuildRows(inspectedComponents);
        }

        if (scrollbar) scrollbar->ClampOffset();

        const auto newRowSignature = buildRowSignature();
        if (newRowSignature == rowSignature && !pendingRebuild) return;

        rowSignature = newRowSignature;
        pendingRebuild = false;
        bindings.clear();
        fieldTable->children.clear();

        const std::size_t scrollOffset = scrollbar ? scrollbar->ScrollOffset() : 0;
        const std::size_t visibleRows = scrollbar ? scrollbar->VisibleRows() : rows.size();
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
            binding.currentBoolSetter = {};
            binding.currentScalarSetter = {};
            binding.currentComponentSetter = {};
            binding.currentEnumSetter = {};

            const auto* inspectedField =
                FindField(inspectedComponents, binding.inspectorRow.componentName, binding.inspectorRow.fieldName);
            if (!inspectedField || inspectedField->valueType != binding.inspectorRow.valueType ||
                !inspectedField->value)
            {
                continue;
            }

            switch (binding.inspectorRow.fieldKind)
            {
            case InspectorFieldKind::Bool:
                if (binding.checkbox && inspectedField->boolValue)
                {
                    binding.checkbox->SetChecked(inspectedField->boolValue());
                }
                if (inspectedField->IsMutable())
                {
                    binding.currentBoolSetter = inspectedField->setBoolValue;
                }
                break;
            case InspectorFieldKind::SignedInteger:
            case InspectorFieldKind::UnsignedInteger:
            case InspectorFieldKind::FloatingPoint:
            case InspectorFieldKind::String:
                if (!binding.valueTexts.empty() && binding.valueTexts[0] && inspectedField->textValue)
                {
                    binding.valueTexts[0]->SetContent(inspectedField->textValue());
                }
                if (inspectedField->IsMutable() && inspectedField->setTextValue)
                {
                    binding.currentScalarSetter =
                        [setter = inspectedField->setTextValue, options = inspectedField->options](
                            const std::string& submittedValue) { return setter(submittedValue, options); };
                }
                break;
            case InspectorFieldKind::Vector2:
            case InspectorFieldKind::Vector3:
            case InspectorFieldKind::Color:
                if (inspectedField->componentTextValue)
                {
                    const std::size_t componentCount =
                        std::min(binding.valueTexts.size(), binding.inspectorRow.componentLabels.size());
                    for (std::size_t i = 0; i < componentCount; ++i)
                    {
                        if (binding.valueTexts[i])
                            binding.valueTexts[i]->SetContent(inspectedField->componentTextValue(i));
                    }
                }
                if (inspectedField->IsMutable() && inspectedField->setComponentTextValue)
                {
                    binding.currentComponentSetter =
                        [setter = inspectedField->setComponentTextValue, options = inspectedField->options](
                            const std::size_t componentIndex, const std::string& submittedValue) {
                            return setter(componentIndex, submittedValue, options);
                        };
                }
                break;
            case InspectorFieldKind::Enum:
                if (binding.dropdown)
                {
                    if (binding.dropdown->GetOptions() != inspectedField->enumOptions)
                    {
                        binding.dropdown->SetOptions(inspectedField->enumOptions);
                    }
                    if (inspectedField->enumIndexValue)
                    {
                        if (const auto selected = inspectedField->enumIndexValue())
                        {
                            binding.dropdown->SetSelectedIndex(*selected);
                        }
                    }
                }
                if (inspectedField->IsMutable())
                {
                    binding.currentEnumSetter = inspectedField->setEnumIndexValue;
                }
                break;
            default:
                break;
            }
        }
    }

    std::size_t InspectorFieldBuilder::TotalRows() const
    {
        return rows.size();
    }

    std::size_t InspectorFieldBuilder::VisibleRows() const
    {
        return INSPECTOR_VISIBLE_ROWS;
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
                if (!IsSupportedFieldKind(inspectedField.kind)) continue;

                auto inspectorRow = FieldRow{
                    .type = FieldRow::Type::Field,
                    .componentName = inspectedComponent.displayName,
                    .fieldName = inspectedField.label,
                    .fieldKind = inspectedField.kind,
                    .valueType = inspectedField.valueType,
                    .componentLabels = inspectedField.componentLabels,
                    .enumOptions = inspectedField.enumOptions,
                    .isMutable = inspectedField.IsMutable()};

                rows.push_back(std::move(inspectorRow));
                ++supportedFields;
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
        if (inspectorRow.fieldKind == InspectorFieldKind::Bool)
        {
            createBoolRow(inspectorRow);
        }
        else if (IsScalarFieldKind(inspectorRow.fieldKind))
        {
            createScalarRow(inspectorRow);
        }
        else if (IsComponentFieldKind(inspectorRow.fieldKind))
        {
            createComponentRow(inspectorRow);
        }
        else if (inspectorRow.fieldKind == InspectorFieldKind::Enum)
        {
            createEnumRow(inspectorRow);
        }
    }

    void InspectorFieldBuilder::createBoolRow(const FieldRow& boolRow)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
        const std::size_t bindingIndex = bindings.size();

        auto* labelCell = uiRow->CreateTableCell(82.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), boolRow.fieldName + ":");

        auto* valueCell = uiRow->CreateTableCell(18.0f, Padding{1, 1, 2, 2});
        auto checkbox =
            std::make_unique<Checkbox>(ui, valueCell, false, VertAlignment::MIDDLE, HoriAlignment::CENTER);
        auto* checkboxPtr = valueCell->CreateCheckbox(std::move(checkbox));
        if (!boolRow.isMutable)
        {
            checkboxPtr->stateLocked = true;
        }
        else
        {
            checkboxPtr->onValueChanged.Subscribe(
                [this, bindingIndex](const bool checked) { setBoolValue(bindingIndex, checked); });
        }

        bindings.push_back(FieldBinding{.inspectorRow = boolRow, .checkbox = checkboxPtr});
    }

    void InspectorFieldBuilder::createScalarRow(const FieldRow& scalarRow)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
        const std::size_t bindingIndex = bindings.size();

        auto* labelCell = uiRow->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), scalarRow.fieldName + ":");

        auto* valueCell = uiRow->CreateTableCell(66.0f, Padding{1, 1, 2, 2});
        const auto valueAlignment =
            scalarRow.fieldKind == InspectorFieldKind::String ? HoriAlignment::LEFT : HoriAlignment::RIGHT;
        TextBox* valueText = nullptr;
        if (scalarRow.isMutable)
        {
            auto input = std::make_unique<TextInput>(
                ui,
                valueCell,
                [this, bindingIndex](const std::string& submittedValue) {
                    setScalarValue(bindingIndex, submittedValue);
                },
                EditorInspectorInputFontInfo(),
                VertAlignment::MIDDLE,
                valueAlignment);
            valueText = valueCell->CreateTextbox(std::move(input), "");
        }
        else
        {
            auto text = std::make_unique<TextBox>(
                ui, valueCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, valueAlignment);
            valueText = valueCell->CreateTextbox(std::move(text), "");
        }

        bindings.push_back(FieldBinding{.inspectorRow = scalarRow, .valueTexts = {valueText}});
    }

    void InspectorFieldBuilder::createComponentRow(const FieldRow& componentRow)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
        const std::size_t bindingIndex = bindings.size();

        auto* labelCell = uiRow->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), componentRow.fieldName + ":");

        const std::size_t componentCount = std::max<std::size_t>(1, componentRow.componentLabels.size());
        const float axisLabelWidth = 4.0f;
        const float valueWidth =
            (66.0f - axisLabelWidth * static_cast<float>(componentCount)) / static_cast<float>(componentCount);
        std::vector<TextBox*> valueTexts;
        valueTexts.reserve(componentCount);

        const auto addAxis = [this, uiRow, bindingIndex, &componentRow, valueWidth](
                                 const std::string& axis, const std::size_t axisIndex) {
            auto* axisCell = uiRow->CreateTableCell(4.0f, Padding{1, 1, 1, 1});
            auto axisLabel = std::make_unique<TextBox>(
                ui, axisCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::CENTER);
            axisCell->CreateTextbox(std::move(axisLabel), axis);

            auto* valueCell = uiRow->CreateTableCell(valueWidth, Padding{1, 1, 2, 2});
            if (!componentRow.isMutable)
            {
                auto valueText = std::make_unique<TextBox>(
                    ui, valueCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::RIGHT);
                return valueCell->CreateTextbox(std::move(valueText), "");
            }

            auto valueText = std::make_unique<TextInput>(
                ui,
                valueCell,
                [this, bindingIndex, axisIndex](const std::string& submittedValue) {
                    setComponentValue(bindingIndex, axisIndex, submittedValue);
                },
                EditorInspectorInputFontInfo(),
                VertAlignment::MIDDLE,
                HoriAlignment::RIGHT);
            return valueCell->CreateTextbox(std::move(valueText), "");
        };

        for (std::size_t i = 0; i < componentRow.componentLabels.size(); ++i)
        {
            valueTexts.push_back(addAxis(componentRow.componentLabels[i], i));
        }

        bindings.push_back(FieldBinding{.inspectorRow = componentRow, .valueTexts = std::move(valueTexts)});
    }

    void InspectorFieldBuilder::createEnumRow(const FieldRow& enumRow)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
        const std::size_t bindingIndex = bindings.size();

        auto* labelCell = uiRow->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), enumRow.fieldName + ":");

        auto* valueCell = uiRow->CreateTableCell(66.0f, Padding{1, 1, 2, 2});
        auto dropdown = std::make_unique<DropdownList>(
            ui,
            valueCell,
            enumRow.enumOptions,
            0,
            EditorInspectorInputFontInfo(),
            VertAlignment::MIDDLE,
            HoriAlignment::LEFT);
        dropdown->SetMaxVisibleOptions(5);
        auto* dropdownPtr = valueCell->CreateDropdownList(std::move(dropdown));
        if (!enumRow.isMutable)
        {
            dropdownPtr->stateLocked = true;
        }
        else
        {
            dropdownPtr->onSelectionChanged.Subscribe(
                [this, bindingIndex](const std::size_t optionIndex, const std::string&) {
                    setEnumValue(bindingIndex, optionIndex);
                });
        }

        bindings.push_back(FieldBinding{.inspectorRow = enumRow, .dropdown = dropdownPtr});
    }

    void InspectorFieldBuilder::setBoolValue(const std::size_t bindingIndex, const bool submittedValue)
    {
        if (bindingIndex >= bindings.size()) return;
        if (bindings[bindingIndex].currentBoolSetter)
        {
            bindings[bindingIndex].currentBoolSetter(submittedValue);
        }
    }

    void InspectorFieldBuilder::setScalarValue(const std::size_t bindingIndex, const std::string& submittedValue)
    {
        if (bindingIndex >= bindings.size()) return;
        if (bindings[bindingIndex].currentScalarSetter)
        {
            bindings[bindingIndex].currentScalarSetter(submittedValue);
        }
    }

    void InspectorFieldBuilder::setComponentValue(
        const std::size_t bindingIndex, const std::size_t componentIndex, const std::string& submittedValue)
    {
        if (bindingIndex >= bindings.size()) return;
        if (bindings[bindingIndex].currentComponentSetter)
        {
            bindings[bindingIndex].currentComponentSetter(componentIndex, submittedValue);
        }
    }

    void InspectorFieldBuilder::setEnumValue(const std::size_t bindingIndex, const std::size_t optionIndex)
    {
        if (bindingIndex >= bindings.size()) return;
        if (bindings[bindingIndex].currentEnumSetter)
        {
            bindings[bindingIndex].currentEnumSetter(optionIndex);
        }
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
                if (IsSupportedFieldKind(inspectedField.kind))
                {
                    signature << inspectedField.label << ':' << inspectedField.valueType.name() << ':'
                              << static_cast<int>(inspectedField.kind) << ':'
                              << (inspectedField.IsMutable() ? "rw" : "ro") << ':';
                    for (const auto& label : inspectedField.componentLabels)
                    {
                        signature << label << ',';
                    }
                    signature << ':';
                    for (const auto& option : inspectedField.enumOptions)
                    {
                        signature << option << ',';
                    }
                    signature << ';';
                }
            }
            signature << '}';
        }
        return signature.str();
    }

    std::string InspectorFieldBuilder::buildRowSignature() const
    {
        std::ostringstream signature;
        const std::size_t scrollOffset = scrollbar ? scrollbar->ScrollOffset() : 0;
        signature << blueprintSignature << "scroll:" << scrollOffset << "rows:" << rows.size();
        return signature.str();
    }
} // namespace sage::editor
