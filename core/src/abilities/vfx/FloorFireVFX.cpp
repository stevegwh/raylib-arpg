//
// Created by Steve Wheeler on 26/07/2024.
//

#include "FloorFireVFX.hpp"

#include "GameData.hpp"

#include "ResourceManager.hpp"

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

    void FloorFireVFX::InitSystem(const Vector3& _target)
    {
        active = true;
        texture->Init(_target);
        texture->Enable(true);
    }

    FloorFireVFX::~FloorFireVFX()
    {
    }

    FloorFireVFX::FloorFireVFX(GameData* _gameData)
        : VisualFX(_gameData),
          texture(std::make_unique<TextureTerrainOverlay>(
              gameData->registry,
              gameData->navigationGridSystem.get(),
              "resources/textures/cursor/rainoffire_cursor.png",
              WHITE,
              "resources/shaders/floorfirefx.fs")),
          shader(gameData->registry->get<Renderable>(texture->entity).shader.value())
    {
        secondsLoc = GetShaderLocation(shader, "seconds");
        screenSizeLoc = GetShaderLocation(shader, "screenSize");
        screenSize = {static_cast<float>(gameData->settings->screenWidth), static_cast<float>(gameData->settings->screenHeight)};
        SetShaderValue(shader, screenSizeLoc, &screenSize, SHADER_UNIFORM_VEC2);
    }
} // namespace sage