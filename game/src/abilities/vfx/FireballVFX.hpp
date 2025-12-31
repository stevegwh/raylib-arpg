//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "engine/slib.hpp"
#include "raylib.h"

namespace lq
{
    class Systems;

    class FireballVFX : public VisualFX
    {
        Shader shader{};
        int secondsLoc;

        Vector3 origin{};
        float time = 0.0f;
        sage::ModelSafe model;

      public:
        const sage::ModelSafe& GetModel();
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~FireballVFX() override = default;
        explicit FireballVFX(Systems* _sys, Ability* _ability);
    };
} // namespace lq