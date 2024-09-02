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

    void RenderSystem::Draw() const
    {
        const auto normalView = registry->view<Renderable, sgTransform>(entt::exclude<RenderableDeferred>);
        const auto deferredView = registry->view<Renderable, sgTransform, RenderableDeferred>();

        auto renderEntity = [this](const auto& r, const auto& t, const entt::entity entity) {
            if (!r.active) return;

            if (r.reqShaderUpdate) r.reqShaderUpdate(entity);

            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};
            DrawModelEx(
                r.model,
                t.GetWorldPos(),
                rotationAxis,
                t.GetRotation().y,
                {t.GetScale(), t.GetScale(), t.GetScale()},
                r.hint);
        };

        for (auto entity : normalView)
        {
            const auto& r = normalView.get<Renderable>(entity);
            const auto& t = normalView.get<sgTransform>(entity);
            renderEntity(r, t, entity);
        }

        for (auto entity : deferredView)
        {
            const auto& r = deferredView.get<Renderable>(entity);
            const auto& t = deferredView.get<sgTransform>(entity);
            renderEntity(r, t, entity);
        }
    }

    RenderSystem::~RenderSystem()
    {
    }

    RenderSystem::RenderSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
    }
} // namespace sage
