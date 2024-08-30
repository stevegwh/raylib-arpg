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

    class WhirlwindVFX : public VisualFX
    {
        Shader shader{};
        int secondsLoc;
        Texture2D texture;
        Texture2D texture2;

        float time = 0.0f;
        Model slashModel;

      public:
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~WhirlwindVFX();
        explicit WhirlwindVFX(GameData* _gameData, sgTransform* _transform);
    };
} // namespace sage