//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "VisualFX.hpp"

#include "TextureTerrainOverlay.hpp"

#include "raylib.h"

#include <memory>

namespace sage
{
    class GameData;
    class FloorFireVFX : public VisualFX
    {
        Vector2 screenSize{};
        float time = 0;
        int secondsLoc;
        int screenSizeLoc;

        std::unique_ptr<TextureTerrainOverlay> texture;

      public:
        Shader shader{};
        void InitSystem() override;
        void Update(float dt) override;
        void Draw3D() const override;
        explicit FloorFireVFX(GameData* _gameData, sgTransform* _transform);
    };
} // namespace sage