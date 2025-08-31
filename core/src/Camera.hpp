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
        static constexpr int zoom = 10;

        // Camera height smoothing
        float verticalSmoothingTargetY{};
        float verticalSmoothingCurrentY{};
        static constexpr float verticalEasingSpeed = 0.075;

        float cameraScrollVelY = 0.0;
        static constexpr float cameraInitialVelY = 2.0f;
        static constexpr float cameraScrollDeceleration = 0.075f;
        static constexpr float cameraMaxY = 130.0f;

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
        void handleMouseScroll();

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
