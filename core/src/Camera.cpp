//
// Created by Steve Wheeler on 12/02/2024.
//

#include "Camera.hpp"

#include "components/sgTransform.hpp"
#include "slib.hpp"
#include "Systems.hpp"
#include "UserInput.hpp"

#include "components/NavigationGridSquare.hpp"
#include "components/Renderable.hpp"
#include "raymath.h"
#include "rcamera.h"
#include "systems/CollisionSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

namespace sage
{
    /*
     * Smooths out variations in the camera's height
     */
    void Camera::cameraHeightSmoothing()
    {
        GridSquare square{};
        if (!sys->navigationGridSystem->WorldToGridSpace(rlCamera.target, square)) return;

        float floorHeight = sys->navigationGridSystem->GetGridSquare(square.row, square.col)->GetTerrainHeight();
        const float targetOffsetY = 8.0f; // Offset from the floor

        float idealTargetY = floorHeight + targetOffsetY;
        float idealPositionY = idealTargetY + (rlCamera.position.y - rlCamera.target.y);
        verticalSmoothingTargetY = Lerp(verticalSmoothingTargetY, idealTargetY, verticalEasingSpeed);
        verticalSmoothingCurrentY = Lerp(verticalSmoothingCurrentY, idealPositionY, verticalEasingSpeed);
        rlCamera.target.y = verticalSmoothingTargetY;
        rlCamera.position.y = verticalSmoothingCurrentY;
    }

    void Camera::handleMouseScroll()
    {
        if (cameraScrollVelY > 0)
        {
            cameraScrollVelY -= cameraScrollDeceleration;
            if (cameraScrollVelY < 0)
            {
                cameraScrollVelY = 0;
            }
        }
        else if (cameraScrollVelY < 0)
        {
            cameraScrollVelY += cameraScrollDeceleration;
            if (cameraScrollVelY > 0)
            {
                cameraScrollVelY = 0;
            }
        }
        else
        {
            cameraScrollVelY = 0;
        }

        if (scrollEnabled)
        {

            auto mouseScroll = GetMouseWheelMoveV();
            if (mouseScroll.y > 0)
            {
                cameraScrollVelY = cameraInitialVelY;
            }
            else if (mouseScroll.y < 0)
            {
                cameraScrollVelY = -cameraInitialVelY;
            }
            Vector3 up = Vector3MultiplyByValue(GetCameraUp(&rlCamera), abs(cameraScrollVelY));
            if (cameraScrollVelY > 0)
            {
                auto nextPos = rlCamera.position;
                nextPos = Vector3Subtract(nextPos, up);
                nextPos = Vector3Add(nextPos, GetCameraForward(&rlCamera));
                if (nextPos.y > rlCamera.target.y)
                {
                    rlCamera.position = nextPos;
                }
                else
                {
                    rlCamera.position.y = rlCamera.target.y;
                    cameraScrollVelY = 0;
                }
            }
            else if (cameraScrollVelY < 0)
            {
                auto nextPos = rlCamera.position;
                nextPos = Vector3Add(nextPos, up);
                nextPos = Vector3Subtract(nextPos, GetCameraForward(&rlCamera));

                if (nextPos.y < cameraMaxY)
                {
                    rlCamera.position = nextPos;
                }
                else
                {
                    rlCamera.position.y = cameraMaxY;
                    cameraScrollVelY = 0;
                }
            }
            verticalSmoothingCurrentY = rlCamera.position.y; // Update verticalSmoothingCurrentY
        }
    }

    void Camera::handleInput()
    {
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_ALT) ||
            IsKeyDown(KEY_RIGHT_ALT) || lockInput)
            return;

        handleMouseScroll();

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

        cameraHeightSmoothing();
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

    void Camera::CutscenePose(const sgTransform& location, const Vector3& localOffset)
    {
        cameraSave = CameraSave{rlCamera, verticalSmoothingTargetY, verticalSmoothingCurrentY};

        rlCamera.position.y = rlCamera.target.y;

        auto [rotx, roty, rotz] = location.GetWorldRot();
        const Matrix rotationMatrix = MatrixRotateXYZ({rotx * DEG2RAD, roty * DEG2RAD, rotz * DEG2RAD});

        const Vector3 rotatedOffset = Vector3Transform(localOffset, rotationMatrix);
        const Vector3 cameraPosition = Vector3Add(location.GetWorldPos(), rotatedOffset);
        const Vector3 cameraTarget = Vector3Add(location.GetWorldPos(), {0.0f, 1.0f, 0.0f});

        rlCamera.position = cameraPosition;
        rlCamera.target = cameraTarget;
    }

    void Camera::CutsceneEnd()
    {
        SetCamera(cameraSave.rlCamera.position, cameraSave.rlCamera.target);
    }

    void Camera::SetCamera(Vector3 _pos, Vector3 _target)
    {
        rlCamera.position = _pos;
        rlCamera.target = _target;
    }

    void Camera::FocusSelectedActor()
    {
        auto actorId = sys->controllableActorSystem->GetSelectedActor();
        auto& transform = registry->get<sgTransform>(actorId);

        auto diff = Vector3Subtract(rlCamera.position, rlCamera.target);
        SetCamera(Vector3Add(transform.GetWorldPos(), diff), transform.GetWorldPos());

        GridSquare square{};
        if (!sys->navigationGridSystem->WorldToGridSpace(rlCamera.target, square)) return;
        const float floorHeight =
            sys->navigationGridSystem->GetGridSquare(square.row, square.col)->GetTerrainHeight();
        constexpr float targetOffsetY = 8.0f; // Offset from the floor
        const float idealTargetY = floorHeight + targetOffsetY;
        const float idealPositionY = idealTargetY + (rlCamera.position.y - rlCamera.target.y);

        rlCamera.target.y = idealTargetY;
        rlCamera.position.y = idealPositionY;
        verticalSmoothingCurrentY = rlCamera.position.y;
        verticalSmoothingTargetY = rlCamera.target.y;
    }

    void Camera::DrawDebug()
    {
        DrawCube(getRaylibCam()->target, 2, 2, 2, RED);
    }

    void Camera::Update()
    {
        handleInput();
        UpdateCameraPro(&rlCamera, {0, 0, 0}, {0, 0, 0}, 0);
    }

    Camera::Camera(entt::registry* _registry, UserInput* userInput, Systems* _sys)
        : registry(_registry), sys(_sys), rlCamera({0})
    {
        rlCamera.position = {-40.0f, 85.0f, 20.0f};
        rlCamera.target = {-30.0f, 8.0f, -50.0f};
        rlCamera.up = {0.0f, 1.0f, 0.0f};
        rlCamera.fovy = 45.0f;
        rlCamera.projection = CAMERA_PERSPECTIVE;
        verticalSmoothingCurrentY = rlCamera.position.y;
        verticalSmoothingTargetY = rlCamera.target.y;

        userInput->keyWPressed.Subscribe([this]() { forwardKeyDown = true; });
        userInput->keySPressed.Subscribe([this]() { backKeyDown = true; });
        userInput->keyAPressed.Subscribe([this]() { leftKeyDown = true; });
        userInput->keyDPressed.Subscribe([this]() { rightKeyDown = true; });
        userInput->keyEPressed.Subscribe([this]() { rotateLeftKeyDown = true; });
        userInput->keyQPressed.Subscribe([this]() { rotateRightKeyDown = true; });
        userInput->keyWUp.Subscribe([this]() { forwardKeyDown = false; });
        userInput->keySUp.Subscribe([this]() { backKeyDown = false; });
        userInput->keyAUp.Subscribe([this]() { leftKeyDown = false; });
        userInput->keyDUp.Subscribe([this]() { rightKeyDown = false; });
        userInput->keyEUp.Subscribe([this]() { rotateLeftKeyDown = false; });
        userInput->keyQUp.Subscribe([this]() { rotateRightKeyDown = false; });
    }
} // namespace sage
