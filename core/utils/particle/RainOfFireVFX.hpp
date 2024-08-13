//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "FlameEffect.hpp"

#include "raylib.h"

#include <memory>
#include <vector>

namespace sage
{
    struct Fireball
    {
        Vector3 position;
        Vector3 velocity;
        float radius;
        std::unique_ptr<FlameEffect> flameEffect;
    };

    class RainOfFireVFX : public VisualFX
    {
        Shader shader{};
        Vector3 target{};
        Vector3 baseSpawnPoint{};
        float initialHeight{};
        float minHeight{};
        float impactRadius{};
        const float initialOffset = 1.0f; // Initial diagonal offset from the target
        const float spawnAreaRadius =
            2.0f; // Radius around the spawn point to vary starting positions
        std::vector<std::unique_ptr<Fireball>> fireballs;
        void generateFireball(Fireball& fireball);

      public:
        void InitSystem(const Vector3& _target) override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~RainOfFireVFX();
        explicit RainOfFireVFX(Camera3D* _camera);
    };
} // namespace sage