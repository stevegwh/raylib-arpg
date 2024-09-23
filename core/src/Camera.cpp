//
// Created by Steve Wheeler on 12/02/2024.
//

#include "Camera.hpp"

#include "components/sgTransform.hpp"
#include "GameData.hpp"
#include "slib.hpp"
#include "UserInput.hpp"

#include "components/NavigationGridSquare.hpp"
#include "components/Renderable.hpp"
#include "raymath.h"
#include "rcamera.h"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

namespace sage
{

    void Camera::onForwardKeyPressed()
    {
        forwardKeyDown = true;
    }

    void Camera::onLeftKeyPressed()
    {
        leftKeyDown = true;
    }

    void Camera::onRightKeyPressed()
    {
        rightKeyDown = true;
    }

    void Camera::onBackKeyPressed()
    {
        backKeyDown = true;
    }

    void Camera::onRotateLeftKeyPressed()
    {
        rotateLeftKeyDown = true;
    }

    void Camera::onRotateRightKeyPressed()
    {
        rotateRightKeyDown = true;
    }

    void Camera::onForwardKeyUp()
    {
        forwardKeyDown = false;
    }

    void Camera::onLeftKeyUp()
    {
        leftKeyDown = false;
    }

    void Camera::onRightKeyUp()
    {
        rightKeyDown = false;
    }

    void Camera::onBackKeyUp()
    {
        backKeyDown = false;
    }

    void Camera::onRotateLeftKeyUp()
    {
        rotateLeftKeyDown = false;
    }

    void Camera::onRotateRightKeyUp()
    {
        rotateRightKeyDown = false;
    }

    void Camera::updateTarget()
    {
        GridSquare square{};
        if (!gameData->navigationGridSystem->WorldToGridSpace(rlCamera.target, square)) return;

        float floorHeight = gameData->navigationGridSystem->GetGridSquare(square.row, square.col)->terrainHeight;
        const float targetOffsetY = 8.0f; // Offset from the floor

        float idealTargetY = floorHeight + targetOffsetY;
        float idealPositionY = idealTargetY + (rlCamera.position.y - rlCamera.target.y);
        currentTargetY = Lerp(currentTargetY, idealTargetY, easeSpeed);
        currentPositionY = Lerp(currentPositionY, idealPositionY, easeSpeed);
        rlCamera.target.y = currentTargetY;
        rlCamera.position.y = currentPositionY;
    }

    void Camera::handleInput()
    {
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_ALT) ||
            IsKeyDown(KEY_RIGHT_ALT) || lockInput)
            return;

        if (backKeyDown)
        {
            auto right = GetCameraRight(&rlCamera);
            right = Vector3RotateByAxisAngle(right, {0, 1, 0}, DEG2RAD * 90);
            rlCamera.position = Vector3Subtract(rlCamera.position, right);
            rlCamera.target = Vector3Subtract(rlCamera.target, right);
        }

        if (forwardKeyDown)
        {
            auto right = GetCameraRight(&rlCamera);
            right = Vector3RotateByAxisAngle(right, {0, 1, 0}, DEG2RAD * 90);
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

        if (scrollEnabled)
        {
            auto mouseScroll = GetMouseWheelMoveV();
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
                    currentPositionY = rlCamera.position.y; // Update currentPositionY
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
                currentPositionY = rlCamera.position.y; // Update currentPositionY
            }
        }
    }

    Camera3D* Camera::getRaylibCam()
    {
        return &rlCamera;
    }

    void Camera::LockInput()
    {
        lockInput = true;
    }

    void Camera::UnlockInput()
    {
        lockInput = false;
    }

    void Camera::ScrollEnable()
    {
        scrollEnabled = true;
    }

    void Camera::ScrollDisable()
    {
        scrollEnabled = false;
    }

    Vector3 Camera::GetForward()
    {
        return GetCameraForward(getRaylibCam());
    }

    Vector3 Camera::GetRight()
    {
        return GetCameraRight(getRaylibCam());
    }

    Vector3 Camera::GetBackward()
    {
        return NegateVector(GetForward());
    }

    Vector3 Camera::GetLeft()
    {
        return NegateVector(GetRight());
    }

    Vector3 Camera::GetPosition() const
    {
        return rlCamera.position;
    }

    void Camera::CutscenePose(const sgTransform& npcTrans)
    {
        rlCamera.position.y = rlCamera.target.y;

        // Calculate the camera's position behind the actor's shoulder
        Vector3 cameraOffset = {5.0f, 10.0f, 18.0f}; // TODO: Shouldn't be hardcoded

        Vector3 cameraPosition = Vector3Add(npcTrans.GetWorldPos(), cameraOffset);

        // Calculate the camera's target position slightly above the actor's position
        Vector3 cameraTarget = Vector3Add(npcTrans.GetWorldPos(), {0.0f, 1.0f, 0.0f});

        // Set the camera's position and target
        rlCamera.position = cameraPosition;
        rlCamera.target = cameraTarget;
    }

    void Camera::SetCamera(Vector3 _pos, Vector3 _target)
    {
        rlCamera.position = _pos;
        rlCamera.target = _target;
    }

    void Camera::Update()
    {
        updateTarget();
        handleInput();
        UpdateCameraPro(&rlCamera, {0, 0, 0}, {0, 0, 0}, 0);
    }

    Camera::Camera(entt::registry* _registry, UserInput* userInput, GameData* _gameData)
        : registry(_registry), gameData(_gameData), rlCamera({0})
    {
        rlCamera.position = {20.0f, 40.0f, 20.0f};
        rlCamera.target = {0.0f, 8.0f, 0.0f};
        rlCamera.up = {0.0f, 1.0f, 0.0f};
        rlCamera.fovy = 45.0f;
        rlCamera.projection = CAMERA_PERSPECTIVE;
        currentPositionY = rlCamera.position.y;
        currentTargetY = rlCamera.target.y;

        {
            entt::sink keyWPressed{userInput->keyWPressed};
            keyWPressed.connect<&Camera::onForwardKeyPressed>(this);
        }
        {
            entt::sink keySPressed{userInput->keySPressed};
            keySPressed.connect<&Camera::onBackKeyPressed>(this);
        }
        {
            entt::sink keyAPressed{userInput->keyAPressed};
            keyAPressed.connect<&Camera::onLeftKeyPressed>(this);
        }
        {
            entt::sink keyDPressed{userInput->keyDPressed};
            keyDPressed.connect<&Camera::onRightKeyPressed>(this);
        }
        {
            entt::sink keyEPressed{userInput->keyEPressed};
            keyEPressed.connect<&Camera::onRotateLeftKeyPressed>(this);
        }
        {
            entt::sink keyQPressed{userInput->keyQPressed};
            keyQPressed.connect<&Camera::onRotateRightKeyPressed>(this);
        }
        {
            entt::sink keyWUp{userInput->keyWUp};
            keyWUp.connect<&Camera::onForwardKeyUp>(this);
        }
        {
            entt::sink keySUp{userInput->keySUp};
            keySUp.connect<&Camera::onBackKeyUp>(this);
        }
        {
            entt::sink keyAUp{userInput->keyAUp};
            keyAUp.connect<&Camera::onLeftKeyUp>(this);
        }
        {
            entt::sink keyDUp{userInput->keyDUp};
            keyDUp.connect<&Camera::onRightKeyUp>(this);
        }
        {
            entt::sink keyEUp{userInput->keyEUp};
            keyEUp.connect<&Camera::onRotateLeftKeyUp>(this);
        }
        {
            entt::sink keyQUp{userInput->keyQUp};
            keyQUp.connect<&Camera::onRotateRightKeyUp>(this);
        }
    }
} // namespace sage
