//
// Created by Steve Wheeler on 09/04/2024.
//
#include "LightSubSystem.hpp"

#include "Camera.hpp"
#include "components/Renderable.hpp"

namespace sage
{

    void LightSubSystem::LinkRenderableToLight(entt::entity entity) const
    {
        auto& renderable = registry->get<Renderable>(entity);
        for (int i = 0; i < renderable.GetModel()->GetMaterialCount(); ++i)
        {
            renderable.GetModel()->SetShader(shader, i);
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

    void LightSubSystem::AddLight(Vector3 pos, Color col)
    {
        if (lightCount >= lights.max_size())
        {
            std::cout << "Scene: Max light sources reached. Ignoring. \n";
            return;
        }
        unsigned char g = 0.464 * 255.0;
        unsigned char b = 255.0f * 0.023f;
        col = Color{255, g, b, 1};

        lights[lightCount++] = CreateLight(LIGHT_POINT, pos, {}, col, shader);
    }

    void LightSubSystem::Update() const
    {
        auto [x, y, z] = camera->GetPosition();
        const float cameraPos[3] = {x, y, z};
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
    }

    LightSubSystem::LightSubSystem(entt::registry* _registry, Camera* _camera)
        : registry(_registry), camera(_camera)
    {
        shader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/custom/lighting.vs", "resources/shaders/custom/lighting.fs");
        // Ambient light level (some basic lighting)
        float ambientValue[4] = {0.2f, 0.2f, 0.2f, 0.0f};
        int ambientLoc = GetShaderLocation(shader, "ambient");
        SetShaderValue(shader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);
    }
} // namespace sage
