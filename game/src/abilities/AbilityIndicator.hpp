#pragma once

#include "engine/TextureTerrainOverlay.hpp"
#include "entt/entt.hpp"

#include <memory>
#include <string>

#include "raylib.h"

namespace sage
{
    class NavigationGridSystem;
}

namespace lq
{
    class AbilityIndicator
    {
        std::unique_ptr<sage::TextureTerrainOverlay> indicatorTexture;

        // Depending on the indicator, it'd likely be cursor position, player position or player to cursor
        // (until a point).

      public:
        void Init(Vector3 mouseRayHit);
        void Update(Vector3 mouseRayHit);
        void Enable(bool enable);
        AbilityIndicator(
            entt::registry* _registry,
            sage::NavigationGridSystem* _navigationGridSystem,
            const std::string& assetId);
    };
} // namespace lq