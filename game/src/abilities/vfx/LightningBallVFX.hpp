//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "engine/slib.hpp"
#include "raylib.h"

#include <memory>
#include <vector>

namespace lq
{

    class LightningBallVFX : public VisualFX
    {
        Shader shader{};
        int secondsLoc;

        float time = 0.0f;
        sage::ModelSafe model;

      public:
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~LightningBallVFX() override = default;
        explicit LightningBallVFX(Systems* _sys, Ability* _ability);
    };
} // namespace lq