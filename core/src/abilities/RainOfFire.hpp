#pragma once

#include "TextureTerrainOverlay.hpp"
#include "utils/CursorAbility.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class NavigationGridSystem;
    class Cursor;
    class Camera;
    class AbilityData;

    class RainOfFire : public CursorAbility
    {
      public:
        RainOfFire(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            NavigationGridSystem* _navigationGridSystem);
    };

} // namespace sage