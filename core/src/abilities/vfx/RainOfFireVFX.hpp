//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "raylib.h"

#include <memory>
#include <vector>

namespace sage
{
    struct Fireball;

    class RainOfFireVFX : public VisualFX
    {
        Shader shader{};
        Vector3 target{};
        Vector3 baseSpawnPoint{};
        float initialHeight{};
        float minHeight{};
        float impactRadius{};
        const float initialOffset = 1.0f; // Initial diagonal offset from the target
        std::vector<std::unique_ptr<Fireball>> fireballs;
        void generateFireball(Fireball& fireball);

      public:
        void InitSystem(const Vector3& _target) override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~RainOfFireVFX();
        explicit RainOfFireVFX(Camera* _camera);
    };
} // namespace sage