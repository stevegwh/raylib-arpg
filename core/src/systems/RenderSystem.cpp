//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"
#include "raylib.h"
#include "../Application.hpp"

#include "../components/Transform.hpp"

namespace sage
{
RenderSystem::~RenderSystem()
{
}

void RenderSystem::Update()
{
}

void RenderSystem::Draw() const
{
    const auto& view = registry->view<Renderable, Transform>();
    view.each([](const auto& r, const auto& t) 
    {
        Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
        DrawModelEx(r.model, t.position, rotationAxis, t.rotation.y, {t.scale, t.scale, t.scale}, WHITE);
    });
}

RenderSystem::RenderSystem(entt::registry* _registry) : BaseSystem<Renderable>(_registry) {}

}

