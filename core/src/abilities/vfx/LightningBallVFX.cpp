//
// Created by Steve Wheeler on 26/07/2024.
//

#include "LightningBallVFX.hpp"

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
    void LightningBallVFX::Draw3D() const
    {
        // Draw model (if needed)
        rlDisableBackfaceCulling();
        DrawModelEx(model, origin, Vector3{0, 1, 0}, 0, Vector3{1, 1, 1}, WHITE);
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

    void LightningBallVFX::SetOrigin(const Vector3& _origin)
    {
        origin = _origin;
    }

    void LightningBallVFX::InitSystem(const Vector3& _target)
    {
        active = true;
        origin = _target;
        origin.y = 5; // TODO: tmp
        time = 0;
    }

    LightningBallVFX::~LightningBallVFX()
    {
        UnloadShader(shader);
        UnloadTexture(texture);
        UnloadTexture(texture2);
        UnloadModel(model);
    }

    LightningBallVFX::LightningBallVFX(GameData* _gameData) : VisualFX(_gameData)
    {
        // Texture/Material
        texture =
            LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_53.png"); // TODO: Is first texture used?
        texture2 = LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_53.png");

        shader = LoadShader(nullptr, "resources/shaders/custom/lightning.fs");
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