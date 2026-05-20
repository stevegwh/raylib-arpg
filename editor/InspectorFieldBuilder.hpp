#pragma once

#include "EditorInspector.hpp"

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <variant>
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

      public:
        // Per-kind live UI views. The variant alternative encodes the field type
        // implicitly; std::visit + overloaded createFieldView/Update functions handle dispatch.
        struct BoolFieldView   { Checkbox*                 checkbox = nullptr; bool* data = nullptr; };
        template <class T>
        struct ScalarFieldView { TextBox*                  text     = nullptr; T*    data = nullptr; };
        struct Vec2FieldView   { std::array<TextBox*, 2>   texts{};            Vector2* data = nullptr; };
        struct Vec3FieldView   { std::array<TextBox*, 3>   texts{};            Vector3* data = nullptr; };
        struct ColorFieldView  { std::array<TextBox*, 4>   texts{};            ::Color* data = nullptr; };
        struct EnumFieldView
        {
            DropdownList* dropdown = nullptr;
            std::vector<std::string> options;
            std::function<std::size_t()> getIndex;
        };

        using FieldView = std::variant<
            BoolFieldView,
            ScalarFieldView<int>,
            ScalarFieldView<unsigned int>,
            ScalarFieldView<std::uint64_t>,
            ScalarFieldView<float>,
            ScalarFieldView<std::string>,
            Vec2FieldView,
            Vec3FieldView,
            ColorFieldView,
            EnumFieldView>;

      private:
        GameUIEngine* ui{};
        Table* fieldTable{};
        Scrollbar* scrollbar{};
        std::vector<FieldRow> rows;      // current plan, recomputed every Rebuild()
        std::vector<FieldRow> builtRows; // plan in effect for the live widget tree
        std::size_t builtScrollOffset = 0;
        std::vector<FieldView> fieldViews;

        void rebuildRows(const std::vector<InspectedComponent>& inspectedComponents);
        void createHeaderRow(const std::string& label) const;

      public:
        InspectorFieldBuilder();
        ~InspectorFieldBuilder();

        void Attach(GameUIEngine* ui, Table* fieldTable);
        void AttachScrollbar(Scrollbar* sb);
        void Rebuild(const std::vector<InspectedComponent>& inspectedComponents);
        void Draw() const;

        [[nodiscard]] std::size_t TotalRows() const;
        [[nodiscard]] std::size_t VisibleRows() const;
    };
} // namespace sage::editor
