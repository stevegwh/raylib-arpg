#include "InspectorFieldBlueprint.hpp"

#include "engine/GameUiEngine.hpp"
#include "engine/ui/UIElements.hpp"
#include "engine/ui/UILayout.hpp"

#include <cmath>
#include <exception>
#include <format>
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

        [[nodiscard]] std::optional<InspectorTransformField> TransformFieldFor(
            const std::string& componentName, const std::string& fieldName, const char axis)
        {
            if (componentName != "Transform") return std::nullopt;

            if (fieldName == "Position")
            {
                if (axis == 'X') return InspectorTransformField::PositionX;
                if (axis == 'Y') return InspectorTransformField::PositionY;
                if (axis == 'Z') return InspectorTransformField::PositionZ;
            }
            if (fieldName == "Rotation")
            {
                if (axis == 'X') return InspectorTransformField::RotationX;
                if (axis == 'Y') return InspectorTransformField::RotationY;
                if (axis == 'Z') return InspectorTransformField::RotationZ;
            }
            if (fieldName == "Scale")
            {
                if (axis == 'X') return InspectorTransformField::ScaleX;
                if (axis == 'Y') return InspectorTransformField::ScaleY;
                if (axis == 'Z') return InspectorTransformField::ScaleZ;
            }
            return std::nullopt;
        }
    } // namespace

    struct InspectorFieldBlueprint::FieldBinding
    {
        const FieldRowBlueprint* blueprint = nullptr;
        std::vector<TextBox*> valueTexts;
    };

    class InspectorFieldBlueprint::FieldRowBlueprint
    {
      public:
        FieldRowBlueprint(std::string componentName, std::string fieldName, std::type_index valueType)
            : componentName(std::move(componentName)), fieldName(std::move(fieldName)), valueType(valueType)
        {
        }

        virtual ~FieldRowBlueprint() = default;

        [[nodiscard]] const std::string& ComponentName() const
        {
            return componentName;
        }

        [[nodiscard]] const std::string& FieldName() const
        {
            return fieldName;
        }

        [[nodiscard]] std::type_index ValueType() const
        {
            return valueType;
        }

        virtual void CreateRow(
            GameUIEngine* ui,
            Table* table,
            const Callbacks& callbacks,
            std::vector<FieldBinding>& bindings) const = 0;
        virtual void Draw(const InspectorField& field, const FieldBinding& binding) const = 0;

      private:
        std::string componentName;
        std::string fieldName;
        std::type_index valueType;
    };

    class InspectorFieldBlueprint::Vector3FieldRowBlueprint final : public FieldRowBlueprint
    {
      public:
        Vector3FieldRowBlueprint(std::string componentName, std::string fieldName)
            : FieldRowBlueprint(std::move(componentName), std::move(fieldName), typeid(Vector3))
        {
        }

        void CreateRow(
            GameUIEngine* ui,
            Table* table,
            const Callbacks& callbacks,
            std::vector<FieldBinding>& bindings) const override
        {
            auto* row = table->CreateTableRow(Padding{2, 2, 2, 2});

            auto* labelCell = row->CreateTableCell(34.0f, Padding{1, 1, 2, 4});
            auto label = std::make_unique<TextBox>(
                ui, labelCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
            labelCell->CreateTextbox(std::move(label), FieldName() + ":");

            const auto transformSetter = [this, &callbacks](const char axis) -> std::function<void(float)> {
                const auto transformField = TransformFieldFor(ComponentName(), FieldName(), axis);
                if (!transformField.has_value()) return {};

                return [&callbacks, field = *transformField](const float value) {
                    if (callbacks.setTransform) callbacks.setTransform(field, value);
                };
            };

            const auto addAxis = [ui, row](const char* axis, std::function<void(float)> setter) {
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

            bindings.push_back(FieldBinding{.blueprint = this, .valueTexts = {xText, yText, zText}});
        }

        void Draw(const InspectorField& field, const FieldBinding& binding) const override
        {
            if (!field.value || binding.valueTexts.size() < 3) return;

            const auto& value = *static_cast<const Vector3*>(field.value);
            if (binding.valueTexts[0]) binding.valueTexts[0]->SetContent(FormatFloat(value.x));
            if (binding.valueTexts[1]) binding.valueTexts[1]->SetContent(FormatFloat(value.y));
            if (binding.valueTexts[2]) binding.valueTexts[2]->SetContent(FormatFloat(value.z));
        }
    };

    class InspectorFieldBlueprint::ComponentBlueprint
    {
      public:
        explicit ComponentBlueprint(std::string displayName) : displayName(std::move(displayName))
        {
        }

        void AddField(std::unique_ptr<FieldRowBlueprint> field)
        {
            fields.push_back(std::move(field));
        }

        [[nodiscard]] bool Empty() const
        {
            return fields.empty();
        }

        [[nodiscard]] std::size_t RowCount() const
        {
            return fields.empty() ? 0 : fields.size() + 1;
        }

        void CreateRows(
            GameUIEngine* ui,
            Table* table,
            const Callbacks& callbacks,
            std::size_t& rowCursor,
            std::size_t& visibleCreated,
            const std::size_t scrollOffset,
            const std::size_t visibleRows,
            std::vector<FieldBinding>& bindings) const
        {
            const auto shouldShowRow = [&]() { return rowCursor >= scrollOffset && visibleCreated < visibleRows; };
            const auto advanceRow = [&]() {
                ++rowCursor;
                if (rowCursor > scrollOffset && visibleCreated < visibleRows)
                {
                    ++visibleCreated;
                }
            };

            if (fields.empty()) return;

            if (shouldShowRow())
            {
                auto* headerRow = table->CreateTableRow(Padding{2, 2, 2, 2});
                auto* headerCell = headerRow->CreateTableCell(Padding{2, 2, 2, 2});
                auto header = std::make_unique<TextBox>(
                    ui, headerCell, EditorInspectorFontInfo(), VertAlignment::MIDDLE, HoriAlignment::LEFT);
                headerCell->CreateTextbox(std::move(header), displayName);
            }
            advanceRow();

            for (const auto& field : fields)
            {
                if (shouldShowRow())
                {
                    field->CreateRow(ui, table, callbacks, bindings);
                }
                advanceRow();
            }
        }

      private:
        std::string displayName;
        std::vector<std::unique_ptr<FieldRowBlueprint>> fields;
    };

    InspectorFieldBlueprint::InspectorFieldBlueprint() = default;

    InspectorFieldBlueprint::~InspectorFieldBlueprint() = default;

    void InspectorFieldBlueprint::Attach(GameUIEngine* ui, Table* fieldTable)
    {
        this->ui = ui;
        this->fieldTable = fieldTable;
    }

    void InspectorFieldBlueprint::SetCallbacks(Callbacks callbacks)
    {
        this->callbacks = std::move(callbacks);
    }

    void InspectorFieldBlueprint::SetScrollbarControls(TextBox* upText, TextBox* trackText, TextBox* downText)
    {
        scrollbarUpText = upText;
        scrollbarTrackText = trackText;
        scrollbarDownText = downText;
        updateScrollbarText();
    }

    void InspectorFieldBlueprint::Rebuild(
        const std::vector<InspectedComponent>& inspectedComponents, const Rectangle* mouseWheelBounds)
    {
        if (!fieldTable || !ui) return;

        const auto signature = buildBlueprintSignature(inspectedComponents);
        if (signature != blueprintSignature)
        {
            blueprintSignature = signature;
            rebuildBlueprints(inspectedComponents);
        }

        updateRowMetrics();
        if (mouseWheelBounds && HasOverflow()) scrollFromMouseWheel(*mouseWheelBounds);
        updateScrollbarText();

        const auto newRowSignature = buildRowSignature();
        if (newRowSignature == rowSignature) return;

        rowSignature = newRowSignature;
        bindings.clear();
        fieldTable->children.clear();

        std::size_t rowCursor = 0;
        std::size_t visibleCreated = 0;
        for (const auto& component : components)
        {
            component.CreateRows(
                ui, fieldTable, callbacks, rowCursor, visibleCreated, scrollOffset, visibleRows, bindings);
        }

        fieldTable->InitLayout();
    }

    void InspectorFieldBlueprint::Draw(const std::vector<InspectedComponent>& inspectedComponents) const
    {
        for (const auto& binding : bindings)
        {
            if (!binding.blueprint) continue;

            const auto* field =
                FindField(inspectedComponents, binding.blueprint->ComponentName(), binding.blueprint->FieldName());
            if (!field || field->valueType != binding.blueprint->ValueType()) continue;

            binding.blueprint->Draw(*field, binding);
        }
    }

    void InspectorFieldBlueprint::Scroll(const int amount)
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

    bool InspectorFieldBlueprint::HasOverflow() const
    {
        return totalRows > visibleRows;
    }

    std::size_t InspectorFieldBlueprint::TotalRows() const
    {
        return totalRows;
    }

    std::size_t InspectorFieldBlueprint::VisibleRows() const
    {
        return visibleRows;
    }

    std::size_t InspectorFieldBlueprint::ScrollOffset() const
    {
        return scrollOffset;
    }

    void InspectorFieldBlueprint::rebuildBlueprints(const std::vector<InspectedComponent>& inspectedComponents)
    {
        components.clear();
        rowSignature.clear();

        for (const auto& inspectedComponent : inspectedComponents)
        {
            auto component = ComponentBlueprint{inspectedComponent.displayName};
            for (const auto& field : inspectedComponent.fields)
            {
                if (field.valueType == typeid(Vector3))
                {
                    component.AddField(
                        std::make_unique<Vector3FieldRowBlueprint>(inspectedComponent.displayName, field.label));
                }
            }

            if (!component.Empty()) components.push_back(std::move(component));
        }
    }

    void InspectorFieldBlueprint::updateRowMetrics()
    {
        visibleRows = INSPECTOR_VISIBLE_ROWS;
        totalRows = 0;
        for (const auto& component : components)
        {
            totalRows += component.RowCount();
        }

        const std::size_t maxOffset = totalRows > visibleRows ? totalRows - visibleRows : 0;
        scrollOffset = std::min(scrollOffset, maxOffset);
    }

    void InspectorFieldBlueprint::updateScrollbarText() const
    {
        const bool showScrollbar = HasOverflow();
        if (scrollbarUpText) scrollbarUpText->SetContent(showScrollbar ? "^" : "");
        if (scrollbarTrackText) scrollbarTrackText->SetContent(showScrollbar ? " " : "");
        if (scrollbarDownText) scrollbarDownText->SetContent(showScrollbar ? "v" : "");
    }

    void InspectorFieldBlueprint::scrollFromMouseWheel(const Rectangle& bounds)
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

    std::string InspectorFieldBlueprint::buildBlueprintSignature(
        const std::vector<InspectedComponent>& inspectedComponents) const
    {
        std::ostringstream signature;
        for (const auto& component : inspectedComponents)
        {
            signature << component.displayName << '{';
            for (const auto& field : component.fields)
            {
                if (field.valueType != typeid(Vector3)) continue;
                signature << field.label << ':' << field.valueType.name() << ';';
            }
            signature << '}';
        }
        return signature.str();
    }

    std::string InspectorFieldBlueprint::buildRowSignature() const
    {
        std::ostringstream signature;
        signature << blueprintSignature << "scroll:" << scrollOffset << "rows:" << totalRows;
        return signature.str();
    }
} // namespace sage::editor
