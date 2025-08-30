//
// Created by Steve Wheeler on 12/02/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class Systems;
    class UserInput;
    class sgTransform;

    class Camera
    {
        struct CameraSave
        {
            Camera3D rlCamera{};
            float currentTargetY{};
            float currentPositionY{};
        };

        CameraSave cameraSave;

        entt::registry* registry;
        Systems* sys;
        Camera3D rlCamera;
        int zoom = 10;

        // Camera height smoothing
        float verticalSmoothingTargetY{};
        float verticalSmoothingCurrentY{};
        float verticalEasingSpeed = 0.075;

        bool forwardKeyDown{};
        bool backKeyDown{};
        bool leftKeyDown{};
        bool rightKeyDown{};
        bool rotateLeftKeyDown{};
        bool rotateRightKeyDown{};
        bool scrollEnabled = true;
        bool lockInput = false;

        void cameraHeightSmoothing();
        void handleInput();

      public:
        Camera3D* getRaylibCam();
        void ScrollEnable();
        void ScrollDisable();
        void LockInput();
        void UnlockInput();
        [[nodiscard]] Vector3 GetForward();
        [[nodiscard]] Vector3 GetRight();
        [[nodiscard]] Vector3 GetBackward();
        [[nodiscard]] Vector3 GetLeft();
        [[nodiscard]] Vector3 GetPosition() const;
        void CutscenePose(const sgTransform& location, const Vector3& localOffset);
        void CutsceneEnd();
        void SetCamera(Vector3 _pos, Vector3 _target);
        void FocusSelectedActor();
        void DrawDebug();
        void Update();
        explicit Camera(entt::registry* _registry, UserInput* userInput, Systems* _sys);
    };
} // namespace sage
