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

    class FireballVFX : public VisualFX
    {
        Shader shader{};
        int secondsLoc;
        Texture2D texture;
        Texture2D texture2;

        Vector3 origin{};
        float time = 0.0f;
        Model model;

      public:
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~FireballVFX();
        explicit FireballVFX(GameData* _gameData, sgTransform* _transform);
    };
} // namespace sage