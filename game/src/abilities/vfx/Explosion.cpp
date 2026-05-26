//
// Created by Steve Wheeler on 03/08/2024.
//

#include "Explosion.hpp"

#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/systems/TransformSystem.hpp"

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
        registry->get<sage::sgTransform>(entity).scale.world = {scale, scale, scale};
        if (scale >= maxScale)
        {
            auto& r = registry->get<sage::Renderable>(entity);
            r.active = false;
        }
    }

    void Explosion::SetOrigin(Vector3 origin)
    {
        registry->get<sage::sgTransform>(entity).position.world = origin;
    }

    Explosion::Explosion(entt::registry* _registry, sage::EngineSystems* _sys) : sys(_sys)
    {
        registry = _registry;
        auto sphere = sage::ResourceManager::GetInstance().CreateModelMutable("primitive_hemisphere");
        sphere.SetShader(
            sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"), 0);

        entity = registry->create();
        registry->emplace<sage::sgTransform>(entity);
        auto& renderable =
            registry->emplace<sage::Renderable>(entity, std::move(sphere), MatrixIdentity());
        renderable.hint = Color{255, 0, 0, 100};
    }
} // namespace lq
