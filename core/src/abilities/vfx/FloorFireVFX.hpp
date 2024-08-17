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
    class FloorFireVFX : public VisualFX
    {
        Vector3 targetPos;
        float time = 0;
        int secondsLoc;
        int screenSizeLoc;
        Shader shader{};
        Model renderModel;

      public:
        void InitSystem(const Vector3& _target) override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~FloorFireVFX();
        explicit FloorFireVFX(Camera* _camera);
    };
} // namespace sage