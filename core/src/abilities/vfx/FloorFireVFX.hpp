//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "TextureTerrainOverlay.hpp"

#include "raylib.h"

#include <memory>

namespace sage
{
    class FloorFireVFX : public VisualFX
    {
        Vector2 screenSize;
        Vector3 targetPos;
        float time = 0;
        int secondsLoc;
        int screenSizeLoc;

        std::unique_ptr<TextureTerrainOverlay> texture;

        Model renderModel;

      public:
        Shader shader{};
        void InitSystem(const Vector3& _target) override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~FloorFireVFX();
        explicit FloorFireVFX(Camera* _camera);
    };
} // namespace sage