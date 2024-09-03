//
// Created by Steve Wheeler on 09/04/2024.
//

#pragma once

#include "raylib.h"
#include "rlights.h"
#include <entt/entt.hpp>

namespace sage
{
    class LightSubSystem
    {
        entt::registry* registry;

      public:
        Shader shader;
        Light lights[MAX_LIGHTS]{};
        void LinkRenderableToLight(entt::entity entity) const;
        void DrawDebugLights();
        explicit LightSubSystem(entt::registry* _registry);
    };
} // namespace sage
