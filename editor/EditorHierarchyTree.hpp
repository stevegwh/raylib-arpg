#pragma once

#include "EditorGui.hpp"

#include "entt/entt.hpp"

#include <string>
#include <vector>

namespace sage
{
    class EngineSystems;
}

namespace sage::editor
{
    class EditorHierarchyTree
    {
        EngineSystems* sys;

        void appendSceneObjectEntry(
            std::vector<EditorGui::SceneObjectEntry>& entries,
            entt::entity entity,
            int depth) const;

      public:
        explicit EditorHierarchyTree(EngineSystems* sys);

        [[nodiscard]] std::string DescribeEntity(entt::entity entity) const;
        [[nodiscard]] std::vector<EditorGui::SceneObjectEntry> CollectSceneObjectEntries() const;
    };
} // namespace sage::editor
