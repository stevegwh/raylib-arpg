//
// Created by Steve Wheeler on 02/01/2025.
//

#include "FullscreenTextOverlayFactory.hpp"

#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"

#include <format>

namespace sage
{

    void FullscreenTextOverlayFactory::RemoveOverlay()
    {
        enabled = false;
        fadeIn = 0;
        fadeOut = 0;
        currentTextIdx = 0;
        overlayText.clear();
        timer.Reset();
    }

    void FullscreenTextOverlayFactory::setNextText()
    {
        ++currentTextIdx;
        auto time = overlayText.at(currentTextIdx).second;
        timer.SetMaxTime(time);
        timer.Restart();
    }

    void FullscreenTextOverlayFactory::SetOverlay(
        const std::vector<std::pair<std::string, float>>& _overlayText, float _fadeIn, float _fadeOut)
    {
        enabled = true;
        fadeIn = _fadeIn;
        fadeOut = _fadeOut;
        for (const auto& p : _overlayText)
        {
            overlayText.emplace_back(p);
        }
        // Set final overlay to fade out
        if (fadeOut > 0)
        {
            overlayText.emplace_back("", fadeOut);
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
        const auto [width, height] = gameData->settings->GetViewPort();
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
        const char* text = overlayText.at(currentTextIdx).first.c_str();
        auto textSize = MeasureTextEx(font, text, 32.0f, 1.5f);
        DrawTextEx(
            font,
            std::format("{}", text).c_str(),
            Vector2{(width - textSize.x) / 2, (gameData->settings->GetViewPort().y - textSize.y) / 2},
            32,
            1.5f,
            textCol);
    }

    FullscreenTextOverlayFactory::FullscreenTextOverlayFactory(GameData* _gameData)
        : font(ResourceManager::GetInstance().FontLoad(
              "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf")),
          timer({}),
          gameData(_gameData)
    {
    }

} // namespace sage