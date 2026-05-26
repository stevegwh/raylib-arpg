//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "Serializer.hpp"

#include "cereal/cereal.hpp"
#include "raylib.h"

#include <algorithm>
#include <cmath>

namespace sage
{
    struct Settings
    {
      private:
        bool* exitProgram;

        // Current settings
        int screenWidth = 1280;
        int screenHeight = 720;
        int viewportWidth = 1920;
        int viewportHeight = 1080;
        int renderViewportWidth = 1920;
        int renderViewportHeight = 1080;
        int renderViewportOffsetX = 0;
        int renderViewportOffsetY = 0;
        bool preserveAspectRatio = true;

        // Serialized settings (loaded from settings.xml)
        int screenWidthUser{};
        int screenHeightUser{};

        // Hardcoded defaults
        static constexpr int SCREEN_WIDTH = 1920;
        static constexpr int SCREEN_HEIGHT = 1080;

      public:
        static constexpr float TARGET_SCREEN_WIDTH = 1920.0f;
        static constexpr float TARGET_SCREEN_HEIGHT = 1080.0f;

        bool toggleFullScreenRequested = false;

        void ExitProgram()
        {
            *exitProgram = true;
        }

        void SetScreenSize(int w, int h)
        {
            screenWidth = w;
            screenHeight = h;
            UpdateViewport();
        }

        void SetPreserveAspectRatio(const bool preserve)
        {
            preserveAspectRatio = preserve;
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

        [[nodiscard]] Vector2 GetRenderViewPort() const
        {
            return {static_cast<float>(renderViewportWidth), static_cast<float>(renderViewportHeight)};
        }

        void UpdateViewport()
        {
            if (!preserveAspectRatio)
            {
                viewportWidth = screenWidth;
                viewportHeight = screenHeight;
                ResetRenderViewportToAppViewport();
                return;
            }

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
            ResetRenderViewportToAppViewport();
        }

        [[nodiscard]] Vector2 GetViewportOffset() const
        {
            return {
                std::floor((static_cast<float>(screenWidth) - static_cast<float>(viewportWidth)) * 0.5f),
                std::floor((static_cast<float>(screenHeight) - static_cast<float>(viewportHeight)) * 0.5f)};
        }

        [[nodiscard]] Rectangle GetViewportScreenRect() const
        {
            const auto viewportOffset = GetViewportOffset();
            return {
                viewportOffset.x,
                viewportOffset.y,
                static_cast<float>(viewportWidth),
                static_cast<float>(viewportHeight)};
        }

        [[nodiscard]] bool IsPointInViewport(const Vector2 point) const
        {
            return CheckCollisionPointRec(point, GetViewportScreenRect());
        }

        [[nodiscard]] Vector2 ScreenToViewportPosition(const Vector2 point) const
        {
            const auto viewportOffset = GetViewportOffset();
            return {point.x - viewportOffset.x, point.y - viewportOffset.y};
        }

        [[nodiscard]] Vector2 ViewportToScreenPosition(const Vector2 point) const
        {
            const auto viewportOffset = GetViewportOffset();
            return {point.x + viewportOffset.x, point.y + viewportOffset.y};
        }

        [[nodiscard]] Vector2 GetRenderViewportOffset() const
        {
            return {static_cast<float>(renderViewportOffsetX), static_cast<float>(renderViewportOffsetY)};
        }

        [[nodiscard]] Rectangle GetRenderViewportRect() const
        {
            return {
                static_cast<float>(renderViewportOffsetX),
                static_cast<float>(renderViewportOffsetY),
                static_cast<float>(renderViewportWidth),
                static_cast<float>(renderViewportHeight)};
        }

        [[nodiscard]] Rectangle GetRenderViewportScreenRect() const
        {
            const auto viewportOffset = GetViewportOffset();
            return {
                viewportOffset.x + static_cast<float>(renderViewportOffsetX),
                viewportOffset.y + static_cast<float>(renderViewportOffsetY),
                static_cast<float>(renderViewportWidth),
                static_cast<float>(renderViewportHeight)};
        }

        [[nodiscard]] bool IsPointInRenderViewport(const Vector2 point) const
        {
            return CheckCollisionPointRec(point, GetRenderViewportScreenRect());
        }

        [[nodiscard]] Vector2 ScreenToRenderViewportPosition(const Vector2 point) const
        {
            const auto viewportOffset = GetViewportOffset();
            return {
                point.x - viewportOffset.x - static_cast<float>(renderViewportOffsetX),
                point.y - viewportOffset.y - static_cast<float>(renderViewportOffsetY)};
        }

        void SetRenderViewport(const int width, const int height, const Vector2 offset)
        {
            renderViewportWidth = std::max(1, width);
            renderViewportHeight = std::max(1, height);
            renderViewportOffsetX = static_cast<int>(offset.x);
            renderViewportOffsetY = static_cast<int>(offset.y);
        }

        void ResetRenderViewportToAppViewport()
        {
            renderViewportWidth = viewportWidth;
            renderViewportHeight = viewportHeight;
            renderViewportOffsetX = 0;
            renderViewportOffsetY = 0;
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

        explicit Settings(bool* _exitProgram) : exitProgram(_exitProgram)
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
    };
} // namespace sage
