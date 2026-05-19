#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <optional>

namespace sage
{
    class EngineSystems;
}

namespace sage::editor
{
    class EditorPickingService
    {
        EngineSystems* sys;

      public:
        explicit EditorPickingService(EngineSystems* sys);

        [[nodiscard]] std::optional<entt::entity> PickSceneEntity(
            Vector2 screenPosition,
            entt::entity ignoredEntity = entt::null) const;
    };
} // namespace sage::editor
