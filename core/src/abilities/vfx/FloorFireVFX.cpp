//
// Created by Steve Wheeler on 26/07/2024.
//

#include "FloorFireVFX.hpp"

#include "components/Ability.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"
#include "Systems.hpp"

#include "raylib.h"
#include "raymath.h"
#include <iostream>

namespace sage
{

    void FloorFireVFX::Draw3D() const
    {
    }

    void FloorFireVFX::Update(float dt)
    {
        time += 3 * GetFrameTime();
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
    }

    void FloorFireVFX::InitSystem()
    {
        active = true;
        auto& transform = sys->registry->get<sgTransform>(ability->self);
        texture->Init(transform.GetWorldPos());
        texture->Enable(true);
    }

    FloorFireVFX::FloorFireVFX(Systems* _sys, Ability* _ability)
        : VisualFX(_sys, _ability),
          shader(ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/floorfirefx.fs"))
    {
        secondsLoc = GetShaderLocation(shader, "seconds");

        // Screen size likely not used
        screenSizeLoc = GetShaderLocation(shader, "screenSize");
        screenSize = sys->settings->GetScreenSize();
        SetShaderValue(shader, screenSizeLoc, &screenSize, SHADER_UNIFORM_VEC2);

        texture = std::make_unique<TextureTerrainOverlay>(
            _sys->registry, _sys->navigationGridSystem.get(), "IMG_RAINOFFIRE_CURSOR", WHITE, shader);
    }
} // namespace sage