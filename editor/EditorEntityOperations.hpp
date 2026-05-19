#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class EngineSystems;
}

namespace sage::editor
{
    class EditorEntityOperations
    {
        EngineSystems* sys;

        void releaseNavigationOccupation(entt::entity entity) const;

      public:
        explicit EditorEntityOperations(EngineSystems* sys);

        void DeleteEntityAndChildren(entt::entity entity) const;
    };
} // namespace sage::editor
