//
// Created by Steve Wheeler on 09/04/2024.
//
#include "LightSubSystem.hpp"

#include "components/Renderable.hpp"

namespace sage
{

    void LightSubSystem::LinkAllRenderablesToLight()
    {
        registry->view<Renderable>().each(
            [&](auto entity, Renderable& renderable) { LinkRenderableToLight(&renderable); });
    }

    void LightSubSystem::LinkRenderableToLight(Renderable* renderable)
    {
        for (int i = 0; i < renderable->model.materialCount; ++i)
        {
            renderable->model.materials[i].shader = shader;
        }
    }

    void LightSubSystem::DrawDebugLights()
    {
        for (auto& light : lights)
        {
            if (light.enabled)
                DrawSphereEx(light.position, 0.2f, 8, 8, light.color);
            else
                DrawSphereWires(light.position, 0.2f, 8, 8, ColorAlpha(light.color, 0.3f));
        }
    }

    LightSubSystem::LightSubSystem(entt::registry* _registry) : registry(_registry)
    {
        shader = ResourceManager::ShaderLoad(
            TextFormat("resources/shaders/glsl%i/lighting.vs", 330),
            TextFormat("resources/shaders/glsl%i/lighting.fs", 330));
        // Ambient light level (some basic lighting)
        float ambientValue[4] = {0.05f, 0.05f, 0.05f, 1.0f};
        int ambientLoc = GetShaderLocation(shader, "ambient");
        SetShaderValue(shader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);
    }
} // namespace sage
