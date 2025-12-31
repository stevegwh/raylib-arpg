//
// Created by Steve Wheeler on 03/08/2024.
//

#include "Explosion.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"

namespace lq
{
    void Explosion::Restart()
    {
        scale = 0;
        auto& r = registry->get<sage::Renderable>(entity);
        r.active = true;
    }

    void Explosion::Update()
    {
        if (scale >= maxScale) return;
        scale += increment * GetFrameTime();
        auto& transform = registry->get<sage::sgTransform>(entity);
        transform.SetScale(scale);
        if (scale >= maxScale)
        {
            auto& r = registry->get<sage::Renderable>(entity);
            r.active = false;
        }
    }

    void Explosion::SetOrigin(Vector3 origin)
    {
        auto& transform = registry->get<sage::sgTransform>(entity);
        transform.SetPosition(origin);
    }

    Explosion::Explosion(entt::registry* _registry)
    {
        registry = _registry;
        auto sphere = LoadModelFromMesh(GenMeshHemiSphere(1.0f, 16, 16));
        entity = registry->create();
        registry->emplace<sage::sgTransform>(entity, entity);
        auto& renderable = registry->emplace<sage::Renderable>(entity, sage::ModelSafe(sphere), MatrixIdentity());
        renderable.hint = Color{255, 0, 0, 100};
        renderable.GetModel()->SetShader(
            sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"), 0);
    }
} // namespace sage