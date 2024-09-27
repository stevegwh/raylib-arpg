

#include "LightSubSystem.hpp"

#include "Camera.hpp"
#include "components/Light.hpp"
#include "components/Renderable.hpp"

namespace sage
{

    Light LightSubSystem::createLight(int type, Vector3 position, Vector3 target, Color color, Shader shader)
    {
        Light light = {0};

        if (lightsCount < MAX_LIGHTS)
        {
            light.enabled = true;
            light.type = type;
            light.position = position;
            light.target = target;
            light.color = color;

            light.SetShader(shader, lightsCount);

            lightsCount++;
            SetShaderValue(shader, lightsCountLoc, &lightsCount, SHADER_UNIFORM_INT);
        }

        return light;
    }

    void LightSubSystem::LinkRenderableToLight(entt::entity entity) const
    {
        auto& renderable = registry->get<Renderable>(entity);
        for (int i = 0; i < renderable.GetModel()->GetMaterialCount(); ++i)
        {
            renderable.GetModel()->SetShader(shader, i);
        }
    }

    void LightSubSystem::UpdateAmbientLight(float r, float g, float b, float a) const
    {
        // Ambient light level (some basic lighting)
        float ambientValue[4] = {r, g, b, a};
        SetShaderValue(shader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);
    }

    void LightSubSystem::RefreshLights()
    {
        for (const auto view = registry->view<Light>(); auto& entity : view)
        {
            auto& light = registry->get<Light>(entity);
            if (lightsCount < MAX_LIGHTS)
            {
                light.enabled = true;
                light.SetShader(shader, lightsCount);
                lightsCount++;
            }
            else
            {
                std::cout << "Scene: Max light sources reached. Ignoring. \n";
            }
        }
        SetShaderValue(shader, lightsCountLoc, &lightsCount, SHADER_UNIFORM_INT);
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
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
    }

    LightSubSystem::LightSubSystem(entt::registry* _registry, Camera* _camera)
        : registry(_registry), camera(_camera)
    {
        shader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/custom/lighting.vs", "resources/shaders/custom/lighting.fs");
        ambientLoc = GetShaderLocation(shader, "ambient");
        UpdateAmbientLight(0.4f, 0.1f, 0.6f, 1.0f);
        lightsCountLoc = GetShaderLocation(shader, "lightsCount");
        SetShaderValue(shader, lightsCountLoc, &lightsCount, SHADER_UNIFORM_INT);
        RefreshLights();
    }
} // namespace sage
