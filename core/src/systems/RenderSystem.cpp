//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

#include "components/UberShaderComponent.hpp"
#include "raylib.h"

namespace sage
{
    std::string stripPath(const std::string& fullPath)
    {
        size_t lastSlash = fullPath.find_last_of("/\\");

        size_t lastDot = fullPath.find_last_of('.');

        size_t startPos = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
        size_t length = (lastDot == std::string::npos) ? std::string::npos : lastDot - startPos;

        return fullPath.substr(startPos, length);
    }

    // Very inefficient way to find the entity id of a renderable
    entt::entity RenderSystem::FindRenderableByMeshName(const std::string& name) const
    {
        auto meshKey = stripPath(name);
        auto view = registry->view<Renderable>();

        for (const auto& entity : view)
        {
            const auto& renderable = registry->get<Renderable>(entity);
            const auto& key = stripPath(renderable.GetModel()->GetKey());
            if (key == meshKey) return entity;
        }
        return entt::null;
    }

    // Very inefficient way to find the entity id of a renderable
    entt::entity RenderSystem::FindRenderableByName(const std::string& name) const
    {
        auto view = registry->view<Renderable>();

        for (const auto& entity : view)
        {
            const auto& renderable = registry->get<Renderable>(entity);
            if (renderable.name == name) return entity;
        }
        return entt::null;
    }

    void RenderSystem::Update()
    {
    }

    void RenderSystem::Draw() // Can't be const as GetModel returns pointers
    {
        auto normalView =
            registry->view<Renderable, sgTransform>(entt::exclude<RenderableDeferred, UberShaderComponent>);
        auto deferredView =
            registry->view<Renderable, sgTransform, RenderableDeferred>(entt::exclude<UberShaderComponent>);
        auto uberView = registry->view<Renderable, sgTransform, UberShaderComponent>();

        auto renderEntity = [this](auto& renderable, const auto& transform, const entt::entity entity) {
            if (!renderable.active) return;

            if (renderable.reqShaderUpdate) renderable.reqShaderUpdate(entity);
            auto& model = renderable.GetModel()->rlmodel;

            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};

            renderable.GetModel()->Draw(
                transform.GetWorldPos(),
                rotationAxis,
                transform.GetWorldRot().y,
                transform.GetScale(),
                renderable.hint);
        };

        // TODO: Unsure if having three separate views will cause issues or not.

        for (auto entity : normalView)
        {
            auto& r = normalView.get<Renderable>(entity);
            const auto& t = normalView.get<sgTransform>(entity);
            renderEntity(r, t, entity);
        }

        for (auto entity : uberView)
        {
            auto& renderable = normalView.get<Renderable>(entity);
            if (!renderable.active) continue;

            const auto& transform = normalView.get<sgTransform>(entity);
            auto& uber = registry->get<UberShaderComponent>(entity);
            if (renderable.reqShaderUpdate) renderable.reqShaderUpdate(entity);

            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};

            renderable.GetModel()->DrawUber(
                &uber,
                transform.GetWorldPos(),
                rotationAxis,
                transform.GetWorldRot().y,
                transform.GetScale(),
                renderable.hint);
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
