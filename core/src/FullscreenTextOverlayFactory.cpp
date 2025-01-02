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
    }

    void FullscreenTextOverlayFactory::SetOverlay(const std::string& _overlayText)
    {
        enabled = true;
        overlayText = _overlayText;
    }

    void FullscreenTextOverlayFactory::SetOverlayTimed(
        const std::string& _overlayText, float time, float _fadeIn, float _fadeOut)
    {
        assert((fadeIn + fadeOut) < time);
        fadeIn = _fadeIn;
        fadeOut = _fadeOut;
        timer.emplace(Timer{});
        timer->SetMaxTime(time);
        timer->Start();
        SetOverlay(_overlayText);
    }

    void FullscreenTextOverlayFactory::Update()
    {
        if (timer.has_value())
        {
            if (timer->HasFinished())
            {
                RemoveOverlay();
                timer.reset();
            }
            else
            {
                timer->Update(GetFrameTime());
            }
        }
    }

    void FullscreenTextOverlayFactory::Draw2D() const
    {
        if (!enabled) return;
        const auto [width, height] = gameData->settings->GetViewPort();
        unsigned char a = 255;
        if (timer->GetCurrentTime() < fadeIn)
        {
            a = static_cast<unsigned char>((timer->GetCurrentTime() / fadeIn) * 255);
        }
        else if (timer->GetRemainingTime() < fadeOut)
        {
            a = static_cast<unsigned char>((timer->GetRemainingTime() / fadeOut) * 255);
        }

        auto col1 = Color{0, 0, 0, a};
        DrawRectangle(0, 0, width, height, col1);
        auto col2 = Color{255, 255, 255, a};

        auto textSize = MeasureTextEx(font, overlayText.c_str(), 32.0f, 1.5f);
        DrawTextEx(
            font,
            std::format("{}", overlayText).c_str(),
            Vector2{(width - textSize.x) / 2, (gameData->settings->GetViewPort().y - textSize.y) / 2},
            32,
            1.5f,
            col2);
    }

    FullscreenTextOverlayFactory::FullscreenTextOverlayFactory(GameData* _gameData)
        : font(ResourceManager::GetInstance().FontLoad(
              "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf")),
          gameData(_gameData)
    {
    }

} // namespace sage