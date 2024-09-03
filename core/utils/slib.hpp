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

    /**
     * Defines a memory safe wrapper for raylib model. Does not safeguard against access to underlying pointers.
     */
    class ModelSafe
    {
        Model model{};

      public:
        Model& rlModel();
        // Shader GetShader();
        // void SetShader(const char* path);
        // void SetModel(Model& _model);
        ModelSafe(const ModelSafe&) = delete;
        ModelSafe& operator=(const ModelSafe&) = delete;
        ModelSafe(ModelSafe&& other) noexcept;
        ModelSafe& operator=(ModelSafe&& other) noexcept;
        ~ModelSafe();
        explicit ModelSafe(const char* path);
        explicit ModelSafe(Model& _model);
        ModelSafe() = default;
    };

    Vector2 Vec3ToVec2(const Vector3& vec3);
    Vector3 NegateVector(const Vector3& vec3);
    Vector3 Vector3MultiplyByValue(const Vector3& vec3, float value);
    BoundingBox CalculateModelBoundingBox(const Model& model);
    Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer);
} // namespace sage
