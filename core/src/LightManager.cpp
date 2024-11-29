

#include "LightManager.hpp"

#include "Camera.hpp"
#include "components/Renderable.hpp"
#include "Light.hpp"

#include <algorithm>

namespace sage
{
    void LightManager::updateShaderLights(Shader& _shader)
    {
        updateAmbientLight(_shader);
        lightsCount = 0;
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
                std::cout << "Scene: Max light sources reached. \n";
                break;
            }
        }

        auto lightsCountLoc = GetShaderLocation(_shader, "lightsCount");
        SetShaderValue(_shader, lightsCountLoc, &lightsCount, SHADER_UNIFORM_INT);
        auto gammaLoc = GetShaderLocation(_shader, "gamma");
        SetShaderValue(_shader, gammaLoc, &gamma, SHADER_UNIFORM_FLOAT);
    }

    void LightManager::updateAmbientLight(Shader& _shader) const
    {
        auto ambientLoc = GetShaderLocation(_shader, "ambient");
        float ambientValue[4] = {ambient[0], ambient[1], ambient[2], ambient[3]};
        SetShaderValue(_shader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);
    }

    void LightManager::RemoveLight(entt::entity light)
    {
        registry->destroy(light);
        RefreshLights();
    }

    entt::entity LightManager::CreateLight(int type, Vector3 position, Vector3 target, Color color)
    {
        if (lightsCount < MAX_LIGHTS)
        {
            auto entity = registry->create();
            auto& light = registry->emplace<Light>(entity);
            light.type = type;
            light.position = position;
            light.target = target;
            light.color = color;
            RefreshLights();
            return entity;
        }
        std::cout << "Scene: Max light sources reached. Ignoring. \n";
        return entt::null;
    }

    void LightManager::LinkShaderToLights(Shader& _shader)
    {
        auto it = std::find_if(shaders.begin(), shaders.end(), [&_shader](const Shader& existingShader) {
            return existingShader.id == _shader.id;
        });

        if (it == shaders.end())
        {
            shaders.push_back(_shader);
        }
        updateAmbientLight(_shader);
        updateShaderLights(_shader);
    }

    void LightManager::SetAmbientLight(float r, float g, float b, float a)
    {
        ambient = {r, g, b, a};
        RefreshLights();
    }

    void LightManager::SetGamma(float g)
    {
        gamma = g;
        RefreshLights();
    }

    void LightManager::RefreshLights()
    {
        for (auto& _shader : shaders)
        {
            updateShaderLights(_shader);
        }
    }

    void LightManager::LinkRenderableToLight(entt::entity entity) const
    {
        auto& renderable = registry->get<Renderable>(entity);
        for (int i = 0; i < renderable.GetModel()->GetMaterialCount(); ++i)
        {
            renderable.GetModel()->SetShader(defaultShader, i);
        }
    }

    void LightManager::DrawDebugLights() const
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

    void LightManager::Update() const
    {
        auto [x, y, z] = camera->GetPosition();
        const float cameraPos[3] = {x, y, z};
        for (auto& shader : shaders)
        {
            SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        }
    }

    LightManager::LightManager(entt::registry* _registry, Camera* _camera) : registry(_registry), camera(_camera)
    {
        defaultShader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/custom/lighting.vs", "resources/shaders/custom/lighting.fs");

        SetAmbientLight(0.6f, 0.2f, 0.8f, 1.0f);
        LinkShaderToLights(defaultShader);
    }
} // namespace sage
