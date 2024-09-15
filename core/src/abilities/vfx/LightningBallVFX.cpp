//
// Created by Steve Wheeler on 26/07/2024.
//

#include "LightningBallVFX.hpp"

#include "Camera.hpp"
#include "components/Ability.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <iostream>

namespace sage
{
    void LightningBallVFX::Draw3D() const
    {
        // Draw model (if needed)
        rlDisableBackfaceCulling();
        auto& transform = gameData->registry->get<sgTransform>(ability->self);
        model.Draw(transform.GetWorldPos(), Vector3{0, 1, 0}, 0, Vector3{1, 1, 1}, WHITE);
        rlEnableBackfaceCulling();
    }

    void LightningBallVFX::Update(float dt)
    {
        time += dt;
        if (time > 0.5) // TODO: Magic number
        {
            time = 0;
            active = false;
        }
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
    }

    void LightningBallVFX::InitSystem()
    {
        active = true;
        time = 0;
    }

    LightningBallVFX::LightningBallVFX(GameData* _gameData, Ability* _ability) : VisualFX(_gameData, _ability)
    {
        // Texture/Material
        auto texture = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_NOISE53);
        auto texture2 = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_NOISE53);

        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/lightning.fs");
        secondsLoc = GetShaderLocation(shader, "seconds");
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
        model = ResourceManager::GetInstance().GetModelCopy(AssetID::MDL_VFX_SPHERE);

        model.SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
        model.SetTexture(texture2, 0, MATERIAL_MAP_EMISSION);
        model.SetShader(shader, 0);

        shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
    }
} // namespace sage