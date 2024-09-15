//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "FlamePartSys.hpp"

#include "raylib.h"

#include <memory>
#include <vector>

namespace sage
{
    struct Fireball
    {
        Vector3 position;
        Vector3 velocity;
        std::unique_ptr<FlamePartSys> flameEffect;
    };

    class RainOfFireVFX : public VisualFX
    {
        Vector3 target{};
        Vector3 baseSpawnPoint{};
        float initialHeight{};
        float minHeight{};
        float impactRadius{};
        const float initialOffset = 1.0f; // Initial diagonal offset from the target
        std::vector<Fireball> fireballs;
        void generateFireball(Fireball& fireball);

      public:
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        explicit RainOfFireVFX(GameData* _gameData, Ability* _ability);
    };
} // namespace sage