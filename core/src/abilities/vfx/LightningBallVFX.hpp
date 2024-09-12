//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "raylib.h"
#include "slib.hpp"

#include <memory>
#include <vector>

namespace sage
{

    class LightningBallVFX : public VisualFX
    {
        Shader shader{};
        int secondsLoc;

        float time = 0.0f;
        ModelSafe model;

      public:
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~LightningBallVFX() override = default;
        explicit LightningBallVFX(GameData* _gameData, sgTransform* _transform);
    };
} // namespace sage