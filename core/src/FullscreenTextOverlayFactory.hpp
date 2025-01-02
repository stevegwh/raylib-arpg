//
// Created by Steve Wheeler on 02/01/2025.
//

#pragma once

#include "raylib.h"
#include "Timer.hpp"
#include <optional>
#include <string>

namespace sage
{
    struct GameData;

    class FullscreenTextOverlayFactory
    {
        Font font;
        float fadeIn{};
        float fadeOut{};
        std::optional<Timer> timer;
        GameData* gameData;
        std::string overlayText;
        bool enabled = false;

      public:
        void Update();
        void Draw2D() const;
        void SetOverlay(const std::string& _overlayText);
        void SetOverlayTimed(const std::string& _overlayText, float time, float _fadeIn = 0, float _fadeOut = 0);
        void RemoveOverlay();
        explicit FullscreenTextOverlayFactory(GameData* _gameData);
    };

} // namespace sage
