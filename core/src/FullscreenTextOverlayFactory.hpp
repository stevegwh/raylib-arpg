//
// Created by Steve Wheeler on 02/01/2025.
//

#pragma once

#include "raylib.h"
#include "Timer.hpp"
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace sage
{
    struct GameData;

    class FullscreenTextOverlayFactory
    {
        Font font;
        float fadeIn{};
        float fadeOut{};
        Timer timer;
        GameData* gameData;
        unsigned int currentTextIdx = 0;
        std::vector<std::pair<std::vector<std::string>, float>> overlayText;
        bool enabled = false;
        void setNextText();
        static std::vector<std::string> divideTextOnNewLine(const std::string& str);

      public:
        void Update();
        void Draw2D() const;
        void SetOverlay(
            const std::vector<std::pair<std::string, float>>& _overlayText, float _fadeIn = 0, float _fadeOut = 0);
        void RemoveOverlay();
        explicit FullscreenTextOverlayFactory(GameData* _gameData);
    };

} // namespace sage
