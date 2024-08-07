//
// Created by Steve Wheeler on 09/04/2024.
//

#pragma once

#include "raylib.h"
#include "rlights.h"
#include <entt/entt.hpp>

#include <vector>

namespace sage
{
    using Renderable = struct Renderable; // forward dec
    class LightSubSystem
    {
        entt::registry* registry;
        std::vector<Renderable*> renderables;

      public:
        Shader shader;
        Light lights[MAX_LIGHTS]{};

        void LinkAllRenderablesToLight();
        void LinkRenderableToLight(Renderable* renderable);
        void DrawDebugLights();
        explicit LightSubSystem(entt::registry* _registry);
    };
} // namespace sage
