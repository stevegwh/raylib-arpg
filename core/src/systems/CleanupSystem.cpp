//
// Created by Steve Wheeler on 06/01/2025.
//

#include "CleanupSystem.hpp"

#include "components/DeleteEntityComponent.hpp"

namespace sage
{

    void CleanupSystem::Execute() const
    {
        const auto view = registry->view<DeleteEntityComponent>();

        for (auto entity : view)
        {
            registry->destroy(entity);
        }
    }

    CleanupSystem::CleanupSystem(entt::registry* _registry) : registry(_registry)
    {
    }

} // namespace sage