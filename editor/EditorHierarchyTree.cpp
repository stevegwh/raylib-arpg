#include "EditorHierarchyTree.hpp"

#include "EditorComponents.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/Light.hpp"

#include <algorithm>
#include <format>

namespace sage::editor
{
    EditorHierarchyTree::EditorHierarchyTree(EngineSystems* _sys) : sys(_sys)
    {
    }

    std::string EditorHierarchyTree::DescribeEntity(const entt::entity entity) const
    {
        if (sys->registry->valid(entity) && sys->registry->any_of<EditorObjectDescriptor>(entity))
        {
            const auto& descriptor = sys->registry->get<EditorObjectDescriptor>(entity);
            if (!descriptor.name.empty()) return descriptor.name;
        }
        if (sys->registry->valid(entity) && sys->registry->any_of<Light>(entity))
        {
            return std::format("light_{}", entt::to_integral(entity));
        }
        return std::format("entity_{}", entt::to_integral(entity));
    }

    std::vector<EditorGui::SceneObjectEntry> EditorHierarchyTree::CollectSceneObjectEntries() const
    {
        std::vector<entt::entity> roots;
        auto view = sys->registry->view<sgTransform>();
        for (const auto entity : view)
        {
            const auto parent = view.get<sgTransform>(entity).GetParent();
            if (parent == entt::null || !sys->registry->valid(parent) ||
                !sys->registry->any_of<sgTransform>(parent))
            {
                roots.push_back(entity);
            }
        }

        std::ranges::sort(roots, [](const entt::entity lhs, const entt::entity rhs) {
            return entt::to_integral(lhs) < entt::to_integral(rhs);
        });

        std::vector<EditorGui::SceneObjectEntry> entries;
        entries.reserve(roots.size());
        for (const auto root : roots)
        {
            appendSceneObjectEntry(entries, root, 0);
        }
        return entries;
    }

    void EditorHierarchyTree::appendSceneObjectEntry(
        std::vector<EditorGui::SceneObjectEntry>& entries, const entt::entity entity, const int depth) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        entries.push_back({.entity = entity, .displayName = DescribeEntity(entity), .depth = depth});

        auto children = sys->registry->get<sgTransform>(entity).GetChildren();
        std::ranges::sort(children, [](const entt::entity lhs, const entt::entity rhs) {
            return entt::to_integral(lhs) < entt::to_integral(rhs);
        });

        for (const auto child : children)
        {
            appendSceneObjectEntry(entries, child, depth + 1);
        }
    }
} // namespace sage::editor
