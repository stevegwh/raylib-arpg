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

        Vector3 origin{};
        float time = 0.0f;
        Model slashModel;

      public:
        void InitSystem(const Vector3& _target) override;
        void Update(float dt) override;
        void Draw3D() const override;
        void SetOrigin(const Vector3& origin) override;
        ~WhirlwindVFX();
        explicit WhirlwindVFX(GameData* _gameData);
    };
} // namespace sage