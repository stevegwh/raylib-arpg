//
// Created by Steve Wheeler on 26/07/2024.
//

#include "FireballVFX.hpp"

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
    void FireballVFX::Draw3D() const
    {
        rlDisableBackfaceCulling();
        DrawModelEx(model, transform->GetWorldPos(), Vector3{0, 1, 0}, 0, Vector3{1, 1, 1}, WHITE);
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

    FireballVFX::~FireballVFX()
    {
        UnloadShader(shader);
        UnloadTexture(texture);
        UnloadTexture(texture2);
        UnloadModel(model);
    }

    FireballVFX::FireballVFX(GameData* _gameData, sgTransform* _transform) : VisualFX(_gameData, _transform)
    {
        // Texture/Material
        texture = LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_50.png");
        texture2 = LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_45.png");

        shader = LoadShader(nullptr, "resources/shaders/custom/fireball.fs");
        secondsLoc = GetShaderLocation(shader, "seconds");
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
        model = LoadModel("resources/models/obj/sphere.obj");
        // slashModel.transform = MatrixRotateY(180);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
        // Using MATERIAL_MAP_EMISSION as a spare slot to use for 2nd texture
        model.materials[0].maps[MATERIAL_MAP_EMISSION].texture = texture2;
        shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
        model.materials[0].shader = shader;
    }
} // namespace sage