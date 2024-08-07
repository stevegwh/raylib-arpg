//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"

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
        view.each([](const auto& r, const auto& t) {
            if (!r.active) return;
            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};
            DrawModelEx(
                r.model,
                t.position(),
                rotationAxis,
                t.rotation().y,
                {t.scale(), t.scale(), t.scale()},
                r.hint);
        });
    }

    RenderSystem::~RenderSystem()
    {
    }

    RenderSystem::RenderSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
    }
} // namespace sage
