//
// Created by Steve Wheeler on 26/07/2024.
//

#include "FloorFireVFX.hpp"

#include "ResourceManager.hpp"

#include "raylib.h"
#include "raymath.h"
#include "slib.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace sage
{

    void FloorFireVFX::Draw3D() const
    {
        BeginShaderMode(shader);
        // Draw quad here (Just make it a renderable?)
        DrawModel(renderModel, targetPos, 1.0f, WHITE);
        EndShaderMode();
    }

    void FloorFireVFX::Update(float dt)
    {
        time += 3 * dt;
        SetShaderValue(shader, secondsLoc, &time, SHADER_UNIFORM_FLOAT);
    }

    void FloorFireVFX::InitSystem(const Vector3& _target)
    {
        targetPos = _target;
        secondsLoc = GetShaderLocation(shader, "seconds");
        screenSizeLoc = GetShaderLocation(shader, "screenSize");

        Vector2 screenSize = {(float)1280, (float)720}; // TODO: Guess I'll need to pass in settings here
        SetShaderValue(shader, screenSizeLoc, &screenSize, SHADER_UNIFORM_VEC2);
    }

    FloorFireVFX::~FloorFireVFX()
    {

        UnloadShader(shader);
        std::cout << "FloorFireVFX destroyed" << std::endl;
        UnloadModel(renderModel);
    }

    FloorFireVFX::FloorFireVFX(Camera* _camera) : VisualFX(_camera)
    {
        shader = ResourceManager::ShaderLoad(nullptr, "resources/floorfirefx.fs");
        renderModel = LoadModelFromMesh(GenMeshPlane(2.0f, 2.0f, 1, 1));
    }
} // namespace sage