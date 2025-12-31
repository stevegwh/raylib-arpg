//
// Created by Steve Wheeler on 26/07/2024.
//

#include "FloorFireVFX.hpp"

#include "components/Ability.hpp"
#include "Systems.hpp"

#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Settings.hpp"
#include "engine/TextureTerrainOverlay.hpp"

#include "raylib.h"
#include "raymath.h"
#include <iostream>

namespace lq
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
        auto& transform = sys->registry->get<sage::sgTransform>(ability->self);
        texture->Init(transform.GetWorldPos());
        texture->Enable(true);
    }

    FloorFireVFX::FloorFireVFX(Systems* _sys, Ability* _ability)
        : VisualFX(_sys, _ability),
          shader(sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/floorfirefx.fs"))
    {
        secondsLoc = GetShaderLocation(shader, "seconds");

        // Screen size likely not used
        screenSizeLoc = GetShaderLocation(shader, "screenSize");
        screenSize = sys->settings->GetScreenSize();
        SetShaderValue(shader, screenSizeLoc, &screenSize, SHADER_UNIFORM_VEC2);

        texture = std::make_unique<sage::TextureTerrainOverlay>(
            _sys->registry, _sys->navigationGridSystem.get(), "indicator_rainoffire", WHITE, shader);
    }
} // namespace lq