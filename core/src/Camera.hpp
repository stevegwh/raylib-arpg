//
// Created by Steve Wheeler on 12/02/2024.
//

#pragma once

#include "raylib.h"

namespace sage
{
    class GameData;
    class UserInput;
    class sgTransform;

    class Camera
    {
        GameData* gameData;
        Camera3D rlCamera;
        int zoom = 10;

        bool forwardKeyDown{};
        bool backKeyDown{};
        bool leftKeyDown{};
        bool rightKeyDown{};
        bool rotateLeftKeyDown{};
        bool rotateRightKeyDown{};
        bool scrollEnabled = true;
        bool lockInput = false;

        void onForwardKeyPressed();
        void onLeftKeyPressed();
        void onRightKeyPressed();
        void onBackKeyPressed();
        void onRotateLeftKeyPressed();
        void onRotateRightKeyPressed();
        void onForwardKeyUp();
        void onLeftKeyUp();
        void onRightKeyUp();
        void onBackKeyUp();
        void onRotateLeftKeyUp();
        void onRotateRightKeyUp();

        void updateTarget();
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
        void CutscenePose(const sgTransform& location);
        void SetCamera(Vector3 _pos, Vector3 _target);
        void Update();
        explicit Camera(UserInput* userInput, GameData* _gameData);
    };
} // namespace sage
