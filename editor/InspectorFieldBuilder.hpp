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
        // Per-kind widget+data bindings. The variant alternative encodes the field type
        // implicitly; std::visit + overloaded makeBinding/Update functions handle dispatch.
        struct BoolBinding   { Checkbox*                 checkbox = nullptr; bool* data = nullptr; };
        template <class T>
        struct ScalarBinding { TextBox*                  text     = nullptr; T*    data = nullptr; };
        struct Vec2Binding   { std::array<TextBox*, 2>   texts{};            Vector2* data = nullptr; };
        struct Vec3Binding   { std::array<TextBox*, 3>   texts{};            Vector3* data = nullptr; };
        struct ColorBinding  { std::array<TextBox*, 4>   texts{};            ::Color* data = nullptr; };
        struct EnumBinding
        {
            DropdownList* dropdown = nullptr;
            std::vector<std::string> options;
            std::function<std::size_t()> getIndex;
        };

        using BindingV = std::variant<
            BoolBinding,
            ScalarBinding<int>,
            ScalarBinding<unsigned int>,
            ScalarBinding<std::uint64_t>,
            ScalarBinding<float>,
            ScalarBinding<std::string>,
            Vec2Binding,
            Vec3Binding,
            ColorBinding,
            EnumBinding>;

      private:
        GameUIEngine* ui{};
        Table* fieldTable{};
        Scrollbar* scrollbar{};
        std::vector<FieldRow> rows;      // current plan, recomputed every Rebuild()
        std::vector<FieldRow> builtRows; // plan in effect for the live widget tree
        std::size_t builtScrollOffset = 0;
        std::vector<BindingV> bindings;

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
