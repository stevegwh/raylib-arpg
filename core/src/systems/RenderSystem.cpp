//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

#include "raylib.h"

namespace sage
{
    void RenderSystem::Update()
    {
    }

    void RenderSystem::Draw() // Can't be const as GetModel returns pointers
    {
        auto normalView = registry->view<Renderable, sgTransform>(entt::exclude<RenderableDeferred>);
        auto deferredView = registry->view<Renderable, sgTransform, RenderableDeferred>();

        auto renderEntity = [this](auto& renderable, const auto& transform, const entt::entity entity) {
            if (!renderable.active) return;

            if (renderable.reqShaderUpdate) renderable.reqShaderUpdate(entity);

            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};
            DrawModelEx(
                renderable.GetModel(),
                transform.GetWorldPos(),
                rotationAxis,
                transform.GetRotation().y,
                {transform.GetScale(), transform.GetScale(), transform.GetScale()},
                renderable.hint);
        };

        for (auto entity : normalView)
        {
            auto& r = normalView.get<Renderable>(entity);
            const auto& t = normalView.get<sgTransform>(entity);
            renderEntity(r, t, entity);
        }

        for (auto entity : deferredView)
        {
            auto& r = deferredView.get<Renderable>(entity);
            const auto& t = deferredView.get<sgTransform>(entity);
            renderEntity(r, t, entity);
        }
    }

    RenderSystem::RenderSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
    }
} // namespace sage
