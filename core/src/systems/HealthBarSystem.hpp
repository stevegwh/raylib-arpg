//
// Created by steve on 20/05/2024.
//

#pragma once

#include "components/HealthBar.hpp"
#include "systems/BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class Camera;

    class HealthBarSystem : public BaseSystem
    {
        Camera* camera;

        void updateHealthBarTextures();

      public:
        void Draw2D();
        void Draw3D() override;
        void Update() override;
        HealthBarSystem(entt::registry* _registry, Camera* _camera);
    };
} // namespace sage
