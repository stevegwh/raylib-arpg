//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "components/Renderable.hpp"
#include "slib.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class RenderSystem : public BaseSystem
    {

      public:
        [[nodiscard]] entt::entity FindRenderableByMeshName(const std::string& name) const
        {
            return FindRenderableByMeshName<>(name);
        }

        // Very inefficient way to find the entity id of a renderable
        template <typename... Components>
        [[nodiscard]] entt::entity FindRenderableByMeshName(const std::string& name) const
        {
            auto meshKey = StripPath(name);
            auto view = registry->view<Renderable, Components...>();

            for (const auto& entity : view)
            {
                const auto& renderable = registry->get<Renderable>(entity);
                const auto& key = StripPath(renderable.GetModel()->GetKey());
                if (key == meshKey) return entity;
            }
            return entt::null;
        }

        [[nodiscard]] entt::entity FindRenderableByName(const std::string& name) const
        {
            return FindRenderableByName<>(name);
        }

        // Very inefficient way to find the entity id of a renderable
        template <typename... Components>
        [[nodiscard]] entt::entity FindRenderableByName(const std::string& name) const
        {
            auto nameStripped = StripPath(name);
            auto view = registry->view<Renderable, Components...>();

            for (const auto& entity : view)
            {
                const auto& renderable = registry->get<Renderable>(entity);
                if (renderable.name == nameStripped) return entity;
            }
            return entt::null;
        }
        void Update() override;
        void Draw();
        explicit RenderSystem(entt::registry* _registry);
    };
} // namespace sage
