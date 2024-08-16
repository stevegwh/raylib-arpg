//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "FountainEffect.hpp"

#include "raylib.h"

#include <memory>
#include <vector>

namespace sage
{

    class SpiralFountainVFX : public VisualFX
    {
        Shader shader{};

        Vector3 spiralCentre{};
        float minSpiralRadius = 2.0f;         // Minimum radius of the spiral
        float maxSpiralRadius = 5.0f;         // Maximum radius of the spiral
        float spiralRadius = minSpiralRadius; // Current radius of the spiral
        float spiralSpeed = 10.0f;            // Speed of rotation (in radians per second)
        float spiralAngle = 0.0f;             // Current angle of rotation
        float spiralGrowth = 0.5f;            // How much the radius grows per second
        std::unique_ptr<FountainEffect> fountain;

      public:
        bool active = false;
        ~SpiralFountainVFX();
        explicit SpiralFountainVFX(Camera3D* camera);
        void InitSystem(const Vector3& _target) override;
        void Update(float dt) override;
        void Draw3D() const override;
        void SetOrigin(const Vector3& origin) override;
    };
} // namespace sage