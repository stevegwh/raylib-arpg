#pragma once

#include "TextureTerrainOverlay.hpp"
#include <entt/entt.hpp>

#include <memory>
#include <string>

#include "raylib.h"

namespace sage
{
    class NavigationGridSystem;
    class AbilityIndicator
    {
        std::unique_ptr<TextureTerrainOverlay> indicatorTexture;

        // Depending on the indicator, it'd likely be cursor position, player position or player to cursor
        // (until a point).

      public:
        void Init(Vector3 mouseRayHit);
        void Update(Vector3 mouseRayHit);
        void Enable(bool enable);
        AbilityIndicator(
            entt::registry* _registry,
            NavigationGridSystem* _navigationGridSystem,
            const std::string& cursorTexturePath);
    };
} // namespace sage