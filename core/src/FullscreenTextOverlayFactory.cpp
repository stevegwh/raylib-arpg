//
// Created by Steve Wheeler on 02/01/2025.
//

#include "FullscreenTextOverlayFactory.hpp"

#include "ResourceManager.hpp"
#include "Settings.hpp"
#include "Systems.hpp"

#include <format>
#include <sstream>

namespace sage
{

    std::vector<std::string> FullscreenTextOverlayFactory::divideTextOnNewLine(const std::string& str)
    {
        std::vector<std::string> out;
        std::stringstream ss(str);

        std::string buff;
        while (std::getline(ss, buff, '\n'))
        {
            out.push_back(buff);
        }
        return out;
    }

    void FullscreenTextOverlayFactory::setNextText()
    {
        ++currentTextIdx;
        auto time = overlayText.at(currentTextIdx).second;
        timer.SetMaxTime(time);
        timer.Restart();
        if (currentTextIdx == overlayText.size() - 1)
        {
            onOverlayEnding.Publish();
        }
    }

    void FullscreenTextOverlayFactory::RemoveOverlay()
    {
        enabled = false;
        fadeIn = 0;
        fadeOut = 0;
        currentTextIdx = 0;
        overlayText.clear();
        timer.Reset();
        onOverlayEnd.Publish();
    }

    void FullscreenTextOverlayFactory::SetOverlay(
        const std::vector<std::pair<std::string, float>>& _overlayText, float _fadeIn, float _fadeOut)
    {
        enabled = true;
        fadeIn = _fadeIn;
        fadeOut = _fadeOut;
        for (const auto& [str, time] : _overlayText)
        {
            overlayText.emplace_back(divideTextOnNewLine(str), time);
        }
        // Set final overlay to fade out
        if (fadeOut > 0)
        {
            overlayText.emplace_back(std::vector<std::string>{""}, fadeOut);
        }
        currentTextIdx = 0;
        timer.SetMaxTime(overlayText.at(0).second);
        timer.Start();
    }

    void FullscreenTextOverlayFactory::Update()
    {
        timer.Update(GetFrameTime());
        if (timer.HasFinished())
        {
            if (currentTextIdx + 1 < overlayText.size())
            {
                setNextText();
            }
            else
            {
                RemoveOverlay();
            }
        }
    }

    void FullscreenTextOverlayFactory::Draw2D() const
    {
        if (!enabled) return;
        const auto [width, height] = sys->settings->GetViewPort();
        unsigned char a = 255;

        const bool last = currentTextIdx == overlayText.size() - 1;
        if (timer.GetRemainingTime() < fadeOut || last)
        {
            a = static_cast<unsigned char>((timer.GetRemainingTime() / fadeOut) * 255);
        }
        else if (timer.GetCurrentTime() < fadeIn)
        {
            a = static_cast<unsigned char>((timer.GetCurrentTime() / fadeIn) * 255);
        }

        auto bgCol = Color{0, 0, 0, 255};
        if (last)
        {
            bgCol.a = a;
        }
        DrawRectangle(0, 0, width, height, bgCol);

        auto textCol = Color{255, 255, 255, a};

        float spacing = 24;
        auto size = overlayText.at(currentTextIdx).first.size();
        float allTextHeight = 0;

        for (const auto& str : overlayText.at(currentTextIdx).first)
        {
            allTextHeight += MeasureTextEx(font, str.c_str(), 32.0f, 1.5f).y;
        }
        allTextHeight += (spacing * (size - 1));

        float startY = (sys->settings->GetViewPort().y - allTextHeight) / 2;

        for (unsigned int i = 0; i < size; ++i)
        {
            const char* text = overlayText.at(currentTextIdx).first.at(i).c_str();
            auto textSize = MeasureTextEx(font, text, 32.0f, 1.5f);

            DrawTextEx(
                font,
                std::format("{}", text).c_str(),
                Vector2{(width - textSize.x) / 2, startY + (i * (textSize.y + spacing))},
                32,
                1.5f,
                textCol);
        }
    }

    FullscreenTextOverlayFactory::FullscreenTextOverlayFactory(Systems* _sys)
        : font(ResourceManager::GetInstance().FontLoad(
              "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf")),
          timer({}),
          sys(_sys)
    {
    }

} // namespace sage