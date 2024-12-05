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
            auto& model = renderable.GetModel()->rlmodel;

            // Find emissive colors
            int colIndex = -1;
            for (int i = 0; i < model.materialCount; ++i)
            {
                //                auto _col = model.materials[i].maps[MATERIAL_MAP_EMISSION].color;
                //                if (_col.r != 0 || _col.g != 0 || _col.b != 0)
                //                {
                //                    colIndex = i;
                //                    break;
                //                }

                auto _col = model.materials[i].maps[MATERIAL_MAP_EMISSION].texture;
                if (_col.id > 1) // has texture
                {
                    colIndex = i;
                    break;
                }
            }

            if (colIndex >= 0)
            {
                auto col = model.materials[colIndex].maps[MATERIAL_MAP_EMISSION].texture;
                SetShaderValue(
                    model.materials[colIndex].shader,
                    model.materials[colIndex].shader.locs[SHADER_LOC_MAP_EMISSION],
                    &col,
                    SHADER_UNIFORM_SAMPLER2D);
            }

            Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};
            renderable.GetModel()->Draw(
                transform.GetWorldPos(),
                rotationAxis,
                transform.GetWorldRot().y,
                transform.GetScale(),
                renderable.hint);
        };

        for (auto entity : normalView)
        {
            auto& r = normalView.get<Renderable>(entity);
            const auto& t = normalView.get<sgTransform>(entity);
            if (registry->any_of<UberShaderComponent>(entity))
            {
                auto& uber = registry->get<UberShaderComponent>(entity);
                uber.SetShaderLocs();
            }
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
