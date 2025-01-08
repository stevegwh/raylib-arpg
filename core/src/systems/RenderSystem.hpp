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
                auto model = renderable.GetModel();
                if (!model) continue;
                const auto& key = StripPath(model->GetKey());
                if (key == meshKey) return entity;
            }
            return entt::null;
        }

        [[nodiscard]] entt::entity FindRenderableByName(const std::string& name) const
        {
            return FindRenderableByName<>(name);
        }

        template <typename... Components>
        [[nodiscard]] entt::entity FindRenderable(const std::string& name) const
        {
            auto entity = FindRenderableByName<Components...>(name);
            if (entity == entt::null)
            {
                entity = FindRenderableByMeshName<Components...>(name);
            }
            return entity;
        }

        [[nodiscard]] entt::entity FindRenderable(const std::string& name) const
        {
            auto entity = FindRenderableByName(name);
            if (entity == entt::null)
            {
                entity = FindRenderableByMeshName(name);
            }
            return entity;
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
                if (renderable.GetName() == nameStripped) return entity;
            }
            return entt::null;
        }
        void Update() override;
        void Draw();
        explicit RenderSystem(entt::registry* _registry);
    };
} // namespace sage
