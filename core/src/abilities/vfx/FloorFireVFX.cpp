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
        // texture->Update(gameData->cursor->terrainCollision().point);
    }

    void FloorFireVFX::InitSystem()
    {
        active = true;
        texture->Init(transform->GetWorldPos());
        texture->Enable(true);
    }

    FloorFireVFX::~FloorFireVFX()
    {
    }

    FloorFireVFX::FloorFireVFX(GameData* _gameData, sgTransform* _transform)
        : VisualFX(_gameData, _transform),
          texture(std::make_unique<TextureTerrainOverlay>(
              _gameData->registry,
              _gameData->navigationGridSystem.get(),
              "resources/textures/cursor/rainoffire_cursor.png",
              WHITE,
              "resources/shaders/floorfirefx.fs")),
          shader(_gameData->registry->get<Renderable>(texture->entity).shader.value())
    {
        secondsLoc = GetShaderLocation(shader, "seconds");
        screenSizeLoc = GetShaderLocation(shader, "screenSize");
        screenSize = {
            static_cast<float>(gameData->settings->screenWidth),
            static_cast<float>(gameData->settings->screenHeight)};
        SetShaderValue(shader, screenSizeLoc, &screenSize, SHADER_UNIFORM_VEC2);
    }
} // namespace sage