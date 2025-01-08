//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "Serializer.hpp"

#include "cereal/cereal.hpp"
#include "raylib.h"

namespace sage
{
    struct Settings
    {
      private:
        // Current settings
        int screenWidth = 1280;
        int screenHeight = 720;
        int viewportWidth = 1920;
        int viewportHeight = 1080;

      public:
        static constexpr float TARGET_SCREEN_WIDTH = 1920.0f;
        static constexpr float TARGET_SCREEN_HEIGHT = 1080.0f;

        bool toggleFullScreenRequested = false;

        void SetScreenSize(int w, int h)
        {
            screenWidth = w;
            screenHeight = h;
            UpdateViewport();
        }

        [[nodiscard]] Vector2 GetScreenSize() const
        {
            return {static_cast<float>(screenWidth), static_cast<float>(screenHeight)};
        }

        [[nodiscard]] Vector2 GetViewPort() const
        {
            return {static_cast<float>(viewportWidth), static_cast<float>(viewportHeight)};
        }

        void UpdateViewport()
        {
            float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
            static constexpr float targetAspectRatio = 16.0f / 9.0f;

            // Calculate viewport dimensions while maintaining aspect ratio
            if (aspectRatio > targetAspectRatio)
            {
                // Screen is wider than target ratio - fit to height
                viewportHeight = screenHeight;
                viewportWidth = static_cast<int>(screenHeight * targetAspectRatio);
            }
            else
            {
                // Screen is taller than target ratio - fit to width
                viewportWidth = screenWidth;
                viewportHeight = static_cast<int>(screenWidth / targetAspectRatio);
            }
        }

        static float GetScreenScaleFactor(float width, float height)
        {

            float scaleX = width / TARGET_SCREEN_WIDTH;
            float scaleY = height / TARGET_SCREEN_HEIGHT;

            // Use the smaller scale factor to maintain aspect ratio
            return std::min(scaleX, scaleY);
        }

        [[nodiscard]] float GetCurrentScaleFactor() const
        {
            return GetScreenScaleFactor(viewportWidth, viewportHeight);
        }

        [[nodiscard]] float ScaleValueMaintainRatio(const float toScale) const
        {
            return toScale * GetCurrentScaleFactor();
        }

        [[nodiscard]] float ScaleValueHeight(const float toScale) const
        {
            float scaleY = viewportHeight / TARGET_SCREEN_HEIGHT;
            return toScale * scaleY;
        }

        [[nodiscard]] float ScaleValueWidth(const float toScale) const
        {
            float scaleX = viewportWidth / TARGET_SCREEN_WIDTH;
            return toScale * scaleX;
        }

        [[nodiscard]] Vector2 ScalePos(Vector2 toScale) const
        {
            return {ScaleValueWidth(toScale.x), ScaleValueHeight(toScale.y)};
        }

        void ResetToUserDefined()
        {
            screenWidth = screenWidthUser;
            screenHeight = screenHeightUser;
            UpdateViewport();
        }

        void ResetToDefaults()
        {
            screenWidthUser = SCREEN_WIDTH;
            screenHeightUser = SCREEN_HEIGHT;
            ResetToUserDefined();
        }

        Settings()
        {
            serializer::DeserializeJsonFile<Settings>("resources/settings.json", *this);
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
