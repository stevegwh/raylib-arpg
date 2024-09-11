//
// Created by Steve Wheeler on 26/07/2024.
//

#include "FloorFireVFX.hpp"

#include "GameData.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"

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
        texture->Init(transform->GetWorldPos());
        texture->Enable(true);
    }

    FloorFireVFX::FloorFireVFX(GameData* _gameData, sgTransform* _transform)
        : VisualFX(_gameData, _transform),
          shader(ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/floorfirefx.fs"))
    {
        secondsLoc = GetShaderLocation(shader, "seconds");

        // Screen size likely not used
        screenSizeLoc = GetShaderLocation(shader, "screenSize");
        screenSize = {
            static_cast<float>(gameData->settings->screenWidth),
            static_cast<float>(gameData->settings->screenHeight)};
        SetShaderValue(shader, screenSizeLoc, &screenSize, SHADER_UNIFORM_VEC2);

        texture = std::make_unique<TextureTerrainOverlay>(
            _gameData->registry,
            _gameData->navigationGridSystem.get(),
            AssetID::TEX_RAINOFFIRE_CURSOR,
            WHITE,
            shader);
    }
} // namespace sage