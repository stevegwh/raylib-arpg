//
// Created by Steve Wheeler on 26/07/2024.
//

#include "FireballVFX.hpp"

#include "Camera.hpp"
#include "components/Ability.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "ResourceManager.hpp"
#include "Systems.hpp"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <iostream>

namespace sage
{
    const ModelSafe& FireballVFX::GetModel()
    {
        return model;
    }

    void FireballVFX::Draw3D() const
    {
        rlDisableBackfaceCulling();
        auto& transform = sys->registry->get<sgTransform>(ability->self);
        model.Draw(transform.GetWorldPos(), Vector3{0, 1, 0}, 0, Vector3{1, 1, 1}, WHITE);
        rlEnableBackfaceCulling();
    }

    void FireballVFX::Update(float dt)
    {
        time += dt;
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
    }

    void FireballVFX::InitSystem()
    {
        active = true;
        time = 0;
    }

    FireballVFX::FireballVFX(Systems* _sys, Ability* _ability) : VisualFX(_sys, _ability)
    {
        // Texture/Material
        auto texture = ResourceManager::GetInstance().TextureLoad("IMG_NOISE50");
        auto texture2 = ResourceManager::GetInstance().TextureLoad("IMG_NOISE45");

        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/fireball.fs");
        secondsLoc = GetShaderLocation(shader, "seconds");
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
        model = ResourceManager::GetInstance().GetModelDeepCopy("MDL_VFX_SPHERE");
        model.SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
        model.SetTexture(texture2, 0, MATERIAL_MAP_EMISSION);
        model.SetShader(shader, 0);

        shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
    }
} // namespace sage