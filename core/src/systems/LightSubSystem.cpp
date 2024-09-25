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

    Light LightSubSystem::CreateLight(int type, Vector3 position, Vector3 target, Color color, Shader shader)
    {
        Light light = {0};

        if (lightsCount < MAX_LIGHTS)
        {
            light.enabled = true;
            light.type = type;
            light.position = position;
            light.target = target;
            light.color = color;

            // NOTE: Lighting shader naming must be the provided ones
            light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
            light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
            light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
            light.targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
            light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));

            UpdateLightValues(shader, light);

            lightsCount++;
        }

        return light;
    }

    void LightSubSystem::UpdateLightValues(Shader shader, Light light)
    {
        // Send to shader light enabled state and type
        SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
        SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);

        // Send to shader light position values
        float position[3] = {light.position.x, light.position.y, light.position.z};
        SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

        // Send to shader light target position values
        float target[3] = {light.target.x, light.target.y, light.target.z};
        SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);

        // Send to shader light color values
        float color[4] = {
            static_cast<float>(light.color.r) / static_cast<float>(255),
            static_cast<float>(light.color.g) / static_cast<float>(255),
            static_cast<float>(light.color.b) / static_cast<float>(255),
            static_cast<float>(light.color.a) / static_cast<float>(255)};
        SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);
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
        if (lightsCount >= lights.max_size())
        {
            std::cout << "Scene: Max light sources reached. Ignoring. \n";
            return;
        }

        unsigned char g = 0.464 * 255.0;
        unsigned char b = 255.0f * 0.023f;
        col = Color{255, g, b, 1};

        lights[lightsCount] = CreateLight(LIGHT_POINT, pos, {}, col, shader);
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
        float ambientValue[4] = {0.6f, 0.3f, 0.8f, 1.0f};
        int ambientLoc = GetShaderLocation(shader, "ambient");
        SetShaderValue(shader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);
    }
} // namespace sage
