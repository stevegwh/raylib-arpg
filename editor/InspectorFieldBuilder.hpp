#pragma once

#include "EditorInspector.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace sage
{
    class Checkbox;
    class DropdownList;
    class GameUIEngine;
    class Scrollbar;
    class Table;
    class TextBox;
} // namespace sage

namespace sage::editor
{
    class InspectorFieldBuilder
    {
        struct FieldRow;
        struct FieldBinding;

        GameUIEngine* ui{};
        Table* fieldTable{};
        Scrollbar* scrollbar{};
        std::vector<FieldRow> rows;       // current plan, recomputed every Rebuild()
        std::vector<FieldRow> builtRows;  // plan in effect for the live widget tree
        std::size_t builtScrollOffset = 0;
        std::vector<FieldBinding> bindings;

        void rebuildRows(const std::vector<InspectedComponent>& inspectedComponents);
        void createHeaderRow(const std::string& label) const;
        void createFieldRow(FieldBinding& binding, const InspectorField& field);
        void createBoolRow(FieldBinding& binding, const InspectorField& field);
        void createScalarRow(FieldBinding& binding, const InspectorField& field);
        void createComponentRow(FieldBinding& binding, const InspectorField& field);
        void createEnumRow(FieldBinding& binding, const InspectorField& field);

      public:
        InspectorFieldBuilder();
        ~InspectorFieldBuilder();

        void Attach(GameUIEngine* ui, Table* fieldTable);
        void AttachScrollbar(Scrollbar* sb);
        void Rebuild(const std::vector<InspectedComponent>& inspectedComponents);
        void Draw(const std::vector<InspectedComponent>& inspectedComponents) const;

        [[nodiscard]] std::size_t TotalRows() const;
        [[nodiscard]] std::size_t VisibleRows() const;
    };
} // namespace sage::editor
