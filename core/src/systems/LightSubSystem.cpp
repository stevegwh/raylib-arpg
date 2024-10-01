

#include "LightSubSystem.hpp"

#include "Camera.hpp"
#include "components/Light.hpp"
#include "components/Renderable.hpp"

namespace sage
{
    // Not really used
    Light LightSubSystem::createLight(Shader& _shader, int type, Vector3 position, Vector3 target, Color color)
    {
        Light light = {0};
        auto lightsCountLoc = GetShaderLocation(_shader, "lightsCount");
        if (lightsCount < MAX_LIGHTS)
        {
            light.enabled = true;
            light.type = type;
            light.position = position;
            light.target = target;
            light.color = color;

            light.LinkShader(_shader, lightsCount);

            lightsCount++;
            SetShaderValue(_shader, lightsCountLoc, &lightsCount, SHADER_UNIFORM_INT);
        }

        return light;
    }

    void LightSubSystem::LinkShaderToLights(Shader& _shader)
    {
        shaders.push_back(_shader);
        UpdateAmbientLight(_shader, 0.6f, 0.2f, 0.8f, 1.0f); // TODO
        RefreshLights(_shader);
    }

    void LightSubSystem::LinkRenderableToLight(entt::entity entity) const
    {
        auto& renderable = registry->get<Renderable>(entity);
        for (int i = 0; i < renderable.GetModel()->GetMaterialCount(); ++i)
        {
            renderable.GetModel()->SetShader(defaultShader, i);
        }
    }

    void LightSubSystem::UpdateAmbientLight(Shader& _shader, float r, float g, float b, float a) const
    {
        auto ambientLoc = GetShaderLocation(_shader, "ambient");
        // Ambient light level (some basic lighting)
        float ambientValue[4] = {r, g, b, a};
        SetShaderValue(_shader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);
    }

    void LightSubSystem::RefreshLights(Shader& _shader)
    {
        auto lightsCountLoc = GetShaderLocation(_shader, "lightsCount");
        SetShaderValue(_shader, lightsCountLoc, &lightsCount, SHADER_UNIFORM_INT);
        for (const auto view = registry->view<Light>(); auto& entity : view)
        {
            auto& light = registry->get<Light>(entity);
            if (lightsCount < MAX_LIGHTS)
            {
                light.enabled = true;
                light.LinkShader(_shader, lightsCount);
                lightsCount++;
            }
            else
            {
                std::cout << "Scene: Max light sources reached. Ignoring. \n";
            }
        }
        SetShaderValue(_shader, lightsCountLoc, &lightsCount, SHADER_UNIFORM_INT);
    }

    void LightSubSystem::DrawDebugLights() const
    {
        for (const auto view = registry->view<Light>(); auto& entity : view)
        {
            auto& light = registry->get<Light>(entity);
            if (light.enabled)
                DrawSphereEx(light.position, 0.2f, 8, 8, light.color);
            else
                DrawSphereWires(light.position, 0.2f, 8, 8, ColorAlpha(light.color, 0.3f));
        }
    }

    void LightSubSystem::Update() const
    {
        auto [x, y, z] = camera->GetPosition();
        const float cameraPos[3] = {x, y, z};
        for (auto& shader : shaders)
        {
            SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        }
    }

    LightSubSystem::LightSubSystem(entt::registry* _registry, Camera* _camera)
        : registry(_registry), camera(_camera)
    {
        defaultShader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/custom/lighting.vs", "resources/shaders/custom/lighting.fs");
        LinkShaderToLights(defaultShader);
    }
} // namespace sage
