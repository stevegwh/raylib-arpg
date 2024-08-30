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
        const auto& view = registry->view<Renderable, sgTransform>();
        for (auto entity : view)
        {
            auto& r = registry->get<Renderable>(entity);
            if (!r.active) continue;
            if (r.reqShaderUpdate) r.reqShaderUpdate(entity);
            auto& t = registry->get<sgTransform>(entity);
            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};
            DrawModelEx(
                r.model,
                t.GetWorldPos(),
                rotationAxis,
                t.GetRotation().y,
                {t.GetScale(), t.GetScale(), t.GetScale()},
                r.hint);
        };
    }

    RenderSystem::~RenderSystem()
    {
    }

    RenderSystem::RenderSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
    }
} // namespace sage
