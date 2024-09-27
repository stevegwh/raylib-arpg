//
// Created by Steve Wheeler on 26/09/2024.
//

#pragma once

#include "raylib-cereal.hpp"
#include "raylib.h"

namespace sage
{
    struct Light
    {
        int type;
        bool enabled;
        Vector3 position;
        Vector3 target;
        Color color;
        float attenuation;

        // Shader locations
        int enabledLoc;
        int typeLoc;
        int positionLoc;
        int targetLoc;
        int colorLoc;
        int attenuationLoc;

        void SetShader(const Shader shader, const int lightsCount)
        {
            // NOTE: Lighting shader naming must be the provided ones
            enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
            typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
            positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
            targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
            colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));
            attenuationLoc = GetShaderLocation(shader, TextFormat("lights[%i].attenuation", lightsCount));
            UpdateLightValues(shader, *this);
        }

        static void UpdateLightValues(Shader shader, Light light)
        {
            float position[3] = {light.position.x, light.position.y, light.position.z};
            float target[3] = {light.target.x, light.target.y, light.target.z};
            float color[4] = {
                static_cast<float>(light.color.r) / static_cast<float>(255),
                static_cast<float>(light.color.g) / static_cast<float>(255),
                static_cast<float>(light.color.b) / static_cast<float>(255),
                static_cast<float>(light.color.a) / static_cast<float>(255)};
            SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
            SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);
            SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, light.attenuationLoc, &light.attenuation, SHADER_UNIFORM_FLOAT);
        }

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(type, position, target, color);
        }
    };
} // namespace sage