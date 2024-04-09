//
// Created by Steve Wheeler on 09/04/2024.
//

#include "LightSubSystem.hpp"
#include "Renderable.hpp"

namespace sage
{

LightSubSystem::LightSubSystem()
{
    shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", 330),
                        TextFormat("resources/shaders/glsl%i/lighting.fs", 330));
    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);
}

void LightSubSystem::LinkRenderableToLight(sage::Renderable* renderable)
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
        if (light.enabled) DrawSphereEx(light.position, 0.2f, 8, 8, light.color);
        else DrawSphereWires(light.position, 0.2f, 8, 8, ColorAlpha(light.color, 0.3f));
    }
}

}


