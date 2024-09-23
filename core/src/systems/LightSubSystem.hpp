//
// Created by Steve Wheeler on 09/04/2024.
//

#pragma once

#include "raylib.h"
#include "rlights.h"
#include <array>
#include <entt/entt.hpp>

namespace sage
{
    class LightSubSystem
    {
        entt::registry* registry;
        int lightCount{};

      public:
        Shader shader{};
        std::array<Light, MAX_LIGHTS> lights{};
        void AddLight(Vector3 pos, Color col);
        void LinkRenderableToLight(entt::entity entity) const;
        void DrawDebugLights();
        explicit LightSubSystem(entt::registry* _registry);
    };
} // namespace sage
