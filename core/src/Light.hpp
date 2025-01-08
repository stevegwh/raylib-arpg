//
// Created by Steve Wheeler on 26/09/2024.
//

#pragma once

// #include "raylib-cereal.hpp"
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
        float brightness;
        // TODO: Allow user to set below values
        float constant = 0;
        float linear = 1.0;
        float quadratic = 0.5;

        void LinkShader(const Shader shader, const int lightsCount) const
        {
            // NOTE: Lighting shader naming must be the provided ones
            int enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
            int typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
            int positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
            int targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
            int colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));
            int brightnessLoc = GetShaderLocation(shader, TextFormat("lights[%i].brightness", lightsCount));
            int constantLoc = GetShaderLocation(shader, TextFormat("lights[%i].constant", lightsCount));
            int linearLoc = GetShaderLocation(shader, TextFormat("lights[%i].linear", lightsCount));
            int quadraticLoc = GetShaderLocation(shader, TextFormat("lights[%i].quadratic", lightsCount));

            // UpdateLightValues(shader, *this);
            float _position[3] = {position.x, position.y, position.z};
            float _target[3] = {target.x, target.y, target.z};
            float _color[4] = {
                static_cast<float>(color.r) / static_cast<float>(255),
                static_cast<float>(color.g) / static_cast<float>(255),
                static_cast<float>(color.b) / static_cast<float>(255),
                static_cast<float>(color.a) / static_cast<float>(255)};
            SetShaderValue(shader, enabledLoc, &enabled, SHADER_UNIFORM_INT);
            SetShaderValue(shader, typeLoc, &type, SHADER_UNIFORM_INT);
            SetShaderValue(shader, positionLoc, _position, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, targetLoc, _target, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, colorLoc, _color, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, brightnessLoc, &brightness, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, constantLoc, &constant, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, linearLoc, &linear, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, quadraticLoc, &quadratic, SHADER_UNIFORM_FLOAT);
        }

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(type, position, target, color, brightness);
        }
    };
} // namespace sage