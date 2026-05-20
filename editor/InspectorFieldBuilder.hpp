#pragma once

#include "EditorInspector.hpp"

#include "engine/Event.hpp"
#include "raylib.h"

#include <string>
#include <vector>

namespace sage
{
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
        Subscription scrollSub{};
        bool pendingRebuild = false;
        std::string blueprintSignature;
        std::string rowSignature;
        std::vector<FieldRow> rows;
        std::vector<FieldBinding> bindings;

        void rebuildRows(const std::vector<InspectedComponent>& inspectedComponents);
        void createHeaderRow(const std::string& label) const;
        void executeFieldRowBuilder(const FieldRow& row);
        void createBoolRow(const FieldRow& row);
        void createScalarRow(const FieldRow& row);
        void createComponentRow(const FieldRow& row);
        void createEnumRow(const FieldRow& row);
        void setBoolValue(std::size_t bindingIndex, bool submittedValue);
        void setScalarValue(std::size_t bindingIndex, const std::string& submittedValue);
        void setComponentValue(
            std::size_t bindingIndex, std::size_t componentIndex, const std::string& submittedValue);
        void setEnumValue(std::size_t bindingIndex, std::size_t optionIndex);
        [[nodiscard]] static std::string buildBlueprintSignature(
            const std::vector<InspectedComponent>& inspectedComponents);
        [[nodiscard]] std::string buildRowSignature() const;

      public:
        InspectorFieldBuilder();
        ~InspectorFieldBuilder();

        void Attach(GameUIEngine* ui, Table* fieldTable);
        void AttachScrollbar(Scrollbar* sb);
        void Rebuild(const std::vector<InspectedComponent>& inspectedComponents);
        void Draw(const std::vector<InspectedComponent>& inspectedComponents);

        [[nodiscard]] std::size_t TotalRows() const;
        [[nodiscard]] std::size_t VisibleRows() const;
    };
} // namespace sage::editor
