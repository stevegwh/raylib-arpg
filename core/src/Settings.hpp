//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "cereal/cereal.hpp"

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

        float GetScreenScaleFactor() const
        {
            // Calculate scaling factor based on screen dimensions
            // You can use either width, height, or both depending on your needs
            float scaleWidth = screenWidth / TARGET_SCREEN_WIDTH;
            float scaleHeight = screenHeight / TARGET_SCREEN_HEIGHT;

            // Choose scaling method:

            // Option 1: Scale based on width only
            float scaleFactor = scaleWidth;

            // Option 2: Scale based on height only
            // float scaleFactor = scaleHeight;

            // Option 3: Scale based on smallest ratio to prevent overlarge fonts
            // float scaleFactor = std::min(scaleWidth, scaleHeight);

            // Option 4: Scale based on average of both dimensions
            // float scaleFactor = (scaleWidth + scaleHeight) * 0.5f;
            return scaleFactor;
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
        static constexpr int SCREEN_WIDTH = 1280;
        static constexpr int SCREEN_HEIGHT = 720;
    };
} // namespace sage
