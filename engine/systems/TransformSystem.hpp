#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class TransformSystem
    {
        entt::registry* registry;

        void propagate(entt::entity entity, bool ancestorDirty);
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        void Update();

        explicit TransformSystem(entt::registry* _registry);
    };
} // namespace sage
