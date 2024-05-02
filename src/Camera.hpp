//
// Created by Steve Wheeler on 12/02/2024.
//

#pragma once
#include <raylib.h>

namespace sage
{

class Camera
{
    Camera3D rlCamera;
    Vector3 position {};
    Vector3 target {};
    Vector3 rotation {};
    int zoom = 10;
    void handleInput();
public:
    Camera3D* getCamera();
    Camera()
    : rlCamera({0})
    {
        rlCamera.position = { 20.0f, 40.0f, 20.0f };
        rlCamera.target = { 0.0f, 8.0f, 0.0f };
        rlCamera.up = { 0.0f, 1.0f, 0.0f };
        rlCamera.fovy = 45.0f;
        rlCamera.projection = CAMERA_PERSPECTIVE;
    }
    
    void Update();
};

} // sage
