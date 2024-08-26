//
// Created by Steve Wheeler on 26/07/2024.
//

#include "WhirlwindVFX.hpp"

#include "GameData.hpp"

#include "Camera.hpp"

#include "raylib.h"
#include "raymath.h"
#include "ResourceManager.hpp"
#include "rlgl.h"

#include <cmath>

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include <iostream>

namespace sage
{
    void WhirlwindVFX::Draw3D() const
    {
        // Draw model (if needed)
        rlDisableBackfaceCulling();
        DrawModelEx(slashModel, origin, Vector3{0, 1, 0}, time * 1000, Vector3{5.0, 1.0, 5.0}, WHITE);
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
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
    }

    void WhirlwindVFX::SetOrigin(const Vector3& _origin)
    {
        origin = _origin;
    }

    void WhirlwindVFX::InitSystem(const Vector3& _target)
    {
        active = true;
        origin = _target;
        origin.y = 5; // TODO: tmp
        time = 0;
    }

    WhirlwindVFX::~WhirlwindVFX()
    {
        UnloadShader(shader);
        UnloadTexture(texture);
        UnloadTexture(texture2);
        UnloadModel(slashModel);
    }

    WhirlwindVFX::WhirlwindVFX(GameData* _gameData) : VisualFX(_gameData)
    {
        // Texture/Material
        texture = LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_59.png");
        texture2 = LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_45.png");

        shader = LoadShader(nullptr, "resources/shaders/custom/swordslash.fs");
        secondsLoc = GetShaderLocation(shader, "seconds");
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
        slashModel = LoadModel("resources/models/obj/flattorus.obj");
        // slashModel.transform = MatrixRotateY(180);

        slashModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
        // Using MATERIAL_MAP_EMISSION as a spare slot to use for 2nd texture
        slashModel.materials[0].maps[MATERIAL_MAP_EMISSION].texture = texture2;
        shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
        slashModel.materials[0].shader = shader;
    }
} // namespace sage