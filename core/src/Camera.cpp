//
// Created by Steve Wheeler on 12/02/2024.
//

#include "Camera.hpp"
#include "rcamera.h"
#include "raymath.h"
#include <iostream>

namespace sage
{

// TODO: this should all be registered to events to the UserInput (but... this gets deleted every time because its part of the ECS :)))
void Camera::handleInput()
{
    if (IsKeyDown(KEY_LEFT_CONTROL)|| IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
        return;

    if (IsKeyDown(KEY_S))
    {
        auto right = GetCameraRight(&rlCamera);
        right = Vector3RotateByAxisAngle(right, { 0, 1, 0 }, DEG2RAD * 90);
        rlCamera.position = Vector3Subtract(rlCamera.position, right);
        rlCamera.target = Vector3Subtract(rlCamera.target, right);
    }

    if (IsKeyDown(KEY_W))
    {
        auto right = GetCameraRight(&rlCamera);
        right = Vector3RotateByAxisAngle(right, { 0, 1, 0 }, DEG2RAD * 90);
        rlCamera.position = Vector3Add(right, rlCamera.position);
        rlCamera.target = Vector3Add(right, rlCamera.target);
    }

    if (IsKeyDown(KEY_A))
    {
        rlCamera.position = Vector3Subtract(rlCamera.position, GetCameraRight(&rlCamera));
        rlCamera.target = Vector3Subtract(rlCamera.target, GetCameraRight(&rlCamera));
    }

    if (IsKeyDown(KEY_D))
    {
        rlCamera.position = Vector3Add(GetCameraRight(&rlCamera), rlCamera.position);
        rlCamera.target = Vector3Add(GetCameraRight(&rlCamera), rlCamera.target);
    }

    if (IsKeyDown(KEY_E))
    {
        rlCamera.position = Vector3Add(GetCameraRight(&rlCamera), rlCamera.position);
    }

    if (IsKeyDown(KEY_Q))
    {
        rlCamera.position = Vector3Subtract(rlCamera.position, GetCameraRight(&rlCamera));
    }
    

    auto mouseScroll= GetMouseWheelMoveV();
    if (mouseScroll.y > 0)
    {
        if (rlCamera.position.y > rlCamera.target.y)
        {
            Vector3 up = GetCameraUp(&rlCamera);
            up.x *= 2.0f;
            up.y *= 2.0f;
            up.z *= 2.0f;
            rlCamera.position = Vector3Subtract(rlCamera.position, up);
            rlCamera.position = Vector3Add(GetCameraForward(&rlCamera), rlCamera.position);
        }        
    }
    if (mouseScroll.y < 0)
    {
        Vector3 up = GetCameraUp(&rlCamera);
        up.x *= 2.0f;
        up.y *= 2.0f;
        up.z *= 2.0f;
        rlCamera.position = Vector3Add(up, rlCamera.position);
        rlCamera.position = Vector3Subtract(rlCamera.position, GetCameraForward(&rlCamera));
    }

}

void Camera::Update()
{
    handleInput();
    UpdateCameraPro(&rlCamera, {0, 0, 0}, { 0, 0, 0 }, 0);
}

Camera3D* Camera::getCamera()
{
    return &rlCamera;
}

} // sage