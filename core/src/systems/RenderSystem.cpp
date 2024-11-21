//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "LightSubSystem.hpp"

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
        auto skinnedView = registry->view<Renderable, sgTransform, RenderableSkinned>();
        auto litView = registry->view<Renderable, sgTransform, RenderableLit>();

        auto renderEntity = [this](auto& renderable, const auto& transform, const entt::entity entity) {
            if (!renderable.active) return;

            if (renderable.reqShaderUpdate) renderable.reqShaderUpdate(entity);

            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};
            // renderable.GetModel()->SetShader(_shader);
            renderable.GetModel()->Draw(
                transform.GetWorldPos(),
                rotationAxis,
                transform.GetWorldRot().y,
                transform.GetScale(),
                renderable.hint);
        };

        BeginShaderMode(lightSubSystem->GetDefaultShader());
        for (auto entity : skinnedView)
        {
            auto& r = skinnedView.get<Renderable>(entity);
            r.GetModel()->SetShader(skinningShader);
            const auto& t = skinnedView.get<sgTransform>(entity);
            renderEntity(r, t, entity);
        }

        for (auto entity : litView)
        {
            auto& r = litView.get<Renderable>(entity);
            const auto& t = litView.get<sgTransform>(entity);
            renderEntity(r, t, entity);
        }
        // EndShaderMode();

        // BeginShaderMode(skinningShader);

        EndShaderMode();

        // for (auto entity : normalView)
        // {
        //     auto& r = normalView.get<Renderable>(entity);
        //     const auto& t = normalView.get<sgTransform>(entity);
        //     renderEntity(r, t, entity);
        // }

        // for (auto entity : deferredView)
        // {
        //     auto& r = deferredView.get<Renderable>(entity);
        //     const auto& t = deferredView.get<sgTransform>(entity);
        //     renderEntity(r, t, entity);
        // }
    }

    RenderSystem::RenderSystem(entt::registry* _registry, LightSubSystem* _lightSubSystem)
        : BaseSystem(_registry), lightSubSystem(_lightSubSystem)
    {
        skinningShader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/glsl330/skinning.vs", "resources/shaders/glsl330/skinning.fs");
    }
} // namespace sage
