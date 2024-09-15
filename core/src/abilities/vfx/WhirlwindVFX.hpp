//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "raylib.h"
#include <slib.hpp>

#include <memory>
#include <vector>

namespace sage
{

    class WhirlwindVFX : public VisualFX
    {
        Shader shader{};
        int secondsLoc;

        float time = 0.0f;
        ModelSafe slashModel;

      public:
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~WhirlwindVFX() override = default;
        explicit WhirlwindVFX(GameData* _gameData, Ability* _ability);
    };
} // namespace sage