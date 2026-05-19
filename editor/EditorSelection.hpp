#pragma once

#include "entt/entt.hpp"

#include <optional>

namespace sage
{
    class EngineSystems;
}

namespace sage::editor
{
    class EditorSelection
    {
        EngineSystems* sys;
        std::optional<entt::entity> selectedEntity;

      public:
        explicit EditorSelection(EngineSystems* sys);

        bool Select(entt::entity entity);
        void Clear();

        [[nodiscard]] bool HasSelection() const;
        [[nodiscard]] std::optional<entt::entity> Current() const;
        [[nodiscard]] std::optional<entt::entity> ActiveTransformEntity() const;
    };
} // namespace sage::editor
