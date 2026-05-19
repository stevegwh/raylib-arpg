#pragma once

#include "EditorInspector.hpp"

#include "raylib.h"

#include <functional>
#include <memory>
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
    enum class InspectorTransformField
    {
        PositionX,
        PositionY,
        PositionZ,
        RotationX,
        RotationY,
        RotationZ,
        ScaleX,
        ScaleY,
        ScaleZ
    };

    class InspectorFieldBlueprint
    {
      public:
        struct Callbacks
        {
            std::function<void(InspectorTransformField, float)> setTransform;
        };

        InspectorFieldBlueprint();
        ~InspectorFieldBlueprint();

        void Attach(GameUIEngine* ui, Table* fieldTable);
        void SetCallbacks(Callbacks callbacks);
        void SetScrollbarControls(TextBox* upText, TextBox* trackText, TextBox* downText);
        void Rebuild(
            const std::vector<InspectedComponent>& inspectedComponents, const Rectangle* mouseWheelBounds);
        void Draw(const std::vector<InspectedComponent>& inspectedComponents) const;
        void Scroll(int amount);

        [[nodiscard]] bool HasOverflow() const;
        [[nodiscard]] std::size_t TotalRows() const;
        [[nodiscard]] std::size_t VisibleRows() const;
        [[nodiscard]] std::size_t ScrollOffset() const;

      private:
        struct FieldBinding;
        class FieldRowBlueprint;
        class Vector3FieldRowBlueprint;
        class ComponentBlueprint;

        GameUIEngine* ui{};
        Table* fieldTable{};
        TextBox* scrollbarUpText{};
        TextBox* scrollbarTrackText{};
        TextBox* scrollbarDownText{};
        Callbacks callbacks;
        std::size_t scrollOffset = 0;
        std::size_t visibleRows = 0;
        std::size_t totalRows = 0;
        std::string blueprintSignature;
        std::string rowSignature;
        std::vector<ComponentBlueprint> components;
        std::vector<FieldBinding> bindings;

        void rebuildBlueprints(const std::vector<InspectedComponent>& inspectedComponents);
        void updateRowMetrics();
        void updateScrollbarText() const;
        void scrollFromMouseWheel(const Rectangle& bounds);
        [[nodiscard]] std::string buildBlueprintSignature(
            const std::vector<InspectedComponent>& inspectedComponents) const;
        [[nodiscard]] std::string buildRowSignature() const;
    };
} // namespace sage::editor
