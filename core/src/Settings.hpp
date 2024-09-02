//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "cereal/cereal.hpp"

namespace sage
{
    struct Settings
    {
        // Current settings
        int screenWidth = 1280;
        int screenHeight = 720;
        bool toggleFullScreenRequested = false;

        void ResetScreenSize()
        {
            screenWidth = screenWidthUser;
            screenHeight = screenHeightUser;
        }

        void ResetDefaultScreenSize()
        {
            screenWidth = SCREEN_WIDTH;
            screenHeight = SCREEN_HEIGHT;
        }

        template <class Archive>
        void serialize(Archive& archive)
        {
            // TODO: Change NVP to remove user
            archive(CEREAL_NVP(screenWidthUser), CEREAL_NVP(screenHeightUser));
        }

      private:
        // Loaded defaults
        int screenWidthUser = 1280;
        int screenHeightUser = 720;

        // Hardcoded defaults
        static constexpr int SCREEN_WIDTH = 1280;
        static constexpr int SCREEN_HEIGHT = 720;
    };
} // namespace sage
