//
// Created by Steve Wheeler on 26/07/2024.
//

#include "WhirlwindVFX.hpp"

#include "components/Ability.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"

#include "raylib.h"
#include "rlgl.h"

namespace sage
{
    void WhirlwindVFX::Draw3D() const
    {
        // Draw model (if needed)
        rlDisableBackfaceCulling();
        auto& transform = gameData->registry->get<sgTransform>(ability->self);
        slashModel.Draw(
            transform.GetWorldPos(),
            Vector3{0, 1, 0},
            -210 + transform.GetWorldRot().y + -(time * 1000),
            Vector3{5.0, 1.0, 5.0},
            WHITE);
        rlEnableBackfaceCulling();
    }

    void WhirlwindVFX::Update(float dt)
    {
        time += dt;
        if (time > 0.1) // TODO: Magic number
        {
            time = 0;
            active = false;
        }
        // SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
    }

    void WhirlwindVFX::InitSystem()
    {
        active = true;
        time = 0;
    }

    WhirlwindVFX::WhirlwindVFX(GameData* _gameData, Ability* _ability) : VisualFX(_gameData, _ability)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/particles/twirl_01.png");
        // Texture/Material
        auto texture = ResourceManager::GetInstance().TextureLoad("resources/textures/particles/twirl_01.png");

        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/swordslash2.fs");
        secondsLoc = GetShaderLocation(shader, "seconds");
        // SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
        // shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");

        slashModel = ResourceManager::GetInstance().GetModelCopy(AssetID::MDL_VFX_FLATTORUS);
        slashModel.SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
        slashModel.SetShader(shader, 0);
    }
} // namespace sage