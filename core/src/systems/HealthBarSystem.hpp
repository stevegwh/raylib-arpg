//
// Created by steve on 20/05/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class Camera;

    class HealthBarSystem
    {
        entt::registry* registry;
        Camera* camera;

        void updateHealthBarTextures() const;

      public:
        void Draw2D();
        void Draw3D();
        void Update();
        HealthBarSystem(entt::registry* _registry, Camera* _camera);
    };
} // namespace sage
