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
