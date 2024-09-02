//
// Created by Steve Wheeler on 26/07/2024.
//

#include "SpiralFountainVFX.hpp"

#include "Camera.hpp"
#include "components/sgTransform.hpp"
#include "GameData.hpp"

#include "raylib.h"
#include "raymath.h"
#include "ResourceManager.hpp"

#include <cmath>

namespace sage
{
    void SpiralFountainVFX::Draw3D() const
    {
        fountain->Draw(shader);
    }

    void SpiralFountainVFX::Update(float dt)
    {
        spiralAngle += spiralSpeed * dt;
        spiralRadius += spiralGrowth * dt;
        if (spiralRadius > maxSpiralRadius)
        {
            spiralRadius = minSpiralRadius;
            spiralAngle = 0.0f;
        }
        Vector3 spiralPos = {
            cosf(spiralAngle) * spiralRadius,
            spiralRadius / 2, // This makes the spiral move upwards as it expands
            sinf(spiralAngle) * spiralRadius};
        fountain->SetOrigin(transform->GetWorldPos());
        fountain->Update(dt);
    }

    void SpiralFountainVFX::InitSystem()
    {
        active = true;
    }

    SpiralFountainVFX::~SpiralFountainVFX()
    {
        UnloadShader(shader);
    }

    SpiralFountainVFX::SpiralFountainVFX(GameData* _gameData, sgTransform* _transform)
        : VisualFX(_gameData, _transform)
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/billboard.fs");
        fountain = std::make_unique<FountainPartSys>(_gameData->camera->getRaylibCam());
    }
} // namespace sage