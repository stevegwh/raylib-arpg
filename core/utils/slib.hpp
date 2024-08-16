//
// Created by Steve Wheeler on 06/07/2024.
//

#pragma once

#include "raylib.h"

#include <string>

namespace sage
{
    struct MaterialPaths
    {
        std::string diffuse{};
        std::string specular{};
        std::string normal{};
        Color emission;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(diffuse, specular, normal, emission);
        };
    };

    Vector2 Vec3ToVec2(const Vector3& vec3);
    Vector3 NegateVector(const Vector3& vec3);
    Vector3 Vector3MultiplyByValue(const Vector3& vec3, float value);
    BoundingBox CalculateModelBoundingBox(Model& model);
    Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer);

} // namespace sage
