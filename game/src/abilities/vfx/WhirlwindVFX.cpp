//
// Created by Steve Wheeler on 26/07/2024.
//

#include "WhirlwindVFX.hpp"

#include "components/Ability.hpp"
#include "Systems.hpp"

#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/ResourceManager.hpp"

#include "raylib.h"
#include "rlgl.h"

namespace lq
{
    void WhirlwindVFX::Draw3D() const
    {
        // Draw model (if needed)
        rlDisableBackfaceCulling();
        auto& transform = sys->registry->get<sage::sgTransform>(ability->self);
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

    WhirlwindVFX::WhirlwindVFX(Systems* _sys, Ability* _ability) : VisualFX(_sys, _ability)
    {
        // Texture/Material
        auto texture =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/particles/twirl_01.png");

        shader =
            sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/swordslash2.fs");
        secondsLoc = GetShaderLocation(shader, "seconds");
        // SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
        // shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");

        slashModel = sage::ResourceManager::GetInstance().GetModelDeepCopy("vfx_flattorus");
        slashModel.SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
        slashModel.SetShader(shader, 0);
    }
} // namespace lq