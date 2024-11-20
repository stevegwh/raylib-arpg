//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "raylib.h"
#include "raymath.h"

namespace sage
{
    struct Settings
    {
        static constexpr float TARGET_SCREEN_WIDTH = 1920.0f;
        static constexpr float TARGET_SCREEN_HEIGHT = 1080.0f;

        // Current settings
        int screenWidth = 1280;
        int screenHeight = 720;
        bool toggleFullScreenRequested = false;

        static float GetScreenScaleFactor(float width, float height)
        {

            float scaleX = width / TARGET_SCREEN_WIDTH;
            float scaleY = height / TARGET_SCREEN_HEIGHT;

            // Use the smaller scale factor to maintain aspect ratio
            return std::min(scaleX, scaleY);
        }

        [[nodiscard]] float GetCurrentScaleFactor() const
        {
            return GetScreenScaleFactor(screenWidth, screenHeight);
        }

        [[nodiscard]] float ScaleValueMaintainRatio(const float toScale) const
        {
            return toScale * GetCurrentScaleFactor();
        }

        [[nodiscard]] float ScaleValueHeight(const float toScale) const
        {
            float scaleY = screenHeight / TARGET_SCREEN_HEIGHT;
            return toScale * scaleY;
        }

        [[nodiscard]] float ScaleValueWidth(const float toScale) const
        {
            float scaleX = screenWidth / TARGET_SCREEN_WIDTH;
            return toScale * scaleX;
        }

        [[nodiscard]] Vector2 ScalePos(Vector2 toScale) const
        {
            float scaleX = screenWidth / TARGET_SCREEN_WIDTH;
            float scaleY = screenHeight / TARGET_SCREEN_HEIGHT;
            return {toScale.x * scaleX, toScale.y * scaleY};
        }

        void ResetToUserDefined()
        {
            screenWidth = screenWidthUser;
            screenHeight = screenHeightUser;
        }

        void ResetToDefaults()
        {
            screenWidthUser = SCREEN_WIDTH;
            screenHeightUser = SCREEN_HEIGHT;
            ResetToUserDefined();
        }

        template <class Archive>
        void serialize(Archive& archive)
        {
            ResetToDefaults();
            archive(
                cereal::make_nvp("screen_width", screenWidthUser),
                cereal::make_nvp("screen_height", screenHeightUser));
            ResetToUserDefined();
        }

      private:
        // Serialized settings (loaded from settings.xml)
        int screenWidthUser{};
        int screenHeightUser{};

        // Hardcoded defaults
        static constexpr int SCREEN_WIDTH = 1920;
        static constexpr int SCREEN_HEIGHT = 1080;
    };
} // namespace sage
