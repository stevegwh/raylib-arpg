//
// Created by Steve Wheeler on 03/08/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{

    class Explosion
    {
        entt::registry* registry;
        entt::entity entity;
        Shader shader;
        float scale = 0;
        float maxScale = 10;
        float increment = 30.0f;

      public:
        void Update();
        void Restart();
        void SetOrigin(Vector3 origin);
        explicit Explosion(entt::registry* _registry);
    };

} // namespace sage
