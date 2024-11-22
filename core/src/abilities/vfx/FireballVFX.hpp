//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "raylib.h"
#include "slib.hpp"

namespace sage
{

    class FireballVFX : public VisualFX
    {
        Shader shader{};
        int secondsLoc;

        Vector3 origin{};
        float time = 0.0f;
        ModelSafe model;

      public:
        const ModelSafe& GetModel();
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        ~FireballVFX() override = default;
        explicit FireballVFX(GameData* _gameData, Ability* _ability);
    };
} // namespace sage