#pragma once

#include "EditorInspector.hpp"

#include "raylib.h"

#include <string>
#include <vector>

namespace sage
{
    class GameUIEngine;
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
        TextBox* scrollbarUpText{};
        TextBox* scrollbarTrackText{};
        TextBox* scrollbarDownText{};
        std::size_t scrollOffset = 0;
        std::size_t visibleRows = 0;
        std::size_t totalRows = 0;
        std::string blueprintSignature;
        std::string rowSignature;
        std::vector<FieldRow> rows;
        std::vector<FieldBinding> bindings;

        void rebuildRows(const std::vector<InspectedComponent>& inspectedComponents);
        void createHeaderRow(const std::string& label) const;
        void createFieldRow(const FieldRow& row);
        void createVector3Row(const FieldRow& row);
        void updateRowMetrics();
        void updateScrollbarText() const;
        void scrollFromMouseWheel(const Rectangle& bounds);
        void setVector3Axis(std::size_t bindingIndex, std::size_t axisIndex, const std::string& submittedValue);
        static void RenderUI(Vector3& value, const FieldBinding& binding);
        static void RenderUI(const Vector3& value, const FieldBinding& binding);
        [[nodiscard]] static std::string buildBlueprintSignature(
            const std::vector<InspectedComponent>& inspectedComponents);
        [[nodiscard]] std::string buildRowSignature() const;

      public:
        InspectorFieldBuilder();
        ~InspectorFieldBuilder();

        void Attach(GameUIEngine* ui, Table* fieldTable);
        void SetScrollbarControls(TextBox* upText, TextBox* trackText, TextBox* downText);
        void Rebuild(
            const std::vector<InspectedComponent>& inspectedComponents, const Rectangle* mouseWheelBounds);
        void Draw(const std::vector<InspectedComponent>& inspectedComponents);
        void Scroll(int amount);

        [[nodiscard]] bool HasOverflow() const;
        [[nodiscard]] std::size_t TotalRows() const;
        [[nodiscard]] std::size_t VisibleRows() const;
        [[nodiscard]] std::size_t ScrollOffset() const;
    };
} // namespace sage::editor
