#pragma once

#include "CursorAbility.hpp"
#include "TextureTerrainOverlay.hpp"

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