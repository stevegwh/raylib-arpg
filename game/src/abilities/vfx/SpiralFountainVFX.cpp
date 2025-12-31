//
// Created by Steve Wheeler on 26/07/2024.
//

#include "SpiralFountainVFX.hpp"

#include "components/Ability.hpp"
#include "Systems.hpp"

#include "engine/Camera.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/ResourceManager.hpp"

#include "raylib.h"
#include "raymath.h"

#include <cmath>

namespace lq
{
    void SpiralFountainVFX::Draw3D() const
    {
        fountain->Draw(shader);
    }

    float pos = 1;

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
        //        auto& transform = sys->registry->get<sgTransform>(ability->self);
        //        fountain->SetOrigin(transform.GetWorldPos());
        fountain->SetOrigin({spiralPos});
        fountain->Update(dt);
    }

    void SpiralFountainVFX::InitSystem()
    {
        active = true;
    }

    SpiralFountainVFX::SpiralFountainVFX(Systems* _sys, Ability* _ability) : VisualFX(_sys, _ability)
    {
        shader =
            sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/billboard.fs");
        fountain = std::make_unique<FountainPartSys>(_sys->camera->getRaylibCam());
    }
} // namespace lq