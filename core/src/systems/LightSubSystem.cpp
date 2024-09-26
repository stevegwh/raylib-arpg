/**********************************************************************************************
 *
 *   raylib.lights - Some useful functions to deal with lights data
 *
 *   CONFIGURATION:
 *
 *   #define RLIGHTS_IMPLEMENTATION
 *       Generates the implementation of the library into the included file.
 *       If not defined, the library is in header only mode and can be included in other headers
 *       or source files without problems. But only ONE file should hold the implementation.
 *
 *   LICENSE: zlib/libpng
 *
 *   Copyright (c) 2017-2023 Victor Fisac (@victorfisac) and Ramon Santamaria (@raysan5)
 *
 *   This software is provided "as-is", without any express or implied warranty. In no event
 *   will the authors be held liable for any damages arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose, including commercial
 *   applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not claim that you
 *     wrote the original software. If you use this software in a product, an acknowledgment
 *     in the product documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *     as being the original software.
 *
 *     3. This notice may not be removed or altered from any source distribution.
 *
 **********************************************************************************************/

#include "LightSubSystem.hpp"

#include "Camera.hpp"
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

    void LightSubSystem::DrawDebugLights() const
    {
        // for (auto& light : lights)
        // {
        //     if (light.enabled)
        //         DrawSphereEx(light.position, 0.2f, 8, 8, light.color);
        //     else
        //         DrawSphereWires(light.position, 0.2f, 8, 8, ColorAlpha(light.color, 0.3f));
        // }
    }

    void LightSubSystem::AddLight(Vector3 pos, Color col)
    {
        if (lightsCount >= MAX_LIGHTS)
        {
            std::cout << "Scene: Max light sources reached. Ignoring. \n";
            return;
        }

        auto light = createLight(LIGHT_POINT, pos, {}, col, shader);
        auto entity = registry->create();
        registry->emplace<Light>(entity, light);
    }

    void LightSubSystem::Update() const
    {
        auto [x, y, z] = camera->GetPosition();
        const float cameraPos[3] = {x, y, z};
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
    }

    void LightSubSystem::LinkAllLights()
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
                return;
            }
        }
    }

    LightSubSystem::LightSubSystem(entt::registry* _registry, Camera* _camera)
        : registry(_registry), camera(_camera)
    {
        shader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/custom/lighting.vs", "resources/shaders/custom/lighting.fs");
        // Ambient light level (some basic lighting)
        float ambientValue[4] = {0.4f, 0.1f, 0.6f, 1.0f};
        int ambientLoc = GetShaderLocation(shader, "ambient");
        SetShaderValue(shader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);

        LinkAllLights();
    }
} // namespace sage
