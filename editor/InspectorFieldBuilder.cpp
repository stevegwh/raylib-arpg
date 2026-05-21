#include "InspectorFieldBuilder.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ui/Scrollbar.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"

#include <algorithm>
#include <cstdint>
#include <format>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace sage::editor
{
    using BoolFieldView = InspectorFieldBuilder::BoolFieldView;
    template <class T>
    using ScalarFieldView = InspectorFieldBuilder::ScalarFieldView<T>;
    using Vec2FieldView = InspectorFieldBuilder::Vec2FieldView;
    using Vec3FieldView = InspectorFieldBuilder::Vec3FieldView;
    using ColorFieldView = InspectorFieldBuilder::ColorFieldView;
    using EnumFieldView = InspectorFieldBuilder::EnumFieldView;
    using FieldView = InspectorFieldBuilder::FieldView;

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

        // --- Per-type formatters (display) -----------------------------------------
        std::string formatScalar(int v)
        {
            return std::to_string(v);
        }
        std::string formatScalar(unsigned int v)
        {
            return std::to_string(v);
        }
        std::string formatScalar(std::uint64_t v)
        {
            return std::to_string(v);
        }
        std::string formatScalar(float v)
        {
            return std::format("{:.2f}", v);
        }
        std::string formatScalar(const std::string& v)
        {
            return v;
        }

        // --- Per-type parsers (TextInput → value) ----------------------------------
        bool parseScalar(const std::string& s, int& out)
        {
            try
            {
                std::size_t n = 0;
                const long v = std::stol(s, &n);
                if (n == 0) return false;
                out = static_cast<int>(v);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        bool parseScalar(const std::string& s, unsigned int& out)
        {
            try
            {
                std::size_t n = 0;
                const long v = std::stol(s, &n);
                if (n == 0) return false;
                out = static_cast<unsigned int>(std::max<long>(0, v));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        bool parseScalar(const std::string& s, std::uint64_t& out)
        {
            try
            {
                std::size_t n = 0;
                const unsigned long long v = std::stoull(s, &n);
                if (n == 0) return false;
                out = static_cast<std::uint64_t>(v);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        bool parseScalar(const std::string& s, float& out)
        {
            try
            {
                std::size_t n = 0;
                const float v = std::stof(s, &n);
                if (n == 0) return false;
                out = v;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        bool parseScalar(const std::string& s, std::string& out)
        {
            out = s;
            return true;
        }

        template <class T>
        void commitField(const LeafField<T>& field, const T& value)
        {
            if (field.setter)
                field.setter(value);
            else if (field.data)
                *field.data = value;
        }

        // --- Layout helpers --------------------------------------------------------
        void createLabel(GameUIEngine* ui, TableRow* row, const std::string& label, float widthPct)
        {
            auto* cell = row->CreateTableCell(widthPct, Padding{1, 1, 2, 4});
            auto t = std::make_unique<TextBox>(
                ui, cell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
            cell->CreateTextbox(std::move(t), label + ":");
        }

        // X/Y/Z (or R/G/B/A) label + TextInput<T> on a single component slot of a Vec/Color row.
        // The onValue callback receives the parsed T and writes it into the underlying member.
        template <class T, class OnValue>
        TextBox* createTextInputCell(
            GameUIEngine* ui,
            TableRow* row,
            const std::string& inputLabel,
            float valueWidth,
            bool editable,
            OnValue&& onValue)
        {
            auto* labelCell = row->CreateTableCell(4.0f, Padding{1, 1, 1, 1});
            auto labelText = std::make_unique<TextBox>(
                ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::CENTER);
            labelCell->CreateTextbox(std::move(labelText), inputLabel);

            auto* valueCell = row->CreateTableCell(valueWidth, Padding{1, 1, 2, 2});
            if (!editable)
            {
                auto tb = std::make_unique<TextBox>(
                    ui, valueCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::RIGHT);
                return valueCell->CreateTextbox(std::move(tb), "");
            }
            auto input = std::make_unique<TextInput>(
                ui,
                valueCell,
                [fn = std::forward<OnValue>(onValue)](const std::string& s) {
                    T parsed{};
                    if (parseScalar(s, parsed)) fn(parsed);
                },
                EditorInspectorInputFontInfo(),
                VertAlignment::MIDDLE,
                HoriAlignment::RIGHT);
            return valueCell->CreateTextbox(std::move(input), "");
        }

        // --- createFieldView overloads — one per variant alternative ----------------

        BoolFieldView createFieldView(
            const LeafField<bool>& field, GameUIEngine* ui, Table* fieldTable, const std::string& label, bool editable)
        {
            auto* row = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
            createLabel(ui, row, label, 82.0f);
            auto* valueCell = row->CreateTableCell(18.0f, Padding{1, 1, 2, 2});
            auto cb =
                std::make_unique<Checkbox>(ui, valueCell, false, VertAlignment::MIDDLE, HoriAlignment::CENTER);
            auto* checkbox = valueCell->CreateCheckbox(std::move(cb));
            if (editable)
                checkbox->onValueChanged.Subscribe([field](const bool v) { commitField(field, v); });
            else
                checkbox->stateLocked = true;
            return {checkbox, field};
        }

        template <class T>
        ScalarFieldView<T> createFieldView(
            const LeafField<T>& field, GameUIEngine* ui, Table* fieldTable, const std::string& label, bool editable)
        {
            auto* row = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
            createLabel(ui, row, label, 34.0f);
            auto* valueCell = row->CreateTableCell(66.0f, Padding{1, 1, 2, 2});

            constexpr auto valueAlignment =
                std::is_same_v<T, std::string> ? HoriAlignment::LEFT : HoriAlignment::RIGHT;

            TextBox* text = nullptr;
            if (editable)
            {
                auto input = std::make_unique<TextInput>(
                    ui,
                    valueCell,
                    [field](const std::string& s) {
                        T parsed{};
                        if (parseScalar(s, parsed)) commitField(field, parsed);
                    },
                    EditorInspectorInputFontInfo(),
                    VertAlignment::MIDDLE,
                    valueAlignment);
                text = valueCell->CreateTextbox(std::move(input), "");
            }
            else
            {
                auto tb = std::make_unique<TextBox>(
                    ui, valueCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, valueAlignment);
                text = valueCell->CreateTextbox(std::move(tb), "");
            }
            return {text, field};
        }

        Vec2FieldView createFieldView(
            const LeafField<Vector2>& field,
            GameUIEngine* ui,
            Table* fieldTable,
            const std::string& label,
            bool editable)
        {
            auto* row = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
            createLabel(ui, row, label, 34.0f);
            constexpr float inputCount = 2.0f;
            constexpr float inputLabelWidth = 4.0f;
            const float vw = (66.0f - inputLabelWidth * inputCount) / inputCount;
            auto* x = createTextInputCell<float>(ui, row, "X", vw, editable, [field](float v) {
                Vector2 next = *field.data;
                next.x = v;
                commitField(field, next);
            });
            auto* y = createTextInputCell<float>(ui, row, "Y", vw, editable, [field](float v) {
                Vector2 next = *field.data;
                next.y = v;
                commitField(field, next);
            });
            return {{x, y}, field};
        }

        Vec3FieldView createFieldView(
            const LeafField<Vector3>& field,
            GameUIEngine* ui,
            Table* fieldTable,
            const std::string& label,
            bool editable)
        {
            auto* row = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
            createLabel(ui, row, label, 34.0f);
            constexpr float inputCount = 3.0f;
            constexpr float inputLabelWidth = 4.0f;
            const float vw = (66.0f - inputLabelWidth * inputCount) / inputCount;
            auto* x = createTextInputCell<float>(ui, row, "X", vw, editable, [field](float v) {
                Vector3 next = *field.data;
                next.x = v;
                commitField(field, next);
            });
            auto* y = createTextInputCell<float>(ui, row, "Y", vw, editable, [field](float v) {
                Vector3 next = *field.data;
                next.y = v;
                commitField(field, next);
            });
            auto* z = createTextInputCell<float>(ui, row, "Z", vw, editable, [field](float v) {
                Vector3 next = *field.data;
                next.z = v;
                commitField(field, next);
            });
            return {{x, y, z}, field};
        }

        ColorFieldView createFieldView(
            const LeafField<::Color>& field,
            GameUIEngine* ui,
            Table* fieldTable,
            const std::string& label,
            const bool editable)
        {
            auto* row = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
            createLabel(ui, row, label, 34.0f);
            constexpr float inputCount = 4.0f;
            constexpr float inputLabelWidth = 4.0f;
            constexpr float vw = (66.0f - inputLabelWidth * inputCount) / inputCount;
            const auto clamp = [](const int v) -> unsigned char {
                return static_cast<unsigned char>(std::clamp(v, 0, 255));
            };
            auto* r = createTextInputCell<int>(ui, row, "R", vw, editable, [field, clamp](int v) {
                Color next = *field.data;
                next.r = clamp(v);
                commitField(field, next);
            });
            auto* g = createTextInputCell<int>(ui, row, "G", vw, editable, [field, clamp](int v) {
                Color next = *field.data;
                next.g = clamp(v);
                commitField(field, next);
            });
            auto* b = createTextInputCell<int>(ui, row, "B", vw, editable, [field, clamp](int v) {
                Color next = *field.data;
                next.b = clamp(v);
                commitField(field, next);
            });
            auto* a = createTextInputCell<int>(ui, row, "A", vw, editable, [field, clamp](int v) {
                Color next = *field.data;
                next.a = clamp(v);
                commitField(field, next);
            });
            return {{r, g, b, a}, field};
        }

        EnumFieldView createFieldView(
            const EnumField& src, GameUIEngine* ui, Table* fieldTable, const std::string& label, bool editable)
        {
            auto* row = fieldTable->CreateTableRow(Padding{2, 2, 2, 2});
            createLabel(ui, row, label, 34.0f);
            auto* valueCell = row->CreateTableCell(66.0f, Padding{1, 1, 2, 2});
            auto dropdown = std::make_unique<DropdownList>(
                ui,
                valueCell,
                src.options,
                0,
                EditorInspectorInputFontInfo(),
                VertAlignment::MIDDLE,
                HoriAlignment::LEFT);
            dropdown->SetMaxVisibleOptions(5);
            auto* dd = valueCell->CreateDropdownList(std::move(dropdown));
            if (editable)
            {
                auto setter = src.setIndex;
                dd->onSelectionChanged.Subscribe([setter](const std::size_t idx, const std::string&) {
                    if (setter) setter(idx);
                });
            }
            else
            {
                dd->stateLocked = true;
            }
            return {dd, src.options, src.getIndex};
        }

        // --- Update overloads — refresh widget contents from cached data ----------

        void Update(const BoolFieldView& b)
        {
            if (b.checkbox && b.field.data) b.checkbox->SetChecked(*b.field.data);
        }

        template <class T>
        void Update(const ScalarFieldView<T>& b)
        {
            if (b.text && b.field.data) b.text->SetContent(formatScalar(*b.field.data));
        }

        void Update(const Vec2FieldView& b)
        {
            if (!b.field.data) return;
            if (b.texts[0]) b.texts[0]->SetContent(formatScalar(b.field.data->x));
            if (b.texts[1]) b.texts[1]->SetContent(formatScalar(b.field.data->y));
        }

        void Update(const Vec3FieldView& b)
        {
            if (!b.field.data) return;
            if (b.texts[0]) b.texts[0]->SetContent(formatScalar(b.field.data->x));
            if (b.texts[1]) b.texts[1]->SetContent(formatScalar(b.field.data->y));
            if (b.texts[2]) b.texts[2]->SetContent(formatScalar(b.field.data->z));
        }

        void Update(const ColorFieldView& b)
        {
            if (!b.field.data) return;
            if (b.texts[0]) b.texts[0]->SetContent(std::to_string(static_cast<int>(b.field.data->r)));
            if (b.texts[1]) b.texts[1]->SetContent(std::to_string(static_cast<int>(b.field.data->g)));
            if (b.texts[2]) b.texts[2]->SetContent(std::to_string(static_cast<int>(b.field.data->b)));
            if (b.texts[3]) b.texts[3]->SetContent(std::to_string(static_cast<int>(b.field.data->a)));
        }

        void Update(const EnumFieldView& b)
        {
            if (!b.dropdown) return;
            if (b.dropdown->GetOptions() != b.options) b.dropdown->SetOptions(b.options);
            if (b.getIndex) b.dropdown->SetSelectedIndex(b.getIndex());
        }

        // --- Field lookup (for the row plan diff) ---------------------------------
        const void* FieldDataAddress(const FieldValue& value)
        {
            return std::visit(
                [](const auto& v) -> const void* {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, EnumField>)
                        return v.data;
                    else
                        return v.data;
                },
                value);
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
        std::size_t valueIndex = 0; // matches InspectorField::value.index() when type == Field
        const void* data = nullptr;
        bool editable = false;

        friend bool operator==(const FieldRow&, const FieldRow&) = default;
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
    }

    void InspectorFieldBuilder::Rebuild(const std::vector<InspectedComponent>& inspectedComponents)
    {
        if (!fieldTable || !ui) return;

        rebuildRows(inspectedComponents);
        if (scrollbar) scrollbar->ClampOffset();
        const std::size_t scrollOffset = scrollbar ? scrollbar->ScrollOffset() : 0;

        if (rows == builtRows && scrollOffset == builtScrollOffset) return;
        builtRows = rows;
        builtScrollOffset = scrollOffset;
        fieldViews.clear();
        fieldTable->children.clear();

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

            fieldViews.push_back(
                std::visit(
                    [&](auto&& v) -> FieldView {
                        return createFieldView(v, ui, fieldTable, field->label, field->editable);
                    },
                    field->value));
        }

        fieldTable->InitLayout();
    }

    void InspectorFieldBuilder::Draw() const
    {
        for (const auto& b : fieldViews)
            std::visit([](const auto& x) { Update(x); }, b);
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
                        .valueIndex = field.value.index(),
                        .data = FieldDataAddress(field.value),
                        .editable = field.editable});
            }

            if (rows.size() == headerIndex + 1)
            {
                // No fields under this header — drop it to avoid an empty section.
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
} // namespace sage::editor
