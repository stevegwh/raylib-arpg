//
// Created by Steve Wheeler on 02/01/2025.
//

#pragma once

#include "Event.hpp"
#include "Timer.hpp"

#include "raylib.h"

#include <string>
// #include <utility>
#include <vector>

namespace sage
{
    class Systems;

    class FullscreenTextOverlayFactory
    {
        Font font;
        float fadeIn{};
        float fadeOut{};
        Timer timer;
        Systems* sys;
        unsigned int currentTextIdx = 0;
        std::vector<std::pair<std::vector<std::string>, float>> overlayText;
        bool enabled = false;
        void setNextText();
        static std::vector<std::string> divideTextOnNewLine(const std::string& str);

      public:
        Event<> onOverlayEnd{};
        Event<> onOverlayEnding{};
        void Update();
        void Draw2D() const;
        void SetOverlay(
            const std::vector<std::pair<std::string, float>>& _overlayText, float _fadeIn = 0, float _fadeOut = 0);
        void RemoveOverlay();
        explicit FullscreenTextOverlayFactory(Systems* _sys);
    };

} // namespace sage
