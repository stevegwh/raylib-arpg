//
// Created by Steve Wheeler on 26/07/2024.
//

#include "LightningBallVFX.hpp"

#include "engine/Camera.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/ResourceManager.hpp"

#include "components/Ability.hpp"
#include "Systems.hpp"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <iostream>

namespace lq
{
    void LightningBallVFX::Draw3D() const
    {
        // Draw model (if needed)
        rlDisableBackfaceCulling();
        auto& transform = sys->registry->get<sage::sgTransform>(ability->self);
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

    LightningBallVFX::LightningBallVFX(Systems* _sys, Ability* _ability) : VisualFX(_sys, _ability)
    {
        // Texture/Material
        auto texture = sage::ResourceManager::GetInstance().TextureLoad("T_Random_53");
        auto texture2 = sage::ResourceManager::GetInstance().TextureLoad("T_Random_53");

        shader = sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/lightning.fs");
        secondsLoc = GetShaderLocation(shader, "seconds");
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
        model = sage::ResourceManager::GetInstance().GetModelCopy("MDL_VFX_SPHERE");

        model.SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
        model.SetTexture(texture2, 0, MATERIAL_MAP_EMISSION);
        model.SetShader(shader, 0);

        shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
    }
} // namespace lq