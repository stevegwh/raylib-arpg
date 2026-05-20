#include "InspectorFieldBuilder.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ui/Scrollbar.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <format>
#include <memory>
#include <sstream>
#include <string_view>
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

        std::string formatFloat(float v)
        {
            return std::format("{:.2f}", v);
        }

        bool parseFloat(const std::string& s, float& out)
        {
            try
            {
                std::size_t consumed = 0;
                const float v = std::stof(s, &consumed);
                if (consumed == 0) return false;
                out = v;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool parseInt(const std::string& s, int& out)
        {
            try
            {
                std::size_t consumed = 0;
                const long v = std::stol(s, &consumed);
                if (consumed == 0) return false;
                out = static_cast<int>(v);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool parseUInt(const std::string& s, unsigned int& out)
        {
            try
            {
                std::size_t consumed = 0;
                const long v = std::stol(s, &consumed);
                if (consumed == 0) return false;
                out = static_cast<unsigned int>(std::max<long>(0, v));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool parseUInt64(const std::string& s, std::uint64_t& out)
        {
            try
            {
                std::size_t consumed = 0;
                const unsigned long long v = std::stoull(s, &consumed);
                if (consumed == 0) return false;
                out = static_cast<std::uint64_t>(v);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        std::vector<std::string> ComponentLabelsFor(const InspectorField::Kind kind)
        {
            switch (kind)
            {
            case InspectorField::Kind::Vec2:
                return {"X", "Y"};
            case InspectorField::Kind::Vec3:
                return {"X", "Y", "Z"};
            case InspectorField::Kind::Color:
                return {"R", "G", "B", "A"};
            default:
                return {};
            }
        }

        bool IsComponentKind(const InspectorField::Kind kind)
        {
            return kind == InspectorField::Kind::Vec2 || kind == InspectorField::Kind::Vec3 ||
                   kind == InspectorField::Kind::Color;
        }

        bool IsSingleScalarKind(const InspectorField::Kind kind)
        {
            return kind == InspectorField::Kind::Int || kind == InspectorField::Kind::UInt ||
                   kind == InspectorField::Kind::UInt64 || kind == InspectorField::Kind::Float ||
                   kind == InspectorField::Kind::String;
        }

        const InspectorField* FindField(
            const std::vector<InspectedComponent>& components,
            const std::string_view componentName,
            const std::string_view fieldName)
        {
            for (const auto& c : components)
            {
                if (c.displayName != componentName) continue;
                for (const auto& f : c.fields)
                {
                    if (f.label == fieldName) return &f;
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
        InspectorField::Kind fieldKind = InspectorField::Kind::Bool;
        bool editable = false;
    };

    struct InspectorFieldBuilder::FieldBinding
    {
        std::string componentName;
        std::string fieldName;
        InspectorField::Kind kind = InspectorField::Kind::Bool;
        Checkbox* checkbox = nullptr;
        DropdownList* dropdown = nullptr;
        std::vector<TextBox*> valueTexts;
    };

    InspectorFieldBuilder::InspectorFieldBuilder() = default;
    InspectorFieldBuilder::~InspectorFieldBuilder() = default;

    void InspectorFieldBuilder::Attach(GameUIEngine* _ui, Table* _fieldTable)
    {
        ui = _ui;
        fieldTable = _fieldTable;
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
            const auto& row = rows[i];
            if (row.type == FieldRow::Type::ComponentHeader)
            {
                createHeaderRow(row.componentName);
                continue;
            }

            const auto* field = FindField(inspectedComponents, row.componentName, row.fieldName);
            if (!field) continue;

            FieldBinding binding{
                .componentName = row.componentName, .fieldName = row.fieldName, .kind = field->kind};
            createFieldRow(binding, *field);
            bindings.push_back(std::move(binding));
        }

        fieldTable->InitLayout();
    }

    void InspectorFieldBuilder::Draw(const std::vector<InspectedComponent>& inspectedComponents) const
    {
        for (auto& binding : bindings)
        {
            const auto* field = FindField(inspectedComponents, n, binding.fieldName);
            if (!field || field->kind != binding.kind || !field->data) continue;

            switch (binding.kind)
            {
            case InspectorField::Kind::Bool:
                if (binding.checkbox) binding.checkbox->SetChecked(*static_cast<bool*>(field->data));
                break;
            case InspectorField::Kind::Int:
                if (!binding.valueTexts.empty() && binding.valueTexts[0])
                    binding.valueTexts[0]->SetContent(std::to_string(*static_cast<int*>(field->data)));
                break;
            case InspectorField::Kind::UInt:
                if (!binding.valueTexts.empty() && binding.valueTexts[0])
                    binding.valueTexts[0]->SetContent(std::to_string(*static_cast<unsigned int*>(field->data)));
                break;
            case InspectorField::Kind::UInt64:
                if (!binding.valueTexts.empty() && binding.valueTexts[0])
                    binding.valueTexts[0]->SetContent(std::to_string(*static_cast<std::uint64_t*>(field->data)));
                break;
            case InspectorField::Kind::Float:
                if (!binding.valueTexts.empty() && binding.valueTexts[0])
                    binding.valueTexts[0]->SetContent(formatFloat(*static_cast<float*>(field->data)));
                break;
            case InspectorField::Kind::String:
                if (!binding.valueTexts.empty() && binding.valueTexts[0])
                    binding.valueTexts[0]->SetContent(*static_cast<std::string*>(field->data));
                break;
            case InspectorField::Kind::Vec2: {
                const auto* v = static_cast<Vector2*>(field->data);
                if (binding.valueTexts.size() >= 2)
                {
                    if (binding.valueTexts[0]) binding.valueTexts[0]->SetContent(formatFloat(v->x));
                    if (binding.valueTexts[1]) binding.valueTexts[1]->SetContent(formatFloat(v->y));
                }
                break;
            }
            case InspectorField::Kind::Vec3: {
                const auto* v = static_cast<Vector3*>(field->data);
                if (binding.valueTexts.size() >= 3)
                {
                    if (binding.valueTexts[0]) binding.valueTexts[0]->SetContent(formatFloat(v->x));
                    if (binding.valueTexts[1]) binding.valueTexts[1]->SetContent(formatFloat(v->y));
                    if (binding.valueTexts[2]) binding.valueTexts[2]->SetContent(formatFloat(v->z));
                }
                break;
            }
            case InspectorField::Kind::Color: {
                const auto* c = static_cast<Color*>(field->data);
                if (binding.valueTexts.size() >= 4)
                {
                    if (binding.valueTexts[0])
                        binding.valueTexts[0]->SetContent(std::to_string(static_cast<int>(c->r)));
                    if (binding.valueTexts[1])
                        binding.valueTexts[1]->SetContent(std::to_string(static_cast<int>(c->g)));
                    if (binding.valueTexts[2])
                        binding.valueTexts[2]->SetContent(std::to_string(static_cast<int>(c->b)));
                    if (binding.valueTexts[3])
                        binding.valueTexts[3]->SetContent(std::to_string(static_cast<int>(c->a)));
                }
                break;
            }
            case InspectorField::Kind::Enum:
                if (binding.dropdown)
                {
                    if (binding.dropdown->GetOptions() != field->enumOptions)
                        binding.dropdown->SetOptions(field->enumOptions);
                    if (field->getEnumIndex) binding.dropdown->SetSelectedIndex(field->getEnumIndex());
                }
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

        for (const auto& component : inspectedComponents)
        {
            const auto headerIndex = rows.size();
            rows.push_back(
                FieldRow{.type = FieldRow::Type::ComponentHeader, .componentName = component.displayName});

            for (const auto& field : component.fields)
            {
                rows.push_back(
                    FieldRow{
                        .type = FieldRow::Type::Field,
                        .componentName = component.displayName,
                        .fieldName = field.label,
                        .fieldKind = field.kind,
                        .editable = field.editable});
            }

            if (rows.size() == headerIndex + 1)
            {
                // No fields under this header — drop the header to avoid an empty section.
                rows.pop_back();
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

    void InspectorFieldBuilder::createFieldRow(FieldBinding& binding, const InspectorField& field)
    {
        if (field.kind == InspectorField::Kind::Bool)
        {
            createBoolRow(binding, field);
        }
        else if (IsSingleScalarKind(field.kind))
        {
            createScalarRow(binding, field);
        }
        else if (IsComponentKind(field.kind))
        {
            createComponentRow(binding, field);
        }
        else if (field.kind == InspectorField::Kind::Enum)
        {
            createEnumRow(binding, field);
        }
    }

    void InspectorFieldBuilder::createBoolRow(FieldBinding& binding, const InspectorField& field)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});

        auto* labelCell = uiRow->CreateTableCell(82.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), field.label + ":");

        auto* valueCell = uiRow->CreateTableCell(18.0f, Padding{1, 1, 2, 2});
        auto checkbox =
            std::make_unique<Checkbox>(ui, valueCell, false, VertAlignment::MIDDLE, HoriAlignment::CENTER);
        auto* cb = valueCell->CreateCheckbox(std::move(checkbox));
        if (!field.editable)
        {
            cb->stateLocked = true;
        }
        else
        {
            auto* p = static_cast<bool*>(field.data);
            cb->onValueChanged.Subscribe([p](const bool v) { *p = v; });
        }
        binding.checkbox = cb;
    }

    void InspectorFieldBuilder::createScalarRow(FieldBinding& binding, const InspectorField& field)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});

        auto* labelCell = uiRow->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), field.label + ":");

        auto* valueCell = uiRow->CreateTableCell(66.0f, Padding{1, 1, 2, 2});
        const auto valueAlignment =
            field.kind == InspectorField::Kind::String ? HoriAlignment::LEFT : HoriAlignment::RIGHT;

        TextBox* valueText = nullptr;
        if (field.editable)
        {
            const auto kind = field.kind;
            void* data = field.data;
            auto input = std::make_unique<TextInput>(
                ui,
                valueCell,
                [kind, data](const std::string& s) {
                    switch (kind)
                    {
                    case InspectorField::Kind::Int: {
                        int v = 0;
                        if (parseInt(s, v)) *static_cast<int*>(data) = v;
                        break;
                    }
                    case InspectorField::Kind::UInt: {
                        unsigned int v = 0;
                        if (parseUInt(s, v)) *static_cast<unsigned int*>(data) = v;
                        break;
                    }
                    case InspectorField::Kind::UInt64: {
                        std::uint64_t v = 0;
                        if (parseUInt64(s, v)) *static_cast<std::uint64_t*>(data) = v;
                        break;
                    }
                    case InspectorField::Kind::Float: {
                        float v = 0;
                        if (parseFloat(s, v)) *static_cast<float*>(data) = v;
                        break;
                    }
                    case InspectorField::Kind::String:
                        *static_cast<std::string*>(data) = s;
                        break;
                    default:
                        break;
                    }
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

        binding.valueTexts = {valueText};
    }

    void InspectorFieldBuilder::createComponentRow(FieldBinding& binding, const InspectorField& field)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});

        auto* labelCell = uiRow->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), field.label + ":");

        const auto axisLabels = ComponentLabelsFor(field.kind);
        const std::size_t componentCount = std::max<std::size_t>(1, axisLabels.size());
        const float axisLabelWidth = 4.0f;
        const float valueWidth =
            (66.0f - axisLabelWidth * static_cast<float>(componentCount)) / static_cast<float>(componentCount);
        std::vector<TextBox*> valueTexts;
        valueTexts.reserve(componentCount);

        for (std::size_t i = 0; i < axisLabels.size(); ++i)
        {
            auto* axisCell = uiRow->CreateTableCell(axisLabelWidth, Padding{1, 1, 1, 1});
            auto axisLabel = std::make_unique<TextBox>(
                ui, axisCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::CENTER);
            axisCell->CreateTextbox(std::move(axisLabel), axisLabels[i]);

            auto* valueCell = uiRow->CreateTableCell(valueWidth, Padding{1, 1, 2, 2});
            if (!field.editable)
            {
                auto text = std::make_unique<TextBox>(
                    ui, valueCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::RIGHT);
                valueTexts.push_back(valueCell->CreateTextbox(std::move(text), ""));
                continue;
            }

            const auto kind = field.kind;
            void* data = field.data;
            const std::size_t axisIndex = i;
            auto input = std::make_unique<TextInput>(
                ui,
                valueCell,
                [kind, data, axisIndex](const std::string& s) {
                    if (kind == InspectorField::Kind::Vec2)
                    {
                        auto* v = static_cast<Vector2*>(data);
                        float parsed = 0;
                        if (!parseFloat(s, parsed)) return;
                        if (axisIndex == 0)
                            v->x = parsed;
                        else if (axisIndex == 1)
                            v->y = parsed;
                    }
                    else if (kind == InspectorField::Kind::Vec3)
                    {
                        auto* v = static_cast<Vector3*>(data);
                        float parsed = 0;
                        if (!parseFloat(s, parsed)) return;
                        if (axisIndex == 0)
                            v->x = parsed;
                        else if (axisIndex == 1)
                            v->y = parsed;
                        else if (axisIndex == 2)
                            v->z = parsed;
                    }
                    else if (kind == InspectorField::Kind::Color)
                    {
                        auto* c = static_cast<Color*>(data);
                        int parsed = 0;
                        if (!parseInt(s, parsed)) return;
                        const auto clamped = static_cast<unsigned char>(std::clamp(parsed, 0, 255));
                        if (axisIndex == 0)
                            c->r = clamped;
                        else if (axisIndex == 1)
                            c->g = clamped;
                        else if (axisIndex == 2)
                            c->b = clamped;
                        else if (axisIndex == 3)
                            c->a = clamped;
                    }
                },
                EditorInspectorInputFontInfo(),
                VertAlignment::MIDDLE,
                HoriAlignment::RIGHT);
            valueTexts.push_back(valueCell->CreateTextbox(std::move(input), ""));
        }

        binding.valueTexts = std::move(valueTexts);
    }

    void InspectorFieldBuilder::createEnumRow(FieldBinding& binding, const InspectorField& field)
    {
        auto* uiRow = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});

        auto* labelCell = uiRow->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
        auto label = std::make_unique<TextBox>(
            ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
        labelCell->CreateTextbox(std::move(label), field.label + ":");

        auto* valueCell = uiRow->CreateTableCell(66.0f, Padding{1, 1, 2, 2});
        auto dropdown = std::make_unique<DropdownList>(
            ui,
            valueCell,
            field.enumOptions,
            0,
            EditorInspectorInputFontInfo(),
            VertAlignment::MIDDLE,
            HoriAlignment::LEFT);
        dropdown->SetMaxVisibleOptions(5);
        auto* dd = valueCell->CreateDropdownList(std::move(dropdown));
        if (!field.editable)
        {
            dd->stateLocked = true;
        }
        else
        {
            auto setter = field.setEnumIndex;
            dd->onSelectionChanged.Subscribe([setter](const std::size_t idx, const std::string&) {
                if (setter) setter(idx);
            });
        }
        binding.dropdown = dd;
    }

    std::string InspectorFieldBuilder::buildBlueprintSignature(
        const std::vector<InspectedComponent>& inspectedComponents)
    {
        std::ostringstream s;
        for (const auto& c : inspectedComponents)
        {
            s << c.displayName << '{';
            for (const auto& f : c.fields)
            {
                s << f.label << ':' << static_cast<int>(f.kind) << ':' << (f.editable ? "rw" : "ro") << ';';
            }
            s << '}';
        }
        return s.str();
    }

    std::string InspectorFieldBuilder::buildRowSignature() const
    {
        std::ostringstream s;
        const std::size_t scrollOffset = scrollbar ? scrollbar->ScrollOffset() : 0;
        s << blueprintSignature << "scroll:" << scrollOffset << "rows:" << rows.size();
        return s.str();
    }
} // namespace sage::editor
