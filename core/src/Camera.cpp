//
// Created by Steve Wheeler on 12/02/2024.
//

#include "Camera.hpp"
#include "UserInput.hpp"
#include "rcamera.h"
#include "raymath.h"

namespace sage
{

void Camera::OnForwardKeyPressed()
{
    forwardKeyDown = true;
}

void Camera::OnLeftKeyPressed()
{
    leftKeyDown = true;
}

void Camera::OnRightKeyPressed()
{
    rightKeyDown = true;
}

void Camera::OnBackKeyPressed()
{
    backKeyDown = true;
}

void Camera::OnRotateLeftKeyPressed()
{
    rotateLeftKeyDown = true;
}

void Camera::OnRotateRightKeyPressed()
{
    rotateRightKeyDown = true;
}

void Camera::OnForwardKeyUp()
{
    forwardKeyDown = false;
}

void Camera::OnLeftKeyUp()
{
    leftKeyDown = false;
}

void Camera::OnRightKeyUp()
{
    rightKeyDown = false;
}

void Camera::OnBackKeyUp()
{
    backKeyDown = false;
}

void Camera::OnRotateLeftKeyUp()
{
    rotateLeftKeyDown = false;
}

void Camera::OnRotateRightKeyUp()
{
    rotateRightKeyDown = false;
}

void Camera::handleInput()
{
    if (IsKeyDown(KEY_LEFT_CONTROL)|| IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
        return;

    if (backKeyDown)
    {
        auto right = GetCameraRight(&rlCamera);
        right = Vector3RotateByAxisAngle(right, { 0, 1, 0 }, DEG2RAD * 90);
        rlCamera.position = Vector3Subtract(rlCamera.position, right);
        rlCamera.target = Vector3Subtract(rlCamera.target, right);
    }

    if (forwardKeyDown)
    {
        auto right = GetCameraRight(&rlCamera);
        right = Vector3RotateByAxisAngle(right, { 0, 1, 0 }, DEG2RAD * 90);
        rlCamera.position = Vector3Add(right, rlCamera.position);
        rlCamera.target = Vector3Add(right, rlCamera.target);
    }

    if (leftKeyDown)
    {
        rlCamera.position = Vector3Subtract(rlCamera.position, GetCameraRight(&rlCamera));
        rlCamera.target = Vector3Subtract(rlCamera.target, GetCameraRight(&rlCamera));
    }

    if (rightKeyDown)
    {
        rlCamera.position = Vector3Add(GetCameraRight(&rlCamera), rlCamera.position);
        rlCamera.target = Vector3Add(GetCameraRight(&rlCamera), rlCamera.target);
    }

    if (rotateLeftKeyDown)
    {
        rlCamera.position = Vector3Add(GetCameraRight(&rlCamera), rlCamera.position);
    }

    if (rotateRightKeyDown)
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

Camera3D* Camera::getRaylibCam()
{
    return &rlCamera;
}
Camera::Camera(UserInput* userInput)
    : rlCamera({0})
{
    rlCamera.position = { 20.0f, 40.0f, 20.0f };
    rlCamera.target = { 0.0f, 8.0f, 0.0f };
    rlCamera.up = { 0.0f, 1.0f, 0.0f };
    rlCamera.fovy = 45.0f;
    rlCamera.projection = CAMERA_PERSPECTIVE;
    
    userInput->dKeyWPressed.connect<&Camera::OnForwardKeyPressed>(this);
    userInput->dKeySPressed.connect<&Camera::OnBackKeyPressed>(this);
    userInput->dKeyAPressed.connect<&Camera::OnLeftKeyPressed>(this);
    userInput->dKeyDPressed.connect<&Camera::OnRightKeyPressed>(this);
    userInput->dKeyEPressed.connect<&Camera::OnRotateLeftKeyPressed>(this);
    userInput->dKeyQPressed.connect<&Camera::OnRotateRightKeyPressed>(this);

    userInput->dKeyWUp.connect<&Camera::OnForwardKeyUp>(this);
    userInput->dKeySUp.connect<&Camera::OnBackKeyUp>(this);
    userInput->dKeyAUp.connect<&Camera::OnLeftKeyUp>(this);
    userInput->dKeyDUp.connect<&Camera::OnRightKeyUp>(this);
    userInput->dKeyEUp.connect<&Camera::OnRotateLeftKeyUp>(this);
    userInput->dKeyQUp.connect<&Camera::OnRotateRightKeyUp>(this);
}

} // sage